/*
 * DataStruct.h
 *
 *  Created on: 2023. szept. 23.
 *      Author: Dell
 */

#ifndef MAIN_DATASTRUCT_H_
#define MAIN_DATASTRUCT_H_

#include <bitset>

typedef struct {
    int batteryVoltage;
    std::bitset<2> pins;
} sDataStruct;



#endif /* MAIN_DATASTRUCT_H_ */