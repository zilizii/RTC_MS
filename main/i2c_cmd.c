/*
 * i2c_cmd.c
 *
 *  Created on: 2022. jún. 8.
 *      Author: Dell
 */

//#include "driver/i2c.h"
//#include "driver/gpio.h"
#include "soc/dport_reg.h"
#include "hal/gpio_types.h"
#include "i2c_cmd.h"
#include "sdkconfig.h"


/*
 *  I2C Communication Frame structure
 *
 *    Start Bit    Address    Read/Write   ACK/NACK Bit   Data Frame 1  ACK/NACK Bit  Data Frame x   ACK/NACK Bit      Stop
 *  | Condition |  7-10bit  |     1bit   |      1bit   |     8bit      |    1bit    |    8bit      |     1bit     | Condition |
 *
 *
 *  Start Condition : SDA High -> Low before the SCL line switch High to Low
 *  Stop Condition  : SDA Low to High after the SCL line switch Low to High
 *
 */

/**
 * @brief i2c master initialization
 */
esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags =  I2C_SCLK_SRC_FLAG_FOR_NOMAL;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

esp_err_t i2c_master_deinit(void) {
	int i2c_master_port = I2C_MASTER_NUM;
	return i2c_driver_delete((i2c_port_t )i2c_master_port);
}



/*
 * Read register(s) from slave
 *
 */

esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t Address, uint8_t Offset, uint8_t *data_rd, size_t size)
{
    if (size == 0) {
        return ESP_OK;
    }
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (Address << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, Offset, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (Address << 1) | READ_BIT, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}



esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t Address, uint8_t Offset ,uint8_t *data_wr, size_t size)
{
	if (size == 0) {
		return ESP_OK;
	}
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (Address << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, Offset, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS   );
    i2c_cmd_link_delete(cmd);
    return ret;
}
