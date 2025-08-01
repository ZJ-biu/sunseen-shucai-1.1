#include "json_sender.h"
#include "Serial.h"
#include "Stdio.h"
#include "Delay.h"

// 将PLC数据点格式化为JSON并发送
// send_func: 一个函数指针，决定数据通过哪个物理端口发送 (如USART1或USART2)
void SendAllDataAsJsonArray(PlcDataPoint *data_points, int count, void (*send_func)(char *))
{
    char item_buffer[80];

    // 将数据点两两分组，每组生成一个独立的JSON包
    for (int i = 0; i < count; i += 2)
    {
        send_func("[");

        // 处理第一个数据点
        switch (data_points[i].type)
        {
        case TYPE_UINT:
            snprintf(item_buffer, sizeof(item_buffer),
                     "{\"id\":\"%s\",\"value\":\"%lu\"}",
                     data_points[i].json_key, data_points[i].value);
            break;
        case TYPE_FLOAT:
            snprintf(item_buffer, sizeof(item_buffer),
                     "{\"id\":\"%s\",\"value\":\"%lu.%lu\"}",
                     data_points[i].json_key,
                     data_points[i].value / 10,
                     data_points[i].value % 10);
            break;
        }
        send_func(item_buffer);

        // 如果存在第二个数据点，则处理
        if (i + 1 < count)
        {
            send_func(",");

            switch (data_points[i + 1].type)
            {
            case TYPE_UINT:
                snprintf(item_buffer, sizeof(item_buffer),
                         "{\"id\":\"%s\",\"value\":\"%lu\"}",
                         data_points[i + 1].json_key, data_points[i + 1].value);
                break;
            case TYPE_FLOAT:
                snprintf(item_buffer, sizeof(item_buffer),
                         "{\"id\":\"%s\",\"value\":\"%lu.%lu\"}",
                         data_points[i + 1].json_key,
                         data_points[i + 1].value / 10,
                         data_points[i + 1].value % 10);
                break;
            }
            send_func(item_buffer);
        }
        send_func("]\r\n");
		Delay_ms(500);
    } 
}

// 将继电器状态格式化为单个JSON数组并发送
// send_func: 一个函数指针，决定数据通过哪个物理端口发送
void SendRelayDataAsJsonArray(RelayDataPoint *data_points, int count, void (*send_func)(char *))
{
    char json_buffer[200];
    char item_buffer[64];
    int buffer_len = 0;

    buffer_len += snprintf(json_buffer + buffer_len, sizeof(json_buffer) - buffer_len, "[");

    for (int i = 0; i < count; i++)
    {
        snprintf(item_buffer, sizeof(item_buffer),
                 "{\"id\":\"%s\",\"value\":\"%d\"}",
                 data_points[i].json_key, data_points[i].value);

        buffer_len += snprintf(json_buffer + buffer_len, sizeof(json_buffer) - buffer_len, "%s", item_buffer);

        if (i < count - 1)
        {
            buffer_len += snprintf(json_buffer + buffer_len, sizeof(json_buffer) - buffer_len, ",");
        }
    }

    buffer_len += snprintf(json_buffer + buffer_len, sizeof(json_buffer) - buffer_len, "]\r\n");
    send_func(json_buffer);
}

// 文件: Json_sender.c

void SendBitDataAsJsonArray(BitDataPoint *data_points, int count, void (*send_func)(char *))
{
    char item_buffer[80]; // 用于存储单个JSON对象的缓冲区

    // 将数据点两两分组，每组生成一个独立的JSON包
    for (int i = 0; i < count; i += 2)
    {
        send_func("["); // 发送JSON数组的起始括号

        // 1. 处理组内的第一个数据点
        snprintf(item_buffer, sizeof(item_buffer), 
                 "{\"id\":\"%s\",\"value\":\"%d\"}", 
                 data_points[i].json_key, data_points[i].value);
        send_func(item_buffer);

        // 2. 如果组内存在第二个数据点，则处理它
        if (i + 1 < count)
        {
            send_func(","); // 发送逗号分隔符

            snprintf(item_buffer, sizeof(item_buffer),
                     "{\"id\":\"%s\",\"value\":\"%d\"}",
                     data_points[i + 1].json_key, data_points[i + 1].value);
            send_func(item_buffer);
        }

        // 3. 发送JSON数组的结束括号和换行符
        send_func("]\r\n");
        // 增加延时，与另一个发送函数保持一致，避免数据发送过快
		Delay_ms(500); 
    }
}
