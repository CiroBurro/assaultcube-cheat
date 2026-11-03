#pragma once

#include "process.h"
#include <stdint.h>
#include <memory>
#include <Windows.h>
#include <memoryapi.h>
#include <span>
#include <vector>

namespace mem {
	// Wrapper functions for ReadProcessMemory

	// Template function to read a single value of type T from the specified address in the target process
	template <typename T>
	bool read(Process& process, uintptr_t address, T& value) noexcept
	{
		return ReadProcessMemory(process.getHandle(), reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr) != 0;
	};

	// Template function to read an array of values of type T from the specified address in the target process
	template <typename T, size_t N>
	bool read(Process& process, uintptr_t address, T(&value)[N]) noexcept
	{
		return ReadProcessMemory(process.getHandle(), reinterpret_cast<LPCVOID>(address), value, sizeof(T) * N, nullptr) != 0;
	};

	// Template function to read a span of values of type T from the specified address in the target process
	template <typename T>
	bool read(Process& process, uintptr_t address, std::span<T> values) noexcept
	{
		return ReadProcessMemory(process.getHandle(), reinterpret_cast<LPCVOID>(address), values.data(), values.size_bytes(), nullptr) != 0;
	};


	// Wrapper functions for WriteProcessMemory

	// Template function to write a single value of type T to the specified address in the target process
	template <typename T>
	bool write(Process& process, uintptr_t address, const T& value) noexcept
	{
		return WriteProcessMemory(process.getHandle(), reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr) != 0;
	};

	// Template function to write an array of values of type T to the specified address in the target process
	template <typename T, size_t N>
	bool write(Process& process, uintptr_t address, const T(&value)[N]) noexcept
	{
		return WriteProcessMemory(process.getHandle(), reinterpret_cast<LPVOID>(address), value, sizeof(T) * N, nullptr) != 0;
	};

	// Template function to write a span of values of type T to the specified address in the target process
	template <typename T>
	bool write(Process& process, uintptr_t address, std::span<const T> values) noexcept
	{
		return WriteProcessMemory(process.getHandle(), reinterpret_cast<LPVOID>(address), values.data(), values.size_bytes(), nullptr) != 0;
	};

	// Functions to patch instructions  in the target process
	bool patch(Process &process, uintptr_t address, std::span<const BYTE> patchData)
	{
		return write(process, address, patchData);
	}
	bool patch(Process& process, uintptr_t address, char* patchData)
	{
		return write(process, address, std::span{reinterpret_cast<const BYTE*>(patchData), strlen(patchData)});
	}

	// Function to fill a region of memory in the target process with NOP instructions
	bool nop(Process& process, uintptr_t address, size_t length)
	{
		USHORT arch = process.getArchitecture();
		if (arch != IMAGE_FILE_MACHINE_AMD64 && arch != IMAGE_FILE_MACHINE_I386) {
			throw std::runtime_error("[!] Unsupported architecture for NOP operation.");
		}

		std::vector<BYTE> nopBytes(length, 0x90);

		return write(process, address, std::span<const BYTE>{nopBytes});
	}
}