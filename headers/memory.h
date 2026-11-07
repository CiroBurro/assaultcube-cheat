#pragma once

#include "process.h"
#include <stdint.h>
#include <memory>
#include <Windows.h>
#include <memoryapi.h>
#include <stdexcept>
#include <span>
#include <cstring>
#include <vector>

class Memory {
private:
	Process& process;

public:
	Memory(Process& process) : process(process) {}

	const uintptr_t getModuleBaseAddress(const std::string_view moduleName) const
	{
		return process.getModuleBaseAddress(moduleName);
	}

	const Process& getProcess() const
	{
		return process;
	}

	/* Template function to read a value of type T from the specified memory address.
	* Throws a runtime error if the read operation fails.
	* 
	* Args:
	* - address: The memory address to read from.
	* 
	* Returns:
	* - The value read from the specified memory address.
	* 
	* Usage:
	* int x = mem.Read<int>(address);
	*/
	template <typename T>
	constexpr T Read(uintptr_t address) const
	{
		T value;
		SIZE_T bytesRead;
		if (!ReadProcessMemory(process.getHandle(), reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &bytesRead) ||
			bytesRead != sizeof(T)) {
			throw std::runtime_error("[!] Failed to read memory.");
		}
		return value;
	};

	/* Template function to read an array of values of type T from the specified memory address.
	* Throws a runtime error if the read operation fails.
	* 
	* Args:
	* - address: The memory address to read from.
	* 
	* Returns:
	* - The array of values read from the specified memory address.
	* 
	* Usage:
	* auto arr = mem.Read<int, 5>(address);
	*/
	template <typename T, size_t N>
	std::array<T, N> Read(uintptr_t address) const
	{
		std::array<T, N> value{};
		SIZE_T bytesRead{};

		if (!ReadProcessMemory(process.getHandle(), reinterpret_cast<LPCVOID>(address), value.data(), sizeof(value), &bytesRead) ||
			bytesRead != sizeof(value)) {
			throw std::runtime_error("[!] Failed to read array from memory.");
		}

		return value;
	}

	/* Function to read a vector of bytes of type T from the specified memory address.
	* Throws a runtime error if the read operation fails.
	* 
	* Args:
	* - address: The memory address to read from.
	* - byteCount: The number of bytes to read.
	* 
	* Returns:
	* - The vector of bytes read from the specified memory address.
	* 
	* Usage:
	* std::vector<BYTE> bytes = mem.Read(address, 10);
	*/
	std::vector<BYTE> Read(uintptr_t address, size_t byteCount) const
	{
		std::vector<BYTE> values(byteCount);
		SIZE_T bytesRead;
		if (!ReadProcessMemory(process.getHandle(), reinterpret_cast<LPCVOID>(address), values.data(), byteCount, &bytesRead) ||
			bytesRead != byteCount) {
			throw std::runtime_error("[!] Failed to read span from memory.");
		}
		return values;
	};

	/* Template function to write a value of type T to the specified memory address.
	* 
	* Args:
	* - address: The memory address to write to.
	* - value: The value to write.
	* 
	* Returns:
	* - true if the write operation was successful, false otherwise.
	* 
	* Usage:
	* mem.Write<int>(address, 10);
	*/
	template <typename T>
	bool Write(uintptr_t address, const T& value) const noexcept
	{
		SIZE_T bytesWritten;
		return (WriteProcessMemory(process.getHandle(), reinterpret_cast<LPVOID>(address), &value, sizeof(T), &bytesWritten) &&
			bytesWritten == sizeof(T));
	};

	/* Template function to write an array of values of type T to the specified memory address.
	* Args:
	* - address: The memory address to write to.
	* - value: The array of values to write.
	* 
	* Returns:
	* - true if the write operation was successful, false otherwise.
	* 
	* Usage:
	* mem.Write<int, 5>(address, std::array<int, 5>{1, 2, 3, 4, 5});
	*/
	template <typename T, size_t N>
	bool Write(uintptr_t address, const std::array<T, N>& value) const noexcept
	{
		SIZE_T bytesWritten;
		return (WriteProcessMemory(process.getHandle(), reinterpret_cast<LPVOID>(address), value.data(), sizeof(value), &bytesWritten) &&
			bytesWritten == sizeof(value));
	};

	/* Template function to write a span of values of type T to the specified memory address.
	* 
	* Args:
	* - address: The memory address to write to.
	* - values: The span of values to write.
	* 
	* Returns:
	* - true if the write operation was successful, false otherwise.
	* 
	* Usage:
	* mem.Write<int>(address, std::span<const int>{1, 2, 3, 4, 5});
	*/
	template <typename T>
	bool Write(uintptr_t address, std::span<const T> values) const noexcept
	{
		SIZE_T bytesWritten;
		return (WriteProcessMemory(process.getHandle(), reinterpret_cast<LPVOID>(address), values.data(), values.size_bytes(), &bytesWritten) &&
			bytesWritten == values.size_bytes());
	};

	/* Function to patch a memory address with the specified data.
	* 
	* Args:
	* - address: The memory address to the instruction to patch.
	* - patchData: Span of bytes (new instruction) to write.
	* 
	* Returns:
	* - true if the patch operation was successful, false otherwise.
	* 
	* Usage:
	* std::vector<BYTE> patchData = { 0x01, 0x73, 0x04 };
	* mem.Patch(address, std::span<const BYTE>{patchData});
	*/
	bool Patch(uintptr_t address, std::span<const BYTE> patchData) const noexcept
	{
		return Write(address, patchData);
	}

	/* Function to patch a memory address with the specified data (C-style string).
	* 
	* Args:
	* - address: The memory address to the instruction to patch.
	* - patchData: C-style string (new instruction) to write.
	* 
	* Returns:
	* - true if the patch operation was successful, false otherwise.
	* 
	* Usage:
	* mem.Patch(address, "\x01\x73\x04");
	*/
	bool Patch(uintptr_t address, char* patchData) const noexcept
	{
		return Write(address, std::span{ reinterpret_cast<const BYTE*>(patchData), strlen(patchData) });
	}

	/* Function to nop out a range of memory.
	* 
	* Args:
	* - address: The memory address to start nopping from.
	* - length: The number of bytes to nop out.
	* 
	* Returns:
	* - true if the nop operation was successful, false otherwise.
	* 
	* Usage:
	* mem.Nop(address, 5);
	*/
	bool Nop(uintptr_t address, size_t length) const
	{
		USHORT arch = process.getArchitecture();
		if (arch != IMAGE_FILE_MACHINE_AMD64 && arch != IMAGE_FILE_MACHINE_I386) {
			throw std::runtime_error("[!] Unsupported architecture for NOP operation.");
		}

		std::vector<BYTE> nopBytes(length, 0x90);

		return Write(address, std::span<const BYTE>{nopBytes});
	}
};