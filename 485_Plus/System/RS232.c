#include "stm32f10x.h"
#include <stdarg.h>
#include <stdio.h>
#include "RS232.h"

// 初始化USART2用于RS232通信
void RS232_Init(void)
{
    // 1. 使能GPIOA和USART2的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // 2. 初始化GPIO引脚
    GPIO_InitTypeDef GPIO_InitStructure;
    // 配置 PA2 (USART2_TX) 为复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 PA3 (USART2_RX) 为浮空输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. 初始化USART2参数 (115200, 8N1)
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);

    // 4. 使能USART2
    USART_Cmd(USART2, ENABLE);
}

// 通过RS232发送一个字节 (阻塞方式)
void RS232_SendByte(uint8_t Byte)
{
    USART_SendData(USART2, Byte);
    // 等待发送数据寄存器为空
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

// 通过RS232发送一个字符串
void RS232_SendString(char *String)
{
    for (uint8_t i = 0; String[i] != '\0'; i++)
    {
        RS232_SendByte(String[i]);
    }
}

// 通过RS232发送格式化的字符串 (printf)
void RS232_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    // 使用vsnprintf防止缓冲区溢出
    vsnprintf(String, sizeof(String), format, arg);
    va_end(arg);
    RS232_SendString(String);
}
