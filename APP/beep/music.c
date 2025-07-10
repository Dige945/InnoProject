#include "music.h"
#include "beep.h"
#include "SysTick.h"

// 音乐播放控制变量
u8 music_status = MUSIC_STOP;
u8 current_song = 0;
u16 current_note = 0;
u16 note_timer = 0;
u8 total_songs = 3; // 总共有3首歌
u8 music_speed = 3; // 固定3倍速播放

// 新格式音乐播放控制变量
u8 new_music_status = MUSIC_STOP;
u16 new_current_note = 0;
u16 new_note_timer = 0;

// 音调频率表（仿照51程序，包含常用音调）
const u16 tone_freq_table[] = {
    0,   // 0 - 休止符
    131, // 1 - C3
    147, // 2 - D3
    165, // 3 - E3
    175, // 4 - F3
    196, // 5 - G3
    220, // 6 - A3
    247, // 7 - B3
    262, // 8 - C4
    294, // 9 - D4
    330, // 10 - E4
    349, // 11 - F4
    392, // 12 - G4
    440, // 13 - A4
    494, // 14 - B4
    523, // 15 - C5
    587, // 16 - D5
    659, // 17 - E5
    698, // 18 - F5
    784, // 19 - G5
    880, // 20 - A5
    988  // 21 - B5
};

// 小星星音乐数据 - 加快播放速度
// const Note song_little_star[] = {
//     {NOTE_C4, 200}, {NOTE_C4, 200}, {NOTE_G4, 200}, {NOTE_G4, 200}, {NOTE_A4, 200}, {NOTE_A4, 200}, {NOTE_G4, 400}, {NOTE_F4, 200}, {NOTE_F4, 200}, {NOTE_E4, 200}, {NOTE_E4, 200}, {NOTE_D4, 200}, {NOTE_D4, 200}, {NOTE_C4, 400}, {NOTE_G4, 200}, {NOTE_G4, 200}, {NOTE_F4, 200}, {NOTE_F4, 200}, {NOTE_E4, 200}, {NOTE_E4, 200}, {NOTE_D4, 400}, {NOTE_G4, 200}, {NOTE_G4, 200}, {NOTE_F4, 200}, {NOTE_F4, 200}, {NOTE_E4, 200}, {NOTE_E4, 200}, {NOTE_D4, 400}, {NOTE_C4, 200}, {NOTE_C4, 200}, {NOTE_G4, 200}, {NOTE_G4, 200}, {NOTE_A4, 200}, {NOTE_A4, 200}, {NOTE_G4, 400}, {NOTE_F4, 200}, {NOTE_F4, 200}, {NOTE_E4, 200}, {NOTE_E4, 200}, {NOTE_D4, 200}, {NOTE_D4, 200}, {NOTE_C4, 400}};
// 修正后的小星星音乐数据 - 标准旋律
const Note song_little_star[] = {
    // 第一段
    {NOTE_C4, 200},
    {NOTE_C4, 200},
    {NOTE_G4, 200},
    {NOTE_G4, 200},
    {NOTE_A4, 200},
    {NOTE_A4, 200},
    {NOTE_G4, 400},
    // 第二段
    {NOTE_F4, 200},
    {NOTE_F4, 200},
    {NOTE_E4, 200},
    {NOTE_E4, 200},
    {NOTE_D4, 200},
    {NOTE_D4, 200},
    {NOTE_C4, 400},
    // 第三段
    {NOTE_G4, 200},
    {NOTE_G4, 200},
    {NOTE_F4, 200},
    {NOTE_F4, 200},
    {NOTE_E4, 200},
    {NOTE_E4, 200},
    {NOTE_D4, 400},
    // 第四段 (重复第三段)
    {NOTE_G4, 200},
    {NOTE_G4, 200},
    {NOTE_F4, 200},
    {NOTE_F4, 200},
    {NOTE_E4, 200},
    {NOTE_E4, 200},
    {NOTE_D4, 400},
    // 第五段 (重复第一段)
    {NOTE_C4, 200},
    {NOTE_C4, 200},
    {NOTE_G4, 200},
    {NOTE_G4, 200},
    {NOTE_A4, 200},
    {NOTE_A4, 200},
    {NOTE_G4, 400},
    // 第六段 (重复第二段)
    {NOTE_F4, 200},
    {NOTE_F4, 200},
    {NOTE_E4, 200},
    {NOTE_E4, 200},
    {NOTE_D4, 200},
    {NOTE_D4, 200},
    {NOTE_C4, 400}};
