/*
* Modern effects for a modern Streamer
* Copyright (C) 2017 Michael Fabian Dirks
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "util-crashmanager.h"
#include "nodeobs_api.h"
#include <iostream>
#include <sstream>
#include <obs.h>
#include <map>
#include <vector>
#include <queue>
#include <string>

#if defined(_WIN32)

#include "Shlobj.h"
#include <WinBase.h>
#include "DbgHelp.h"
#pragma comment(lib, "Dbghelp.lib")
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <windows.h>

#endif

// Global/static variables
util::CrashManager::CrashpadInfo*	s_CrashpadInfo = nullptr;
std::map<std::string, std::string>	s_CustomAnnotations;
std::vector<std::string>            s_HandledOBSCrashes;

// Forward
std::string FormatVAString(const char* const format, va_list args);
std::string RewindCallStack(uint32_t maximumEntries);
std::string RequestOBSLog();

// Class specific
util::CrashManager::~CrashManager()
{
	if (s_CrashpadInfo != nullptr)
		delete s_CrashpadInfo;
}

bool util::CrashManager::Initialize() 
{
#ifndef _DEBUG

	if (!SetupCrashpad())
		return false;

	// Handler for obs errors (mainly for bcrash() calls)
	base_set_crash_handler(
	    [](const char* format, va_list args, void* param) {

			std::string errorMessage = FormatVAString(format, args);

		    // Check if this crash error is handled internally (if this is a known
			// error that we can't do anything about it, just let the application
			// crash normally
		    if (!TryHandledCrash(std::string(format), errorMessage))
			    HandleCrash(errorMessage);
		},
	    nullptr);

	// Redirect all the calls from std::terminate
	std::set_terminate([](){
		HandleCrash("Direct call to std::terminate");
	});

#if defined(_WIN32)

	// Setup the windows exeption filter to 
	SetUnhandledExceptionFilter([](struct _EXCEPTION_POINTERS * ExceptionInfo) {

		/* don't use if a debugger is present */
		if (IsDebuggerPresent())
			return LONG(EXCEPTION_CONTINUE_SEARCH);
		
		HandleCrash("UnhandledExceptionFilter");

		// Unreachable statement
		return LONG(EXCEPTION_CONTINUE_SEARCH);
	});

#endif

	// The atexit will check if obs was safelly closed
	std::atexit(HandleExit);
	std::at_quick_exit(HandleExit);

#endif

	return true;
}

void util::CrashManager::Configure()
{
	// Add all obs crash messages that are supposed to be handled by our application and
	// shouldn't cause a crash report (because there is no point on reporting since we
	// cannot control them)
	{
		s_HandledOBSCrashes.push_back("Failed to recreate D3D11");
		// ...
	}
}

bool util::CrashManager::SetupCrashpad()
{
#ifndef _DEBUG

	std::wstring appdata_path;

#if defined(_WIN32)
	HRESULT hResult;
	PWSTR   ppszPath;

	hResult = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &ppszPath);
	appdata_path.assign(ppszPath);
	appdata_path.append(L"\\obs-studio-node-server");
	CoTaskMemFree(ppszPath);

#endif

	std::map<std::string, std::string> annotations;
	std::vector<std::string>           arguments;
	arguments.push_back("--no-rate-limit");
	std::wstring handler_path(L"crashpad_handler.exe");
	std::string  url(
        "https://submit.backtrace.io/streamlabs/513fa5577d6a193ed34965e18b93d7b00813e9eb2f4b0b7059b30e66afebe4fe/"
        "minidump");

	base::FilePath db(appdata_path);
	base::FilePath handler(handler_path);

	// Setup the crashpad info that will be used in case the obs throws an error
	s_CrashpadInfo            = new CrashpadInfo();
	s_CrashpadInfo->handler   = handler;
	s_CrashpadInfo->db        = db;
	s_CrashpadInfo->url       = url;
	s_CrashpadInfo->arguments = arguments;

	s_CrashpadInfo->database = crashpad::CrashReportDatabase::Initialize(db);
	if (s_CrashpadInfo->database == nullptr || s_CrashpadInfo->database->GetSettings() == NULL)
		return false;

	s_CrashpadInfo->database->GetSettings()->SetUploadsEnabled(true);

	if (!s_CrashpadInfo->client.StartHandler(handler, db, db, url, annotations, arguments, true, true))
		return false;

	if (!s_CrashpadInfo->client.WaitForHandlerStart(INFINITE))
		return false;
#endif

	return true;
}

void util::CrashManager::HandleExit() noexcept
{
	// If we are exiting normally and obs is active, we have a problem because some
	// modules and threads could potentially be running and this will result in a
	// crash that is masked. The real issue here is why we are exiting without
	// finishing the obs first.
	if (obs_initialized()) {
		
		// Proceed to add more info to our crash reporter and finish with a call to
		// abort since we never should close obs without finishing it first.
		HandleCrash("AtExit");
	}
}

