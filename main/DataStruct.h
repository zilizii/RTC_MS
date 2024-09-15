/*
 * DataStruct.h
 *
 *  Created on: 2023. szept. 23.
 *      Author: Dell
 */

#ifndef MAIN_DATASTRUCT_H_
#define MAIN_DATASTRUCT_H_

#include <bitset>

/*************************************************************************************************
 * DataStructure for Data to send out the device in case of wake up
 * Parameter : 
 *  @epoch           : time stamp
 *  @batteryVolatage : mV battery measured voltage
 *  @pins            : reason of wake up -> Button, Reed switch state, RTC interrupt.     
 ************************************************************************************************/


typedef struct {
    long epoch;
    int batteryVoltage;
    std::bitset<3> pins;
} sDataStruct;



#endif /* MAIN_DATASTRUCT_H_ */