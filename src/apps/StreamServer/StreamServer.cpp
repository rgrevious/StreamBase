/*
This is a modified version of "Named Pipe Server Using Overlapped I/O" from
https://docs.microsoft.com/en-us/windows/desktop/ipc/named-pipe-server-using-overlapped-i-o

This method allows multiple client connections through multiple pipe instances

The original sample code has been simplified with the CStreamEvent(Event.h)
-CStreamBase processes messages as events and processes them in EventManager.cpp or queues them by priority for later
-CStreamEvent always has a user attached to it as a CUser object so we know where it originated
-Serialization uses the built in MFC CObject classes to write and read from the pipe
-Users are stored in memory and physical storage with CMapStringToOb 
-CMapStringToOb allows file storage of CObject collections with a complexity of O(1);

*/

//#include <windows.h> //not allowed with afx.h
#include <afx.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <StreamBase.h>


#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 
#define INSTANCES 4 
#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096

typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	TCHAR chRequest[BUFSIZE];
	DWORD cbRead;
	TCHAR chReply[BUFSIZE];
	DWORD cbToWrite;
	DWORD dwState;
	BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;


VOID DisconnectAndReconnect(DWORD);
BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);
VOID GetAnswerToRequest(LPPIPEINST);
VOID GetBinaryAnswerToBinaryRequest(LPPIPEINST);


PIPEINST Pipe[INSTANCES];
HANDLE hEvents[INSTANCES];

