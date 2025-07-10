#include "ws2812.h"
#include "SysTick.h"

u8 g_rgb_databuf[3][RGB_LED_XWIDTH][RGB_LED_YHIGH]; // RGB

const u8 g_rgb_num_buf[][5] =
	{
		{0x70, 0x88, 0x88, 0x88, 0x70}, // 0
		{0x00, 0x48, 0xF8, 0x08, 0x00}, // 1
		{0x48, 0x98, 0xA8, 0x48, 0x00}, // 2
		{0x00, 0x88, 0xA8, 0x50, 0x00}, // 3
		{0x20, 0x50, 0x90, 0x38, 0x10}, // 4
		{0x00, 0xE8, 0xA8, 0xB8, 0x00}, // 5
		{0x00, 0x70, 0xA8, 0xA8, 0x30}, // 6
		{0x80, 0x98, 0xA0, 0xC0, 0x00}, // 7
		{0x50, 0xA8, 0xA8, 0xA8, 0x50}, // 8
		{0x40, 0xA8, 0xA8, 0xA8, 0x70}, // 9
		{0x38, 0x50, 0x90, 0x50, 0x38}, // A
		{0xF8, 0xA8, 0xA8, 0x50, 0x00}, // B
		{0x70, 0x88, 0x88, 0x88, 0x00}, // C
		{0xF8, 0x88, 0x88, 0x50, 0x20}, // D
		{0xF8, 0xA8, 0xA8, 0xA8, 0x00}, // E
		{0x00, 0xF8, 0xA0, 0xA0, 0x00}, // F
};

// 彩虹颜色数组，用于五彩斑斓的像素显示
const u32 rainbow_colors[] = {
	RGB_COLOR_RED,	   // 红色
	RGB_COLOR_ORANGE,  // 橙色
	RGB_COLOR_YELLOW,  // 黄色
	RGB_COLOR_LIME,	   // 青柠色
	RGB_COLOR_GREEN,   // 绿色
	RGB_COLOR_CYAN,	   // 青色
	RGB_COLOR_BLUE,	   // 蓝色
	RGB_COLOR_PURPLE,  // 紫色
	RGB_COLOR_MAGENTA, // 品红色
	RGB_COLOR_PINK,	   // 粉色
	RGB_COLOR_WHITE,   // 白色
	RGB_COLOR_INDIGO   // 靛蓝色
};

#define RAINBOW_COLOR_COUNT 12

void RGB_LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_SetBits(GPIOE, GPIO_Pin_5);

	RGB_LED_Clear();
}

void delay(u32 ns) // 100ns
{
	while (ns--)
		;
}

/********************************************************/
//
/********************************************************/
void RGB_LED_Write0(void)
{
	RGB_LED_HIGH;
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	RGB_LED_LOW;
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
}

/********************************************************/
//
/********************************************************/

void RGB_LED_Write1(void)
{
	RGB_LED_HIGH;
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	RGB_LED_LOW;
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
}

void RGB_LED_Reset(void)
{
	RGB_LED_LOW;
	delay_us(80);
	RGB_LED_HIGH;
}

void RGB_LED_Write_Byte(uint8_t byte)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		if (byte & 0x80)
		{
			RGB_LED_Write1();
		}
		else
		{
			RGB_LED_Write0();
		}
		byte <<= 1;
	}
}

void RGB_LED_Write_24Bits(uint8_t green, uint8_t red, uint8_t blue)
{
	RGB_LED_Write_Byte(green);
	RGB_LED_Write_Byte(red);
	RGB_LED_Write_Byte(blue);
}

// ������ɫ�趨��������ɫ�Դ�����
void RGB_LED_Red(void)
{
	uint8_t i;
	// LEDȫ�ʵ�
	for (i = 0; i < 25; i++)
	{
		RGB_LED_Write_24Bits(0, 0xff, 0);
	}
}

void RGB_LED_Green(void)
{
	uint8_t i;

	for (i = 0; i < 25; i++)
	{
		RGB_LED_Write_24Bits(0xff, 0, 0);
	}
}

void RGB_LED_Blue(void)
{
	uint8_t i;

	for (i = 0; i < 25; i++)
	{
		RGB_LED_Write_24Bits(0, 0, 0xff);
	}
}

