/*
 * IsChangedSingletone.h
 *
 *  Created on: 2023. szept. 18.
 *      Author: Dell
 */

#ifndef MAIN_ISCHANGEDSINGLETONE_H_
#define MAIN_ISCHANGEDSINGLETONE_H_

class IsChangedSingletone {
private:
	bool isChanged;
	static IsChangedSingletone * instancePtr;
	IsChangedSingletone() { isChanged = false;}

public:
	IsChangedSingletone(IsChangedSingletone const&) = delete;
    void operator=(IsChangedSingletone const&)  = delete;
    static IsChangedSingletone* GetInstance();
    void setToChanged() {isChanged = true;}
    bool getIsChanged() {return isChanged;}
	virtual ~IsChangedSingletone();

};

#endif /* MAIN_ISCHANGEDSINGLETONE_H_ */
