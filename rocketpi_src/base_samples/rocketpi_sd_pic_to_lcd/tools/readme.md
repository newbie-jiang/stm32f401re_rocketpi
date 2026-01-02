# 工具目录说明

本目录当前包含 `convert_pic_to_bin.py`，用于把 `pic/` 目录下的 PNG/JPG/BMP/JPEG 等图片批量转换为 RGB565（小端）原始数据，方便直接拷贝到 `pic_bin/` 后由固件读取并显示。

## convert_pic_to_bin.py
- **功能**：遍历输入目录下的所有图片，按递增编号（`000.bin`、`001.bin` …）输出到目标目录。每张图片都会被转换成 240×240 的 RGB565 小端格式，适配 ST7789 显示屏的帧缓存。
- **依赖**：需要 Python 3 以及安装好的 `ffmpeg`

### 参数
- `--input`：源图片根目录，默认 `pic`。
- `--output`：输出 `.bin` 文件根目录，默认 `pic_bin`。
- `--ffmpeg`：`ffmpeg` 可执行文件名称或路径，默认 `ffmpeg`。
- `--force`：若目标文件已存在，添加该参数可覆盖；未添加时会跳过已生成的文件。

### 用法示例
```powershell
python tools\convert_pic_to_bin.py `
  --input pic\clock_240x240_300frames_png_6fps `
  --output pic_bin `
  --force
```

运行结束后，脚本会显示成功生成的帧数量。将新的 `.bin` 文件复制到 SD 卡的 `PIC_BIN`（或固件中配置的目录）即可在设备上播放新的帧动画。



![image-20251218045512532](https://cloud.rocketpi.club/cloud/image-20251218045512532.png)