void RGB_Screen_Update(void)
{
	u8 i, j;
	for (i = 0; i < RGB_LED_YHIGH; i++)
	{
		for (j = 0; j < RGB_LED_XWIDTH; j++)
			RGB_LED_Write_24Bits(g_rgb_databuf[1][j][i], g_rgb_databuf[0][j][i], g_rgb_databuf[2][j][i]);
	}
	RGB_LED_Reset(); // 发送完数据后再复位来锁存数据
}

void RGB_Buffer_Clear(void)
{
	u8 i, j;
	for (i = 0; i < RGB_LED_YHIGH; i++)
	{
		for (j = 0; j < RGB_LED_XWIDTH; j++)
		{
			g_rgb_databuf[0][j][i] = 0;
			g_rgb_databuf[1][j][i] = 0;
			g_rgb_databuf[2][j][i] = 0;
		}
	}
}

void RGB_LED_Clear(void)
{
	RGB_Buffer_Clear();
	RGB_Screen_Update();
	delay_ms(1);
}

// x,y:λ
// status1:0:Ϩ
// colorRGBɫ
void RGB_Set_Pixel(u8 x, u8 y, u32 color)
{
	if (x >= RGB_LED_XWIDTH || y >= RGB_LED_YHIGH)
		return;

	g_rgb_databuf[0][x][y] = (color >> 16) & 0xFF; // r
	g_rgb_databuf[1][x][y] = (color >> 8) & 0xFF;  // g
	g_rgb_databuf[2][x][y] = color & 0xFF;		   // b
}

void RGB_DrawDotColor(u8 x, u8 y, u8 status, u32 color)
{
	if (status)
	{
		g_rgb_databuf[0][x][y] = color >> 16; // r
		g_rgb_databuf[1][x][y] = color >> 8;  // g
		g_rgb_databuf[2][x][y] = color;		  // b
	}
	else
	{
		g_rgb_databuf[0][x][y] = 0x00;
		g_rgb_databuf[1][x][y] = 0x00;
		g_rgb_databuf[2][x][y] = 0x00;
	}
}

