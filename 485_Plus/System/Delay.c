#include "stm32f10x.h"

void Delay_us(uint32_t xus) // 微秒级延时
{
	SysTick->LOAD = 72 * xus;	// 设置定时器重装值
	SysTick->VAL = 0x00;		// 清空当前计数值
	SysTick->CTRL = 0x00000005; // 设置时钟源为HCLK，启动定时器
	while (!(SysTick->CTRL & 0x00010000))
		;						// 等待计数到0
	SysTick->CTRL = 0x00000004; // 关闭定时器
}

void Delay_ms(uint32_t xms) // 毫秒级延时
{
	while (xms--)
	{
		Delay_us(1000);
	}
}

void Delay_s(uint32_t xs) // 秒级延时
{
	while (xs--)
	{
		Delay_ms(1000);
	}
}
