/*
 * ConfigurationHandler.h
 *
 *  Created on: 2023. júl. 26.
 *      Author: Dell
 */

#ifndef MAIN_CONFIGURATIONHANDLER_H_
#define MAIN_CONFIGURATIONHANDLER_H_
#include <algorithm>
#include <iostream>
#include <list>
#include <cJSON.h>
#include <SavingInterfaceClass.h>
#include "IsChangedSingletone.h"
#include <string_view>

class ConfigurationHandler {
private:
	std::string _path;
	std::list<SavingInterfaceClass*> _ll;
public:
	ConfigurationHandler(std::string path) :
			_path(path) {
	}
	;
	virtual ~ConfigurationHandler();
	void registerClass(SavingInterfaceClass*);
	void LoadAllConfiguration();
	void SaveAllConfiguration();
	void ForcedSave();
	
	//SavingInterfaceClass * getClassPointer(std::string name); 
	SavingInterfaceClass * getClassPointer(std::string_view name);
};

#endif /* MAIN_CONFIGURATIONHANDLER_H_ */
