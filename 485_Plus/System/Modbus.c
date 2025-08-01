#include "stm32f10x.h"
#include "stdint.h"
#include "Delay.h"

// 全局标志位，当一帧Modbus数据接收完成时，由定时器中断置1
volatile uint8_t modbus_frame_received = 0;

uint8_t frame[8];
uint16_t crc;
uint16_t i = 0;
uint8_t buffer[7];
volatile uint8_t rx_buffer[256]; // Modbus接收缓冲区
volatile uint16_t rx_index = 0;  // 接收缓冲区的索引

// 初始化USART3用于Modbus (RS485)通信
void USART3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能GPIOA, GPIOB和USART3的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // 配置PB10为USART3的TX引脚 (复用推挽输出)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置PB11为USART3的RX引脚 (浮空输入)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置PA11为RS485收发器的方向控制引脚 (推挽输出)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置USART3参数 (115200波特率, 8位数据, 1位停止位, 无校验)
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    // 使能USART3的接收中断(RXNE)
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    // 配置NVIC (嵌套向量中断控制器)
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStructure);

    // 使能USART3外设
    USART_Cmd(USART3, ENABLE);
}

// 设置RS485为发送模式
void RS485_SetSendMode(void) { GPIO_SetBits(GPIOA, GPIO_Pin_11); }
// 设置RS485为接收模式
void RS485_SetReceiveMode(void) { GPIO_ResetBits(GPIOA, GPIO_Pin_11); }

// 计算Modbus RTU的CRC16校验码
uint16_t calculateCrc16(uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

// 构建一个Modbus RTU读取请求帧 (8字节)
void buildModbusRtuRequest(uint8_t slave_address, uint8_t function_code, uint16_t start_addr, uint16_t num_regs, uint8_t *frame)
{
    uint16_t idx = 0;
    frame[idx++] = slave_address;
    frame[idx++] = function_code;
    frame[idx++] = (start_addr >> 8) & 0xFF; // 地址高位
    frame[idx++] = start_addr & 0xFF;        // 地址低位
    frame[idx++] = (num_regs >> 8) & 0xFF;   // 数量高位
    frame[idx++] = num_regs & 0xFF;          // 数量低位
    uint16_t crc = calculateCrc16(frame, idx);
    frame[idx++] = crc & 0xFF;               // CRC低位
    frame[idx++] = (crc >> 8) & 0xFF;        // CRC高位
}

// 通过USART3发送Modbus请求帧
void sendModbusRtuRequest(uint8_t *request_frame, uint16_t length)
{
    RS485_SetSendMode(); // 切换到发送模式
    for (uint16_t i = 0; i < length; i++)
    {
        // 等待发送数据寄存器为空
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, request_frame[i]);
    }
    // 等待发送完成
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
    RS485_SetReceiveMode(); // 切换回接收模式
}