void util::CrashManager::HandleCrash(std::string _crashInfo) noexcept 
{
	// If for any reason this is true, it means that we are crashing inside this same
	// method, if that happens just call abort and ignore any remaining processing since
	// we cannot continue.
	static bool insideCrashMethod = false;
	if (insideCrashMethod)
		abort();

	insideCrashMethod = true;

	// This will manually rewind the callstack (using the input as the maximum number of
	// entries to retrieve), we will use this info to populate an crashpad attribute, 
	// avoiding some cases that the memory dump is corrupted and we don't have access to 
	// the callstack.
	std::string callStack = RewindCallStack(18);

	// Setup all the custom annotations that are important too our crash report
	s_CustomAnnotations.insert({"Crash Info", _crashInfo});
	s_CustomAnnotations.insert({"CallStack", callStack});
	s_CustomAnnotations.insert({"OBS Status", obs_initialized() ? "Initialized" : "Shutdown"});
	s_CustomAnnotations.insert({"OBS Total Leaks", std::to_string(bnum_allocs())});
	s_CustomAnnotations.insert({"OBS Log", RequestOBSLog()});

	// Since there is no other way (known at the moment) to add annotations on the go to 
	// the current handler, we will recreate it here adding our custom info
	bool rc = s_CrashpadInfo->client.StartHandler(
	    s_CrashpadInfo->handler,
	    s_CrashpadInfo->db,
	    s_CrashpadInfo->db,
	    s_CrashpadInfo->url,
	    s_CustomAnnotations,
	    s_CrashpadInfo->arguments,
	    true,
	    true);
	rc = s_CrashpadInfo->client.WaitForHandlerStart(5000);

	// Finish with a call to abort since there is no point on continuing
	abort();

	// Unreachable statement
	insideCrashMethod = false;
}

bool util::CrashManager::TryHandledCrash(std::string _format, std::string _crashMessage)
{
	bool crashIsHandled = false;
	for (auto& handledCrashes : s_HandledOBSCrashes) {
		if (std::string(_format).find(handledCrashes) != std::string::npos) {		
			crashIsHandled = true;
			break;
		}
	}

	if (!crashIsHandled)
		return false;

	// If we are here this is a known crash that we don't wanna propagate to the crashpad 
	// (because we don't need to handle it or we cannot control it), the ideal would be just
	// a call to stop the crashpad handler but since it doesn't have a method like this we
	// can try to finish the application normally to avoid any crash report
	if (s_CrashpadInfo != nullptr) {
		delete s_CrashpadInfo;
		s_CrashpadInfo = nullptr;
	}

	// Optionally send a message to the user
	if (false) {
		std::string errorMessage = std::string(
		    "The Streamlabs OBS encontered an internal error and will now close, if you are receiving this "
		    "type of error frequently please contact our support! Error message: "
		    + _crashMessage);

		MessageBox(
		    nullptr,
		    std::wstring(errorMessage.begin(), errorMessage.end()).c_str(),
		    TEXT("Streamlabs OBS Crash"),
		    MB_OK);
	}

	// If we cannot destroy the obs and exit normally without causing a crash report, 
	// proceed with a crash
	try {
		OBS_API::destroyOBS_API();
		exit(0);
	} catch (...) {
		util::CrashManager::HandleCrash(_crashMessage);
	}
	
	// Unreachable statement
	return true;
}

// Format a var arg string into a c++ std::string type
std::string FormatVAString(const char* const format, va_list args)
{
	auto temp   = std::vector<char>{};
	auto length = std::size_t{63};
	while (temp.size() <= length) {
		temp.resize(length + 1);
		const auto status = std::vsnprintf(temp.data(), temp.size(), format, args);
		if (status < 0)
			throw std::runtime_error{"string formatting error"};
		length = static_cast<std::size_t>(status);
	}
	return std::string{temp.data(), length};
}

std::string RewindCallStack(uint32_t maximumEntries)
{
#ifndef _DEBUG
#if defined(_WIN32)

	// Get the function to rewing the callstack
	typedef USHORT(WINAPI * CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
	CaptureStackBackTraceType func =
	    (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary(L"kernel32.dll"), "RtlCaptureStackBackTrace"));
	if (func == NULL)
		return ""; // WOE 29.SEP.2010

	// Quote from Microsoft Documentation:
	// ## Windows Server 2003 and Windows XP:
	// ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
	const int      kMaxCallers = 62;
	void*          callers_stack[kMaxCallers];
	unsigned short frames;
	SYMBOL_INFO*   symbol;
	HANDLE         process;

	// Get the callstack information
	process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);
	frames               = (func)(0, kMaxCallers, callers_stack, NULL);
	symbol               = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen   = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	std::string callStackString;

	// Currently 18 is the maximum that we can display on backtrace in one single attribute
	const unsigned short MAX_CALLERS_SHOWN = maximumEntries;
	frames                                 = frames < MAX_CALLERS_SHOWN ? frames : MAX_CALLERS_SHOWN;
	for (unsigned int i = 0; i < frames; i++) {
		SymFromAddr(process, (DWORD64)(callers_stack[i]), 0, symbol);

		// Setup a readable callstack string
		std::stringstream buffer;
		buffer << callers_stack[i] << " " << symbol->Name << " - 0x" << symbol->Address << std::endl;
		callStackString.append(symbol->Name + (i == frames - 1 ? "" : std::string(" -> ")));
	}

	free(symbol);

	return callStackString;

