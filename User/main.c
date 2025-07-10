#include "system.h"
#include "SysTick.h"
#include "lsens.h"
#include "../APP/tftlcd/tftlcd.h"
// #include "../APP/tftlcd/bk_image.h"  // 移除图片显示
#include "../APP/dht11/dht11.h"
#include "../APP/rtc/rtc.h"
#include "../APP/key/key.h"
#include "../APP/beep/beep.h"
#include "../APP/beep/music.h"
#include "../APP/hc05/hc05.h"
#include "../APP/hwjs/hwjs.h"
#include "../APP/pwm/pwm.h"
#include "../APP/ws2812/ws2812.h"
#include "../APP/ws2812/rgb_display.h"
#include "usart.h"
#include "usart3.h"
#include "string.h"
#include "led.h"
#include "stdlib.h"
#include "math.h"

// 声明外部函数
extern void USART3_Init(u32 bound);
extern void u3_printf(char *fmt, ...);

// 声明外部变量，用于访问蜂鸣器状态和音量
extern u8 beep_status;
extern u16 beep_duty;
extern u16 beep_period;

// 函数前向声明
void Show_Sensor_Info(u8 temp, u8 humi, u8 lsens_value);
void Show_Help_Page(void);
void Show_BT_Info(void);
void Init_Sensor_Info_Layout(void);
void Update_Sensor_Values(u8 temp, u8 humi, u8 lsens_value);
void Exit_Clock_Mode(void);
void Display_SmartMaster_Title(void);

// 时钟相关常量
#define PI 3.1415926

// 红外遥控按键编码
#define IR_KEY1 0x00FF30CF // 按键1的编码（根据实际遥控器可能需要修改）
#define IR_KEY2 0x00FF18E7 // 按键2的编码（根据实际遥控器可能需要修改）
#define IR_KEY3 0x00FF7A85 // 按键3的编码
#define IR_KEY4 0x00FF10EF // 按键4的编码
#define IR_KEY5 0x00FF38C7 // 按键5的编码
#define IR_KEY6 0x00FF5AA5 // 按键6的编码
#define IR_KEY7 0x00FF42BD // 按键7的编码 - 增大电机功率
#define IR_KEY8 0x00FF4AB5 // 按键8的编码 - 减小电机功率
#define IR_KEY9 0x00FF52AD // 按键9的编码
#define IR_PREV 0x00FF02FD // 上一首按钮
#define IR_NEXT 0x00FFC23D // 下一首按钮

// 当前功能模式: 0=传感器模式, 1=蓝牙模式, 2=时钟模式
u8 current_mode = 0;

// 蓝牙初始化状态: 0=未初始化, 1=已初始化成功, 2=初始化失败, 3=初始化被中断
u8 bt_initialized = 0;

// 蓝牙相关变量
u8 sendmask = 0;
u8 sendcnt = 0;
u8 sendbuf[20];

// 蜂鸣器延时控制变量
u16 beep_delay = 0;		  // 延时计数器，单位：100ms
u8 beep_timer_active = 0; // 0: 未激活, 1: 已激活

// RGB爱心显示控制变量
u16 heart_display_timer = 0; // 爱心显示计时器，单位：100ms
u8 heart_display_active = 0; // 0: 未激活, 1: 已激活

// BEEP设置模式
u8 beep_setting_mode = 0;	  // 0: 非设置模式, 1: 设置模式
u16 beep_setting_seconds = 0; // 设置的延时秒数

// 直流电机控制变量
u16 motor_power = 0;	   // 当前电机功率值(0-500)
u16 motor_power_max = 499; // 最大电机功率值

// 光敏控制电机联动变量
u8 motor_auto_control = 0; // 0: 手动控制, 1: 自动控制
u16 motor_timer = 0;	   // 电机运行计时器，单位：100ms
u8 motor_running = 0;	   // 0: 电机停止, 1: 电机运行
u8 last_light_status = 0;  // 上次光照状态，0: 正常, 1: 暗

// 显示页面控制变量
u8 display_page = 0; // 0: 传感器数据页面, 1: 操作说明页面

// 超时控制变量
u16 idle_timer = 0;	   // 空闲计时器，单位：100ms
u16 idle_timeout = 50; // 5秒超时(50 * 100ms = 5000ms)

// 时钟界面控制变量
u8 clock_mode_active = 0; // 0: 非时钟模式, 1: 时钟模式激活
u8 last_sec = 0xFF;		  // 上一秒钟值，用于检测秒钟变化
u8 previous_mode = 0;	  // 记录进入时钟模式前的模式

char buf[32];

// 数字0-9对应的颜色数组（每个数字都有不同的颜色）
const u32 digit_colors[10] = {
	RGB_COLOR_RED,	   // 0 - 红色
	RGB_COLOR_GREEN,   // 1 - 绿色
	RGB_COLOR_BLUE,	   // 2 - 蓝色
	RGB_COLOR_YELLOW,  // 3 - 黄色
	RGB_COLOR_PURPLE,  // 4 - 紫色
	RGB_COLOR_CYAN,	   // 5 - 青色
	RGB_COLOR_ORANGE,  // 6 - 橙色
	RGB_COLOR_PINK,	   // 7 - 粉色
	RGB_COLOR_MAGENTA, // 8 - 品红色
	RGB_COLOR_LIME	   // 9 - 青柠色
};

// 显示SmartMaster标题的通用函数
void Display_SmartMaster_Title(void)
{
	BACK_COLOR = WHITE; // 设置背景色为白色
	FRONT_COLOR = RED;	// 设置前景色为红色
	LCD_ShowString(50, 10, 200, 24, 24, (u8 *)"SmartMaster");
	FRONT_COLOR = BLACK; // 恢复默认颜色
}

// 时钟绘制函数
void get_circle(int x, int y, int r, int col)
{
	int xc = 0;
	int yc, p;
	yc = r;
	p = 3 - (r << 1);
	while (xc <= yc)
	{
		LCD_DrawFRONT_COLOR(x + xc, y + yc, col);
		LCD_DrawFRONT_COLOR(x + xc, y - yc, col);
		LCD_DrawFRONT_COLOR(x - xc, y + yc, col);
		LCD_DrawFRONT_COLOR(x - xc, y - yc, col);
		LCD_DrawFRONT_COLOR(x + yc, y + xc, col);
		LCD_DrawFRONT_COLOR(x + yc, y - xc, col);
		LCD_DrawFRONT_COLOR(x - yc, y + xc, col);
		LCD_DrawFRONT_COLOR(x - yc, y - xc, col);
		if (p < 0)
		{
			p += (xc++ << 2) + 6;
		}
		else
			p += ((xc++ - yc--) << 2) + 10;
	}
}

