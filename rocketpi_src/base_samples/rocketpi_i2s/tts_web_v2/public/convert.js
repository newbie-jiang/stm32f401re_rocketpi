const SAMPLE_RATES = [8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000];
const DEFAULT_SAMPLE_RATE = 16000;
const DEFAULT_CHANNELS = 2;

const form = document.getElementById('convert-form');
const fileInput = document.getElementById('file');
const sampleRateSelect = document.getElementById('sampleRate');
const channelSelect = document.getElementById('channels');
const outputSelect = document.getElementById('output');
const statusEl = document.getElementById('status');

function sanitizeFileBase(input) {
  return (input || '')
    .toLowerCase()
    .replace(/[^a-z0-9]+/gi, '-')
    .replace(/^-+|-+$/g, '')
    .slice(0, 40);
}

function defaultBaseName() {
  const now = new Date();
  const pad = (value) => String(value).padStart(2, '0');
  return `audio_${now.getFullYear()}${pad(now.getMonth() + 1)}${pad(now.getDate())}_${pad(now.getHours())}${pad(
    now.getMinutes(),
  )}${pad(now.getSeconds())}`;
}

function resolveFileName(custom, output) {
  const cleaned = (custom || '').replace(/\.[^/.]+$/, '');
  const base = sanitizeFileBase(cleaned) || defaultBaseName();
  return output === 'header' ? `${base}.h` : `${base}.bin`;
}

function setStatus(message, type = '') {
  statusEl.textContent = message || '';
  statusEl.className = ['status', type].filter(Boolean).join(' ');
}

function renderSampleRateOptions() {
  sampleRateSelect.innerHTML = SAMPLE_RATES.map(
    (rate) => `<option value="${rate}" ${rate === DEFAULT_SAMPLE_RATE ? 'selected' : ''}>${rate / 1000} kHz</option>`,
  ).join('');
}

function clampSample(value) {
  const v = Math.max(-1, Math.min(1, value));
  return Math.round(v * 32767);
}

function resampleChannel(src, srcRate, targetRate, targetLength) {
  const dst = new Float32Array(targetLength);
  const ratio = srcRate / targetRate;
  for (let i = 0; i < targetLength; i += 1) {
    const srcPos = i * ratio;
    const idx = Math.floor(srcPos);
    const frac = srcPos - idx;
    const s0 = src[idx] || 0;
    const s1 = src[idx + 1] || s0;
    dst[i] = s0 + (s1 - s0) * frac;
  }
  return dst;
}

function convertToPCM(audioBuffer, targetRate, targetChannels) {
  const srcRate = audioBuffer.sampleRate;
  const srcChannels = audioBuffer.numberOfChannels || 1;
  const srcData = [];
  for (let ch = 0; ch < Math.min(srcChannels, 2); ch += 1) {
    srcData.push(audioBuffer.getChannelData(ch));
  }
  if (srcData.length === 0) {
    throw new Error('音频无有效声道');
  }

  const targetLength = Math.round((audioBuffer.length * targetRate) / srcRate);
  const left = resampleChannel(srcData[0], srcRate, targetRate, targetLength);
  let right = null;

  if (targetChannels === 2) {
    if (srcData.length >= 2) {
      right = resampleChannel(srcData[1], srcRate, targetRate, targetLength);
    } else {
      right = left;
    }
  } else if (srcData.length >= 2) {
    const alt = resampleChannel(srcData[1], srcRate, targetRate, targetLength);
    for (let i = 0; i < left.length; i += 1) {
      left[i] = (left[i] + alt[i]) * 0.5;
    }
  }

  const totalSamples = targetLength * targetChannels;
  const pcm = new Int16Array(totalSamples);
  if (targetChannels === 1) {
    for (let i = 0; i < targetLength; i += 1) {
      pcm[i] = clampSample(left[i]);
    }
  } else {
    for (let i = 0; i < targetLength; i += 1) {
      const l = clampSample(left[i]);
      const r = clampSample(right[i]);
      pcm[i * 2] = l;
      pcm[i * 2 + 1] = r;
    }
  }

  return { pcm, frames: targetLength, sampleRate: targetRate, channels: targetChannels };
}

