/*
Events are derived from the built in MFC CObject class for easy data Serialization across an ipc
The ipc can be a pipe or socket since both are type HANDLE that can be written to with CObject

reference:
https://msdn.microsoft.com/en-us/library/00hh13h0.aspx

*/

#pragma once

#include "User.h"
#include <queue> 
using namespace std;

/*
Event registration and message control are combined into one as EVENTTYPE for the sake of time
CUser events can be masked and compared to when the server is ready to send an event to users
The server will only send events to users that have a matching EVENTTYPE flag
*/

//TODO: finish implementing events  
enum EVENTTYPE
{
	ESTARTED = 1,
	ELOGIN = (ESTARTED) << 1,
	EACK = (ESTARTED) << 2,  //for asynchronous events
	EEXIT = (ESTARTED) << 3,
	EREGISTEREVENT = (ESTARTED) << 4,
	EMESSAGE = (ESTARTED) << 5,
	EREGISTERED = (ESTARTED) << 6,//register user account
	ESUBSCRIBED = (ESTARTED) << 7,
	EDONATED = (ESTARTED) << 8,
	ENEWCHANNEL = (ESTARTED) << 9,
	ECHANNELEXIT = (ESTARTED) << 10,
	ESERVERCHANGE = (ESTARTED) << 11,
	ESERVEREXIT = (ESTARTED) << 12,
	ELIVESTREAMER = (ESTARTED) << 13

};


class CStreamEvent : public CUser
{
public:
	DECLARE_SERIAL(CStreamEvent)
	typedef bool(*TWaitOnRead)(HANDLE hPipe, BYTE * pBuffer, DWORD * bytesRead);
	typedef bool(*TAsyncCallBack)(HANDLE hPipe, CStreamEvent & event);//response in the form of an event

	CStreamEvent() {};
	virtual ~CStreamEvent() {};

	CStreamEvent(CStreamEvent &event) 
		:CUser(event)
	{
		m_type = event.GetType();
		m_value = event.GetValue();
		m_message = event.GetMessage();
		m_async = event.IsAsync();
	}

	CStreamEvent(HANDLE ipc) {Read(ipc);}

	CStreamEvent(CUser &user, short type) 
		:CUser(user)
	{
		SetEvent(type);
	}

	CStreamEvent(BYTE * pBuffer, int size) { DeSerializeFromBuffer(pBuffer, size); }
	bool IsAsync() { return m_async; }
	bool DeSerializeFromBuffer(BYTE * pbuffer, int size);

	bool SetEvent(short type) { 
		//TODO: check if this is a valid event
		m_type = type; 
		return true; 
	}
	void SetValue(int value) {
		m_value = value;
	}

	void SetMessage(CString message) {
		m_message = message;
	}

	void SetUser(CUser & user) {
		*(CUser *)this = user;
	}

	bool Write(HANDLE ipc); 
	bool Read(HANDLE ipc);

	void SetReadHandler(TWaitOnRead waitOnDataFunc) { m_WaitOnReadFunc = waitOnDataFunc; }
	bool Send(HANDLE hPipe,bool async=FALSE);
	bool Wait(HANDLE hPipe, TAsyncCallBack completionRoutine=NULL);

	void Serialize(CArchive& archive);
	int GetPriority() { return CUser::GetPriority(); }
	EVENTTYPE GetType() const { return (EVENTTYPE)m_type; }
	int GetValue() { return m_value; }
	CString GetMessage() { return m_message; }

	bool operator<(CStreamEvent& event)
	{
		return GetPriority() < event.GetPriority();
	}
	bool operator>(CStreamEvent& event)
	{
		return GetPriority() > event.GetPriority();
	}

private:
	short m_type;
	int m_value;
	bool m_end;//false if more events are coming
	CString m_message;
	TWaitOnRead m_WaitOnReadFunc;
	bool m_async;
	bool m_ServerResponse;//hack to differentiate the user associated with this event and server responding
};
