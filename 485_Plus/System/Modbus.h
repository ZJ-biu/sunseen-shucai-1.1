#ifndef __MODBUS_H
#define __MODBUS_H

void USART3_Init(void);//
void USART2_Init(void);
void RS485_SetSendMode(void);	//
void RS485_SetReceiveMode(void);//
void USART3_SendData(uint8_t *data, uint16_t len);
void buildModbusRtuRequest(uint8_t slave_address, uint8_t function_code, uint16_t start_addr, uint16_t num_regs, uint8_t *frame);//
void sendModbusRtuRequest(uint8_t *request_frame, uint16_t length);//
void USART3_IRQHandler(void);//
void parseModbusRtuResponse(uint8_t *frame, uint16_t length, uint32_t *register_values);//
void readModbusData(uint8_t slave_address, uint8_t function_code, uint16_t start_addr, uint16_t num_regs, uint32_t *register_values);//

uint16_t calculateCrc16(uint8_t *data, uint16_t length);//
uint16_t receiveModbusRtuResponse(uint8_t *buffer, uint16_t max_len);
void readCoilData(uint8_t slave_address, uint8_t function_code, uint16_t start_addr, uint16_t num_coils, uint8_t *coil_values);
#endif
