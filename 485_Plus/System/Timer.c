#include "stm32f10x.h"

// 引用在Modbus.c中定义的全局标志位
extern volatile uint8_t modbus_frame_received;

// 初始化TIM2作为Modbus RTU的T3.5帧间隔定时器
void Timer_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 5000 - 1;   // 计数值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;  // 预分频值 (72MHz / 72 = 1MHz)
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure); // 溢出时间 = 5000 / 1MHz = 5ms

	// 配置为单脉冲模式，触发一次后自动停止
	TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Single);

	// 清除并使能更新中断
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	// 配置TIM2的NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStructure);
}

// TIM2中断服务程序
// 当此中断触发时，表明总线已静默超过设定时间，一帧数据已接收完成
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		modbus_frame_received = 1; // 设置全局标志位，通知主循环处理数据
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // 清除中断标志
	}
}
