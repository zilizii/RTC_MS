/*
 * IsChangedSingletone.cpp
 *
 *  Created on: 2023. szept. 18.
 *      Author: Dell
 */

#include "IsChangedSingletone.h"

//initialization of the Singleton class pointer - because of static member should initialize in the source file
IsChangedSingletone * IsChangedSingletone::instancePtr = nullptr;

IsChangedSingletone* IsChangedSingletone::GetInstance() {
	if (instancePtr == nullptr) {
			instancePtr = new IsChangedSingletone();
	}
	return instancePtr;
}
IsChangedSingletone::~IsChangedSingletone() {
	// TODO Auto-generated destructor stub
}