void draw_circle() // 画圆
{
	// 时钟位置居中：屏幕中心(120, 240)
	get_circle(120, 240, 100, YELLOW);
	get_circle(120, 240, 99, YELLOW);
	get_circle(120, 240, 98, YELLOW);
	get_circle(120, 240, 97, YELLOW);
	get_circle(120, 240, 5, YELLOW);
}

void draw_dotline() // 画格点
{
	u8 i;
	u8 rome[][3] = {"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"}; // 表盘数字
	int x1, y1, x2, y2, x3, y3;
	for (i = 0; i < 60; i++)
	{
		x1 = (int)(120 + (sin(i * PI / 30) * 92));
		y1 = (int)(240 - (cos(i * PI / 30) * 92));
		x2 = (int)(120 + (sin(i * PI / 30) * 97));
		y2 = (int)(240 - (cos(i * PI / 30) * 97));
		FRONT_COLOR = RED;
		LCD_DrawLine(x1, y1, x2, y2);

		if (i % 5 == 0)
		{
			x1 = (int)(120 + (sin(i * PI / 30) * 85));
			y1 = (int)(240 - (cos(i * PI / 30) * 85));
			x2 = (int)(120 + (sin(i * PI / 30) * 97));
			y2 = (int)(240 - (cos(i * PI / 30) * 97));
			LCD_DrawLine(x1, y1, x2, y2);

			x3 = (int)(112 + (sin((i)*PI / 30) * 80));
			y3 = (int)(232 - (cos((i)*PI / 30) * 80));
			FRONT_COLOR = YELLOW;
			LCD_ShowString(x3, y3, tftlcd_data.width, tftlcd_data.height, 16, rome[i / 5]);
		}
	}
}

void draw_hand(int hhour, int mmin, int ssec) // 画指针
{
	int xhour, yhour, xminute, yminute, xsecond, ysecond; // 表心坐标系指针坐标
	xhour = (int)(60 * sin(hhour * PI / 6 + mmin * PI / 360 + ssec * PI / 1800));
	yhour = (int)(60 * cos(hhour * PI / 6 + mmin * PI / 360 + ssec * PI / 1800));
	xminute = (int)(90 * sin(mmin * PI / 30 + ssec * PI / 1800));
	yminute = (int)(90 * cos(mmin * PI / 30 + ssec * PI / 1800));
	xsecond = (int)(100 * sin(ssec * PI / 30));
	ysecond = (int)(100 * cos(ssec * PI / 30));

	FRONT_COLOR = RED;
	LCD_DrawLine(120 + xhour, 240 - yhour, 120 - xhour / 6, 240 + yhour / 6);
	FRONT_COLOR = BLUE;
	LCD_DrawLine(120 + xminute, 240 - yminute, 120 - xminute / 4, 240 + yminute / 4);
	FRONT_COLOR = GREEN;
	LCD_DrawLine(120 + xsecond, 240 - ysecond, 120 - xsecond / 3, 240 + ysecond / 3);
}

void draw_hand_clear(int hhour, int mmin, int ssec) // 擦指针
{
	int xhour, yhour, xminute, yminute, xsecond, ysecond; // 表心坐标系指针坐标
	xhour = (int)(60 * sin(hhour * PI / 6 + mmin * PI / 360 + ssec * PI / 1800));
	yhour = (int)(60 * cos(hhour * PI / 6 + mmin * PI / 360 + ssec * PI / 1800));
	xminute = (int)(90 * sin(mmin * PI / 30 + ssec * PI / 1800));
	yminute = (int)(90 * cos(mmin * PI / 30 + ssec * PI / 1800));
	xsecond = (int)(100 * sin(ssec * PI / 30));
	ysecond = (int)(100 * cos(ssec * PI / 30));

	FRONT_COLOR = BLACK;
	LCD_DrawLine(120 + xhour, 240 - yhour, 120 - xhour / 6, 240 + yhour / 6);
	LCD_DrawLine(120 + xminute, 240 - yminute, 120 - xminute / 4, 240 + yminute / 4);
	LCD_DrawLine(120 + xsecond, 240 - ysecond, 120 - xsecond / 3, 240 + ysecond / 3);
}

// 初始化时钟界面
void Init_Clock_Display(void)
{
	LCD_Clear(BLACK);
	FRONT_COLOR = YELLOW;
	BACK_COLOR = BLACK;

	// 显示SmartMaster标题（在黑色背景上显示红色标题）
	BACK_COLOR = BLACK; // 时钟页面背景为黑色
	FRONT_COLOR = RED;	// 设置前景色为红色
	LCD_ShowString(50, 10, 200, 24, 24, (u8 *)"SmartMaster");
	FRONT_COLOR = YELLOW; // 恢复黄色

	// 显示小标题（向下调整位置）
	LCD_ShowString(60, 70, 200, 24, 24, (u8 *)"Digital Clock");

	// 绘制表盘
	draw_circle();
	draw_dotline();

	// 显示操作提示
	LCD_ShowString(40, 420, 200, 16, 16, (u8 *)"Press any key to return");

	clock_mode_active = 1;
	last_sec = 0xFF; // 强制首次更新
	printf("Clock mode activated\r\n");
}

// 更新时钟显示
void Update_Clock_Display(void)
{
	char time_buf[16];

	// 只在秒钟变化时更新
	if (calendar.sec != last_sec)
	{
		// 擦除旧指针
		if (last_sec != 0xFF)
		{
			draw_hand_clear(calendar.hour, calendar.min, last_sec);
		}

		// 重新绘制表盘（确保表盘完整）
		draw_circle();
		draw_dotline();

		// 绘制新指针
		draw_hand(calendar.hour, calendar.min, calendar.sec);

		// 显示数字时间（位置稍微向下调整）
		sprintf(time_buf, "%.2d:%.2d:%.2d", calendar.hour, calendar.min, calendar.sec);
		LCD_Fill(80, 110, 160, 126, BLACK); // 清除旧时间显示
		FRONT_COLOR = YELLOW;
		LCD_ShowString(80, 110, 200, 16, 16, (u8 *)time_buf);

		// 显示日期
		sprintf(time_buf, "%04d-%02d-%02d", calendar.w_year, calendar.w_month, calendar.w_date);
		LCD_Fill(60, 130, 180, 146, BLACK); // 清除旧日期显示
		LCD_ShowString(60, 130, 200, 16, 16, (u8 *)time_buf);

		last_sec = calendar.sec;
	}
}

// 退出时钟模式
void Exit_Clock_Mode(void)
{
	clock_mode_active = 0;
	current_mode = previous_mode; // 恢复之前的模式
	printf("Clock mode deactivated, returning to mode %d\r\n", current_mode);

	// 根据之前的模式恢复界面
	if (previous_mode == 0)
	{
		// 返回传感器模式
		if (display_page == 0)
		{
			// 需要重新读取传感器数据
			u8 temp = 0, humi = 0, lsens_value = 0;
			DHT11_Read_Data(&temp, &humi);
			lsens_value = Lsens_Get_Val();
			Show_Sensor_Info(temp, humi, lsens_value);
		}
		else
		{
			Show_Help_Page();
		}
	}
	else if (previous_mode == 1)
	{
		// 返回蓝牙模式
		Show_BT_Info();
	}
}

