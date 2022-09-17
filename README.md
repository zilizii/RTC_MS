# RV-8263-C7 RTC Helper Class Development Project

This example demonstrates usage of automotive grade external I2C RTC in ESP-IDF environment.


## How to use example

### Hardware Required

This example should be able to run on any commonly available ESP32 development board.
RTC RV-8263-C7 connected to SDA - GPIO 18 (configurable) and SCL to GPIO 19 (configurable)





### Configure the project

```
idf.py menuconfig
```
Please use the 'I2C Config' menu to set SCL, SDA GPIO pins, I2C port and the frequency
Please use the 'RTC_MS' menu to setup basic parameters of the RTC timer  


### Build and Flash

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

```
app_main starting
Time: 22:55:33
Date: 2022-09-17
0

