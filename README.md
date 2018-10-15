# StreamBase
A client/server application to manage data on a Windows system

### System Features: 

-Register and save a user on the server  
-Login with an email that was registered on the server  
-Client waits for server to confirm the registration  
-Interactive server that will prompt(cin) StreamServer.exe instance for registration confirmation  
-Multiple client connections(untested)  

### Internal Features:

-Creates user objects on the server that can be saved and modified by client or server  
-Server uses a hashmap to store users that can be serialized to a file  
-Client creates event objects on the server  
-Server can create an event on the client(this is only in the form of a message event for now)  
-Defined callbacks on both client and server handle asynchronous or synchronous events  
-Events can be stored in a priority queue based on user status and processed later(untested)  

Internal Notes:  
For the sake of time, events and ipc messages are treated as one object  

### Requirements:  
StreamServer must be started in a console first before StreamClient can connect  

### Automatic Test Case:  
Run StreamServer and/or StreamClient with out any arguments to generate, store and print default users  
StreamClient will run a command once and exit when it receives and displays a server response  

### Binaries:

/StreamBase/build/output/Windows/Win32/Release/bin

### StreamServer:  

Expanded version of "Named Pipe Server Using Overlapped I/O" from  
https://docs.microsoft.com/en-us/windows/desktop/ipc/named-pipe-server-using-overlapped-i-o

StreamServer Supported Usage:  
StreamServer  
StreamServer \<servername>  
StreamServer \<servername> \<filestoragepath>  

StreamServer Startup Actions:  
case 1: StreamServer with no arguments  
expected result:   
-users.dat created in the current directory with a few test accounts created on startup and printed  
-\\.\pipe\testserver created as the default pipe name  

case 2: StreamServer with \<servername> entered  
expected result:  
-users.dat created in the current directory with no test accounts(unless they were already created from the previous run)  
-\\.\pipe\servername created as the default pipe name  

case 3: StreamServer with \<filestoragepath> entered  
expected result: Invalid!  
-For the sake of time, if \<filestoragepath> is specified then \<servername> must be specified too   
TODO: implement command processing routine  

example:  
StreamServer ryanserver database\users.dat


### StreamClient:

Simple modified version of "Named Pipe Client" from  
https://docs.microsoft.com/en-us/windows/desktop/ipc/named-pipe-client

StreamClient Supported Usage:  
StreamClient  
StreamClient \<servername> \<command> \<param1> \<param2>...  

example:  
StreamClient testserver register ryan ryan.grevious@gmail.com  
StreamClient testserver login ryan.grevious@gmail.com  

-StreamClient is required to enter the server name as the first parameter every time  
-StreamClient will run a command once and exit once it receives and displays a server response  

Test: StreamClient with no arguments  
expected result:   
Connects to "testserver" and tries to log in with ryan.grevious@gmail.com  


### Build Instructions:

Option 1: CMake  
-run cmake from the build directory that points to StreamBase/src
eg. cd StreamBase\build\cache\Windows\cmake  
    cmake -G "Visual Studio 15 2017" ..\..\..\..\src  
The solution file will include 3 projects(StreamBase,StreamClient,StreamServer)  

Option 2: Visual Studio Solution
The solution file has already been generated at StreamBase/build/cache/Windows/ 
-Load the StreamBase Solution from /StreamBase/build/cache/Windows/  
-Set Visual Studio configuration to x86. x64 is not setup yet.  
-Both applications depend on StreamBase.lib  
-Build the solution. If successful, binaries will be built to /StreamBase/build/output/Windows/Win32    

### Sources:  
StreamClient: /StreamBase/src/apps/StreamClient  
StreamServer: /StreamBase/src/apps/StreamServer  
StreamBase:/StreamBase/src/libs/StreamBase  

Output:  
/StreamBase/build/output/Windows/Win32/Debug/  
/StreamBase/build/output/Windows/Win32/Release/