// 重置空闲计时器（当有用户操作时调用）
void Reset_Idle_Timer(void)
{
	idle_timer = 0;
}

// 检查是否有任何用户操作（仅在时钟模式下使用）
u8 Check_Clock_Mode_Activity(void)
{
	u8 key_val = KEY_Scan(1); // 使用mode=1强制检测按键状态

	// 检查按键
	if (key_val != 0)
	{
		printf("Key pressed in clock mode: %d, exiting clock mode\r\n", key_val);
		Exit_Clock_Mode();
		return 1; // 返回1表示有活动且已处理
	}

	// 检查红外遥控
	if (hw_jsbz)
	{
		printf("IR signal detected in clock mode, exiting clock mode\r\n");
		hw_jsbz = 0; // 清除红外标志
		Exit_Clock_Mode();
		return 1; // 返回1表示有活动且已处理
	}

	return 0; // 无用户活动
}

// 初始化蓝牙模块，可被KEY2中断
// 返回值: 0=成功, 1=失败, 2=被中断
u8 Initialize_Bluetooth(void)
{
	// 声明变量（必须在函数开头）
	u8 retry;
	u8 key_val;

	// 如果已经初始化过且成功，直接返回
	if (bt_initialized == 1)
		return 0;

	LCD_ShowString(10, 100, 200, 16, 16, (u8 *)"Initializing HC05...");
	LCD_ShowString(10, 230, 220, 16, 16, (u8 *)"KEY2:Cancel & Return");

	// 等待蓝牙模块上电稳定
	delay_ms(500);

	// 初始化串口3
	USART3_Init(9600);

	// 尝试初始化HC05
	retry = 3; // 最多尝试3次
	while (retry--)
	{
		// 每次尝试前检查KEY2是否按下
		key_val = KEY_Scan(0);
		if (key_val == KEY2_PRESS)
		{
			LCD_ShowString(10, 100, 200, 16, 16, (u8 *)"Init Canceled!     ");
			printf("HC05 Init canceled by user\r\n");
			bt_initialized = 3; // 标记为被中断
			delay_ms(500);
			return 2; // 返回被中断状态
		}

		if (HC05_Init() == 0)
		{
			LCD_ShowString(10, 100, 200, 16, 16, (u8 *)"HC05 Init OK!      ");
			printf("HC05 OK!\r\n");
			bt_initialized = 1;
			delay_ms(500);
			return 0;
		}

		printf("HC05 Init failed, retrying...\r\n");
		LCD_ShowString(10, 100, 200, 16, 16, (u8 *)"HC05 Error!      ");
		delay_ms(300);

		// 再次检查KEY2是否按下
		key_val = KEY_Scan(0);
		if (key_val == KEY2_PRESS)
		{
			LCD_ShowString(10, 100, 200, 16, 16, (u8 *)"Init Canceled!     ");
			printf("HC05 Init canceled by user\r\n");
			bt_initialized = 3; // 标记为被中断
			delay_ms(500);
			return 2; // 返回被中断状态
		}

		LCD_ShowString(10, 100, 200, 16, 16, (u8 *)"Retrying...      ");
		delay_ms(300);

		// 再次检查KEY2是否按下
		key_val = KEY_Scan(0);
		if (key_val == KEY2_PRESS)
		{
			LCD_ShowString(10, 100, 200, 16, 16, (u8 *)"Init Canceled!     ");
			printf("HC05 Init canceled by user\r\n");
			bt_initialized = 3; // 标记为被中断
			delay_ms(500);
			return 2; // 返回被中断状态
		}
	}

	// 初始化失败
	printf("HC05 Error!\r\n");
	LCD_ShowString(10, 100, 200, 16, 16, (u8 *)"HC05 Init Failed! ");
	bt_initialized = 2;
	delay_ms(1000);
	return 1;
}

// 显示HC05模块的主从状态
void HC05_Role_Show(void)
{
	if (HC05_Get_Role() == 1)
	{
		LCD_ShowString(10, 140, 200, 16, 16, "ROLE:Master"); // 主机
	}
	else
	{
		LCD_ShowString(10, 140, 200, 16, 16, "ROLE:Slave "); // 从机
	}
}

// 显示HC05模块的连接状态
void HC05_Sta_Show(void)
{
	if (HC05_LED)
	{
		LCD_ShowString(110, 140, 120, 16, 16, "STA:Connected "); // 连接成功
	}
	else
	{
		LCD_ShowString(110, 140, 120, 16, 16, "STA:Disconnect"); // 未连接
	}
}

// 初始化传感器数据显示页面的布局
void Init_Sensor_Info_Layout(void)
{
	// 清屏
	LCD_Clear(WHITE);

	// 显示SmartMaster标题
	Display_SmartMaster_Title();

	// 显示固定的文本标签 - 紧凑布局，减少空白
	LCD_ShowString(10, 120, 60, 16, 16, (u8 *)"Temp:");
	LCD_ShowString(120, 120, 60, 16, 16, (u8 *)"Humi:");
	LCD_ShowString(10, 145, 60, 16, 16, (u8 *)"Light:");
	LCD_ShowString(10, 170, 70, 16, 16, (u8 *)"Status:");
	LCD_ShowString(10, 195, 60, 16, 16, (u8 *)"BEEP:");
	LCD_ShowString(10, 220, 80, 16, 16, (u8 *)"Volume:");
	LCD_ShowString(10, 245, 220, 16, 16, (u8 *)"Press KEY2 to BT mode");
	LCD_ShowString(10, 270, 60, 16, 16, (u8 *)"Music:");
	LCD_ShowString(10, 295, 60, 16, 16, (u8 *)"Timer:");
	LCD_ShowString(10, 320, 60, 16, 16, (u8 *)"LED2:");
	LCD_ShowString(10, 345, 100, 16, 16, (u8 *)"Motor Power:");
}

