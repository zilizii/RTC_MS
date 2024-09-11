/*
 * SavingInterfaceClass.h
 *
 *  Created on: 2023. júl. 13.
 *      Author: Dell
 */

#ifndef MAIN_SAVINGINTERFACECLASS_H_
#define MAIN_SAVINGINTERFACECLASS_H_

#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <string.h>
#include <cJSON.h>
#include "IsChangedSingletone.h"

using namespace std;

class SavingInterfaceClass {
private:
	std::string _name;
	//bool _isChanged;
	IsChangedSingletone *instance;
public:
	SavingInterfaceClass(std::string name) :
			_name(name) {
		this->instance = IsChangedSingletone::GetInstance();
	}
	virtual ~SavingInterfaceClass();
	std::string GetClassFriendlyName(void) {
		return _name;
	}
	bool isChanged(void) {
		return instance->getIsChanged();
	}
	void setToChanged() {
		instance->setToChanged();
	}
	
	virtual cJSON* Save() = 0;
	virtual void Load(cJSON*) = 0;
};

#endif /* MAIN_SAVINGINTERFACECLASS_H_ */
