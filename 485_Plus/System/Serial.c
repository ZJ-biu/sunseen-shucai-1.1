#include "stm32f10x.h"

/**
 * @brief 初始化USART1作为数据发送端口。
 * @note  此驱动经过精简，仅包含发送功能，移除了所有接收和中断逻辑。
 * 硬件连接: PA9 (TX)。
 */
void Serial_Init(void)
{
	// 1. 使能USART1和GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	// 2. 初始化GPIO引脚
	GPIO_InitTypeDef GPIO_InitStructure;
	// 配置 PA9 (USART1_TX) 为复用推挽输出模式
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// 3. 初始化USART1参数
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;                               // 波特率: 115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 数据位: 8
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                        // 停止位: 1
	USART_InitStructure.USART_Parity = USART_Parity_No;                           // 无校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Tx;                               // 模式: 仅发送
	USART_Init(USART1, &USART_InitStructure);

	// 4. 使能USART1外设
	USART_Cmd(USART1, ENABLE);
}

/**
 * @brief 通过USART1发送一个字节 (阻塞方式)。
 * @param Byte 要发送的字节。
 */
void Serial_SendByte(uint8_t Byte)
{
	// 将字节数据写入USART1的数据寄存器 (DR)
	USART_SendData(USART1, Byte);
	// 等待发送数据寄存器为空 (TXE) 标志位置位
	// 这表示数据已被转移至发送移位寄存器，可以安全地发送下一个字节
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

/**
 * @brief 通过USART1发送一个以'\0'结尾的字符串。
 * @param String 要发送的字符串的指针。
 */
void Serial_SendString(char *String)
{
	// 遍历字符串，直到遇到字符串结束符'\0'
	for (uint8_t i = 0; String[i] != '\0'; i++)
	{
		// 调用单字节发送函数来发送当前字符
		Serial_SendByte(String[i]);
	}
}