#endif
#endif

	return "";
}

std::string RequestOBSLog()
{
	// Setup the obs log queue as an attribute
	if (OBS_API::getOBSLogQueue().size() > 0) {
		std::queue<std::string> obsLogQueue = OBS_API::getOBSLogQueue();
		std::string             obsLogMessage;
		while (obsLogQueue.size() > 0) {
			obsLogMessage.append(obsLogQueue.front().append(obsLogQueue.size() == 1 ? " " : " -> "));
			obsLogQueue.pop();
		}

		return obsLogMessage;
	}

	return "";
}

void BindCrtHandlesToStdHandles(bool bindStdIn, bool bindStdOut, bool bindStdErr)
{
	// Re-initialize the C runtime "FILE" handles with clean handles bound to "nul". We do this because it has been
	// observed that the file number of our standard handle file objects can be assigned internally to a value of -2
	// when not bound to a valid target, which represents some kind of unknown internal invalid state. In this state our
	// call to "_dup2" fails, as it specifically tests to ensure that the target file number isn't equal to this value
	// before allowing the operation to continue. We can resolve this issue by first "re-opening" the target files to
	// use the "nul" device, which will place them into a valid state, after which we can redirect them to our target
	// using the "_dup2" function.
	if (bindStdIn) {
		FILE* dummyFile;
		freopen_s(&dummyFile, "nul", "r", stdin);
	}
	if (bindStdOut) {
		FILE* dummyFile;
		freopen_s(&dummyFile, "nul", "w", stdout);
	}
	if (bindStdErr) {
		FILE* dummyFile;
		freopen_s(&dummyFile, "nul", "w", stderr);
	}

	// Redirect unbuffered stdin from the current standard input handle
	if (bindStdIn) {
		HANDLE stdHandle = GetStdHandle(STD_INPUT_HANDLE);
		if (stdHandle != INVALID_HANDLE_VALUE) {
			int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
			if (fileDescriptor != -1) {
				FILE* file = _fdopen(fileDescriptor, "r");
				if (file != NULL) {
					int dup2Result = _dup2(_fileno(file), _fileno(stdin));
					if (dup2Result == 0) {
						setvbuf(stdin, NULL, _IONBF, 0);
					}
				}
			}
		}
	}

	// Redirect unbuffered stdout to the current standard output handle
	if (bindStdOut) {
		HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (stdHandle != INVALID_HANDLE_VALUE) {
			int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
			if (fileDescriptor != -1) {
				FILE* file = _fdopen(fileDescriptor, "w");
				if (file != NULL) {
					int dup2Result = _dup2(_fileno(file), _fileno(stdout));
					if (dup2Result == 0) {
						setvbuf(stdout, NULL, _IONBF, 0);
					}
				}
			}
		}
	}

	// Redirect unbuffered stderr to the current standard error handle
	if (bindStdErr) {
		HANDLE stdHandle = GetStdHandle(STD_ERROR_HANDLE);
		if (stdHandle != INVALID_HANDLE_VALUE) {
			int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
			if (fileDescriptor != -1) {
				FILE* file = _fdopen(fileDescriptor, "w");
				if (file != NULL) {
					int dup2Result = _dup2(_fileno(file), _fileno(stderr));
					if (dup2Result == 0) {
						setvbuf(stderr, NULL, _IONBF, 0);
					}
				}
			}
		}
	}

	// Clear the error state for each of the C++ standard stream objects. We need to do this, as attempts to access the
	// standard streams before they refer to a valid target will cause the iostream objects to enter an error state. In
	// versions of Visual Studio after 2005, this seems to always occur during startup regardless of whether anything
	// has been read from or written to the targets or not.
	if (bindStdIn) {
		std::wcin.clear();
		std::cin.clear();
	}
	if (bindStdOut) {
		std::wcout.clear();
		std::cout.clear();
	}
	if (bindStdErr) {
		std::wcerr.clear();
		std::cerr.clear();
	}
}

void util::CrashManager::OpenConsole()
{
#if defined(_WIN32)

	// Allocate a console window for this process
	AllocConsole();

	// Update the C/C++ runtime standard input, output, and error targets to use the console window
	BindCrtHandlesToStdHandles(true, true, true);

#endif
}