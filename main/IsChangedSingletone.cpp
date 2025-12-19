/*
 * IsChangedSingletone.cpp
 *
 *  Created on: 2023. szept. 18.
 *      Author: Dell
 */

#include "IsChangedSingletone.h"
#include <cassert>
using namespace General;

//initialization of the Singleton class pointer - because of static member should initialize in the source file
//IsChangedSingletone * IsChangedSingletone::instancePtr = nullptr;

IsChangedSingletone* IsChangedSingletone::GetInstance() {
	/*if (instancePtr == nullptr) {
			instancePtr = new IsChangedSingletone();
	}*/

	assert(instancePtr != nullptr && "The IsChangedSingletone class is not initialized! Call init() method before use it!");

	return instancePtr;
}
IsChangedSingletone::~IsChangedSingletone() {
	// TODO Auto-generated destructor stub
	//delete instancePtr;
}

void IsChangedSingletone::init() {
	assert(!instancePtr && "The IsChangedSingletone class is already initialized! Duplicate init() call!");
	instancePtr = new IsChangedSingletone();
}

void IsChangedSingletone::shutDown() {
	
	delete instancePtr;
	instancePtr = nullptr;
	/*if (instancePtr != nullptr) {
		delete instancePtr;
		instancePtr = nullptr;
	}*/
}