void RGB_DrawLine_Color(u16 x1, u16 y1, u16 x2, u16 y2, u32 color)
{
	u16 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // ������������
	delta_y = y2 - y1;
	uRow = x1;
	uCol = y1;
	if (delta_x > 0)
		incx = 1; // ���õ�������
	else if (delta_x == 0)
		incx = 0; // ��ֱ��
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; // ˮƽ��
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; // ѡȡ��������������
	else
		distance = delta_y;
	for (t = 0; t <= distance + 1; t++) // �������
	{
		RGB_Set_Pixel(uRow, uCol, color); //
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

// ������
//(x1,y1),(x2,y2):���εĶԽ�����
void RGB_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u32 color)
{
	RGB_DrawLine_Color(x1, y1, x2, y1, color);
	RGB_DrawLine_Color(x1, y1, x1, y2, color);
	RGB_DrawLine_Color(x1, y2, x2, y2, color);
	RGB_DrawLine_Color(x2, y1, x2, y2, color);
	RGB_Screen_Update();
}

// ָλûһָСԲ
//(x,y):ĵ
// r    :뾶
void RGB_Draw_Circle(u16 x0, u16 y0, u8 r, u32 color)
{
	int a, b;
	int di;
	a = 0;
	b = r;
	di = 3 - (r << 1); // ж¸λõı־
	while (a <= b)
	{
		RGB_Set_Pixel(x0 + a, y0 - b, color); // 5
		RGB_Set_Pixel(x0 + b, y0 - a, color); // 0
		RGB_Set_Pixel(x0 + b, y0 + a, color); // 4
		RGB_Set_Pixel(x0 + a, y0 + b, color); // 6
		RGB_Set_Pixel(x0 - a, y0 + b, color); // 1
		RGB_Set_Pixel(x0 - b, y0 + a, color);
		RGB_Set_Pixel(x0 - a, y0 - b, color); // 2
		RGB_Set_Pixel(x0 - b, y0 - a, color); // 7
		a++;
		// ʹBresenham㷨Բ
		if (di < 0)
			di += 4 * a + 6;
		else
		{
			di += 10 + 4 * (a - b);
			b--;
		}
	}
	RGB_Screen_Update();
}

void RGB_ShowCharNum(u8 num, u32 color)
{
	u8 i = 0, j;
	u8 x = 0, y = 0;
	u8 temp = 0;
	u8 color_index = 0; // 彩虹颜色索引

	// 先清除缓冲区
	RGB_Buffer_Clear();

	// 使用原始代码的坐标映射逻辑：按列填充，先填满y轴再填x轴
	for (j = 0; j < 5; j++)
	{
		temp = g_rgb_num_buf[num][j];
		for (i = 0; i < 5; i++)
		{
			if (temp & 0x80)
			{
				// 每个像素使用不同的彩虹颜色
				RGB_DrawDotColor(x, y, 1, rainbow_colors[color_index % RAINBOW_COLOR_COUNT]);
				color_index++; // 切换到下一个颜色
			}
			else
				RGB_DrawDotColor(x, y, 0, 0); // 熄灭的像素
			temp <<= 1;
			y++;
			if (y == RGB_LED_YHIGH)
			{
				y = 0;
				x++;
				if (x == RGB_LED_XWIDTH)
				{
					// 确保屏幕更新
					RGB_Screen_Update();
					return;
				}
			}
		}
	}
	// 确保屏幕更新
	RGB_Screen_Update();
}

// Heart pattern for 5x5 display
const u8 heart_pattern[5] = {
	0x50, // 01010000
	0xF8, // 11111000
	0xF8, // 11111000
	0x70, // 01110000
	0x20  // 00100000
};

void RGB_ShowHeart(u32 color)
{
	u8 i, j;
	u8 temp;
	u8 color_index = 0; // 彩虹颜色索引

	RGB_Buffer_Clear();
	for (j = 0; j < 5; j++)
	{
		temp = heart_pattern[j];
		for (i = 0; i < 5; i++)
		{
			if (temp & 0x80)
			{
				// 每个像素使用不同的彩虹颜色，让爱心也五彩斑斓
				RGB_Set_Pixel(i, j, rainbow_colors[color_index % RAINBOW_COLOR_COUNT]);
				color_index++; // 切换到下一个颜色
			}
			temp <<= 1;
		}
	}
	RGB_Screen_Update();
}

// 简单的测试函数，显示一个5x5的十字图案
void RGB_ShowTest(u32 color)
{
	u8 i, j;

	RGB_Buffer_Clear();
	printf("Displaying test pattern\r\n");

	// 显示一个十字图案
	for (i = 0; i < 5; i++)
	{
		RGB_Set_Pixel(i, 2, color); // 水平线
		RGB_Set_Pixel(2, i, color); // 垂直线
	}

	RGB_Screen_Update();
	printf("Test pattern sent to RGB\r\n");
}

// 增强版的数字显示函数，带调试输出，使用五彩斑斓的像素显示
void RGB_ShowCharNum_Debug(u8 num, u32 color)
{
	u8 i = 0, j = 0;
	u8 x = 0, y = 0;
	u8 temp = 0;
	u8 color_index = 0; // 彩虹颜色索引

	printf("Displaying number %d with rainbow colors (original position)\r\n", num);

	// 先清除缓冲区
	RGB_Buffer_Clear();

	// 使用原始代码的坐标映射逻辑：按列填充，先填满y轴再填x轴
	for (j = 0; j < 5; j++)
	{
		temp = g_rgb_num_buf[num][j];
		printf("Row %d: 0x%02X = ", j, temp);
		for (i = 0; i < 5; i++)
		{
			if (temp & 0x80)
			{
				printf("*");
				// 每个像素使用不同的彩虹颜色
				RGB_DrawDotColor(x, y, 1, rainbow_colors[color_index % RAINBOW_COLOR_COUNT]);
				color_index++; // 切换到下一个颜色
			}
			else
			{
				printf(".");
				RGB_DrawDotColor(x, y, 0, 0); // 熄灭的像素
			}
			temp <<= 1;
			y++;
			if (y == RGB_LED_YHIGH)
			{
				y = 0;
				x++;
				if (x == RGB_LED_XWIDTH)
				{
					printf("\r\n");
					// 确保屏幕更新
					RGB_Screen_Update();
					printf("Rainbow number display sent to RGB (original position)\r\n");
					return;
				}
			}
		}
		printf("\r\n");
	}

	// 确保屏幕更新
	RGB_Screen_Update();
	printf("Rainbow number display sent to RGB (original position)\r\n");
}
