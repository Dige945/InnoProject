#ifndef _music_H
#define _music_H

#include "system.h"

// 音符频率定义（Hz）
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_G5 784
#define NOTE_A5 880
#define NOTE_B5 988

// 扩展音符定义
#define NOTE_REST 0 // 休止符
#define NOTE_C3 131
#define NOTE_D3 147
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_G3 196
#define NOTE_A3 220
#define NOTE_B3 247

// 音乐播放状态
#define MUSIC_STOP 0
#define MUSIC_PLAY 1
#define MUSIC_PAUSE 2

// 音乐结构体
typedef struct
{
    u16 frequency; // 音符频率
    u16 duration;  // 持续时间（ms）
} Note;

// 新的音乐数据格式（仿照51程序格式）
typedef struct
{
    u8 tone_index; // 音调索引（0=休止符，0xFF=结束标志）
    u8 duration;   // 持续时间（单位：50ms）
} ToneNote;

// 音乐播放控制变量
extern u8 music_status;
extern u8 current_song;
extern u16 current_note;
extern u16 note_timer;
extern u8 total_songs;

// 函数声明
void Music_Init(void);
void Music_Play_Song(u8 song_index);
void Music_Stop(void);
void Music_Pause(void);
void Music_Resume(void);
void Music_Next_Song(void);
void Music_Prev_Song(void);
void Music_Update(void);

// 新格式音乐播放函数
void Music_Play_Lanhua_Grass_New(void);
void Music_Update_New_Format(void);

// 音乐数据声明
extern const Note song_little_star[];
extern const Note song_happy_birthday[];
extern const Note song_lanhua_grass[];
extern const u16 song_little_star_length;
extern const u16 song_happy_birthday_length;
extern const u16 song_lanhua_grass_length;

// 新格式音调频率表和音乐数据
extern const u16 tone_freq_table[];
extern const ToneNote song_lanhua_grass_new[];
extern const u16 song_lanhua_grass_new_length;

#endif