# RV-8263-C7 RTC Helper Class Development Project

This example demonstrates usage of automotive grade external I2C RTC in ESP-IDF environment.
To show a usecase in a real world application a "MailBox Notifier" demo build up. The Application together with the PCB board is able to track a mailbox OPEN or CLOSED state and send out to a receiver uController. The communication via ESP-NOW protocol.
Some of the parameters has a SAVE/LOAD capability to SPIFF file system.


## How to use example

### Hardware Required

This example should be able to run on MailBox Notifier - ESP32 based custom PCB board | PINS are configurable.
RTC RV-8263-C7 connected to SDA - GPIO 21 (configurable) and SCL to GPIO 22 (configurable)
ESP32 set up to 16M with 4M application and 1M SPIFFS --> reconfigurable from 'partitions.csv' file and the 'sdkconfig'

### Configure the project

```
idf.py menuconfig
```
Please use the 'RTC_MS/I2C Config' menu to set SCL, SDA GPIO pins, I2C port and the frequency
Please use the 'RTC_MS/RV-8263-C7 RTC Driver' menu to setup basic parameters of the RTC timer
Please use the 'RTC_MS/HW Dependent' menu to setup the HW versions|setup your PIN layout

### Tuning of the application

```
main\RTC_MS_main.cpp 
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

'QT' quit from application and save configuration to SPIFF
example command : QT
example output : ets Jun  8 2016 00:2▒▒ets Jun  8 2016 00:22:57

'GJ' Save Configuration to SPIFF
example command : GJ
example output :

'FM' Get Free Heap size
example command : GM
example output : 216172 bytes

'LSI' Set LED Light to ON
example command : LSI
example output :

'LSO' Set LED Light to OFF
example command : LSO
example output :


## Example Output  

```
pins: 10
 SPIFF Init done
ADC Channel 27
Battery Read 2754
Battery Read 2754 [mV]
app_main starting
Wake Up by Timer : 0

