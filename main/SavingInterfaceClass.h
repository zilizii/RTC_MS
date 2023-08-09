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

using namespace std;



class SavingInterfaceClass {
private:
	std::string _name;
public:
	SavingInterfaceClass(std::string name) : _name(name) {};
	virtual ~SavingInterfaceClass();
	std::string GetClassFriendlyName(void) {return _name;}
	virtual cJSON * Save() = 0;
	virtual void Load( cJSON * ) = 0;
};



#endif /* MAIN_SAVINGINTERFACECLASS_H_ */
