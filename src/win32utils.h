#pragma once

#include <cstdint>
#include <string>
#include <functional>

namespace utils {

	/**
	Win32 (NT or better) implementation of ProcessIORedirectionPeer
	original code swiped from:  (c) 2002 Andreas Saurwein - saurwein@uniwares.com
	Any mistakes are mine due to the "port" over into the VCF.
	*/

	
	
	class Win32Process;

	class process {
	public:
		enum WaitResult {
			wrTimedOut,
			wrWaitFinished,
			wrWaitFailed
		};

		process(const std::function<void(const std::string&)>& cb);
		~process();

		int getProcessID();
		int getProcessThreadID();
		bool createProcess(const std::string& processName, const std::string& arguments);
		uint32_t terminate();
		WaitResult wait(uint32_t milliseconds);
		WaitResult wait();
		std::string getProcessFileName();
	private:
		Win32Process* impl;
	};


}