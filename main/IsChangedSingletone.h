/*
 * IsChangedSingletone.h
 *
 *  Created on: 2023. szept. 18.
 *      Author: Dell
 */

#ifndef MAIN_ISCHANGEDSINGLETONE_H_
#define MAIN_ISCHANGEDSINGLETONE_H_

namespace General {

class IsChangedSingletone {
private:
	bool isChanged;
	static inline IsChangedSingletone * instancePtr = nullptr;
	IsChangedSingletone() { isChanged = false;}

public:
	IsChangedSingletone(IsChangedSingletone const&) = delete;
    void operator=(IsChangedSingletone const&)  = delete;
    static IsChangedSingletone* GetInstance();
    void setToChanged() {isChanged = true;}
    bool getIsChanged() {return isChanged;}
	virtual ~IsChangedSingletone();
	static void init();
	static void shutDown();

};
}

#endif /* MAIN_ISCHANGEDSINGLETONE_H_ */
