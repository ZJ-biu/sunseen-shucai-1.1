#ifndef __PLC_CONFIG_H
#define __PLC_CONFIG_H
#define RELAY_COUNT 4

#include "stm32f10x.h"

// 定义一个枚举来表示数据类型
typedef enum {
    TYPE_UINT,          // 普通无符号整数
    TYPE_FLOAT          // 定点浮点数 

} PlcDataType;

//458端口数据传输
typedef struct {
    uint8_t   slave_address;      // Modbus从机地址 (例如: 1)
    uint16_t  register_address;   // 寄存器起始地址 (例如: 0x1000)
    uint16_t  register_count;     // 读取的寄存器数量 (例如: 1 代表16位, 2 代表32位)
    const char* json_key;         // 上报到云端时使用的JSON键名 (例如: "temperature")
    uint32_t  value;              // 用于存储从设备读取到的原始数值的缓冲区
    PlcDataType type;             // 数据类型，用于后续解析 
} PlcDataPoint;

extern PlcDataPoint plc_data_points[];
extern const uint16_t PLC_DATA_POINT_COUNT;

//继电器数据传输
typedef struct {
    const char* json_key;   // JSON中的 "id" 键值
    GPIO_TypeDef* port;     // GPIO端口 (例如 GPIOA, GPIOB)
    uint16_t pin;           // GPIO引脚 (例如 GPIO_Pin_6)
    uint8_t value;          // 用于存储读取到的状态 (0 或 1)
} RelayDataPoint;

extern RelayDataPoint relay_data_points[];
extern const uint16_t RELAY_DATA_POINT_COUNT;

//线圈数据传输
typedef struct
{
		uint8_t slave_address; // Modbus从机地址
    uint16_t address;      // 协议地址 (偏移量)
    char json_key[20];     // JSON键名
    uint8_t value;         // 存储读取到的值 (0 或 1)
} BitDataPoint;

extern BitDataPoint fc01_coils[];
extern const uint16_t FC01_COILS_COUNT;

extern BitDataPoint fc02_discrete_inputs[];
extern const uint16_t FC02_DISCRETE_INPUTS_COUNT;

#endif
