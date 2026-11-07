#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <cmath>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3dcompiler.lib")

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

/**
* @brief Represents a 2D vector with x and y coordinates.
*/
struct Vector2
{
	float x = 0.0f;
	float y = 0.0f;

	Vector2() = default;
	Vector2(float x, float y) : x(x), y(y) {}

	Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
	Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
	Vector2 operator*(float scale)          const { return Vector2(x * scale, y * scale); }
	Vector2 operator/(float scale)          const { return Vector2(x / scale, y / scale); }

	/**
	* @brief Calculates the module of the vector.
	* @return The module of the vector.
	*/
	float Module() const { return sqrtf(x * x + y * y); }

	/**
	* @brief Calculates the distance to another point.
	* @param other The other point.
	* @return The distance to the other point.
	*/
	float Distance(const Vector2& other) const { return (*this - other).Module(); }
};

/**
* @brief Represents a 3D vector with x, y, and z coordinates.
*/
struct Vector3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	Vector3() = default;
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
	Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
	Vector3 operator*(float scale)          const { return Vector3(x * scale, y * scale, z * scale); }
	Vector3 operator/(float scale)          const { return Vector3(x / scale, y / scale, z / scale); }
	
	/**
	* @brief Calculates the module of the vector.
	* @return The module of the vector.
	*/
	float Module() const { return sqrtf(x * x + y * y + z * z); }
	
	/**
	* @brief Calculates the distance to another point.
	* @param other The other point.
	* @return The distance to the other point.
	*/
	float Distance(const Vector3& other) const { return (*this - other).Module(); }
};

/**
* @brief Rapresents a 4x4 matrix, typically used for transformations in 3D space.
*/
struct Matrix4x4
{
	float m[4][4];
};

namespace Colors
{
	const ImU32 White = IM_COL32(255, 255, 255, 255);
	const ImU32 Black = IM_COL32(0, 0, 0, 255);
	const ImU32 Red = IM_COL32(255, 0, 0, 255);
	const ImU32 Green = IM_COL32(0, 255, 0, 255);
	const ImU32 Blue = IM_COL32(0, 120, 255, 255);
	const ImU32 Yellow = IM_COL32(255, 255, 0, 255);
	const ImU32 Orange = IM_COL32(255, 165, 0, 255);
	const ImU32 Purple = IM_COL32(170, 0, 255, 255);
}

/**
* @brief Gets the size of the screen.
* @return 2D Vector containing the size of the screen.
*/
inline Vector2 GetScreenSize()
{
	return Vector2((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN));
}

/**
* @brief Draws a line on the screen using ImGui.
* @param from The starting point of the line.
* @param to The ending point of the line.
* @param color The color of the line.
* @param thickness The thickness of the line.
*/
inline void DrawLine(Vector2 from, Vector2 to, ImU32 color = Colors::White, float thickness = 1.0f)
{
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(from.x, from.y), ImVec2(to.x, to.y), color, thickness);
}

/**
* @brief Draws a box on the screen using ImGui.
* @param topLeft The top-left corner of the box.
* @param width The width of the box.
* @param height The height of the box.
* @param color The color of the box.
* @param thickness The thickness of the box's border.
*/
inline void DrawBox(Vector2 topLeft, float width, float height, ImU32 color = Colors::White, float thickness = 1.0f)
{
	ImGui::GetBackgroundDrawList()->AddRect(
		ImVec2(topLeft.x, topLeft.y),
		ImVec2(topLeft.x + width, topLeft.y + height),
		color, 0.0f, 0, thickness);
}