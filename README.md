# RV-8263-C7 RTC Helper Class Development Project

This example demonstrates usage of C++ in ESP-IDF.


## How to use example

### Hardware Required

This example should be able to run on any commonly available ESP32 development board.
RTC RV-8263-C7 connected to SDA - GPIO 21 and SCL to GPIO 22

### Configure the project

```
idf.py menuconfig
```

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
Time: 04:53:16
Date: 2022-07-03
EPOCH : 1656823996
Time: 04:53:17
Date: 2022-07-03
EPOCH : 1656823997
Time: 04:53:18
Date: 2022-07-03

```

