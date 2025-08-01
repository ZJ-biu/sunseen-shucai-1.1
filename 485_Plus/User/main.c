#include "stm32f10x.h"
#include "Delay.h"
#include "LED.h"
#include "Relay.h"
#include "Modbus.h"
#include "Timer.h"
#include "Plc_config.h"
#include "Json_sender.h"
#include "Serial.h"
#include "RS232.h"

uint8_t NEW_R_state[4], s, Flag = 0;
uint8_t choose = 1; // 定义全局模式选择变量: 1=RS485, 2=RS232, 3=继电器
uint8_t bit_read_buffer[256];

extern uint8_t buffer[7];
/**
 * @brief 处理位类型数据（线圈或离散输入）的读取任务
 * @param points 指向 BitDataPoint 配置数组的指针
 * @param count 数组中的元素数量
 * @param function_code 要使用的Modbus功能码 (0x01 读取线圈, 0x02 读取离散输入)
 * @note 此函数通过查找连续的地址块来优化Modbus通信，将多个单点读取合并为一次请求。
 */
void process_bit_tasks(BitDataPoint *points, uint16_t count, uint8_t function_code)
{
    if (count == 0) return;

    uint16_t current_index = 0;
    while (current_index < count)
    {
        uint16_t start_address = points[current_index].address;
        uint8_t start_slave_address = points[current_index].slave_address;
        uint16_t num_points = 1;
        uint16_t next_expected_address = start_address + 1;

        for (int i = current_index + 1; i < count; i++)
        {
            if (points[i].address == next_expected_address && points[i].slave_address == start_slave_address)
            {
                num_points++;
                next_expected_address++;
            }
            else
            {
                break;
            }
        }
        
        readCoilData(start_slave_address, function_code, start_address, num_points, bit_read_buffer);
        
        for (int i = 0; i < num_points; i++)
        {
            points[current_index + i].value = bit_read_buffer[i];
        }
        
        current_index += num_points;
    }
}

int main(void)
{
    // 初始化所有硬件外设
    LED_Init();
    Serial_Init();
    USART3_Init();
    Timer_Init();
    Relay_Init();
		RS232_Init();

    // 根据选择的模式，初始化LED的亮灯状态
    if (choose == 1){
        LED1_ON();
    }
    else if (choose == 2){
        LED2_ON();
    }
	else if (choose == 3){
		LED1_ON();
		LED2_ON();
	}

    uint32_t temp_regs[2];

		static uint8_t previous_relay_states[RELAY_COUNT] = {2, 2, 2, 2};

    // 主循环
    while (1)
    {
        // 模式1：通过RS485读取Modbus数据，并通过USART1发送JSON
        if (choose == 1 || choose == 2)
        {
            // 遍历所有预设的PLC数据点
            for (int i = 0; i < PLC_DATA_POINT_COUNT; i++)
            {
                PlcDataPoint *current_point = &plc_data_points[i];
                temp_regs[0] = 0;
                temp_regs[1] = 0;

                // 读取指定的Modbus寄存器数据
                readModbusData(
                    current_point->slave_address,
                    0x03,
                    current_point->register_address,
                    current_point->register_count,
                    temp_regs);

                // 根据寄存器数量，组合32位数据
                if (current_point->register_count == 1)
                {
                    current_point->value = temp_regs[0];
                }
                else if (current_point->register_count == 2)
                {
                    current_point->value = (temp_regs[1] << 16) | temp_regs[0];
                }
            }
            process_bit_tasks(fc01_coils, FC01_COILS_COUNT, 0x01);

            // --- 读取离散输入 (FC02) ---
            process_bit_tasks(fc02_discrete_inputs, FC02_DISCRETE_INPUTS_COUNT, 0x02);
            
            // --- 根据模式选择发送函数 ---
            void (*send_func_register)(char *) = (choose == 1) ? Serial_SendString : RS232_SendString;
            void (*send_func_bit)(char *) = (choose == 1) ? Serial_SendString : RS232_SendString;
            
            // --- 依次发送所有类型的数据 ---
            if (PLC_DATA_POINT_COUNT > 0) {
                 SendAllDataAsJsonArray(plc_data_points, PLC_DATA_POINT_COUNT, send_func_register);

            }
            if (FC01_COILS_COUNT > 0) {
                SendBitDataAsJsonArray(fc01_coils, FC01_COILS_COUNT, send_func_bit);

            }
            if (FC02_DISCRETE_INPUTS_COUNT > 0) {
                SendBitDataAsJsonArray(fc02_discrete_inputs, FC02_DISCRETE_INPUTS_COUNT, send_func_bit);
            }

            if (choose == 1) LED1_Turn();
            else LED2_Turn();
            
            Delay_s(5);//设置上报时间周期
        }


		else if (choose == 3)
		{
			uint8_t state_has_changed = 0; // 定义状态变化标志位

            // 读取所有继电器的当前GPIO状态
            for (int i = 0; i < RELAY_DATA_POINT_COUNT; i++)
            {
                if (GPIO_ReadInputDataBit(relay_data_points[i].port, relay_data_points[i].pin) == 1)
                {
                    relay_data_points[i].value = 1;
                }
                else
                {
                    relay_data_points[i].value = 0;
                }
            }
            // 将当前状态与上一次存储的状态进行比较
            for (int i = 0; i < RELAY_DATA_POINT_COUNT; i++)
            {
                if (relay_data_points[i].value != previous_relay_states[i])
                {
                    state_has_changed = 1; // 只要有任何一个状态不同，就设置标志位
                    break; 
                }
            }
            // 如果状态已改变，则执行上报
            if (state_has_changed)
            {
                LED2_Turn(); // 翻转LED作为上报指示
                
                // 调用JSON发送函数，上报最新的状态
                SendRelayDataAsJsonArray(relay_data_points, RELAY_DATA_POINT_COUNT, Serial_SendString);

                // 关键：上报后，用当前状态更新“上一次的状态”，为下一次比较做准备
                for (int i = 0; i < RELAY_DATA_POINT_COUNT; i++)
                {
                    previous_relay_states[i] = relay_data_points[i].value;
                }
            }
            // 短暂延时，避免CPU高负载空转
            Delay_ms(100);
			LED1_Turn();
		}
	}	
}
