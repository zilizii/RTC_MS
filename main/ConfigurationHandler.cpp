/*
 * ConfigurationHandler.cpp
 *
 *  Created on: 2023. júl. 26.
 *      Author: Dell
 */

#include "ConfigurationHandler.h"



ConfigurationHandler::~ConfigurationHandler() {
	_ll.clear();
	_path.empty();
}


void ConfigurationHandler::registerClass(SavingInterfaceClass * registerclassptr) {
	this->_ll.push_back(registerclassptr);
}

void ConfigurationHandler::LoadAllConfiguration() {
	if(_ll.empty() == true)
	{
		return;
	}
	string jsonstr;
	//Open File for reading
	FILE * file = fopen(_path.c_str(),"r");
	if(file == NULL)
	{
	   std::cout << "File does not exist : " << _path << std::endl;
	   return;
	}
	char line[256];
	while(fgets(line, sizeof(line), file) != NULL) {
		jsonstr += line;
	}
	fclose(file);
	cJSON * root = cJSON_Parse(jsonstr.c_str());

	//First Lambda expression implemented
	// Do not know if I am happy or not
	std::for_each(_ll.begin(), _ll.end(),[&](SavingInterfaceClass *n) {
		cJSON * objectJSONClass =  cJSON_GetObjectItem( root, n->GetClassFriendlyName().c_str());
		n->Load(objectJSONClass);
	});
	cJSON_Delete(root);
}

void ConfigurationHandler::SaveAllConfiguration() {
	if(_ll.empty() == true) {
		return;
	}

	std::list<SavingInterfaceClass *>::iterator it = _ll.begin();
	if ( (*it)->isChanged() == false ) {
		cout<<"[Log] No SaveToFile Action"<< endl;
		return;
	}

	cJSON * root;
	root = cJSON_CreateObject();
	std::for_each(_ll.begin(), _ll.end(),[&](SavingInterfaceClass *n) {
		cJSON * jsonObj;
		jsonObj = n->Save();
		cJSON_AddItemToObject(root, n->GetClassFriendlyName().c_str(), jsonObj);
	});
	// File handling
	std::ofstream out;
	out.open(_path, std::ios::out);
	out << cJSON_Print(root);
	out.close();
	cJSON_Delete(root);
}
