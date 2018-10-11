#include "EventManager.h"
#include <iostream>
#include <string>
using namespace std;

//TODO: finish implementing the remaining EVENTTYPE events
bool  CEventManager::GetResponse(CStreamEvent & requestEvent, CStreamEvent * presponseEvent) {

	if (presponseEvent == NULL) {
		cout << "Server Error: NULL response argument \n";
		return FALSE;
	}
	//copy the user info over to the response
	CUser user = *(CUser *)&requestEvent;
	presponseEvent->SetUser(user);
	//process and update the user object
	switch (requestEvent.GetType())
	{
	case EREGISTEREVENT:
		presponseEvent->SetEvent(EACK);
		user.RegisterEvent(requestEvent.GetValue());
		OnUserUpdated(user);
		break;
	case ELOGIN:
	{
		CUser * pUser = OnUserLogin(user);//returns a pointer directly to the item in the user map
		presponseEvent->SetEvent(ELOGIN);
		if (pUser != NULL && pUser->IsRegistered()) {
			presponseEvent->SetUser(*pUser);//send back all info assosiated with this account
			presponseEvent->SetValue(TRUE);
			presponseEvent->SetMessage("Logged in.");
		}
		else {
			presponseEvent->SetValue(FALSE);
			presponseEvent->SetMessage("User not found.");
		}
		break;
	}
	case EREGISTERED://register a user account and save to file
	{
		cout << "type \"confirm " << user.GetUser() << "\" to complete " << user.GetUser() << "'s registration \n";
		//AddRequest(requestEvent);//process for later and send when the user connects again
		string command;
		string parameter;
		cin >> command >> parameter;
		if (command == "confirm") {
			OnUserUpdated(user);
			presponseEvent->SetValue(TRUE);
			presponseEvent->SetMessage(CString(user.GetUser() + " is registered and saved to storage"));
		}else {
			presponseEvent->SetValue(FALSE);
			presponseEvent->SetMessage(CString(user.GetUser() + " 's registration failed."));
		}
		break;
	}
	default:
		break;
	}
	return TRUE;
}