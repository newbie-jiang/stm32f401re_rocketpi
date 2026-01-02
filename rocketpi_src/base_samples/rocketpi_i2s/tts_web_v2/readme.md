# TTS Web Helper v2.0

`tts_web_v2` 提供以下能力：

- `edge-tts` 语音合成 + `ffmpeg` 转码。
- 体积极小的前端，支持一键生成 `audio.h` 或 `audio.bin`。
- 可配置的 PCM 采样率（8k/11k/16k/22k/32k/44k/48k/96k）与声道数（1/2）。
- Edge TTS voice 列表可切换，默认保持 `zh-CN-XiaoxiaoNeural`。
- 后端提供 `/api/config`，前端自动读取可选参数，方便扩展。

## 依赖

- Node.js ≥ 18（以 v22.2.0 验证）。 https://nodejs.org/en/download
- `edge-tts`      winget install -e --id Python.Python.3.13     pip install edge-tts
- `ffmpeg`   https://www.ffmpeg.org/download.html

> 如 `edge-tts` 或 `ffmpeg` 不在 `PATH`，可通过环境变量 `EDGE_TTS_CMD`、`FFMPEG_CMD` 指定绝对路径。

确保上述三项安装完成并配置了全部环境变量

## 快速开始

```bash
cd tts_web_v2
npm install
npm start
```

浏览器访问 `http://localhost:5173`，填入文本、音色、速率、音高、采样率、声道、输出格式即可触发下载。

### 后端 API

- `GET /api/config`：返回 voice / sampleRate / channels 等选项。
- `POST /api/synthesize`：JSON 结构
  ```jsonc
  {
    "text": "你好，Rocket！",
    "voice": "zh-CN-XiaoxiaoNeural",
    "rate": "+0%",
    "pitch": "+0Hz",
    "sampleRate": 16000,
    "channels": 2,
    "output": "header", // or "bin"
    "fileName": "rocket_intro" // optional, without extension
  }
  ```

## edge tts 文生PCM文件

![image-20251217230921241](https://cloud.rocketpi.club/cloud/image-20251217230921241.png)

## 使用本地音频生成pcm文件

![image-20251217230932559](https://cloud.rocketpi.club/cloud/image-20251217230932559.png)
