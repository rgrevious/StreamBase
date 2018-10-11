
#include "StreamBase.h"
#include <direct.h>  

void CStreamBase::CreateStorage(TCHAR * storageLocation) {
	TCHAR userStoragePath[512];
	if (storageLocation != NULL)
		_stprintf_s(userStoragePath, _countof(userStoragePath), _T("%s"), storageLocation);
	else
		_tcscpy_s(userStoragePath, _countof(userStoragePath), _T("test\\users.dat"));

	m_userStoragePath = CString(userStoragePath);
}

//SetAt replaces if exists, adds if not
void CStreamBase::UpdateUser(CUser &user) {
	user.SetLoggedIn(true);//if we are storing this user then assume they are registered
	m_users.SetAt(user.GetEmail(), &user);
}

CUser * CStreamBase::GetUser(CUser & user) {
	//if (puser == NULL)
		//return FALSE;
	CObject * pobject = &user;//cmaps only return base CObject pointers
	bool result = m_users.Lookup(user.GetEmail(), pobject);
	return (CUser *)pobject;
}

bool CStreamBase::NotifyEventOnRead(HANDLE pipe, int * read = NULL) {
	try {

		CFile ipc(pipe);
		CArchive ar(&ipc, CArchive::load);
		while (ar.GetFile()->GetLength() <= 0) {
			if (read != NULL)
				if (*read != 0)
					return false;
			//Sleep(1);
			return false;
		}
		//CUser user;
		//user.Serialize(ar);

		CStreamEvent event;
		event.Serialize(ar);
		cout << "received event from " << event.GetUser() << "\n";
		//m_pEventManager->ProcessEvent(event);
		if (read != NULL)
			*read = sizeof(event);
		return true;
	}
	catch (CFileException* pEx) {
		printf("File could not be opened, cause = %d\n",
			pEx->m_cause);
		return FALSE;
	}

}

bool CStreamBase::LoadUsers(TCHAR * filePath) {
	CString storagePath = filePath ? filePath : m_userStoragePath;

	if (storagePath.IsEmpty())
		return false;

	//create the path if it does not exist
	//CFile will not create the parent folders of the file.(even with modeCreate!)
	CString directory(storagePath);
	if (!directory.IsEmpty()){
		int position = storagePath.ReverseFind('\\');
		if (position >= 0) {
			directory.Truncate(position);
			//BOOL result = ::CreateDirectory(directory, nullptr);//
			if (directory.Find("\\"))
				_mkdir(directory);
		}
	}
	try {
		CFile  userFile(storagePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite);	
		m_userStoragePath = storagePath;
		//userFile.SeekToBegin();
		CArchive ar(&userFile, CArchive::load);
		if (ar.GetFile()->GetLength() > 0) {
			m_users.Serialize(ar);
			ar.Close();//required to write
						
		}
		userFile.Close();
	}
	catch (CException * pEx) {
		//pEx->ReportError();
		cout << "File storage " << storagePath << " is empty or could not be created. \n";
		return FALSE;
	}
	return TRUE;
}

bool CStreamBase::StoreUsers(TCHAR * filePath) {
	CString storagePath = filePath ? filePath : m_userStoragePath;

	if (storagePath.IsEmpty())
		return false;

	//create the path if it does not exist
	//CFile will not create the parent folders of the file.(even with modeCreate!)
	CString directory(storagePath);
	if (!directory.IsEmpty()) {
		int position = storagePath.ReverseFind('\\');
		if (position >= 0) {
			directory.Truncate(position);
			//BOOL result = ::CreateDirectory(directory, nullptr);//
			if (directory.Find("\\"))
				_mkdir(directory);
		}
	}

	try {
		CFile  userFile(storagePath, CFile::modeCreate | CFile::modeReadWrite);
		//userFile.SeekToBegin();
		CArchive ar(&userFile, CArchive::store);
		m_users.Serialize(ar);
		ar.Close();//required to write			
		userFile.Close();
	}
	catch (CException * pEx) {
		//pEx->ReportError();
		cout << "File storage " << storagePath << " is empty or could not be created. \n";
		return FALSE;
	}
	return TRUE;
}

void CStreamBase::PrintUserStorage() {
	if (m_users.GetSize() <= 0) {
		CArchive ar(m_pUserFile, CArchive::load);
		if (ar.GetFile()->GetLength() > 0) {
			m_users.Serialize(ar);
			ar.Close();
		}
	}
	POSITION rNextPosition = m_users.GetStartPosition();
	CString rKey;
	CObject * pobject;
	while (rNextPosition) {
		m_users.GetNextAssoc(rNextPosition, rKey, pobject);
		cout << "entry " << rKey << "\n";
		CUser*  puser = (CUser*)pobject;
		puser->Print();
	}
}