const u16 song_little_star_length = sizeof(song_little_star) / sizeof(Note);

// 生日快乐歌音乐数据 - 加快播放速度
const Note song_happy_birthday[] = {
    {NOTE_C4, 150}, {NOTE_C4, 150}, {NOTE_D4, 300}, {NOTE_C4, 300}, {NOTE_F4, 300}, {NOTE_E4, 600}, {NOTE_C4, 150}, {NOTE_C4, 150}, {NOTE_D4, 300}, {NOTE_C4, 300}, {NOTE_G4, 300}, {NOTE_F4, 600}, {NOTE_C4, 150}, {NOTE_C4, 150}, {NOTE_C5, 300}, {NOTE_A4, 300}, {NOTE_F4, 300}, {NOTE_E4, 300}, {NOTE_D4, 600}, {NOTE_A4, 150}, {NOTE_A4, 150}, {NOTE_G4, 300}, {NOTE_F4, 300}, {NOTE_G4, 300}, {NOTE_F4, 600}};

const u16 song_happy_birthday_length = sizeof(song_happy_birthday) / sizeof(Note);

// 兰花草音乐数据 - 经典台湾民歌
// const Note song_lanhua_grass[] = {
//     // "我从山中来，带着兰花草"
//     {NOTE_G4, 300},
//     {NOTE_A4, 300},
//     {NOTE_G4, 300},
//     {NOTE_E4, 300},
//     {NOTE_D4, 300},
//     {NOTE_E4, 300},
//     {NOTE_G4, 600},
//     {NOTE_G4, 300},
//     {NOTE_A4, 300},
//     {NOTE_B4, 300},
//     {NOTE_A4, 300},
//     {NOTE_G4, 300},
//     {NOTE_E4, 300},
//     {NOTE_D4, 600},

//     // "种在小园中，希望花开早"
//     {NOTE_G4, 300},
//     {NOTE_A4, 300},
//     {NOTE_G4, 300},
//     {NOTE_E4, 300},
//     {NOTE_D4, 300},
//     {NOTE_E4, 300},
//     {NOTE_G4, 600},
//     {NOTE_B4, 300},
//     {NOTE_A4, 300},
//     {NOTE_G4, 300},
//     {NOTE_E4, 300},
//     {NOTE_D4, 300},
//     {NOTE_C4, 300},
//     {NOTE_D4, 600},

//     // "一日看三回，看得花时过"
//     {NOTE_G4, 300},
//     {NOTE_A4, 300},
//     {NOTE_B4, 300},
//     {NOTE_C5, 300},
//     {NOTE_B4, 300},
//     {NOTE_A4, 300},
//     {NOTE_G4, 600},
//     {NOTE_G4, 300},
//     {NOTE_A4, 300},
//     {NOTE_B4, 300},
//     {NOTE_A4, 300},
//     {NOTE_G4, 300},
//     {NOTE_E4, 300},
//     {NOTE_D4, 600},

//     // "兰花却依然，苞也无一个"
//     {NOTE_G4, 300},
//     {NOTE_A4, 300},
//     {NOTE_B4, 300},
//     {NOTE_C5, 300},
//     {NOTE_B4, 300},
//     {NOTE_A4, 300},
//     {NOTE_G4, 600},
//     {NOTE_B4, 300},
//     {NOTE_A4, 300},
//     {NOTE_G4, 300},
//     {NOTE_E4, 300},
//     {NOTE_D4, 300},
//     {NOTE_C4, 300},
//     {NOTE_D4, 900} // 结尾延长
// };

