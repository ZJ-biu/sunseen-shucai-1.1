# 数据采集模块 - 程序使用说明文档

## 1. 产品简介

本模块是基于 STM32F10x 系列微控制器设计的一款多功能、可配置的数据采集终端。它能够通过 RS485 总线采集 Modbus RTU 协议设备的数据，或监测本地的数字量输入信号，并将所有信息转换为统一的 JSON 格式，通过串行端口输出。

其核心价值在于高度的可配置性和灵活性，用户仅需修改配置文件即可适应不同的工业现场需求，无需深入复杂的底层代码。

---

## 2. 快速上手指南 (Quick Start)

请遵循以下步骤，快速让您的采集模块运行起来。

### **步骤 1: 硬件连接**

确保您的硬件连接无误。核心连接如下表所示：

| 功能 | MCU 引脚 | 连接到... | 说明 |
| :--- | :--- | :--- | :--- |
| **电源** | `220V`, `GND` | 外部电源 | 为模块提供稳定的电源。 |
| **RS485 (Modbus)** | `PB10` (TX) | RS485收发器 `DI` | Modbus 数据发送。 |
| | `PB11` (RX) | RS485收发器 `RO` | Modbus 数据接收。 |
| | `PA11` (DE/RE) | RS485收发器 `DE/RE` | 方向控制，程序自动管理。 |
| **JSON 输出** | `PA9` (TX) | TTL-USB模块 `RXD` | **模式1和3**的默认数据输出口。 |
| | `PA2` (TX) | RS232收发器 `TXD` | **模式2**的数据输出口。 |
| **本地输入** | `PA6`, `PA7`, `PB0`, `PB1` | 继电器干接点等 | 用于模式3，监测外部开关量信号。 |
| **状态指示** | `PB12`, `PB13` | LED灯 | 用于直观判断模块运行状态。 |

### **步骤 2: 选择工作模式**

打开 `main.c` 文件，找到以下代码行，根据您的需求修改 `choose` 变量的值。

**具体内容见《485 Pus使用文档》**
