
#include "MoveInputListener.h"

MoveInputListener::MoveInputListener(a3_KeyboardKey newKey)
{
	listernKey = newKey; 
}

MoveInputListener::MoveInputListener()
{
	listernKey = a3key_question;
}

MoveInputListener::~MoveInputListener()
{

}

void MoveInputListener::SetListenerKey(a3_KeyboardKey newKey)
{
	listernKey = newKey;
}

void MoveInputListener::HandleEvent(a3_Networking_Event* event)
{

}