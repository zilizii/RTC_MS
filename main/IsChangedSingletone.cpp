/*
 * IsChangedSingletone.cpp
 *
 *  Created on: 2023. szept. 18.
 *      Author: Dell
 */

#include "IsChangedSingletone.h"
#include <cassert>

//initialization of the Singleton class pointer - because of static member should initialize in the source file
IsChangedSingletone * IsChangedSingletone::instancePtr = nullptr;

IsChangedSingletone* IsChangedSingletone::GetInstance() {
	/*if (instancePtr == nullptr) {
			instancePtr = new IsChangedSingletone();
	}*/

	assert(instancePtr != nullptr && "The IsChangedSingletone class is not initialized! Call init() method before use it!");

	return instancePtr;
}
IsChangedSingletone::~IsChangedSingletone() {
	// TODO Auto-generated destructor stub
	delete instancePtr;
}

void IsChangedSingletone::init() {
	instancePtr = new IsChangedSingletone();
	instancePtr->isChanged = false;
}

void IsChangedSingletone::shutDown() {
	if (instancePtr != nullptr) {
		delete instancePtr;
		instancePtr = nullptr;
	}
}

