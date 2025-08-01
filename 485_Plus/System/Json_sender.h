#ifndef __JSON_SENDER_H
#define __JSON_SENDER_H

#include "stm32f10x.h"
#include "plc_config.h" // 需要包含 plc_config.h 来识别 PlcDataPoint 结构体

//增加用于发送整个数据数组的函数
//void SendAllDataAsJsonArray(PlcDataPoint* data_points, int count);
//增加函数用于发送继电器的状态
//void SendRelayDataAsJsonArray(RelayDataPoint* data_points, int count);

void SendAllDataAsJsonArray(PlcDataPoint *data_points, int count, void (*send_func)(char *));
void SendRelayDataAsJsonArray(RelayDataPoint *data_points, int count, void (*send_func)(char *));
void SendBitDataAsJsonArray(BitDataPoint *data_points, int count, void (*send_func)(char *));

#endif
