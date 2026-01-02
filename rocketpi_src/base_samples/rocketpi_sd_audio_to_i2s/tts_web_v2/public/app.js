const form = document.getElementById('tts-form');
const statusEl = document.getElementById('status');
const voiceSelect = document.getElementById('voice');
const sampleRateSelect = document.getElementById('sampleRate');
const channelSelect = document.getElementById('channels');

const configState = {
  defaultVoice: voiceSelect.value,
  sampleRates: [8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000],
  defaultSampleRate: 16000,
  channelOptions: [1, 2],
  defaultChannels: 2,
  voices: Array.from(voiceSelect.options).map((option) => ({
    id: option.value,
    label: option.textContent || option.value,
  })),
};

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

function resolveDownloadName(customName, output) {
  const cleaned = (customName || '').replace(/\.[^/.]+$/, '');
  const baseCandidate = sanitizeFileBase(cleaned);
  const baseName = baseCandidate || defaultBaseName();
  return output === 'header' ? `${baseName}.h` : `${baseName}.bin`;
}

function setStatus(message, type = '') {
  statusEl.textContent = message || '';
  statusEl.className = ['status', type].filter(Boolean).join(' ');
}

function getFilenameFromDisposition(disposition) {
  if (!disposition) return null;
  const match = /filename="?([^";]+)"?/i.exec(disposition);
  return match ? match[1] : null;
}

function renderSampleRateOptions(sampleRates, defaultValue) {
  const formatLabel = (rate) => {
    const khz = rate / 1000;
    return `${Number.isInteger(khz) ? khz : khz.toFixed(3)} kHz`;
  };
  return sampleRates
    .map((rate) => `<option value="${rate}" ${rate === defaultValue ? 'selected' : ''}>${formatLabel(rate)}</option>`)
    .join('');
}

function renderChannelOptions(channels, defaultValue) {
  const label = (value) => (value === 1 ? '单声道' : '双声道');
  return channels
    .map((value) => `<option value="${value}" ${value === defaultValue ? 'selected' : ''}>${label(value)} (${value})</option>`)
    .join('');
}

function renderVoiceOptions(voices, defaultVoice) {
  return voices
    .map((voice) => `<option value="${voice.id}" ${voice.id === defaultVoice ? 'selected' : ''}>${voice.label}</option>`)
    .join('');
}

async function loadConfig() {
  try {
    const response = await fetch('/api/config');
    if (!response.ok) {
      throw new Error(`配置接口返回 ${response.status}`);
    }
    const data = await response.json();
    Object.assign(configState, data);
    if (Array.isArray(data.sampleRates)) {
      sampleRateSelect.innerHTML = renderSampleRateOptions(data.sampleRates, data.defaultSampleRate ?? configState.defaultSampleRate);
    }
    if (Array.isArray(data.channelOptions)) {
      channelSelect.innerHTML = renderChannelOptions(data.channelOptions, data.defaultChannels ?? configState.defaultChannels);
    }
    if (Array.isArray(data.voices) && data.voices.length > 0) {
      voiceSelect.innerHTML = renderVoiceOptions(data.voices, data.defaultVoice ?? configState.defaultVoice);
    }
  } catch (error) {
    console.warn('无法从后端获取配置，继续使用默认值', error);
  }
}

async function handleSubmit(event) {
  event.preventDefault();
  const formData = new FormData(form);
  const text = (formData.get('text') || '').trim();
  if (!text) {
    setStatus('请输入文本内容', 'error');
    return;
  }

  const customName = (formData.get('fileName') || '').trim();
  const payload = {
    text,
    voice: formData.get('voice') || configState.defaultVoice,
    rate: (formData.get('rate') || '+0%').trim(),
    pitch: (formData.get('pitch') || '+0Hz').trim(),
    output: formData.get('output') || 'header',
    sampleRate: Number(formData.get('sampleRate')) || configState.defaultSampleRate,
    channels: Number(formData.get('channels')) || configState.defaultChannels,
  };
  if (customName) {
    payload.fileName = customName;
  }

  setStatus('合成中，请稍候...', 'info');
  try {
    const response = await fetch('/api/synthesize', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload),
    });

    if (!response.ok) {
      const data = await response.json().catch(() => ({}));
      throw new Error(data.error || '请求失败');
    }

    const blob = await response.blob();
    const filename =
      getFilenameFromDisposition(response.headers.get('Content-Disposition')) ||
      resolveDownloadName(customName, payload.output);
    const url = URL.createObjectURL(blob);
    const anchor = document.createElement('a');
    anchor.href = url;
    anchor.download = filename;
    document.body.appendChild(anchor);
    anchor.click();
    anchor.remove();
    URL.revokeObjectURL(url);
    setStatus(`已生成 ${filename}`, 'success');
  } catch (error) {
    console.error(error);
    setStatus(error.message || '生成失败', 'error');
  }
}

form.addEventListener('submit', handleSubmit);
loadConfig();
