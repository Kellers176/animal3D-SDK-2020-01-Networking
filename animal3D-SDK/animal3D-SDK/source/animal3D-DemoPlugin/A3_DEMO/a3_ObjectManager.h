

#ifndef A3_OBJECT_MANAGER_H
#define A3_OBJECT_MANAGER_H

#include <vector>
#include "a3_Object.h"

class a3_ObjectManager
{
public:
	a3_ObjectManager(){}
	~a3_ObjectManager(){}

	void a3_UpdateAllObjects();
	void a3_RenderAllObjects();

	void a3_CreateNewObject();

	a3_Object a3_GetObjectInPos(int objPos) { return listOfObjects[objPos]; }

	void a3_SetObjectPos(int objID, BK_Vector2 newPos);

private:
	std::vector<a3_Object> listOfObjects;
};

#endif