// 只更新传感器数据页面的动态数值
void Update_Sensor_Values(u8 temp, u8 humi, u8 lsens_value)
{
	char buf[32];

	// 更新日期 - 位置下移，使用24号字体
	LCD_Fill(10, 50, 230, 74, WHITE);
	sprintf(buf, "%04d-%02d-%02d", calendar.w_year, calendar.w_month, calendar.w_date);
	LCD_ShowString(60, 50, 160, 24, 24, (u8 *)buf);

	// 更新时间 - 位置下移，使用24号字体
	LCD_Fill(10, 80, 230, 104, WHITE);
	sprintf(buf, "%02d:%02d:%02d", calendar.hour, calendar.min, calendar.sec);
	LCD_ShowString(75, 80, 130, 24, 24, (u8 *)buf);

	// 更新温湿度值 - 紧凑布局
	LCD_Fill(70, 120, 110, 136, WHITE);
	sprintf(buf, "%2dC", temp);
	LCD_ShowString(70, 120, 40, 16, 16, (u8 *)buf);

	LCD_Fill(180, 120, 220, 136, WHITE);
	sprintf(buf, "%2d%%", humi);
	LCD_ShowString(180, 120, 40, 16, 16, (u8 *)buf);

	// 更新光敏值
	LCD_Fill(70, 145, 120, 161, WHITE);
	sprintf(buf, "%3d", lsens_value);
	LCD_ShowString(70, 145, 50, 16, 16, (u8 *)buf);

	// 更新状态指示
	LCD_Fill(80, 170, 220, 186, WHITE);
	if (temp > 28)
	{
		LCD_ShowString(80, 170, 140, 16, 16, (u8 *)"Hot");
	}
	else if (temp < 15)
	{
		LCD_ShowString(80, 170, 140, 16, 16, (u8 *)"Cold");
	}
	else
	{
		LCD_ShowString(80, 170, 140, 16, 16, (u8 *)"Normal");
	}

	// 更新蜂鸣器状态
	LCD_Fill(70, 195, 120, 211, WHITE);
	sprintf(buf, "%s", beep_status ? "ON" : "OFF");
	LCD_ShowString(70, 195, 50, 16, 16, (u8 *)buf);

	// 更新蜂鸣器音量百分比
	LCD_Fill(90, 220, 150, 236, WHITE);
	sprintf(buf, "%d%%", beep_duty * 100 / beep_period);
	LCD_ShowString(90, 220, 60, 16, 16, (u8 *)buf);

	// 更新BEEP状态或倒计时
	LCD_Fill(70, 270, 220, 286, WHITE);
	if (beep_setting_mode)
	{
		LCD_ShowString(70, 270, 150, 16, 16, (u8 *)"SETTING");

		// 更新当前设置的延时时间
		LCD_Fill(70, 295, 220, 311, WHITE);
		sprintf(buf, "%d seconds", beep_setting_seconds);
		LCD_ShowString(70, 295, 150, 16, 16, (u8 *)buf);
	}
	else if (beep_timer_active)
	{
		sprintf(buf, "COUNTDOWN %d.%ds", beep_delay / 10, beep_delay % 10);
		LCD_ShowString(70, 270, 150, 16, 16, (u8 *)buf);
	}
	else
	{
		sprintf(buf, "%s", beep_status ? "ON" : "OFF");
		LCD_ShowString(70, 270, 150, 16, 16, (u8 *)buf);
	}

	// 更新LED2状态
	LCD_Fill(70, 320, 120, 336, WHITE);
	sprintf(buf, "%s", LED2 ? "OFF" : "ON");
	LCD_ShowString(70, 320, 50, 16, 16, (u8 *)buf);

	// 更新电机功率
	LCD_Fill(110, 345, 170, 361, WHITE);
	sprintf(buf, "%d", motor_power);
	LCD_ShowString(110, 345, 60, 16, 16, (u8 *)buf);

	// 更新光敏控制状态
	if (motor_auto_control)
	{
		LCD_Fill(140, 345, 220, 361, WHITE);
		if (motor_running)
		{
			sprintf(buf, "(AUTO %d.%ds)", motor_timer / 10, motor_timer % 10);
			LCD_ShowString(140, 345, 80, 16, 16, (u8 *)buf);
		}
		else
		{
			LCD_ShowString(140, 345, 80, 16, 16, (u8 *)"(AUTO)");
		}
	}
	else if (motor_running)
	{
		LCD_Fill(140, 345, 220, 361, WHITE);
		LCD_ShowString(140, 345, 80, 16, 16, (u8 *)"(MANUAL ON)");
	}
	else
	{
		LCD_Fill(140, 345, 220, 361, WHITE);
		LCD_ShowString(140, 345, 80, 16, 16, (u8 *)"(OFF)");
	}
}

// 更新修改Show_Sensor_Info函数，使其调用上面两个函数
void Show_Sensor_Info(u8 temp, u8 humi, u8 lsens_value)
{
	// 初始化布局
	Init_Sensor_Info_Layout();

	// 更新数值
	Update_Sensor_Values(temp, humi, lsens_value);
}

// 显示操作说明页面
void Show_Help_Page(void)
{
	// 清屏
	LCD_Clear(WHITE);

	// 显示SmartMaster标题
	Display_SmartMaster_Title();

	// 显示操作指南小标题
	LCD_ShowString(70, 50, 150, 16, 16, (u8 *)"Operation Guide");

	// 显示按键操作说明 - 所有位置下移40个像素
	LCD_ShowString(10, 80, 220, 16, 16, (u8 *)"KEY0: Toggle Auto Control");
	LCD_ShowString(10, 100, 220, 16, 16, (u8 *)"KEY0 (Long): RGB LED Demo");
	LCD_ShowString(10, 120, 220, 16, 16, (u8 *)"KEY1: Music Play/Pause");
	LCD_ShowString(10, 140, 220, 16, 16, (u8 *)"KEY2: Switch Mode");
	LCD_ShowString(10, 160, 220, 16, 16, (u8 *)"KEY_UP: Switch Page");

	// 显示红外遥控操作说明
	LCD_ShowString(10, 190, 220, 16, 16, (u8 *)"IR1: RGB Test Pattern");
	LCD_ShowString(10, 210, 220, 16, 16, (u8 *)"IR3: Enter Timer Setup Mode");
	LCD_ShowString(10, 230, 220, 16, 16, (u8 *)"IR4: Increase Timer (+1s)");
	LCD_ShowString(10, 250, 220, 16, 16, (u8 *)"IR5: Decrease Timer (-1s)");
	LCD_ShowString(10, 270, 220, 16, 16, (u8 *)"IR6: Start Countdown (5s)");
	LCD_ShowString(10, 290, 220, 16, 16, (u8 *)"IR7/8: Adjust Motor Power");
	LCD_ShowString(10, 310, 220, 16, 16, (u8 *)"IR9: Emergency Stop");
	LCD_ShowString(10, 330, 220, 16, 16, (u8 *)"IR_PREV/NEXT: Music Control");

	// 显示功能说明
	LCD_ShowString(10, 340, 220, 16, 16, (u8 *)"Auto Control: Light < 20");
	LCD_ShowString(10, 360, 220, 16, 16, (u8 *)"RGB Countdown: Shows colorful");
	LCD_ShowString(10, 380, 220, 16, 16, (u8 *)"numbers on 5x5 LED matrix");
	LCD_ShowString(10, 400, 220, 16, 16, (u8 *)"Timer end: Lanhua Grass song");

	// 显示页面切换提示
	LCD_ShowString(10, 430, 220, 16, 16, (u8 *)"KEY_UP: Back to Data Page");
}

