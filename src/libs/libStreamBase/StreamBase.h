#pragma once
#include "EventManager.h"
#include <afxtempl.h>
using namespace std;

class CStreamBase : public CEventManager
{
public:
	CStreamBase() { m_userStoragePath = "users.dat"; };
	CStreamBase(TCHAR * filePath,int max=50)
		//:m_pEventManager(max)
	{
		m_userStoragePath = "users.dat";
		/*modeNoTruncate 
		  Creates a new file if no file exists; otherwise, if the file already exists, it is attached to the CFile object.
		  */
		try {
			m_pPipe = new CFile(filePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite);
		}
		catch (CFileException* pEx) {
			printf("File could not be opened, cause = %d\n",
				pEx->m_cause);
			return;
		}
	}

	~CStreamBase() {
		if(m_pUserFile!= NULL)
			m_pUserFile->Close();
		m_pUserFile = NULL;
		if (m_pPipe != NULL)
			m_pPipe->Close();
		m_pPipe = NULL;
	}

	int GetCount() { return m_users.GetCount(); }
	bool StoreUsers(TCHAR * filePath = NULL);
	bool LoadUsers(TCHAR * filePath = NULL);
	void CreateStorage(TCHAR * storageLocation = NULL);

	//SetAt replaces if exists, adds if not
	void UpdateUser(CUser &user);
	CUser * GetUser(CUser & user);

	bool NotifyEventOnRead(HANDLE pipe, int * read);


	void PrintUserStorage();
	CUser * OnUserLogin(CUser & user) {
		return GetUser(user);
	}
	virtual void OnUserUpdated(CUser & user) {
		UpdateUser(user);
		StoreUsers();
	}


private:
	CMapStringToOb  m_users;
	CFile * m_pUserFile;
	CFile * 	m_pPipe;
	CEventManager * m_pEventManager;
	CString m_userStoragePath;


};

