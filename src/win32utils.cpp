#include "win32utils.h"

#include <stdexcept>
#include <cassert>
#include <cstdlib>

#include <windows.h>
#include <process.h>


namespace utils {
	
	typedef  void* OSHandleID;

	


	class Win32Process {
	public:




		Win32Process(const std::function<void(const std::string&)>& cb);

		~Win32Process();

		int getProcessID() {
			return processID_;
		}

		int getProcessThreadID() {
			return processThreadID_;
		}

		bool createProcess(const std::string& processName, const std::string& arguments);
		std::string getProcessFileName() {
			return commandLine_;
		}

		OSHandleID getHandleID() {
			return (OSHandleID)childProcess_;
		}
		uint32_t terminate();
		process::WaitResult wait(uint32_t milliseconds);
		process::WaitResult wait();
	protected:
		std::function<void(const std::string&)> outputCallback_;


		//this is the handle to the child process that gets started
		HANDLE childProcess_;
		//ProcessWithRedirectedIO* process_;
		int processID_;
		int processThreadID_;
		UINT readThreadID_;
		std::string commandLine_;
		HANDLE readThread_;

		
		HANDLE	childStdoutRdHandle_;
		HANDLE	childStdoutWrHandle_;		
		HANDLE	savedStdoutHandle_;
		PROCESS_INFORMATION processInfo_;

		void* startInfoPtr_;

		bool canContinueReading_;

		void readPipe();

		static UINT __stdcall ReadPipeThreadProc(LPVOID pParam);
		bool testProcess();
	};


	Win32Process::Win32Process(const std::function<void(const std::string&)>& cb) :
		outputCallback_(cb),
		childProcess_(NULL),
		//process_(NULL),
		processID_(0),
		processThreadID_(0),
		readThreadID_(0),
		readThread_(NULL),		
		childStdoutRdHandle_(NULL),
		childStdoutWrHandle_(NULL),		
		savedStdoutHandle_(NULL),
		startInfoPtr_(NULL),
		canContinueReading_(false)
	{
		memset(&processInfo_, 0, sizeof(processInfo_));

		startInfoPtr_ = new STARTUPINFOA;
		memset(startInfoPtr_, 0, sizeof(STARTUPINFOA));
		((STARTUPINFOA*)startInfoPtr_)->cb = sizeof(STARTUPINFOA);


	}

	Win32Process::~Win32Process()
	{
		canContinueReading_ = false;
		WaitForSingleObject(readThread_, INFINITE);
		CloseHandle(readThread_);

		readThread_ = NULL;


		if ((DWORD)processID_ != DWORD(-1)) {
			DWORD dwExitCode;
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID_);

			// if the process handle is INVALID_HANDLEVALUE or
			// the process handle is valid but ExitCode is set
			if (!hProcess || (hProcess && GetExitCodeProcess(hProcess, &dwExitCode))) {
				processID_ = DWORD(-1);
			}
			else
				if (!CloseHandle(hProcess)) {
					printf("CloseHandle(hProcess) failed\n");
				}
		}

