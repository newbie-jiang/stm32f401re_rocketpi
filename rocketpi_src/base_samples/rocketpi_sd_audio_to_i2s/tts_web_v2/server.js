const express = require('express');
const morgan = require('morgan');
const path = require('path');
const os = require('os');
const fs = require('fs/promises');
const { execFile } = require('child_process');
const { promisify } = require('util');

const execFileAsync = promisify(execFile);

const SUPPORTED_SAMPLE_RATES = [8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000];
const SUPPORTED_CHANNELS = [1, 2];
const DEFAULT_SAMPLE_RATE = 16000;
const DEFAULT_CHANNELS = 2;
const BITS_PER_SAMPLE = 16;
const EDGE_TTS_CMD = process.env.EDGE_TTS_CMD || 'edge-tts';
const FFMPEG_CMD = process.env.FFMPEG_CMD || 'ffmpeg';
const DEFAULT_VOICE = 'zh-CN-XiaoxiaoNeural';
const AVAILABLE_VOICES = [
  { id: 'zh-CN-XiaoxiaoNeural', label: 'zh-CN Xiaoxiao (F)' },
  { id: 'zh-CN-XiaochenNeural', label: 'zh-CN Xiaochen (M)' },
  { id: 'zh-CN-YunxiNeural', label: 'zh-CN Yunxi (M)' },
  { id: 'zh-CN-YunyeNeural', label: 'zh-CN Yunye (M)' },
  { id: 'zh-CN-XiaoyiNeural', label: 'zh-CN Xiaoyi (F)' },
  { id: 'zh-HK-HiuMaanNeural', label: 'zh-HK HiuMaan (F, 粤语)' },
  { id: 'zh-HK-HiuGaaiNeural', label: 'zh-HK HiuGaai (F, 粤语)' },
  { id: 'zh-HK-WanLungNeural', label: 'zh-HK WanLung (M, 粤语)' },
  { id: 'en-US-EmmaMultilingualNeural', label: 'en-US Emma (F)' },
  { id: 'en-US-GuyMultilingualNeural', label: 'en-US Guy (M)' },
];

const app = express();
app.use(express.json({ limit: '2mb' }));
app.use(morgan('dev'));

const publicDir = path.join(__dirname, 'public');
app.use(express.static(publicDir));

function sanitizeFileBase(input) {
  const slug = input
    .toLowerCase()
    .replace(/[^a-z0-9]+/gi, '-')
    .replace(/^-+|-+$/g, '')
    .slice(0, 40);
  return slug;
}

function defaultBaseName() {
  const now = new Date();
  const pad = (value) => String(value).padStart(2, '0');
  const stamp = `${now.getFullYear()}${pad(now.getMonth() + 1)}${pad(now.getDate())}_${pad(now.getHours())}${pad(
    now.getMinutes(),
  )}${pad(now.getSeconds())}`;
  return `audio_${stamp}`;
}

function resolveBaseName(customName) {
  const trimmed = customName ? String(customName).replace(/\.[^/.]+$/, '') : '';
  const sanitized = trimmed ? sanitizeFileBase(trimmed) : '';
  return sanitized || defaultBaseName();
}

