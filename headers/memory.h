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

	/* @brief Gets the base address of a module in the process.
	*
	* @param moduleName The name of the module.
	*
	* @return The base address of the specified module.
	*/
	const uintptr_t getModuleBaseAddress(const std::string_view moduleName) const
	{
		return process.getModuleBaseAddress(moduleName);
	}

	/* @brief Gets the process associated with this memory object.
	*
	* @return A constant reference to the associated process.
	*/
	const Process& getProcess() const
	{
		return process;
	}

	/* @brief Template function to read a value of type T from the specified memory address.
	* @code
	* int x = mem.Read<int>(address);
	* @endcode
	*	
	* @param address The memory address to read from.
	* 
	* @return The value read from the specified memory address.
	* 
	* @throws std::runtime_error If the read operation fails.
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

	/* @brief Template function to read an array of values of type T from the specified memory address.
	* @code
	* auto arr = mem.Read<int, 5>(address);
	* @endcode
	*
	* @param address The memory address to read from.
	*
	* @return The array of values read from the specified memory address.
	*
	* @throws std::runtime_error If the read operation fails.
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

	/* @brief Function to read a vector of bytes from the specified memory address.
	* @code
	* std::vector<BYTE> bytes = mem.Read(address, 10);
	* @endcode
	*
	* @param address The memory address to read from.
	* @param byteCount The number of bytes to read.
	*
	* @return The vector of bytes read from the specified memory address.
	*
	* @throws std::runtime_error If the read operation fails.
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

	/* @brief Template function to write a value of type T to the specified memory address.
	* @code
	* mem.Write<int>(address, 10);
	* @endcode
	*
	* @param address The memory address to write to.
	* @param value The value to write.
	*
	* @return true if the write operation was successful, false otherwise.
	*/
	template <typename T>
	bool Write(uintptr_t address, const T& value) const noexcept
	{
		SIZE_T bytesWritten;
		return (WriteProcessMemory(process.getHandle(), reinterpret_cast<LPVOID>(address), &value, sizeof(T), &bytesWritten) &&
			bytesWritten == sizeof(T));
	};

	/* @brief Template function to write an array of values of type T to the specified memory address.
	* @code
	* mem.Write<int, 5>(address, std::array<int, 5>{1, 2, 3, 4, 5});
	* @endcode
	*
	* @param address The memory address to write to.
	* @param value The array of values to write.
	*
	* @return true if the write operation was successful, false otherwise.
	*/
	template <typename T, size_t N>
	bool Write(uintptr_t address, const std::array<T, N>& value) const noexcept
	{
		SIZE_T bytesWritten;
		return (WriteProcessMemory(process.getHandle(), reinterpret_cast<LPVOID>(address), value.data(), sizeof(value), &bytesWritten) &&
			bytesWritten == sizeof(value));
	};

	/* @brief Template function to write a span of values of type T to the specified memory address.
	* @code
	* mem.Write<int>(address, std::span<const int>{1, 2, 3, 4, 5});
	* @endcode
	*
	* @param address The memory address to write to.
	* @param values The span of values to write.
	*
	* @return true if the write operation was successful, false otherwise.
	*/
	template <typename T>
	bool Write(uintptr_t address, std::span<const T> values) const noexcept
	{
		SIZE_T bytesWritten;
		return (WriteProcessMemory(process.getHandle(), reinterpret_cast<LPVOID>(address), values.data(), values.size_bytes(), &bytesWritten) &&
			bytesWritten == values.size_bytes());
	};

	/* @brief Function to patch a memory address with the specified data.
	* @code
	* std::vector<BYTE> patchData = { 0x01, 0x73, 0x04 };
	* mem.Patch(address, std::span<const BYTE>{patchData});
	* @endcode
	*
	* @param address The memory address of the instruction to patch.
	* @param patchData The span of bytes (new instruction) to write.
	*
	* @return true if the patch operation was successful, false otherwise.
	*/
	bool Patch(uintptr_t address, std::span<const BYTE> patchData) const noexcept
	{
		return Write(address, patchData);
	}

	/* @brief Function to patch a memory address with the specified data (C-style string).
	* @code
	* mem.Patch(address, "\x01\x73\x04");
	* @endcode
	*
	* @param address The memory address of the instruction to patch.
	* @param patchData The C-style string (new instruction) to write.
	*
	* @return true if the patch operation was successful, false otherwise.
	*/
	bool Patch(uintptr_t address, char* patchData) const noexcept
	{
		return Write(address, std::span{ reinterpret_cast<const BYTE*>(patchData), strlen(patchData) });
	}

	/* @brief Function to nop out a range of memory.
	* @code
	* mem.Nop(address, 5);
	* @endcode
	*
	* @param address The memory address to start nopping from.
	* @param length The number of bytes to nop out.
	*
	* @return true if the nop operation was successful, false otherwise.
	*
	* @throws std::runtime_error If the process architecture is unsupported.
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