// USART3中断服务程序，用于接收数据
void USART3_IRQHandler(void)
{
    // 检查是否是接收中断
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        // 重置并启动T3.5帧间隔定时器
        TIM_SetCounter(TIM2, 0);
        TIM_Cmd(TIM2, ENABLE);

        // 读取数据并存入缓冲区
        uint8_t data = USART_ReceiveData(USART3);
        if (rx_index < sizeof(rx_buffer))
        {
            rx_buffer[rx_index++] = data;
        }

        // 清除中断标志位
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

uint8_t data1[10];
uint16_t output_bytes[5];

// 解析Modbus RTU响应帧
void parseModbusRtuResponse(volatile uint8_t *frame, uint16_t length, uint32_t *register_values)
{
    // 检查响应帧的最小长度
    if (length < 5) return;

    // 校验CRC
    uint16_t received_crc = (frame[length - 1] << 8) | frame[length - 2];
    uint16_t calculated_crc = calculateCrc16((uint8_t *)frame, length - 2);
    if (received_crc != calculated_crc) return;

    // 校验数据字节数
    uint8_t byte_count = frame[2];
    uint16_t data_start_idx = 3;
    if (byte_count != (length - 5)) return;

    // 清空目标数组
    for (int i = 0; i < (byte_count / 2); ++i)
    {
        register_values[i] = 0;
    }

    // 解析数据区，每两个字节组成一个16位寄存器值
    for (uint16_t i = 0; i < (byte_count / 2); i++)
    {
        uint16_t reg_val = (frame[data_start_idx + 2 * i] << 8) | frame[data_start_idx + 2 * i + 1];
        register_values[i] = reg_val;
    }
}

// 执行一次完整的Modbus读取操作 (阻塞函数)
void readModbusData(uint8_t slave_address, uint8_t function_code, uint16_t start_addr, uint16_t num_regs, uint32_t *register_values)
{
    uint8_t request_frame[8];
    buildModbusRtuRequest(slave_address, function_code, start_addr, num_regs, request_frame);

    // 重置接收状态
    rx_index = 0;
    modbus_frame_received = 0;
    for (int i = 0; i < sizeof(rx_buffer); i++)
    {
        rx_buffer[i] = 0;
    }

    // 清除可能存在的上一次未读数据
    if (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
    {
        USART_ReceiveData(USART3);
    }

    // 发送请求帧
    sendModbusRtuRequest(request_frame, sizeof(request_frame));

    // 等待响应，带有200ms超时
    uint32_t timeout_ms = 200;
    while (!modbus_frame_received && timeout_ms > 0)
    {
        Delay_ms(1);
        timeout_ms--;
    }

    // 如果成功接收到一帧数据，则进行解析
    if (modbus_frame_received)
    {
        parseModbusRtuResponse(rx_buffer, rx_index, register_values);
    }

    // 重置状态变量，为下一次通信做准备
    modbus_frame_received = 0;
    rx_index = 0;
}

void parseCoilResponse(volatile uint8_t *frame, uint16_t length, uint8_t *coil_values, uint16_t num_coils)
{
    // 基础校验
    if (length < 5) return;
    uint16_t received_crc = (frame[length - 1] << 8) | frame[length - 2];
    uint16_t calculated_crc = calculateCrc16((uint8_t *)frame, length - 2);
    if (received_crc != calculated_crc) return;

    uint8_t byte_count = frame[2];
    uint16_t data_start_idx = 3;
    
    // 清空目标数组
    for(int i = 0; i < num_coils; ++i) {
        coil_values[i] = 0;
    }

    // 解析数据区，每个bit代表一个线圈状态
    for (uint16_t i = 0; i < num_coils; i++)
    {
        uint16_t byte_index = i / 8; // 计算当前bit在哪个字节里
        uint8_t bit_index = i % 8;   // 计算当前bit在这个字节的第几位
        
        if (byte_index < byte_count)
        {
            if ((frame[data_start_idx + byte_index] >> bit_index) & 0x01)
            {
                coil_values[i] = 1;
            }
            else
            {
                coil_values[i] = 0;
            }
        }
    }
}



void readCoilData(uint8_t slave_address, uint8_t function_code, uint16_t start_addr, uint16_t num_coils, uint8_t *coil_values)
{
    uint8_t request_frame[8];
    // 使用传入的 function_code 参数构建请求，而不是写死0x01
    buildModbusRtuRequest(slave_address, function_code, start_addr, num_coils, request_frame);

    // 重置接收状态
    rx_index = 0;
    modbus_frame_received = 0;
    if (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET) { USART_ReceiveData(USART3); }

    // 发送请求
    sendModbusRtuRequest(request_frame, sizeof(request_frame));

    // 等待响应 (带超时)
    uint32_t timeout_ms = 200;
    while (!modbus_frame_received && timeout_ms > 0)
    {
        Delay_ms(1);
        timeout_ms--;
    }

    // 如果成功接收到，则解析
    if (modbus_frame_received)
    {
        // 解析函数 parseCoilResponse 对于FC01和FC02的响应报文格式是通用的，无需修改
        parseCoilResponse(rx_buffer, rx_index, coil_values, num_coils);
    }

    modbus_frame_received = 0;
    rx_index = 0;
}
