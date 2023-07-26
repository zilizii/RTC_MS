/*
 * ConfigurationHandler.h
 *
 *  Created on: 2023. júl. 26.
 *      Author: Dell
 */

#ifndef MAIN_CONFIGURATIONHANDLER_H_
#define MAIN_CONFIGURATIONHANDLER_H_
#include <iostream>
#include <list>
#include <cJSON.h>
#include <SavingInterfaceClass.h>

class ConfigurationHandler {
private:
	std::string _path;
	std::list<SavingInterfaceClass *> _ll;
public:
	ConfigurationHandler();
	virtual ~ConfigurationHandler();
	void registerClass(SavingInterfaceClass *);
	void LoadAllConfiguration();
	void SaveAllConfiguration();
};

#endif /* MAIN_CONFIGURATIONHANDLER_H_ */
