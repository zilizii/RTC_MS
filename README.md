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
example command: SE1672944185  

`GE` read back the UTC epoch time.  
example command: GE  
example output : 
GET UTC EPOCH : 1678402333
 
`ST+TimeZone` set timezone and adjust the time as well.   
example : ST1  

`GT` give back the Time Zone setting.  
example command : GT  
example output : 
GET Time Zone : 1

`SL+epoch` set the local epoch - no Time Zone correction.  
example : SL1672944185  

`GL` give back the local epoch.  
example command: GL
example output : 
GET LOCAL EPOCH : 1678405837  

'GN' get the current local Date and Time and the state of the Wake Up Timer
example command: GN
example output : 
Time: 23:50:37
Date: 2023-03-09
Wake Up by Timer : 0

'GC' get the remaining counter value with the time unit
example command: GC
example output : 
1 Minute

'WS' scanning the Wifi AP
example command : WS
example output :
I (270591) RT_MS: [ESP_446B71][rssi=-27][MAC=30:AE:A4:44:6B:71]
I (270591) RT_MS: [][rssi=-73][MAC=38:31:BF:F4:BB:58]
I (270591) RT_MS: [UPC1819346][rssi=-74][MAC=34:2C:C4:AB:FF:F0]
I (270601) RT_MS: [UPCD198CFD][rssi=-92][MAC=90:5C:44:9B:51:DE]
I (270601) RT_MS: sta scan done


## Example Output  

```
app_main starting
Wake Up by Timer : 1
