#pragma once

#include "utils.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <wow64apiset.h>
#include <string_view>
#include <stdexcept>

class Process
{
private:
	DWORD pid;
	std::string_view name;
	AutoReleaseHandle handle;
	USHORT architecture; // 32 or 64 bit
public:
	explicit Process(std::string_view processName) : name(processName), pid(0), handle(nullptr), architecture(0) {
		// Create a snapshot of all processes in the system
		auto snapshot = AutoReleaseHandle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
		if (!snapshot) {
			throw std::runtime_error("[!] Failed to create processes snapshot.");
		}

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);
		
		// Iterate through the process list to find the process with the specified name
		if (!Process32First(snapshot.get(), &entry)) {
			throw std::runtime_error("[!] Failed to retrieve first process entry.");
		}

		do {
			if (name == entry.szExeFile) {
				pid = entry.th32ProcessID;
				
				handle = (AutoReleaseHandle(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid)));
				
				if (!handle) {
					throw std::runtime_error("[!] Failed to open process.");
				}

				USHORT processMachine = 0;
				USHORT nativeMachine = 0;

				if (IsWow64Process2(handle.get(), &processMachine, &nativeMachine)) {
					architecture = (processMachine == IMAGE_FILE_MACHINE_UNKNOWN)
						? nativeMachine
						: processMachine;
				}
				else {
					throw std::runtime_error("[!] Failed to determine process architecture.");
				}

				break;
			}
		} while (Process32Next(snapshot.get(), &entry));
		
		if (pid == 0 || !handle) {
			throw std::runtime_error("[!] Process not found.");
		}
	}

	const std::uintptr_t getModuleBaseAddress(const std::string_view moduleName) const {
		// Create a snapshot of all modules in the process
		auto snapshot = AutoReleaseHandle(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid));
		if (!snapshot) {
			throw std::runtime_error("[!] Failed to create modules snapshot.");
		}

		MODULEENTRY32 entry;
		entry.dwSize = sizeof(MODULEENTRY32);

		// Iterate through the modules list to find the module with the specified name
		if (!Module32First(snapshot.get(), &entry)) {
			throw std::runtime_error("[!] Failed to retrieve first module entry.");
		}

		do {
			if (moduleName == entry.szModule) {
				return reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
			}
		} while (Module32Next(snapshot.get(), &entry));

		throw std::runtime_error("[!] Module not found.");
	}

	~Process() = default;

	// Getters
	DWORD getPid() const {
		return pid;
	}
	std::string_view getName() const {
		return name;
	}
	HANDLE getHandle() const {
		return handle.get();
	}
	USHORT getArchitecture() const {
		return architecture;
	}

	// Delete copy constructor and copy assignment operator to prevent copying
	Process(const Process&) = delete;
	Process& operator=(const Process&) = delete;

	// Bool conversion operator to check if the handle is valid
	explicit operator bool() const {
		return pid != 0 && handle;
	}

};