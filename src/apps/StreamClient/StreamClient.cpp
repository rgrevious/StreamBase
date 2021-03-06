/*
This is a modified version of "Named Pipe Client" from
https://docs.microsoft.com/en-us/windows/desktop/ipc/named-pipe-client

The original sample code has been simplified with the CStreamEvent(Event.h)
-CStreamEvent wraps command line arguments and sends them over the pipe asynchonously or synchronously.
-CStreamEvent always has a user attached to it as a CUser object so we know where it originated
-Serialization uses the built in MFC CObject classes to write and read from the pipe
*/


#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <StreamBase.h>

#define BUFSIZE 512


//callback for CStreamEvent class to wait for responses from the server.
//Not part of the server event registration requirement
bool WaitOnRead(HANDLE hPipe, BYTE * pBuffer, DWORD * bytesRead) {

	BOOL   fSuccess = FALSE;

	if (hPipe == NULL) {
		printf("Client Error: Null pipe  \n");
		return FALSE;
	}
	if (pBuffer == NULL) {
		printf("Client Error: Null pipe buffer \n");
		return FALSE;
	}

	if (bytesRead == NULL) {
		printf("Client Error: Invalid pipe parameter \n");
		return FALSE;
	}

	// Read from the pipe. 
	fSuccess = ReadFile(
		hPipe,    // pipe handle 
		pBuffer,    // buffer to receive reply 
		BUFSIZE * sizeof(TCHAR),  // size of buffer 
		bytesRead,  // number of bytes read 
		NULL);    // not overlapped 

	return fSuccess;
}

/*
	send messages wrapped in events to the server.
	async = true: event will send with out waiting for a response and Wait() can be called in another thread
	async = false: event will send and block until a response is received
*/
bool SendEvent(HANDLE hPipe, CStreamEvent & event) {
	if (hPipe == NULL) {
		printf("Client Error: Null pipe  \n");
		return FALSE;
	}

	event.SetReadHandler(WaitOnRead);//event that will signal when data is available on the ipc
	BOOL async = true;// do not wait for a response
	BOOL fSuccess = event.Send(hPipe, async);
	if(!event.GetUser().IsEmpty())
		printf("Sending event as %s \n", event.GetUser().GetBuffer());
	else
		printf("Sending event as %s \n", event.GetEmail().GetBuffer());
	if(async)
		event.Wait(hPipe);//the event object will update itself from the server response
	if (event.GetValue() > 0)
		cout << "Server Response: " << event.GetMessage() << "\n";
	else
		cout << "Server Error: " << event.GetMessage() << "\n";

	return fSuccess;
}

//forward declaration for cleanup
void Close(HANDLE hPipe);

int _tmain(int argc, TCHAR *argv[])
{
	HANDLE hPipe;
	BOOL   fSuccess = FALSE;
	DWORD  dwMode;



	//set the string type base on Character Set in project settings
	//const LPCWSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");//Unicode
	//const LPCSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");//Multibyte

	TCHAR lpszPipename[512];
	if (argc < 2) {
		cout << "Initializing with default server name \"testserver\" \n";
		sprintf_s(lpszPipename, 512, "\\\\.\\pipe\\%s", _T("testserver"));
	}else {
		sprintf_s(lpszPipename, 512, "\\\\.\\pipe\\%s", argv[1]);
	}

	// Try to open a named pipe; wait for it, if necessary. 

	while (1)
	{
		hPipe = CreateFile(
			lpszPipename,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

	  // Break if the pipe handle is valid. 

		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 

		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
			return -1;
		}

		// All pipe instances are busy, so wait for 20 seconds. 

		if (!WaitNamedPipe(lpszPipename, 20000))
		{
			printf("Could not open pipe: 20 second wait timed out.");
			return -1;
		}
	}

	// The pipe connected; change to byte-read mode for CObject Serialization. 

	dwMode = PIPE_READMODE_BYTE;//PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if (!fSuccess)
	{
		_tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
		return -1;
	}

	//TODO: create command processing function

	// Send a test message wrapped in an event/user object to the pipe server. 
	if (argc < 2) {
		cout << "Initailizing client with a test user... \n";
		CUser user(CString("ryan"), CString("ryan.grevious@gmail.com"));
		CStreamEvent event(user, ELOGIN);//the server should respond with the user info stored on disk
		cout << "Logging on as " << event.GetEmail() << " with " << event.GetUser() << " \n";
		fSuccess = SendEvent(hPipe, event);
	}
	if (argc > 2) {
		if (strcmp(argv[2], "login") == 0) {
			CString email = argv[3];
			if (email.IsEmpty()) {
				printf("Please provide a valid email address \n \
					For example: StreamClient.exe ryan.grevious@gmail.com \n");
				Close(hPipe);
				return -1;
			}
			CUser user(CString(""), email);
			CStreamEvent event(user, ELOGIN);//the server should respond with the user info stored on disk
			cout << "Logging on with " << event.GetUser() << " \n";
			fSuccess = SendEvent(hPipe, event);
		}


		if (strcmp(argv[2], "register") == 0) {
			if (argc < 5) {
				printf("Please provide a user name followed by an email address \n \
					For example: StreamClient.exe ryan ryan.grevious@gmail.com \n");
				Close(hPipe);
				return -1;
			}
			CString email = argv[4];
			if (email.IsEmpty()) {
				printf("Please provide a valid email address \n \
					For example: StreamClient.exe ryan ryan.grevious@gmail.com \n");
				Close(hPipe);
				return -1;
			}
			CUser user(CString(argv[3]), email);
			CStreamEvent event(user, EREGISTERED);//the server should respond with the user info stored on disk
			cout << "Registering " << event.GetUser() << " with " << event.GetEmail()  << " \n";
			fSuccess = SendEvent(hPipe, event);
		}
	}


	Close(hPipe);
	return !fSuccess;
}

void Close(HANDLE hPipe) {
	FlushFileBuffers(hPipe);
	CloseHandle(hPipe);
}