// 显示蓝牙界面
void Show_BT_Info(void)
{
	char buf[32]; // 将变量声明移到函数开头

	LCD_Clear(WHITE);

	// 显示SmartMaster标题
	Display_SmartMaster_Title();

	// 显示蓝牙相关信息 - 所有位置下移40个像素
	FRONT_COLOR = RED;
	LCD_ShowString(10, 50, 240, 16, 16, (u8 *)"PRECHIN");
	LCD_ShowString(10, 70, 240, 16, 16, (u8 *)"www.prechin.com");
	LCD_ShowString(10, 90, 240, 16, 16, (u8 *)"BT05 BlueTooth Test");
	LCD_ShowString(10, 130, 210, 16, 16, (u8 *)"KEY_UP:ROLE   KEY1:SEND/STOP");
	LCD_ShowString(10, 150, 200, 16, 16, (u8 *)"HC05 Standby!");
	LCD_ShowString(10, 200, 200, 16, 16, (u8 *)"Send:");
	LCD_ShowString(10, 220, 200, 16, 16, (u8 *)"Receive:");
	LCD_ShowString(10, 270, 220, 16, 16, (u8 *)"Press KEY2 to Sensor mode");

	// 如果音乐定时器已激活，显示倒计时
	if (beep_timer_active)
	{
		sprintf(buf, "Lanhua Grass+ in %d.%ds", beep_delay / 10, beep_delay % 10);
		LCD_ShowString(10, 300, 220, 16, 16, (u8 *)buf);
	}

	// 显示LED2状态
	LCD_Fill(10, 330, 220, 346, WHITE);
	sprintf(buf, "LED2: %s", LED2 ? "OFF" : "ON");
	LCD_ShowString(10, 330, 220, 16, 16, (u8 *)buf);

	FRONT_COLOR = BLUE;
	HC05_Role_Show();
	HC05_Sta_Show();
}

// 处理接收到的蓝牙命令
void Process_BT_Command(u8 *buf, u16 len)
{
	// 控制LED2
	if (strcmp((const char *)buf, "+LED2 ON\r\n") == 0)
	{
		LED2 = 0; // 打开LED2
	}
	else if (strcmp((const char *)buf, "+LED2 OFF\r\n") == 0)
	{
		LED2 = 1; // 关闭LED2
	}

	// 直接控制蜂鸣器
	else if (strcmp((const char *)buf, "+BEEP ON\r\n") == 0)
	{
		BEEP_On();
	}
	else if (strcmp((const char *)buf, "+BEEP OFF\r\n") == 0)
	{
		BEEP_Off();
	}

	// 延时控制蜂鸣器
	else if (strncmp((const char *)buf, "+BEEP ", 6) == 0)
	{
		// 解析延时时间（秒）
		u8 seconds = atoi((const char *)&buf[6]);
		if (seconds > 0)
		{
			// 设置延时
			beep_delay = seconds * 10; // 转换为100ms单位
			beep_timer_active = 1;	   // 激活定时器

			// 反馈信息
			if (bt_initialized == 1)
			{
				u3_printf("Will play Lanhua Grass (optimized) in %d seconds\r\n", seconds);
			}
			printf("Will play Lanhua Grass (optimized) in %d seconds\r\n", seconds);

			// 更新显示
			if (current_mode == 0)
			{
				char buf[32];
				LCD_Fill(70, 270, 220, 286, WHITE);
				sprintf(buf, "Lanhua Grass+ in %d.%ds", beep_delay / 10, beep_delay % 10);
				LCD_ShowString(70, 270, 150, 16, 16, (u8 *)buf);
			}
			else
			{
				// 在蓝牙模式下，刷新整个界面来显示倒计时
				Show_BT_Info();
			}
		}
	}
}