		delete ((STARTUPINFOA*)startInfoPtr_);
	}


	bool Win32Process::testProcess()
	{
		if ((DWORD)processID_ != DWORD(-1))
		{
			HANDLE hProcess = this->childProcess_;//::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID_);
			if (hProcess) {
				DWORD exitCode;
				GetExitCodeProcess(hProcess, &exitCode);//WaitForSingleObject(hProcess,0);// 	// fails when the process is active
				//if  (!CloseHandle(hProcess)) {
				//	throw std::runtime_error( MAKE_ERROR_MSG_2("CloseHandle(hProcess)"));
			//	}
				return (exitCode == STILL_ACTIVE) ? true : false;
			}
		}
		return false;
	}

	void Win32Process::readPipe()
	{
		DWORD bytesRead = 0;
		const int BUFSIZE = 256;
		char readBuffer[BUFSIZE + 1];

		DWORD bytesLeftToRead = 1;

		while (bytesLeftToRead > 0) {
			DWORD testBytes = 1;
			int res = PeekNamedPipe(childStdoutRdHandle_, NULL, 0, NULL, &testBytes, NULL);

			//printf("PeekNamedPipe %d, res: %d\n", (int)testBytes, res);

			if (testBytes == 0) {				
				Sleep(100);

				if (!testProcess()) {
					
					//bytesLeftToRead = 0;
					//break;
				}
			}

			if (!ReadFile(childStdoutRdHandle_, readBuffer, BUFSIZE, &bytesRead, NULL) || bytesRead == 0) {

				if (::GetLastError() == ERROR_BROKEN_PIPE)
					break;
				else
					throw std::runtime_error("ReadFile failed");
			}

			readBuffer[bytesRead / sizeof(char)] = 0;

			//notify folks of an OutputReady event
			std::string outputData = readBuffer;
			
			outputCallback_(outputData);

			Sleep(100);
			int err = PeekNamedPipe(childStdoutRdHandle_, NULL, 0, NULL, &bytesLeftToRead, NULL);


			if (!err || bytesLeftToRead == 0) {
				if (!testProcess()) {
					//break;
					bytesLeftToRead = 0;
				}
				else {

					bytesLeftToRead = 1;
				}
			}
		}

		if (bytesLeftToRead > 0) {
			//do one last read
			if (!ReadFile(childStdoutRdHandle_, readBuffer, BUFSIZE, &bytesRead, NULL) || bytesRead == 0) {
				if (::GetLastError() != ERROR_BROKEN_PIPE) {
					//??????
				}
			}

			readBuffer[bytesRead / sizeof(char)] = 0;

			//notify folks of an OutputReady event
			std::string outputData = readBuffer;
			
			//OutputReadyEvent event(process_, outputData);
			outputCallback_(outputData);
		}

		
		if (!CloseHandle(childStdoutWrHandle_)) {
			throw std::runtime_error("CloseHandle(pThis->childStdoutWrHandle_)");
		}

		
		if (!CloseHandle(childStdoutRdHandle_)) {
			throw std::runtime_error("CloseHandle(pThis->childStdoutRdHandle_)");
		}
	}

	UINT __stdcall Win32Process::ReadPipeThreadProc(LPVOID pParam)
	{
		UINT result = 0;

		Win32Process* thisPtr = (Win32Process*)pParam;
		thisPtr->readPipe();

		return result;
	}

	bool Win32Process::createProcess(const std::string& processName, const std::string& arguments)
	{
		bool result = false;

		canContinueReading_ = true;
		commandLine_ = processName + " " + arguments;

		if (NULL != readThread_) {

			::WaitForSingleObject(readThread_, INFINITE);
			::CloseHandle(readThread_);

			
			if (!::CloseHandle(childStdoutWrHandle_)) {
				throw std::runtime_error("CloseHandle(childStdoutWrHandle_) failed");
			}
			
			if (!::CloseHandle(childStdoutRdHandle_)) {
				throw std::runtime_error("CloseHandle(childStdoutRdHandle_) failed");
			}

			readThread_ = NULL;
		}

		HANDLE hChildStdoutRdTmp;
		SECURITY_ATTRIBUTES saAttr;
		memset(&saAttr, 0, sizeof(saAttr));
		BOOL bSuccess;

		// Set the bInheritHandle flag so pipe handles are inherited.
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		//from Andreas notes
		// The steps for redirecting child process's STDOUT:
		//	 1. Save current STDOUT, to be restored later.
		//	 2. Create anonymous pipe to be STDOUT for child process.
		//	 3. Set STDOUT of the parent process to be write handle to
		//		the pipe, so it is inherited by the child process.
		//	 4. Create a noninheritable duplicate of the read handle and
		//		close the inheritable read handle.

		// Save the handle to the current STDOUT.
		savedStdoutHandle_ = ::GetStdHandle(STD_OUTPUT_HANDLE);

		// Create a pipe for the child process's STDOUT.
		if (!::CreatePipe(&hChildStdoutRdTmp, &childStdoutWrHandle_, &saAttr, 0)) {
			throw std::runtime_error("Stdout pipe creation failed");
		}

		// Set a write handle to the pipe to be STDOUT.
		if (!::SetStdHandle(STD_OUTPUT_HANDLE, childStdoutWrHandle_)) {
			throw std::runtime_error("Redirecting STDOUT failed");
		}

		// Create noninheritable read handle and close the inheritable read handle.
		bSuccess = ::DuplicateHandle(::GetCurrentProcess(), hChildStdoutRdTmp,
			::GetCurrentProcess(), &childStdoutRdHandle_,
			0, FALSE, DUPLICATE_SAME_ACCESS);
		if (!bSuccess) {
			throw std::runtime_error("DuplicateHandle failed");
		}

		if (!::CloseHandle(hChildStdoutRdTmp)) {
			throw std::runtime_error("CloseHandle(hChildStdoutRdTmp)");
		}

		

		//now start the child process - this will start a new cmd with the
		//the cmd line sent to it
		childProcess_ = NULL;


		std::string tmp = std::getenv("ComSpec"); //System::getEnvironmentVariable(L"ComSpec");

		if (tmp.empty()) {
			result = false;
		}
		else {
			std::string shellCmdLine = tmp;
			shellCmdLine += " /A /C ";
			shellCmdLine += commandLine_;

			//printf("running: %s\n", shellCmdLine.c_str());


			BOOL retVal = FALSE;

			
			((STARTUPINFOA*)startInfoPtr_)->dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
			((STARTUPINFOA*)startInfoPtr_)->wShowWindow = SW_HIDE;
			
			((STARTUPINFOA*)startInfoPtr_)->hStdOutput = childStdoutWrHandle_;
			((STARTUPINFOA*)startInfoPtr_)->hStdError = childStdoutWrHandle_;

			char* cmdLineBuf = new char[shellCmdLine.size()+1];
			memset(cmdLineBuf, 0, shellCmdLine.size()+1);
			shellCmdLine.copy(cmdLineBuf, shellCmdLine.size());


			retVal = CreateProcessA(NULL,
				cmdLineBuf, 	   // applicatin name
				NULL, 		  // process security attributes
				NULL, 		  // primary thread security attributes
				TRUE, 		  // handles are inherited
				CREATE_NEW_CONSOLE, // creation flags
				NULL, 		  // use parent's environment
				NULL, 		  // use parent's current directory
				(STARTUPINFOA*)startInfoPtr_,  // STARTUPINFO pointer
				&processInfo_);  // receives PROCESS_INFORMATION

			

			if (retVal) {
				processID_ = processInfo_.dwProcessId;
				childProcess_ = processInfo_.hProcess;
				processThreadID_ = processInfo_.dwThreadId;
			}
			else {
				processID_ = GetLastError();
				DebugBreak();
			}


			readThread_ = (HANDLE)_beginthreadex(NULL,
				0,
				Win32Process::ReadPipeThreadProc,
				(void*)this,
				CREATE_SUSPENDED,
				&readThreadID_);


			::SetThreadPriority(readThread_, THREAD_PRIORITY_BELOW_NORMAL);
			//read thread is started
			::ResumeThread(readThread_);

			if (!readThread_) {
				delete[]cmdLineBuf;
				throw std::runtime_error("Cannot start read-redirect thread!");
			}




			WaitForSingleObject(childProcess_, INFINITE);

			//readPipe();


			CloseHandle(childProcess_);



			// After process creation, restore the saved STDIN and STDOUT.
			
			if (!SetStdHandle(STD_OUTPUT_HANDLE, savedStdoutHandle_)) {
				delete[] cmdLineBuf;
				throw std::runtime_error("Re-redirecting Stdout failed");
			}
				

			canContinueReading_ = false;

			::WaitForSingleObject(readThread_, INFINITE);
			::CloseHandle(readThread_);
			readThread_ = NULL;
			childProcess_ = NULL;
			processID_ = 0;

			delete[] cmdLineBuf;

			result = retVal ? true : false;
		}

		canContinueReading_ = false;

		return result;
	}


	uint32_t Win32Process::terminate()
	{
		TerminateProcess(processInfo_.hProcess, -1);
		return -1;
	}

	process::WaitResult Win32Process::wait(uint32_t milliseconds)
	{
		process::WaitResult result(process::wrWaitFailed);

		DWORD  res = WaitForSingleObject(processInfo_.hProcess, milliseconds);
		if (WAIT_TIMEOUT == res) {
			result = process::wrTimedOut;
		}
		else if (WAIT_OBJECT_0 == res) {
			result = process::wrWaitFinished;
		}

		return result;
	}

	process::WaitResult Win32Process::wait()
	{
		process::WaitResult result(process::wrWaitFailed);

		DWORD  res = WaitForSingleObject(processInfo_.hProcess, INFINITE);
		if (WAIT_TIMEOUT == res) {
			result = process::wrTimedOut;
		}
		else if (WAIT_OBJECT_0 == res) {
			result = process::wrWaitFinished;
		}

		return result;
	}







	process::process(const std::function<void(const std::string&)>& cb)
	{
		impl = new Win32Process(cb);
	}

	process::~process()
	{
		delete impl;
	}

	int process::getProcessID() 
	{
		return impl->getProcessID();
	}

	int process::getProcessThreadID()
	{
		return impl->getProcessThreadID();
	}

	bool process::createProcess(const std::string& processName, const std::string& arguments) 
	{ 
		return impl->createProcess(processName, arguments);
	}

	uint32_t process::terminate() 
	{ 
		return impl->terminate(); 
	}

	process::WaitResult process::wait(uint32_t milliseconds) 
	{ 
		return impl->wait(milliseconds); 
	}

	process::WaitResult process::wait() 
	{ 
		return impl->wait();
	}

	std::string process::getProcessFileName() 
	{ 
		return impl->getProcessFileName(); 
	}
}