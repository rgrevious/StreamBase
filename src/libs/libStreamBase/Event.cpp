#include "Event.h"
IMPLEMENT_SERIAL(CStreamEvent, CObject, 1)

void CStreamEvent::Serialize(CArchive& archive)
{
	// call base class function first
	CUser::Serialize(archive);

	if (archive.IsStoring())
		archive << m_type << m_value << m_message  ;
	else
		archive >> m_type >> m_value >> m_message;
		
}

bool CStreamEvent::DeSerializeFromBuffer(BYTE * pbuffer, int size) {
	bool status = false;
	try {

		CMemFile memFile(pbuffer, size);
		CArchive ar(&memFile, CArchive::load);
		if (ar.GetFile()->GetLength() <= 0) {
		}
		Serialize(ar);
		if (!GetUser().IsEmpty())
			cout << "received event for " << GetUser() << "\n";
		else
			cout << "received event for " << GetEmail() << "\n";
		status = true;
	}
	catch (CException* pEx) {
		TCHAR error[512];
		pEx->GetErrorMessage(error, sizeof(error));
		cout << error << "\n";
		//pEx->ReportError();
		//pEx->Delete();
		status = false;
	}
	return status;

}

bool CStreamEvent::Send(HANDLE hPipe, bool async) {

	BOOL   fSuccess = TRUE;
	m_async = async;
	Write(hPipe);

	if (!async)
		fSuccess = Wait(hPipe);//block until server responds

	return fSuccess;
}

bool CStreamEvent::Wait(HANDLE hPipe, TAsyncCallBack completionRoutine) {


	BYTE  chBuf[512];
	BOOL   fSuccess = TRUE;
	DWORD  cbRead;
	if (m_WaitOnReadFunc == NULL) {
		cout << "Server Error: Read sync event not set. \n";
		return false;
	}
	fSuccess = m_WaitOnReadFunc(hPipe, chBuf, &cbRead);
	if (fSuccess && cbRead > 0)
		fSuccess = DeSerializeFromBuffer(chBuf, cbRead);
	if(completionRoutine != NULL)
		completionRoutine(hPipe,*this);
	return fSuccess;
}


bool CStreamEvent::Write(HANDLE ipc) {
	try {
		CFile pipe(ipc);
		CArchive ar(&pipe, CArchive::store);
		Serialize(ar);
		ar.Close();
		return true;
	}
	catch (CException* pEx) {
		TCHAR error[1024];
		pEx->GetErrorMessage(error, 1024);
		cout << error << "\n";
		//pEx->ReportError();
		//pEx->Delete();
		return false;
	}
}

bool CStreamEvent::Read(HANDLE ipc) {
	try {
		CFile pipe(ipc);
		CArchive ar(&pipe, CArchive::load);
		Serialize(ar);
		return true;
	}
	catch (CException* pEx) {
		TCHAR error[1024];
		pEx->GetErrorMessage(error, 1024);
		cout << error << "\n";
		//pEx->ReportError();
		//pEx->Delete();
		return false;
	}
}



