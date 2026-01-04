## USB 音频播放优化说明

- **动态缓冲清理**：在 `AUDIO_PeriodicTC_FS` 中统计每个 USB OUT 包的长度与到达时间，一旦发现包间隔超过 `AUDIO_STREAM_GAP_FLUSH_MS` 或者主机发送短包，就立即停止 I2S DMA、清空循环缓冲，并等待新的数据填满后再重新启动，避免大缓冲导致的尾段重播。
- **延迟启动 DMA**：`Audio_StartPlayback` 不再在 `rd_enable` 为 0 时强行启动 DMA，而是设置 `start_pending`，在 USB 缓冲准备好后由 `Audio_TryStartPending` 触发，完全消除首包空数据造成的 0.5 s 静音。
- **智能欠供修复**：保持原有的欠供检测，用静音填补不足的样本，同时记录事件计数，既防止“滋滋”噪声又方便调试。
- **PLL 频率校准**：将 I2S PLLI2S 重新配置为 128/5，确保 I2S LRCK 精确为 16 kHz，与主机采样率一致，避免缓慢漂移导致定期欠供。



每次更新cubemx之后，将usbd_audio.h 宏定义配置更改，不然切换歌曲会重复声音

```
#defineAUDIO_OUT_PACKET_NUM                          800U
```



