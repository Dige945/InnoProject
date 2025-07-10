#ifndef _RGB_DISPLAY_H
#define _RGB_DISPLAY_H

#include "system.h"

// External declarations of character patterns and colors
extern const u8 custom_char_patterns[][5];
extern const u32 display_colors[];

// Function prototypes
void RGB_ShowCustomChar(const u8 *pattern, u32 color);
void RGB_DisplaySequence(u16 delay_duration_ms);
void RGB_StartDisplay(void);
// Note: RGB_ShowHeart is now declared in ws2812.h

#endif