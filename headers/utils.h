#include <Windows.h>
#include <TlHelp32.h>

class AutoReleaseHandle
{
private:
	HANDLE handle;

public:
	// Constructor
	explicit AutoReleaseHandle(HANDLE h) : handle(h) {}

	~AutoReleaseHandle() {
		if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
			CloseHandle(handle);
		}
	}

	// Delete copy constructor and copy assignment operator to prevent copying
	AutoReleaseHandle(const AutoReleaseHandle&) = delete;
	AutoReleaseHandle& operator=(const AutoReleaseHandle&) = delete;

	
	// Define move constructor and move assignment operator for transferring ownership
	AutoReleaseHandle(AutoReleaseHandle&& other) noexcept : handle(other.handle) {
		other.handle = nullptr;
	}
	AutoReleaseHandle& operator=(AutoReleaseHandle&& other) noexcept {
		if (this != &other) {
			// Release currently owned resource
			if (handle && handle != INVALID_HANDLE_VALUE)
				CloseHandle(handle);

			// Transfer ownership
			handle = other.handle;
			other.handle = nullptr;
		}

		return *this;
	}

	/* @brief Get method to access the underlying handle
	* 
	* @return The underlying HANDLE value.
	*/
	HANDLE get() const {
		return handle;
	}

	// Bool conversion operator to check if the handle is valid
	explicit operator bool() const {
		return handle != nullptr && handle != INVALID_HANDLE_VALUE;
	}
};