function buildHeader(int16Data, meta) {
  const chunkSize = 64;
  const lines = [];
  lines.push(`/* audio export: ${meta.sourceName} (${meta.sampleRate} Hz, ${meta.channels} ch) */`);
  lines.push('#ifndef AUDIO_H');
  lines.push('#define AUDIO_H');
  lines.push('');
  lines.push('#include <stdint.h>');
  lines.push('');
  lines.push(`#define AUDIO_SAMPLE_RATE_HZ ${meta.sampleRate}U`);
  lines.push(`#define AUDIO_NUM_CHANNELS   ${meta.channels}U`);
  lines.push('#define AUDIO_BITS_PER_SAMPLE 16U');
  lines.push('');
  lines.push('static const int16_t audio_track[] = {');
  for (let i = 0; i < int16Data.length; i += chunkSize) {
    const chunk = Array.from(int16Data.slice(i, i + chunkSize)).join(', ');
    const suffix = i + chunkSize < int16Data.length ? ',' : '';
    lines.push(`    ${chunk}${suffix}`);
  }
  lines.push('};');
  lines.push('');
  lines.push('#define AUDIO_TRACK_SAMPLE_COUNT (sizeof(audio_track) / sizeof(audio_track[0]))');
  lines.push('#define AUDIO_TRACK_SIZE_BYTES (AUDIO_TRACK_SAMPLE_COUNT * sizeof(audio_track[0]))');
  lines.push('');
  lines.push('#endif /* AUDIO_H */');
  lines.push('');
  return lines.join('\n');
}

async function decodeFile(file) {
  const arrayBuffer = await file.arrayBuffer();
  const Context = window.AudioContext || window.webkitAudioContext;
  if (!Context) {
    throw new Error('当前浏览器不支持 Web Audio API');
  }
  const ctx = new Context();
  try {
    const decoded = await ctx.decodeAudioData(arrayBuffer);
    return decoded;
  } finally {
    ctx.close();
  }
}

async function handleSubmit(event) {
  event.preventDefault();
  const file = fileInput.files && fileInput.files[0];
  if (!file) {
    setStatus('请选择一个音频文件', 'error');
    return;
  }

  const targetRate = Number(sampleRateSelect.value) || DEFAULT_SAMPLE_RATE;
  const targetChannels = Number(channelSelect.value) || DEFAULT_CHANNELS;
  const output = outputSelect.value === 'header' ? 'header' : 'bin';
  const customName = (form.fileName.value || '').trim();

  try {
    setStatus('解码音频...', 'info');
    const audioBuffer = await decodeFile(file);

    setStatus('重采样并生成 PCM...', 'info');
    const { pcm, frames, sampleRate, channels } = convertToPCM(audioBuffer, targetRate, targetChannels);

    const downloadName = resolveFileName(customName || file.name, output);
    if (output === 'bin') {
      const blob = new Blob([pcm.buffer], { type: 'application/octet-stream' });
      const url = URL.createObjectURL(blob);
      const anchor = document.createElement('a');
      anchor.href = url;
      anchor.download = downloadName;
      document.body.appendChild(anchor);
      anchor.click();
      anchor.remove();
      URL.revokeObjectURL(url);
    } else {
      const headerText = buildHeader(pcm, {
        sourceName: file.name,
        sampleRate,
        channels,
      });
      const blob = new Blob([headerText], { type: 'text/plain;charset=utf-8' });
      const url = URL.createObjectURL(blob);
      const anchor = document.createElement('a');
      anchor.href = url;
      anchor.download = downloadName;
      document.body.appendChild(anchor);
      anchor.click();
      anchor.remove();
      URL.revokeObjectURL(url);
    }

    const sizeKb = (pcm.byteLength / 1024).toFixed(1);
    setStatus(`已生成 ${downloadName} (${channels} ch @ ${sampleRate} Hz，${frames} 帧，${sizeKb} KB)`, 'success');
  } catch (error) {
    console.error(error);
    setStatus(error.message || '转换失败', 'error');
  }
}

renderSampleRateOptions();
form.addEventListener('submit', handleSubmit);