CStreamBase * g_pstreamBase = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD i, dwWait, cbRet, dwErr;
	BOOL fSuccess = FALSE;
	

	//const LPCWSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");
	//const LPCSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");
	//const LPCSTR lpszPipename = TEXT("\\\\.\\pipe");
	TCHAR lpszPipename[512];

	if (argc > 1)
		sprintf_s(lpszPipename, 512, "\\\\.\\pipe\\%s", argv[1]);
	else
		sprintf_s(lpszPipename, 512, "\\\\.\\pipe\\%s", "testserver");

	// The initial loop creates several instances of a named pipe 
	// along with an event object for each instance.  An 
	// overlapped ConnectNamedPipe operation is started for 
	// each instance. 

	for (i = 0; i < INSTANCES; i++)
	{

		// Create an event object for this instance. 

		hEvents[i] = CreateEvent(
			NULL,    // default security attribute 
			TRUE,    // manual-reset event 
			TRUE,    // initial state = signaled 
			NULL);   // unnamed event object 

		if (hEvents[i] == NULL)
		{
			printf("CreateEvent failed with %d.\n", GetLastError());
			return 0;
		}

		Pipe[i].oOverlap.hEvent = hEvents[i];

		Pipe[i].hPipeInst = CreateNamedPipe(
			lpszPipename,            // pipe name 
			PIPE_ACCESS_DUPLEX |     // read/write access 
			FILE_FLAG_OVERLAPPED,    // overlapped mode 
			//PIPE_TYPE_MESSAGE |      // message-type pipe 
			//PIPE_READMODE_MESSAGE |  // message-read mode 
			PIPE_TYPE_BYTE | 
			PIPE_READMODE_BYTE |
			PIPE_WAIT,               // blocking mode 
			INSTANCES,               // number of instances 
			BUFSIZE * sizeof(TCHAR),   // output buffer size 
			BUFSIZE * sizeof(TCHAR),   // input buffer size 
			PIPE_TIMEOUT,            // client time-out 
			NULL);                   // default security attributes 

		if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE)
		{
			printf("CreateNamedPipe failed with %d.\n", GetLastError());
			return 0;
		}



		// Call the subroutine to connect to the new client

		Pipe[i].fPendingIO = ConnectToNewClient(
			Pipe[i].hPipeInst,
			&Pipe[i].oOverlap);

		Pipe[i].dwState = Pipe[i].fPendingIO ?
			CONNECTING_STATE : // still connecting 
			READING_STATE;     // ready to read 
	}

	g_pstreamBase = new CStreamBase(lpszPipename);

	if (argc >= 2)
		g_pstreamBase->LoadUsers(argv[1]);
	else
		g_pstreamBase->LoadUsers();

	if(g_pstreamBase->GetCount() <= 0){
		printf("Initailizing server with test users... \n");
		//add some users to the internal map object
		g_pstreamBase->UpdateUser(*(new CUser(CString("ryan"), CString("ryan.grevious@gmail.com"))));
		g_pstreamBase->UpdateUser(*(new CUser(CString("test"), CString("test.test@gmail.com"))));
		//store users in a physical storage specified by the second command line argument
		//the default will create a .dat file in the current directory
		g_pstreamBase->StoreUsers(); 
	}
	g_pstreamBase->PrintUserStorage();

	while (1)
	{
		// Wait for the event object to be signaled, indicating 
		// completion of an overlapped read, write, or 
		// connect operation. 

		dwWait = WaitForMultipleObjects(
			INSTANCES,    // number of event objects 
			hEvents,      // array of event objects 
			FALSE,        // does not wait for all 
			INFINITE);    // waits indefinitely 

	  // dwWait shows which pipe completed the operation. 

		i = dwWait - WAIT_OBJECT_0;  // determines which pipe 
		if (i < 0 || i >(INSTANCES - 1))
		{
			printf("Index out of range.\n");
			return 0;
		}

		// Get the result if the operation was pending. 

		if (Pipe[i].fPendingIO)
		{
			fSuccess = GetOverlappedResult(
				Pipe[i].hPipeInst, // handle to pipe 
				&Pipe[i].oOverlap, // OVERLAPPED structure 
				&cbRet,            // bytes transferred 
				FALSE);            // do not wait 

			switch (Pipe[i].dwState)
			{
				// Pending connect operation 
			case CONNECTING_STATE:
				if (!fSuccess)
				{
					printf("Error %d.\n", GetLastError());
					return 0;
				}
				Pipe[i].dwState = READING_STATE;
				break;

				// Pending read operation 
			case READING_STATE:
				if (!fSuccess || cbRet == 0)
				{
					DisconnectAndReconnect(i);
					continue;
				}
				Pipe[i].cbRead = cbRet;
				Pipe[i].dwState = WRITING_STATE;
				break;

				// Pending write operation 
			case WRITING_STATE:
				if (!fSuccess || cbRet != Pipe[i].cbToWrite)
				{
					DisconnectAndReconnect(i);
					continue;
				}
				Pipe[i].dwState = READING_STATE;
				break;

			default:
			{
				printf("Invalid pipe state.\n");
				return 0;
			}
			}
		}

		// The pipe state determines which operation to do next. 

		switch (Pipe[i].dwState)
		{
			// READING_STATE: 
			// The pipe instance is connected to the client 
			// and is ready to read a request from the client. 

		case READING_STATE:
		{
			
			DWORD bytesAvail = 0;
			/*
			//Option
			//serialize directly from the pipe but there are no native blocking operations without removing the data.
			//Trade off: Keep the data without an extra copy at the expense of the cpu polling
			//while(GetLastError() == ERROR_IO_PENDING && bytesAvail==0)
				//PeekNamedPipe(Pipe[i].hPipeInst, NULL, 0, NULL, &bytesAvail, NULL);
			//polling done in NotifyEventOnRead instead
			fSuccess = g_pstreamBase->NotifyEventOnRead(Pipe[i].hPipeInst, (int*)&Pipe[i].cbRead);
			*/

			//Option
			//block until data is available and copy the data somewhere else
			//Trade off: Blocks and saves cpu at the expensive adding one copy
			fSuccess = ReadFile(
				Pipe[i].hPipeInst,
				Pipe[i].chRequest,
				BUFSIZE * sizeof(TCHAR),
				&Pipe[i].cbRead,
				&Pipe[i].oOverlap);


			// The read operation completed successfully. 
			if (fSuccess && Pipe[i].cbRead != 0)
			{
				Pipe[i].fPendingIO = FALSE;
				Pipe[i].dwState = WRITING_STATE;

				continue;
			}

			// The read operation is still pending. 

			dwErr = GetLastError();
			//if (!fSuccess)
			if (!fSuccess && (dwErr == ERROR_IO_PENDING))
			{
				Pipe[i].fPendingIO = TRUE;
				continue;
			}

			// An error occurred; disconnect from the client. 

			DisconnectAndReconnect(i);
			break;

			// WRITING_STATE: 
			// The request was successfully read from the client. 
			// Get the reply data and write it to the client. 
		}
		case WRITING_STATE:
		{
			/*
			//GetAnswerToRequest(&Pipe[i]);
			GetBinaryAnswerToBinaryRequest(&Pipe[i]);
			fSuccess = WriteFile(
				Pipe[i].hPipeInst,
				Pipe[i].chReply,
				Pipe[i].cbToWrite,
				&cbRet,
				&Pipe[i].oOverlap);
			*/

			CStreamEvent requestEvent((BYTE *)Pipe[i].chRequest, Pipe[i].cbRead);
			CStreamEvent responseEvent;
			g_pstreamBase->GetResponse(requestEvent, &responseEvent);
			//or queue the event for processing later
			//g_pstreamBase->AddEvent(responseEvent);
			bool async = true;//do not wait for client response
			fSuccess = responseEvent.Send(Pipe[i].hPipeInst,async);

			// The write operation completed successfully. 

			//if (fSuccess && cbRet == Pipe[i].cbToWrite)
			if (fSuccess)
			{
				Pipe[i].fPendingIO = FALSE;
				Pipe[i].dwState = READING_STATE;
				continue;
			}

			// The write operation is still pending. 

			dwErr = GetLastError();
			if (!fSuccess && (dwErr == ERROR_IO_PENDING))
			{
				Pipe[i].fPendingIO = TRUE;
				continue;
			}

			// An error occurred; disconnect from the client. 

			DisconnectAndReconnect(i);
			break;
		}
		default:
		{
			printf("Invalid pipe state.\n");
			return 0;
		}
		}
	}

	return 0;
}


