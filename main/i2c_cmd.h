/*
 * i2c_cmd.h
 *
 *  Created on: 2022. jún. 8.
 *      Author: Dell
 */


#ifndef MAIN_I2C_CMD_H_
#define MAIN_I2C_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_MASTER_PORT_NUM) /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */



esp_err_t i2c_master_init(void);
esp_err_t i2c_master_deinit(void);
esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t Address, uint8_t Offset, uint8_t *data_rd, size_t size);
esp_err_t i2c_master_write_slave(i2c_port_t i2c_num , uint8_t Address, uint8_t Offset ,uint8_t *data_w, size_t size);

#ifdef __cplusplus
}
#endif
#endif /* MAIN_I2C_CMD_H_ */