// 处理红外遥控器输入
void Process_IR_Command(void)
{
	if (hw_jsbz)
	{										   // 有红外数据接收到
		Reset_Idle_Timer();					   // 重置空闲计时器
		printf("IR Code: 0x%08X\r\n", hw_jsm); // 打印接收到的红外代码

		// 根据红外代码控制电机
		if (hw_jsm == IR_KEY1)
		{ // 按键1 - 增大电机功率
			if (motor_power < motor_power_max)
			{
				motor_power += 50;
				if (motor_power > motor_power_max)
					motor_power = motor_power_max;
				TIM_SetCompare2(TIM3, motor_power_max - motor_power);
				printf("Motor power increased to %d\r\n", motor_power);
			}
		}
		else if (hw_jsm == IR_KEY2)
		{ // 按键2 - 减小电机功率
			if (motor_power > 0)
			{
				if (motor_power > 50)
					motor_power -= 50;
				else
					motor_power = 0;
				TIM_SetCompare2(TIM3, motor_power_max - motor_power);
				printf("Motor power decreased to %d\r\n", motor_power);
			}
		}
		else if (hw_jsm == IR_KEY3)
		{ // 按键3 - 切换光敏自动控制模式
			motor_auto_control = !motor_auto_control;
			if (motor_auto_control)
			{
				printf("Motor auto control enabled\r\n");
				// 切换到自动模式时，停止手动控制
				motor_running = 0;
				TIM_SetCompare2(TIM3, motor_power_max);
			}
			else
			{
				printf("Motor manual control enabled\r\n");
				// 切换到手动模式时，停止自动控制
				motor_running = 0;
				motor_timer = 0;
				TIM_SetCompare2(TIM3, motor_power_max);
			}
		}
		else if (hw_jsm == IR_KEY4)
		{ // 按键4 - 手动控制电机开关
			if (!motor_auto_control)
			{
				motor_running = !motor_running;
				if (motor_running)
				{
					TIM_SetCompare2(TIM3, motor_power_max - motor_power);
					printf("Motor turned ON manually\r\n");
				}
				else
				{
					TIM_SetCompare2(TIM3, motor_power_max);
					printf("Motor turned OFF manually\r\n");
				}
			}
		}

		// 根据红外代码控制LED2
		if (hw_jsm == IR_KEY1)
		{ // 按键1 - 改为RGB测试
			printf("Testing RGB LED module...\r\n");
			// 显示测试图案（十字）
			RGB_ShowTest(RGB_COLOR_RED);
			printf("RGB test pattern displayed\r\n");
		}
		else if (hw_jsm == IR_KEY2)
		{			  // 按键2
			LED2 = 1; // 关LED2
			printf("LED2 OFF\r\n");
		}
		else if (hw_jsm == IR_KEY3)
		{ // 按键3 - 进入音乐定时器设置模式
			beep_setting_mode = 1;
			beep_setting_seconds = 0;
			LCD_ShowString(70, 320, 150, 16, 16, (u8 *)"Music Timer Setting Mode");
			printf("Enter music timer setting mode\r\n");
		}
		else if (hw_jsm == IR_KEY4)
		{ // 按键4 - 增加延时时间
			if (beep_setting_mode)
			{
				beep_setting_seconds += 1;
				if (beep_setting_seconds > 600)
					beep_setting_seconds = 600; // 最大600秒

				sprintf(buf, "Timer: %ds", beep_setting_seconds);
				LCD_ShowString(70, 320, 150, 16, 16, (u8 *)buf);
				printf("Music timer set to %d seconds\r\n", beep_setting_seconds);
			}
		}
		else if (hw_jsm == IR_KEY5)
		{ // 按键5 - 减少延时时间
			if (beep_setting_mode && beep_setting_seconds > 0)
			{
				beep_setting_seconds -= 1;
				if (beep_setting_seconds < 0)
					beep_setting_seconds = 0;
				sprintf(buf, "Timer: %ds", beep_setting_seconds);
				LCD_ShowString(70, 320, 150, 16, 16, (u8 *)buf);
				printf("Music timer set to %d seconds\r\n", beep_setting_seconds);
			}
		}
		else if (hw_jsm == IR_KEY6)
		{ // 按键6 - 开始倒计时
			if (beep_setting_mode && beep_setting_seconds > 0)
			{
				// 如果在设置模式且设置了时间，使用设置的时间
				beep_delay = beep_setting_seconds * 10; // 转换为100ms单位
				beep_timer_active = 1;
				beep_setting_mode = 0;
				LCD_ShowString(70, 320, 150, 16, 16, (u8 *)"Music Timer Started!");
				printf("Music timer started for %d seconds\r\n", beep_setting_seconds);

				// 初始化RGB LED并显示倒计时起始数字（五彩斑斓效果）
				if (beep_setting_seconds <= 9)
				{
					RGB_ShowCharNum_Debug(beep_setting_seconds, 0); // 颜色参数已不使用
				}
				else
				{
					// 如果超过9秒，显示个位数
					u8 digit = beep_setting_seconds % 10;
					RGB_ShowCharNum_Debug(digit, 0); // 颜色参数已不使用
				}
			}
			else
			{
				// 如果不在设置模式，直接启动5秒倒计时
				beep_delay = 50; // 5秒 = 50 * 100ms
				beep_timer_active = 1;
				beep_setting_mode = 0;
				printf("Quick countdown started for 5 seconds\r\n");

				// 显示起始数字5
				RGB_ShowCharNum_Debug(5, 0);

				// 更新LCD显示
				if (current_mode == 0 && display_page == 0)
				{
					LCD_Fill(70, 270, 220, 286, WHITE);
					LCD_ShowString(70, 270, 150, 16, 16, (u8 *)"COUNTDOWN 5.0s");
				}
			}
		}
		else if (hw_jsm == IR_KEY7)
		{ // 按键7 - 增大电机功率
			if (motor_power < motor_power_max)
			{
				motor_power += 50;
				if (motor_power > motor_power_max)
					motor_power = motor_power_max;
				TIM_SetCompare2(TIM3, motor_power_max - motor_power);
				printf("Motor power increased to %d\r\n", motor_power);
			}
		}
		else if (hw_jsm == IR_KEY8)
		{ // 按键8 - 减小电机功率
			if (motor_power > 0)
			{
				if (motor_power > 50)
					motor_power -= 50;
				else
					motor_power = 0;
				TIM_SetCompare2(TIM3, motor_power_max - motor_power);
				printf("Motor power decreased to %d\r\n", motor_power);
			}
		}
		else if (hw_jsm == IR_KEY9)
		{ // 按键9 - 紧急停止电机
			motor_running = 0;
			motor_auto_control = 0;
			motor_timer = 0;
			TIM_SetCompare2(TIM3, motor_power_max);
			printf("Motor emergency stop\r\n");
		}
		else if (hw_jsm == IR_PREV)
		{ // 上一首按钮 - 切换到上一首歌
			Music_Prev_Song();
			printf("Previous song selected\r\n");
		}
		else if (hw_jsm == IR_NEXT)
		{ // 下一首按钮 - 切换到下一首歌
			Music_Next_Song();
			printf("Next song selected\r\n");
		}

		hw_jsbz = 0; // 清除接收标志
	}
}

// RGB LED Demo Function - Displays the sequence of characters
void RGB_LED_Demo(void)
{
	u8 key_val;
	u8 demo_running = 1;
	u8 current_char = 0;
	const u8 total_chars = 10; // 9 letters + 1 heart
	u16 i;

	printf("Starting RGB LED Demo...\r\n");

	// Initialize RGB LED module
	RGB_LED_Init();

	printf("RGB LED initialized. Displaying character sequence...\r\n");
	printf("Press any key to exit the demo.\r\n");

	// Display each character and heart in sequence, with different colors
	while (demo_running)
	{
		// Display current character with appropriate color
		RGB_ShowCustomChar(custom_char_patterns[current_char], display_colors[current_char]);

		// Move to next character
		current_char = (current_char + 1) % total_chars;

		// Check for key press to exit demo
		for (i = 0; i < 8; i++) // Check for key press for 800ms (8 * 100ms)
		{
			delay_ms(100);
			key_val = KEY_Scan(0);
			if (key_val != 0) // Any key pressed
			{
				demo_running = 0;
				break;
			}
		}
	}

	// Clear RGB LEDs before exiting
	RGB_LED_Clear();
	printf("RGB LED Demo stopped.\r\n");
}