// DisconnectAndReconnect(DWORD) 
// This function is called when an error occurs or when the client 
// closes its handle to the pipe. Disconnect from this client, then 
// call ConnectNamedPipe to wait for another client to connect. 

VOID DisconnectAndReconnect(DWORD i)
{
	// Disconnect the pipe instance. 

	if (!DisconnectNamedPipe(Pipe[i].hPipeInst))
	{
		printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
	}

	// Call a subroutine to connect to the new client. 

	Pipe[i].fPendingIO = ConnectToNewClient(
		Pipe[i].hPipeInst,
		&Pipe[i].oOverlap);

	Pipe[i].dwState = Pipe[i].fPendingIO ?
		CONNECTING_STATE : // still connecting 
		READING_STATE;     // ready to read 
}

// ConnectToNewClient(HANDLE, LPOVERLAPPED) 
// This function is called to start an overlapped connect operation. 
// It returns TRUE if an operation is pending or FALSE if the 
// connection has been completed. 

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
	BOOL fConnected, fPendingIO = FALSE;

	// Start an overlapped connection for this pipe instance. 
	fConnected = ConnectNamedPipe(hPipe, lpo);

	// Overlapped ConnectNamedPipe should return zero. 
	if (fConnected)
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}

	switch (GetLastError())
	{
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING:
		fPendingIO = TRUE;
		break;

		// Client is already connected, so signal an event. 

	case ERROR_PIPE_CONNECTED:
		if (SetEvent(lpo->hEvent))
			break;

		// If an error occurs during the connect operation... 
	default:
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}
	}

	return fPendingIO;
}

VOID GetAnswerToRequest(LPPIPEINST pipe)
{
	_tprintf(TEXT("[%d] %s\n"), (int)pipe->hPipeInst, pipe->chRequest);
	StringCchCopy(pipe->chReply, BUFSIZE, TEXT("Default answer from server"));
	pipe->cbToWrite = (lstrlen(pipe->chReply) + 1) * sizeof(TCHAR);
}

VOID GetBinaryAnswerToBinaryRequest(LPPIPEINST pipe)
{
	//deserialze the read data into a usable object(CStreamEvent)
	CStreamEvent requestEvent((BYTE *)pipe->chRequest, pipe->cbRead);
	CStreamEvent responseEvent;
	g_pstreamBase->GetResponse(requestEvent, &responseEvent);
	//or queue the event for processing later
	//g_pstreamBase->AddEvent(responseEvent);
	//responseEvent.Send(pipe);
	//pipe->cbToWrite = (lstrlen(pipe->chReply) + 1) * sizeof(TCHAR);
}