function formatHeader(pcmBuffer, meta) {
  const totalSamples = pcmBuffer.length / 2;
  const values = new Array(totalSamples);
  for (let i = 0; i < totalSamples; i += 1) {
    values[i] = pcmBuffer.readInt16LE(i * 2);
  }
  const chunkSize = 64;
  const lines = [];
  const commentSnippet = meta.textSnippet;
  lines.push(`/* edge-tts export: "${commentSnippet}" (voice=${meta.voice}) */`);
  lines.push('#ifndef AUDIO_H');
  lines.push('#define AUDIO_H');
  lines.push('');
  lines.push('#include <stdint.h>');
  lines.push('');
  lines.push(`#define AUDIO_SAMPLE_RATE_HZ ${meta.sampleRate}U`);
  lines.push(`#define AUDIO_NUM_CHANNELS   ${meta.channels}U`);
  lines.push(`#define AUDIO_BITS_PER_SAMPLE ${BITS_PER_SAMPLE}U`);
  lines.push('');
  lines.push('static const int16_t audio_track[] = {');
  for (let i = 0; i < values.length; i += chunkSize) {
    const chunk = values.slice(i, i + chunkSize).join(', ');
    const suffix = i + chunkSize < values.length ? ',' : '';
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

async function runEdgeTTS({ text, voice, rate, pitch, mediaPath }) {
  const args = ['--voice', voice, '--text', text, '--rate', rate, '--pitch', pitch, '--write-media', mediaPath];
  await execFileAsync(EDGE_TTS_CMD, args, { windowsHide: true });
}

async function convertToPCM(mediaPath, pcmPath, { sampleRate, channels }) {
  const args = ['-y', '-i', mediaPath, '-ar', String(sampleRate), '-ac', String(channels), '-f', 's16le', pcmPath];
  await execFileAsync(FFMPEG_CMD, args, { windowsHide: true });
}

function parseSampleRate(value) {
  const numeric = Number(value);
  if (Number.isFinite(numeric) && SUPPORTED_SAMPLE_RATES.includes(numeric)) {
    return numeric;
  }
  return DEFAULT_SAMPLE_RATE;
}

function parseChannels(value) {
  const numeric = Number(value);
  if (Number.isFinite(numeric) && SUPPORTED_CHANNELS.includes(numeric)) {
    return numeric;
  }
  return DEFAULT_CHANNELS;
}

app.get('/api/config', (req, res) => {
  res.json({
    defaultVoice: DEFAULT_VOICE,
    voices: AVAILABLE_VOICES,
    sampleRates: SUPPORTED_SAMPLE_RATES,
    defaultSampleRate: DEFAULT_SAMPLE_RATE,
    channelOptions: SUPPORTED_CHANNELS,
    defaultChannels: DEFAULT_CHANNELS,
  });
});

app.post('/api/synthesize', async (req, res) => {
  const {
    text,
    voice = DEFAULT_VOICE,
    rate = '+0%',
    pitch = '+0Hz',
    output = 'header',
    sampleRate,
    channels,
    fileName,
  } = req.body || {};

  if (!text || !text.trim()) {
    return res.status(400).json({ error: '请输入要合成的文字' });
  }
  if (text.length > 400) {
    return res.status(400).json({ error: '文本过长，请限制在 400 个字符以内' });
  }
  const selectedSampleRate = parseSampleRate(sampleRate);
  const selectedChannels = parseChannels(channels);
  const trimmed = text.trim();
  const tmpDir = await fs.mkdtemp(path.join(os.tmpdir(), 'edge-tts-'));
  const mediaPath = path.join(tmpDir, 'tts_media.wav');
  const pcmPath = path.join(tmpDir, 'tts_audio.pcm');

  try {
    await runEdgeTTS({ text: trimmed, voice, rate, pitch, mediaPath });
    await convertToPCM(mediaPath, pcmPath, { sampleRate: selectedSampleRate, channels: selectedChannels });
    const pcmBuffer = await fs.readFile(pcmPath);
    if (!pcmBuffer.length) {
      throw new Error('PCM 数据为空，可能是 edge-tts 或 ffmpeg 失败');
    }


    const snippet = trimmed.replace(/\s+/g, ' ').slice(0, 64);
    const baseName = resolveBaseName(fileName);

    if (output === 'bin') {
      res.setHeader('Content-Type', 'application/octet-stream');
      res.setHeader('Content-Disposition', `attachment; filename="${baseName}.bin"`);
      return res.send(pcmBuffer);
    }

    const headerText = formatHeader(pcmBuffer, {
      voice,
      textSnippet: snippet,
      sampleRate: selectedSampleRate,
      channels: selectedChannels,
    });
    res.setHeader('Content-Type', 'text/plain; charset=utf-8');
    res.setHeader('Content-Disposition', `attachment; filename="${baseName}.h"`);
    return res.send(headerText);
  } catch (error) {
    console.error(error);
    const message = error.stderr || error.message || '语音合成失败';
    return res.status(500).json({ error: message.toString() });
  } finally {
    await fs.rm(tmpDir, { recursive: true, force: true });
  }
});

const PORT = process.env.PORT || 5173;
app.listen(PORT, () => {
  console.log(`TTS helper running at http://localhost:${PORT}`);
});
