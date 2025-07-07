#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "lsens.h"


/*******************************************************************************
* �� �� ��         : main
* ��������		   : ������
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
int main()
{
	u8 i=0;
	u8 lsens_value=0;
	
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //�ж����ȼ����� ��2��
	LED_Init();
	USART1_Init(115200);
	Lsens_Init();
	
	while(1)
	{
		i++;
		if(i%20==0)
		{
			LED1=!LED1;
		}
		
		if(i%50==0)
		{
			lsens_value=Lsens_Get_Val();
			printf("����ǿ�ȣ�%d\r\n",lsens_value);
		}
		delay_ms(10);	
	}
}
