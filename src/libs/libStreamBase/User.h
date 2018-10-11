/*
CUser is derived from the built in MFC CObject class for easy data Serialization across an ipc
The ipc can be a pipe or socket since both are type HANDLE that can be written to with CObject

reference:
https://msdn.microsoft.com/en-us/library/00hh13h0.aspx

*/


#pragma once
#include <afx.h>
#include <iostream>
using namespace std;


enum EPRIORITY
{
	ESERVER,
	EMODERATOR,
	EHOST,
	ESUBSCRIBER,
	EDONATOR,
	EUSER,
};


class CUser : public CObject
{
public:
	DECLARE_SERIAL(CUser)
	//DECLARE_DYNAMIC(CUser)
	// empty constructor is necessary
	CUser();
	CUser(CUser &user) {
		*this = user;
	}

	CUser(CString name, CString email) {
		m_registered = FALSE;
		m_name = name;
		m_email = email;
	}

	virtual ~CUser();

	CUser& operator=(CUser& user) {
		m_name = user.GetUser();
		m_email = user.GetEmail();
		m_registered = user.IsRegistered();
		m_currentChannel = user.m_currentChannel;
		return *this;
	}
	void Serialize(CArchive& archive);
	CString  GetUser() { return m_name; }//GetUserName is already taken by Win32
	CString  GetEmail() { return m_email; }
	CString  SetEmail(CString email) { return m_email=email; }
	EPRIORITY GetPriority() { return m_priority; };
	HANDLE GetIPC() { return m_ipc; }
	void SetIPC(HANDLE ipc) {  m_ipc = ipc; }
	void SetLoggedIn(BOOL success) { m_registered = success; }
	BOOL IsRegistered() { return m_registered; }

	void* operator new(size_t nSize)
	{
		return malloc(nSize);
	}
	void operator delete(void* p)
	{
		free(p);
	}
/*
	void Dump(CDumpContext &dc) const
	{
		CObject::Dump(dc);
		dc << _T("User = ") << m_name;
		dc << _T("Email = ") << m_email;
	}
*/
	void Print()
	{
		cout << "name=" << m_name << "\n";
		cout << "registered=" << m_registered << "\n";
		cout << "\n";
	}

	//the server will compare the m_events member of each user before sending the event
	void RegisterEvent(int eventType) {
		m_events |= eventType;
	}

	void DeRegisterEvent(int eventType) {
		m_events &= eventType;
	}

private:	
	CString m_email;
	CString m_name;
	bool	m_registered;//if login was successful
	EPRIORITY m_priority;
	int m_events;
	HANDLE m_ipc;//main communication handle for this user

	int		m_userId;
	int	    m_currentChannel;

};