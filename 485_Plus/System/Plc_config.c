#include "plc_config.h"

// 保持寄存器(FC03) 配置
PlcDataPoint plc_data_points[] = {
// 从机地址, 寄存器地址, 数量,    JSON键名,        初始值,     类型

    //单个整数数据
    {0x01,    0x138A,     1,   "LW5002",             0, .type = TYPE_UINT},
    {0x01,    0x138B,     1,   "LW5003",             0, .type = TYPE_UINT},
    {0x01,    0x1393,     1,   "LW5011",             0, .type = TYPE_UINT},
    {0x01,    0x1394,     1,   "LW5012",             0, .type = TYPE_UINT},
		{0x01,    0x1395,     1,   "LW5013",             0, .type = TYPE_UINT},
    {0x01,    0x139F,     1,   "LW5023",            0, .type = TYPE_UINT},

    // 单个浮点数据
    //{0x01,    0x139F,     1,   "LW5023B",            0, .type = TYPE_FLOAT},
		
		//双个整数数据
		{0x01,    0x1388,     2,   "LW5000",             0, .type = TYPE_UINT},
    {0x01,    0x138E,     2,   "LW5006",             0, .type = TYPE_UINT},
    {0x01,    0x13B5,     2,   "LW5045",             0, .type = TYPE_UINT},
		//双个浮点数据
		//{0x01,    0x1388,     2,   "MBoxActualT",        0, .type = TYPE_FLOAT},

};

const uint16_t PLC_DATA_POINT_COUNT = sizeof(plc_data_points) / sizeof(PlcDataPoint);

// 线圈 (FC01) 配置
BitDataPoint fc01_coils[] = {
// 从机地址, 地址,   JSON键名,      初始值
    {0x01,   0x05,    "LW105",          0},
    {0x01,   0x0D,    "LW10D",          0},
    {0x01,   0x11,    "LW111",          0}
		
};
const uint16_t FC01_COILS_COUNT = sizeof(fc01_coils) / sizeof(BitDataPoint);

// 离散输入 (FC02) 配置
BitDataPoint fc02_discrete_inputs[] = {
// 从机地址, 地址,   JSON键名,      初始值
    {0x01,   0x02,    "LW202",          0},
    {0x01,   0x0D,    "LW20D",          0},
    {0x01,   0x0E,    "LW20E",          0},
    {0x01,   0x0F,    "LW20F",          0}
};
const uint16_t FC02_DISCRETE_INPUTS_COUNT = sizeof(fc02_discrete_inputs) / sizeof(BitDataPoint);

// 继电器配置
RelayDataPoint relay_data_points[] = {
    // JSON "id",   端口,    引脚,   初始值
    {"relay1",      GPIOA, GPIO_Pin_6, 0},//继电器1
    {"relay2",      GPIOA, GPIO_Pin_7, 0},//继电器2
    {"relay3",      GPIOB, GPIO_Pin_0, 0},//继电器3
    {"relay4",      GPIOB, GPIO_Pin_1, 0} //继电器4
};

const uint16_t RELAY_DATA_POINT_COUNT = sizeof(relay_data_points) / sizeof(RelayDataPoint);
