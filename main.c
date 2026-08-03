#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <espeak-ng/speak_lib.h>
#include <windows.h>
#include <mmsystem.h> 

#pragma pack(push, 1)

typedef struct
{
	// 由于wav基于RIFF格式
	char riff_id[4]; // 4
	uint32_t riff_size; // 4
	char format[4]; // 4
	
	// fmt
	char fmt_id[4]; // 注意末尾有空格
	uint32_t fmt_size; // 4 PCM通常4字节
	uint16_t audio_fmt; // PCM 为1, 2字节
	uint16_t num_channles; // 声道数 1
	uint32_t sample_rate; // 采样率 44100HZ 4b
	uint32_t bytes_rate; // 每秒采样数
	uint16_t block_align; // 每个样块字节数
	uint16_t bits_per_depth; // 位深，比如16 24 32
	
	// data
	char data_id[4];
	uint32_t data_size; // 音频数据长度 4	
}WavHeader;
#pragma pack(pop) // end

// 初始化静态
static FILE* g_wav_fp = NULL; // 文件指针
static uint32_t g_total_bytes = 0; // 4字节的标志位

// espeak回调函数 16位的short PCM音频数据, int numsamples 样本数 时间链表
static int synth_callback(short* wav, int numsamples, espeak_EVENT* events)
{
	(void)events;
	
	if(wav == NULL)
	{
		return 0; // 没有数据 进行下一次
	}
	
	if(g_wav_fp == NULL || numsamples <= 0)
	{
		return 1; // file open failed || not write data
	}
	
	size_t bytes = numsamples * sizeof(short);
	if(fwrite(wav, 1, bytes, g_wav_fp) != bytes)
	{
		return 1; // write failed
	}

	g_total_bytes += (uint32_t)bytes;

	return 0;
}

static int update_wav_header(void)
{
	if(g_wav_fp == NULL) return -1;
	uint32_t riff_size = 36 + g_total_bytes;

	if(fseek(g_wav_fp, 4, SEEK_SET) != 0)
	{
		return -1;
	}
	if(fwrite(&riff_size, 4, 1, g_wav_fp) != 1) return -1;

	if(fseek(g_wav_fp, 40, SEEK_SET) != 0) return -1; // 40字节在data位置
	
	if(fwrite(&g_total_bytes, 4, 1, g_wav_fp) != 1) return -1;

	return 0;
}
static int tts_to_wav(const char* text, const char* path)
{
	int result = -1;
	// type: 阻塞，同步生成wav和传为语音
	int rate = espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, 0, "E:\\msys2\\mingw64\\share\\espeak-ng-data", 0);
	if(rate < 0)
	{
		fprintf(stderr, "Espeak Init failed!\n");	
		return -1;
	}

	if(espeak_SetVoiceByName("cmn") != EE_OK)
	{
		fprintf(stderr, "Warring: zh voice not found\n");	
	}
	
	espeak_SetSynthCallback(synth_callback); // bind callbeck function
	g_wav_fp = fopen(path, "wb"); // 二进制
	if(g_wav_fp == NULL)
	{
		fprintf(stderr, "FILE pointer init failed\n");	
		espeak_Terminate();	
		return -1;
	}
	WavHeader h = {
		{'R', 'I', 'F', 'F'},
		36, // 目前占位
		{'W', 'A', 'V', 'E'},
		{'f', 'm', 't', ' '},
		16,
		1,
		1,
		(uint32_t)rate,
		(uint32_t)(rate * 2),
		2,
		16,
		{'d', 'a', 't', 'a'},
		0 // 占位
	};
	
	fwrite(&h, sizeof(h), 1, g_wav_fp);
	g_total_bytes = 0; // 重新init zero
	
	unsigned int uid = 0;
	// 包括文本结束'\0'
	if(espeak_Synth(text /*原始文本*/, strlen(text) + 1, 0, POS_CHARACTER, 0, espeakCHARS_UTF8, &uid, NULL) != EE_OK)
	{
		fprintf(stderr, "Synth Failed\n");	
		goto cleanup;
	}
	espeak_Synchronize(); // 等待阻塞	
	if(update_wav_header() != 0)
	{
		fprintf(stderr, "Error: Update failed");	
		goto cleanup;
	}

	printf("Generated: %s (%u bytes)\n", path, g_total_bytes);

	result = 0;
cleanup:
	if(g_wav_fp){fclose(g_wav_fp); g_wav_fp = NULL; } // 防止野指针释放文件指针
	espeak_Terminate();
	return result;
}

static int play_wav(const char* path)
{
	// 同步阻塞播放
	BOOL ok = PlaySoundA(path, NULL, SND_FILENAME | SND_SYNC);
	return ok ? 0 : -1;
}

int main(int argc, char* argv[])
{

	const char* text;
	
	char buf[1024];
	
	const char* outfile ="tts_output.wav";
	if(argc > 1)
	{
		text = argv[1];	
	}else
	{
		printf("输入文字: ");	
		if(fgets(buf, sizeof(buf), stdin) == NULL) return 1;
		// 这是标准输入读取一行
		buf[strcspn(buf, "\n")] = '\0';
		text = buf; // 数组退化指针	
	}

	if(strlen(text) == 0)
	{
		fprintf(stderr, "Error: 没有输入！");
		return 1;
	}
	printf("Text: %s\n", text);

	if((tts_to_wav(text, outfile) != 0)) return 1;

	printf("Playing..\n");
	
	if(play_wav(outfile) != 0)
	{
		fprintf(stderr, "Error: Play failed!");	
		return 1;
	}
	
	printf("Done.\n");
	
	getchar(); // 必须回车看输
	return 0;
	
}