int main()
{
	u8 temp = 0, humi = 0;
	u8 lsens_value = 0;
	u8 key_val = 0;
	u8 t = 0;
	u8 reclen = 0;
	u8 bt_init_result = 0;
	char buf[32]; // 显示缓冲区
	u16 press_time;
	u8 remaining_seconds; // 倒计时剩余秒数

	// 系统初始化
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 中断优先级分组 分2组

	// 外设初始化
	USART1_Init(115200);
	TFTLCD_Init();
	LCD_Clear(WHITE);
	FRONT_COLOR = BLACK;

	// 传感器初始化
	DHT11_Init();
	Lsens_Init();
	RTC_Init();
	RTC_Get(); // 初始化calendar结构体

	// 按键、LED和蜂鸣器初始化
	KEY_Init();
	LED_Init();
	BEEP_Init();

	// 音乐播放模块初始化
	Music_Init();

	// 电机PWM初始化 - 使用原始示例中的方法
	TIM3_CH2_PWM_Init(motor_power_max, 72 - 1);			  // 频率约2KHz
	TIM_SetCompare2(TIM3, motor_power_max - motor_power); // 初始功率为0，反转PWM逻辑

	// 红外接收初始化
	Hwjs_Init();

	// RGB LED初始化
	RGB_LED_Init();
	printf("RGB LED module initialized\r\n");

	// 显示启动信息
	LCD_ShowString(60, 60, 200, 24, 24, (u8 *)"Starting...");
	delay_ms(1000); // 延长显示时间，确保用户能看到

	// 清屏并显示初始界面（传感器模式）
	LCD_Clear(WHITE);

	// 首次读取传感器数据，确保有初始值
	DHT11_Read_Data(&temp, &humi);
	lsens_value = Lsens_Get_Val();

	// 初始化光敏控制变量
	motor_auto_control = 0;							// 默认手动控制
	motor_running = 0;								// 默认电机停止
	motor_timer = 0;								// 计时器清零
	last_light_status = (lsens_value < 20) ? 1 : 0; // 根据初始光照状态设置

	// 显示初始数据 - 第一次显示时，初始化整个布局
	Init_Sensor_Info_Layout();
	Update_Sensor_Values(temp, humi, lsens_value);

	while (1)
	{
		// 在时钟模式下的处理
		if (clock_mode_active)
		{
			// 检查用户活动
			if (Check_Clock_Mode_Activity())
			{
				// 如果有活动，则已退出时钟模式，需要额外延时避免重复触发
				delay_ms(300);
				t += 3; // 补偿延时时间
				continue;
			}

			// 更新RTC时间
			RTC_Get();

			// 更新时钟显示
			Update_Clock_Display();

			// 延时100ms后继续下一轮循环
			delay_ms(100);
			t++;
			continue;
		}

		// 在非时钟模式下增加空闲计时器
		idle_timer++;

		// 检查是否超时（5秒无操作）
		if (idle_timer >= idle_timeout)
		{
			printf("5 seconds idle timeout, entering clock mode from mode %d\r\n", current_mode);
			previous_mode = current_mode; // 记录当前模式
			current_mode = 2;			  // 切换到时钟模式
			Init_Clock_Display();
			Reset_Idle_Timer();

			// 延时100ms后继续下一轮循环
			delay_ms(100);
			t++;
			continue;
		}

		// 处理蜂鸣器定时器
		if (beep_timer_active)
		{
			if (beep_delay > 0)
			{
				beep_delay--;
				if (beep_delay == 0)
				{
					// 延时结束，播放新格式的兰花草歌曲
					Music_Play_Lanhua_Grass_New(); // 播放优化版兰花草
					beep_timer_active = 0;

					// 显示五彩斑斓的爱心并启动爱心显示定时器
					RGB_ShowHeart(0);		  // 颜色参数已不使用，自动五彩斑斓
					heart_display_timer = 30; // 显示3秒 (30 * 100ms)
					heart_display_active = 1;

					// 更新显示
					if (current_mode == 0 && display_page == 0)
					{
						// 更新音乐状态显示
						LCD_Fill(70, 270, 220, 286, WHITE);
						LCD_ShowString(70, 270, 150, 16, 16, (u8 *)"Playing Lanhua Grass+");
					}

					// 反馈信息
					if (bt_initialized == 1)
					{
						u3_printf("Playing Lanhua Grass (optimized)!\r\n");
					}
					printf("Playing Lanhua Grass (optimized)!\r\n");
				}
				else if (beep_delay % 10 == 0 && !heart_display_active)
				{ // 每秒更新一次显示，但不与爱心显示冲突
					// 更新倒计时显示
					if (current_mode == 0 && display_page == 0)
					{
						char buf[32];
						LCD_Fill(70, 270, 220, 286, WHITE);
						sprintf(buf, "COUNTDOWN %d.%ds", beep_delay / 10, beep_delay % 10);
						LCD_ShowString(70, 270, 150, 16, 16, (u8 *)buf);
					}

					// 在RGB模块上显示倒计时数字（五彩斑斓效果）
					remaining_seconds = beep_delay / 10;
					printf("Countdown: %d seconds remaining\r\n", remaining_seconds);
					if (remaining_seconds <= 9)
					{
						RGB_ShowCharNum_Debug(remaining_seconds, 0); // 颜色参数已不使用
					}
					else
					{
						// 如果超过9秒，显示个位数
						u8 digit = remaining_seconds % 10;
						RGB_ShowCharNum_Debug(digit, 0); // 颜色参数已不使用
					}
				}
			}
		}

		// 处理RGB爱心显示定时器
		if (heart_display_active)
		{
			if (heart_display_timer > 0)
			{
				heart_display_timer--;
				if (heart_display_timer == 0)
				{
					// 爱心显示时间结束，清除RGB显示
					RGB_LED_Clear();
					heart_display_active = 0;
					printf("Heart display finished\r\n");
				}
			}
		}

		// 处理红外遥控器输入
		Process_IR_Command();

		// 更新音乐播放
		Music_Update();

		// 更新新格式音乐播放
		Music_Update_New_Format();

		// 光敏控制电机联动逻辑
		if (motor_auto_control)
		{
			// 检查光照状态变化
			if (lsens_value < 20) // 光照低于20，环境较暗
			{
				if (!last_light_status) // 之前是亮的，现在变暗了
				{
					// 启动电机
					motor_running = 1;
					motor_timer = 0;
					TIM_SetCompare2(TIM3, motor_power_max - motor_power);
					printf("Light dim detected, motor started\r\n");
				}
				last_light_status = 1; // 标记当前状态为暗
			}
			else // 光照高于等于20，环境较亮
			{
				if (last_light_status) // 之前是暗的，现在变亮了
				{
					// 停止电机
					motor_running = 0;
					motor_timer = 0;
					TIM_SetCompare2(TIM3, motor_power_max);
					printf("Light bright detected, motor stopped\r\n");
				}
				last_light_status = 0; // 标记当前状态为亮
			}

			// 电机运行计时器
			if (motor_running)
			{
				motor_timer++;
				if (motor_timer >= 50) // 5秒 = 50 * 100ms
				{
					// 5秒后自动关闭电机
					motor_running = 0;
					motor_timer = 0;
					TIM_SetCompare2(TIM3, motor_power_max);
					printf("Motor auto stop after 5 seconds\r\n");
				}
			}
		}

		// 扫描按键
		key_val = KEY_Scan(0);

		// 模式切换（KEY2按键）
		if (key_val == KEY2_PRESS)
		{
			Reset_Idle_Timer(); // 重置空闲计时器

			if (current_mode == 0)
			{
				// 正在从传感器模式切换到蓝牙模式

				// 尝试初始化蓝牙模块(如果还没初始化)
				bt_init_result = Initialize_Bluetooth();

				if (bt_init_result == 0)
				{
					// 初始化成功，切换到蓝牙模式
					current_mode = 1;
					Show_BT_Info();
				}
				else if (bt_init_result == 2)
				{
					// 初始化被用户中断，保持在传感器模式
					if (display_page == 0)
					{
						Show_Sensor_Info(temp, humi, lsens_value);
					}
					else
					{
						Show_Help_Page();
					}
				}
				else
				{
					// 初始化失败，保持在传感器模式
					LCD_ShowString(10, 230, 220, 16, 16, (u8 *)"BT Init Failed!         ");
					delay_ms(1000);
					LCD_ShowString(10, 230, 220, 16, 16, (u8 *)"Press KEY2 to try again");
				}
			}
			else if (current_mode == 1)
			{
				// 从蓝牙模式切换到传感器模式
				current_mode = 0;
				if (display_page == 0)
				{
					Show_Sensor_Info(temp, humi, lsens_value);
				}
				else
				{
					Show_Help_Page();
				}
			}
		}

		if (current_mode == 0)
		{ // 传感器模式
			if (key_val == KEY_UP_PRESS)
			{
				Reset_Idle_Timer(); // 重置空闲计时器
				// KEY_UP: 切换显示页面
				display_page = !display_page;
				if (display_page == 0)
				{
					// 切换到传感器数据页面 - 重新初始化整个布局
					Init_Sensor_Info_Layout();
					Update_Sensor_Values(temp, humi, lsens_value);
				}
				else
				{
					// 切换到操作说明页面
					Show_Help_Page();
				}
			}
			else if (key_val == KEY0_PRESS)
			{
				Reset_Idle_Timer(); // 重置空闲计时器
				// Check if KEY0 was held for a long press (>1 second)
				delay_ms(100); // Debounce delay
				if (KEY0 == 0) // Still pressed
				{
					press_time = 0;
					while (KEY0 == 0 && press_time < 10) // Check for up to 1 second
					{
						delay_ms(100);
						press_time++;
					}

					if (press_time >= 10) // Long press - start RGB LED demo
					{
						LCD_Clear(WHITE);
						LCD_ShowString(50, 200, 180, 24, 24, (u8 *)"Starting RGB Demo");
						delay_ms(500);
						RGB_LED_Demo(); // This will enter a loop until a key is pressed
						// After demo, refresh the screen
						if (display_page == 0)
						{
							Show_Sensor_Info(temp, humi, lsens_value);
						}
						else
						{
							Show_Help_Page();
						}
					}
					else // Short press
					{
						// KEY0: 切换光敏自动控制模式
						motor_auto_control = !motor_auto_control;
						if (motor_auto_control)
						{
							printf("Motor auto control enabled\r\n");
							motor_running = 0;
							TIM_SetCompare2(TIM3, motor_power_max);
						}
						else
						{
							printf("Motor manual control enabled\r\n");
							motor_running = 0;
							motor_timer = 0;
							TIM_SetCompare2(TIM3, motor_power_max);
						}
					}
				}
				else // It was a very short press, treat as normal toggle
				{
					motor_auto_control = !motor_auto_control;
					if (motor_auto_control)
					{
						printf("Motor auto control enabled\r\n");
						motor_running = 0;
						TIM_SetCompare2(TIM3, motor_power_max);
					}
					else
					{
						printf("Motor manual control enabled\r\n");
						motor_running = 0;
						motor_timer = 0;
						TIM_SetCompare2(TIM3, motor_power_max);
					}
				}

				// 立即更新显示（只在数据页面时更新）
				if (display_page == 0)
				{
					Update_Sensor_Values(temp, humi, lsens_value);
				}
			}
			else if (key_val == KEY1_PRESS)
			{
				Reset_Idle_Timer(); // 重置空闲计时器
				// KEY1: 切换音乐播放/停止
				if (music_status == MUSIC_STOP)
				{
					Music_Play_Song(current_song);
				}
				else if (music_status == MUSIC_PLAY)
				{
					Music_Pause();
				}
				else if (music_status == MUSIC_PAUSE)
				{
					Music_Resume();
				}
				// 立即更新显示（只在数据页面时更新）
				if (display_page == 0)
				{
					Update_Sensor_Values(temp, humi, lsens_value);
				}
			}

			RTC_Get();
			// 读取DHT11温湿度数据
			if (DHT11_Read_Data(&temp, &humi) == 0)
			{
				// DHT11读取成功，只在数据页面时更新显示
				if (display_page == 0)
				{
					Update_Sensor_Values(temp, humi, lsens_value);
				}
			}

			// 读取光敏传感器数据
			lsens_value = Lsens_Get_Val();

			// 更新显示（减少刷新频率，只在数据页面时更新数值，不重新绘制整个屏幕）
			if (t % 20 == 0 && display_page == 0) // 每2秒刷新一次（20 * 100ms = 2000ms）
			{
				Update_Sensor_Values(temp, humi, lsens_value);
			}
		}
		else if (current_mode == 1)
		{ // 蓝牙模式
			if (key_val == KEY_UP_PRESS)
			{
				Reset_Idle_Timer(); // 重置空闲计时器
				// 切换模块主从设置
				key_val = HC05_Get_Role();
				if (key_val != 0XFF)
				{
					key_val = !key_val; // 状态取反
					if (key_val == 0)
						HC05_Set_Cmd("AT+ROLE=0");
					else
						HC05_Set_Cmd("AT+ROLE=1");
					HC05_Role_Show();
					HC05_Set_Cmd("AT+RESET"); // 复位HC05模块
					delay_ms(200);
				}
			}
			else if (key_val == KEY1_PRESS)
			{
				Reset_Idle_Timer();	  // 重置空闲计时器
				sendmask = !sendmask; // 发送/停止发送
				if (sendmask == 0)
					LCD_Fill(10 + 40, 160, 240, 160 + 16, WHITE); // 清除显示
			}

			if (t % 50 == 0)
			{
				if (sendmask)
				{ // 定时发送
					sprintf((char *)sendbuf, "PREHICN HC05 %d\r\n", sendcnt);
					LCD_ShowString(10 + 40, 160, 200, 16, 16, sendbuf); // 显示发送数据
					printf("%s\r\n", sendbuf);
					u3_printf("PREHICN HC05 %d\r\n", sendcnt); // 发送到蓝牙模块
					sendcnt++;
					if (sendcnt > 99)
						sendcnt = 0;
				}
				HC05_Sta_Show();
				LED1 = !LED1;
			}

			if (USART3_RX_STA & 0X8000)
			{										// 接收到一次数据了
				Reset_Idle_Timer();					// 重置空闲计时器
				LCD_Fill(10, 200, 240, 320, WHITE); // 清除显示
				reclen = USART3_RX_STA & 0X7FFF;	// 得到数据长度
				USART3_RX_BUF[reclen] = '\0';		// 加入结束符
				printf("reclen=%d\r\n", reclen);
				printf("USART3_RX_BUF=%s\r\n", USART3_RX_BUF);

				// 处理接收到的命令
				Process_BT_Command(USART3_RX_BUF, reclen);

				LCD_ShowString(10, 200, 209, 119, 16, USART3_RX_BUF); // 显示接收到的数据
				USART3_RX_STA = 0;
			}
		}

		// 延时100ms后再次读取
		delay_ms(100);
		t++;
	}
}