// 修正后的兰花草音乐数据 - 经典台湾民歌
const Note song_lanhua_grass[] = {
    // "我从山中来，带着兰花草"
    {NOTE_G4, 300},
    {NOTE_A4, 300},
    {NOTE_G4, 300},
    {NOTE_E4, 300},
    {NOTE_D4, 300},
    {NOTE_E4, 300},
    {NOTE_G4, 600},
    {NOTE_G4, 300},
    {NOTE_A4, 300},
    {NOTE_B4, 300},
    {NOTE_A4, 300},
    {NOTE_G4, 300},
    {NOTE_E4, 300},
    {NOTE_D4, 600},

    // "种在小园中，希望花开早"
    {NOTE_G4, 300},
    {NOTE_A4, 300},
    {NOTE_G4, 300},
    {NOTE_E4, 300},
    {NOTE_D4, 300},
    {NOTE_E4, 300},
    {NOTE_G4, 600},
    {NOTE_B4, 300},
    {NOTE_A4, 300},
    {NOTE_G4, 300},
    {NOTE_E4, 300},
    {NOTE_D4, 300},
    {NOTE_C4, 300},
    {NOTE_D4, 600},

    // "一日看三回，看得花时过"
    {NOTE_G4, 300},
    {NOTE_A4, 300},
    {NOTE_B4, 300},
    {NOTE_C5, 300},
    {NOTE_B4, 300},
    {NOTE_A4, 300},
    {NOTE_G4, 600},
    {NOTE_G4, 300},
    {NOTE_A4, 300},
    {NOTE_B4, 300},
    {NOTE_A4, 300},
    {NOTE_G4, 300},
    {NOTE_E4, 300},
    {NOTE_D4, 600},

    // "兰花却依然，苞也无一个"
    {NOTE_G4, 300},
    {NOTE_A4, 300},
    {NOTE_B4, 300},
    {NOTE_C5, 300},
    {NOTE_B4, 300},
    {NOTE_A4, 300},
    {NOTE_G4, 600},
    {NOTE_B4, 300},
    {NOTE_A4, 300},
    {NOTE_G4, 300},
    {NOTE_E4, 300},
    {NOTE_D4, 300},
    {NOTE_C4, 300},
    {NOTE_D4, 900} // 结尾延长
};
const u16 song_lanhua_grass_length = sizeof(song_lanhua_grass) / sizeof(Note);

// 新格式兰花草音乐数据（仿照51程序格式）
const ToneNote song_lanhua_grass_new[] = {
    // "我从山中来，带着兰花草"
    {12, 6},
    {13, 6},
    {12, 6},
    {10, 6}, // 我从山中来
    {9, 6},
    {10, 6},
    {12, 12}, // 带着兰花草
    {12, 6},
    {13, 6},
    {14, 6},
    {13, 6}, // 种在小园中
    {12, 6},
    {10, 6},
    {9, 12}, // 希望开花早

    // "种在小园中，希望花开早"
    {12, 6},
    {13, 6},
    {12, 6},
    {10, 6}, // 一日看三回
    {9, 6},
    {10, 6},
    {12, 12}, // 看得花时过
    {14, 6},
    {13, 6},
    {12, 6},
    {10, 6}, // 兰花却依然
    {9, 6},
    {8, 6},
    {9, 12}, // 苞也无一个

    // "一日看三回，看得花时过"
    {12, 6},
    {13, 6},
    {14, 6},
    {15, 6}, // 转眼秋天到
    {14, 6},
    {13, 6},
    {12, 12}, // 移兰入暖房
    {12, 6},
    {13, 6},
    {14, 6},
    {13, 6}, // 朝朝频顾惜
    {12, 6},
    {10, 6},
    {9, 12}, // 夜夜不相忘

    // "兰花却依然，苞也无一个"
    {12, 6},
    {13, 6},
    {14, 6},
    {15, 6}, // 期待春花开
    {14, 6},
    {13, 6},
    {12, 12}, // 能将宿愿偿
    {14, 6},
    {13, 6},
    {12, 6},
    {10, 6}, // 满庭花簇簇
    {9, 6},
    {8, 6},
    {9, 18}, // 添得许多香 (结尾延长)

    {0, 4},      // 短暂休止
    {0xFF, 0xFF} // 结束标志
};

const u16 song_lanhua_grass_new_length = sizeof(song_lanhua_grass_new) / sizeof(ToneNote);

// 音乐数组指针
const Note *song_list[] = {song_little_star, song_happy_birthday, song_lanhua_grass};
const u16 song_lengths[] = {song_little_star_length, song_happy_birthday_length, song_lanhua_grass_length};

// 音乐名称
const char *song_names[] = {"Little Star", "Happy Birthday", "Lanhua Grass"};

/*******************************************************************************
 * 函数名         : Music_Init
 * 功能描述       : 音乐播放模块初始化
 * 输入参数       : 无
 * 输出参数       : 无
 *******************************************************************************/
void Music_Init(void)
{
    music_status = MUSIC_STOP;
    current_song = 0;
    current_note = 0;
    note_timer = 0;
    music_speed = 3; // 固定3倍速播放
}

/*******************************************************************************
 * 函数名         : Music_Play_Song
 * 功能描述       : 播放指定歌曲
 * 输入参数       : song_index - 歌曲索引
 * 输出参数       : 无
 *******************************************************************************/
