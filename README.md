# RV-8263-C7 RTC Helper Class Development Project

This example demonstrates usage of automotive grade external I2C RTC in ESP-IDF environment.


## How to use example

### Hardware Required

This example should be able to run on any commonly available ESP32 development board.
RTC RV-8263-C7 connected to SDA - GPIO 21 (configurable) and SCL to GPIO 22 (configurable)


### Configure the project

```
idf.py menuconfig
```
Please use the 'I2C Config' menu to set SCL, SDA GPIO pins, I2C port and the frequency
Please use the 'RTC_MS' menu to setup basic parameters of the RTC timer  

### Tuning of the application

```
main\RTC_MS_main.cpp 
```
File the time and the wake up factors can be changed 

```
ooo->writeTimerValueToRTC(5); // Counter set to 5```
```
Output set to minutes and the Wake up timer set it up
The outcome every 5 minutes wake up...

```
// TD 1/60Hz, TE Enabled, TIE Enabled,  TI_TP Enabled 
ooo->writeTimerModeToRTC(0b11111); // 0b11111
```
Please read the RV-8263-C7 Application Notes for more details.

### Build and Flash

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

### UART Commands

`SE+epoch` set the UTC epoch of the RTC module. For ex.: [epochconverter](https://www.epochconverter.com/) can be used. Check the time zone as well.  
example : SE1672944185  

`GE` read back the UTC epoch time.  
example : GE  
 
`ST+TimeZone` set timezone and adjust the time as well.   
example : ST1  

`GT` give back the Time Zone setting.  
example : GT  

`SL+epoch` set the local epoch - no Time Zone correction.  
example : SL1672944185  

`GL` give back the local epoch.  
example : GL  

## Example Output  

```
app_main starting
Time: 22:55:33
Date: 2022-09-17
0
