# 项目文档

## 简介

这是一个基于 eSpeak 和 Windows PlaySound API 的语音合成与播放工具。

## 功能

- 文字转语音（TTS）
- 同步/异步音频播放
- WAV 文件录制与回放

## 核心 API

### eSpeak 初始化

```c
int sample_rate = espeak_Initialize(
    AUDIO_OUTPUT_SYNCHRONOUS,  // 同步输出模式
    0,                          // 默认缓冲区大小
    NULL,                       // 使用默认数据路径
    0                           // 无额外选项
);

if (sample_rate == -1) {
    fprintf(stderr, "eSpeak 初始化失败
");
    return 1;
}
```

### WAV 文件写入

```c
// 写入音频样本数据
fwrite(wav, 1, bytes, g_wav_fp);

// 回头修改 RIFF chunk 大小
fseek(g_wav_fp, 4, SEEK_SET);
uint32_t riff_size = file_size - 8;
fwrite(&riff_size, sizeof(riff_size), 1, g_wav_fp);
```

### Windows 音频播放

```c
BOOL ok = PlaySoundA(
    path,                       // WAV 文件路径
    NULL,                       // 不关联模块
    SND_FILENAME | SND_SYNC     // 从文件同步播放
);

if (!ok) {
    DWORD err = GetLastError();
    fprintf(stderr, "播放失败，错误码: %lu
", err);
}
```

## 编译

```bash
gcc main.c -lespeak -lwinmm -o tts_tool.exe
```

## 许可证

MIT License