void Music_Play_Song(u8 song_index)
{
    extern u8 beep_status; // 将声明移到函数开头
    u16 freq;              // 将变量声明移到函数开头
    u16 period;            // 将变量声明移到函数开头

    if (song_index < total_songs)
    {
        current_song = song_index;
        current_note = 0;
        note_timer = 0;
        music_status = MUSIC_PLAY;

        // 设置第一个音符的频率
        if (song_lengths[song_index] > 0)
        {
            freq = song_list[song_index][0].frequency;
            // 通过改变PWM频率来改变音调
            period = 1000000 / freq; // 1MHz / freq
            if (period > 0)
            {
                TIM_SetAutoreload(TIM4, period - 1);
                TIM_SetCompare3(TIM4, period / 2); // 50%占空比
                // 启动蜂鸣器
                beep_status = 1;
            }
        }

        printf("Playing song: %s\r\n", song_names[song_index]);
    }
}

/*******************************************************************************
 * 函数名         : Music_Stop
 * 功能描述       : 停止音乐播放
 * 输入参数       : 无
 * 输出参数       : 无
 *******************************************************************************/
void Music_Stop(void)
{
    music_status = MUSIC_STOP;
    current_note = 0;
    note_timer = 0;
    BEEP_Off();
    printf("Music stopped\r\n");
}

/*******************************************************************************
 * 函数名         : Music_Pause
 * 功能描述       : 暂停音乐播放
 * 输入参数       : 无
 * 输出参数       : 无
 *******************************************************************************/
void Music_Pause(void)
{
    if (music_status == MUSIC_PLAY)
    {
        music_status = MUSIC_PAUSE;
        BEEP_Off();
        printf("Music paused\r\n");
    }
}

/*******************************************************************************
 * 函数名         : Music_Resume
 * 功能描述       : 恢复音乐播放
 * 输入参数       : 无
 * 输出参数       : 无
 *******************************************************************************/
void Music_Resume(void)
{
    u16 freq;   // 将变量声明移到函数开头
    u16 period; // 将变量声明移到函数开头

    if (music_status == MUSIC_PAUSE)
    {
        music_status = MUSIC_PLAY;
        // 恢复当前音符
        if (current_note < song_lengths[current_song])
        {
            freq = song_list[current_song][current_note].frequency;
            period = 1000000 / freq;
            if (period > 0)
            {
                TIM_SetAutoreload(TIM4, period - 1);
                TIM_SetCompare3(TIM4, period / 2);
            }
        }
        printf("Music resumed\r\n");
    }
}

/*******************************************************************************
 * 函数名         : Music_Next_Song
 * 功能描述       : 播放下一首歌
 * 输入参数       : 无
 * 输出参数       : 无
 *******************************************************************************/
void Music_Next_Song(void)
{
    u8 next_song = (current_song + 1) % total_songs;
    Music_Play_Song(next_song);
    printf("Next song: %s\r\n", song_names[next_song]);
}

/*******************************************************************************
 * 函数名         : Music_Prev_Song
 * 功能描述       : 播放上一首歌
 * 输入参数       : 无
 * 输出参数       : 无
 *******************************************************************************/
void Music_Prev_Song(void)
{
    u8 prev_song = (current_song == 0) ? (total_songs - 1) : (current_song - 1);
    Music_Play_Song(prev_song);
    printf("Previous song: %s\r\n", song_names[prev_song]);
}

/*******************************************************************************
 * 函数名         : Music_Update
 * 功能描述       : 音乐播放更新函数，需要在主循环中调用
 * 输入参数       : 无
 * 输出参数       : 无
 *******************************************************************************/
void Music_Update(void)
{
    extern u8 beep_status; // 将声明移到函数开头
    u16 freq;              // 将变量声明移到函数开头
    u16 period;            // 将变量声明移到函数开头

    if (music_status == MUSIC_PLAY)
    {
        if (current_song < total_songs && current_note < song_lengths[current_song])
        {
            note_timer += 50 * music_speed; // 根据速度倍率调整更新间隔

            if (note_timer >= song_list[current_song][current_note].duration)
            {
                // 当前音符播放完毕，切换到下一个音符
                current_note++;
                note_timer = 0;

                printf("Note %d/%d completed\r\n", current_note, song_lengths[current_song]);

                if (current_note < song_lengths[current_song])
                {
                    // 设置下一个音符的频率
                    freq = song_list[current_song][current_note].frequency;
                    period = 1000000 / freq;
                    if (period > 0)
                    {
                        TIM_SetAutoreload(TIM4, period - 1);
                        TIM_SetCompare3(TIM4, period / 2);
                        // 确保蜂鸣器状态正确
                        beep_status = 1;
                        printf("Playing note: %d Hz\r\n", freq);
                    }
                }
                else
                {
                    // 歌曲播放完毕，停止播放
                    printf("Song completed\r\n");
                    Music_Stop();
                }
            }
        }
    }
}

