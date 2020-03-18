#ifndef A3_EVENTMANAGER_H
#define A3_EVENTMANAGER_H


//using namespace std;
#include <vector>
#include "a3_Event.h"

class a3_EventManager
{
private:
	std::vector<a3_Event*> curEvents;

public:
	a3_EventManager();
	~a3_EventManager();

	static a3_EventManager* Instance();

	void addEvent(a3_Event* myEvent);
	void deleteEvent(a3_Event* myEvent);
	void deleteAllEvents();
	void processEvents();

	


};



#endif // ! a3_NETWORKING_EVENTMANAGER_H