/*******************************************************************************
 * 函数名         : Music_Play_Lanhua_Grass_New
 * 功能描述       : 播放新格式的兰花草歌曲
 * 输入参数       : 无
 * 输出参数       : 无
 *******************************************************************************/
void Music_Play_Lanhua_Grass_New(void)
{
    extern u8 beep_status;
    u8 tone_index; // 将变量声明移到函数开头
    u16 freq;      // 将变量声明移到函数开头
    u16 period;    // 将变量声明移到函数开头

    // 停止原有的音乐播放
    if (music_status != MUSIC_STOP)
    {
        Music_Stop();
    }

    // 初始化新格式播放参数
    new_music_status = MUSIC_PLAY;
    new_current_note = 0;
    new_note_timer = 0;

    // 设置第一个音符
    if (song_lanhua_grass_new_length > 0)
    {
        tone_index = song_lanhua_grass_new[0].tone_index;
        if (tone_index == 0)
        {
            // 休止符，关闭蜂鸣器
            beep_status = 0;
            BEEP_Off();
        }
        else if (tone_index < (sizeof(tone_freq_table) / sizeof(u16)))
        {
            // 播放指定音调
            freq = tone_freq_table[tone_index];
            if (freq > 0)
            {
                period = 1000000 / freq; // 1MHz / freq
                TIM_SetAutoreload(TIM4, period - 1);
                TIM_SetCompare3(TIM4, period / 2); // 50%占空比
                beep_status = 1;
            }
        }
    }

    printf("Playing Lanhua Grass (New Format)\r\n");
}

/*******************************************************************************
 * 函数名         : Music_Update_New_Format
 * 功能描述       : 新格式音乐播放更新函数
 * 输入参数       : 无
 * 输出参数       : 无
 *******************************************************************************/
void Music_Update_New_Format(void)
{
    extern u8 beep_status;
    u8 duration_ms; // 将变量声明移到函数开头
    u8 tone_index;  // 将变量声明移到函数开头
    u16 freq;       // 将变量声明移到函数开头
    u16 period;     // 将变量声明移到函数开头

    if (new_music_status == MUSIC_PLAY)
    {
        if (new_current_note < song_lanhua_grass_new_length)
        {
            new_note_timer += 50; // 每次增加50ms

            duration_ms = song_lanhua_grass_new[new_current_note].duration * 50; // 转换为毫秒

            if (new_note_timer >= duration_ms)
            {
                // 当前音符播放完毕，切换到下一个音符
                new_current_note++;
                new_note_timer = 0;

                if (new_current_note < song_lanhua_grass_new_length)
                {
                    tone_index = song_lanhua_grass_new[new_current_note].tone_index;

                    if (tone_index == 0xFF)
                    {
                        // 结束标志，停止播放
                        printf("Lanhua Grass (New Format) completed\r\n");
                        new_music_status = MUSIC_STOP;
                        new_current_note = 0;
                        new_note_timer = 0;
                        BEEP_Off();
                        beep_status = 0;
                    }
                    else if (tone_index == 0)
                    {
                        // 休止符，关闭蜂鸣器
                        BEEP_Off();
                        beep_status = 0;
                        printf("Rest note\r\n");
                    }
                    else if (tone_index < (sizeof(tone_freq_table) / sizeof(u16)))
                    {
                        // 播放指定音调
                        freq = tone_freq_table[tone_index];
                        if (freq > 0)
                        {
                            period = 1000000 / freq;
                            TIM_SetAutoreload(TIM4, period - 1);
                            TIM_SetCompare3(TIM4, period / 2);
                            beep_status = 1;
                            printf("Playing tone: %d Hz (index %d)\r\n", freq, tone_index);
                        }
                    }
                }
                else
                {
                    // 歌曲播放完毕
                    printf("Lanhua Grass (New Format) finished\r\n");
                    new_music_status = MUSIC_STOP;
                    new_current_note = 0;
                    new_note_timer = 0;
                    BEEP_Off();
                    beep_status = 0;
                }
            }
        }
    }
}