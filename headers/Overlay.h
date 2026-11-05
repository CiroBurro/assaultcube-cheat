#pragma once

#include <Windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <cmath>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3dcompiler.lib")


// =====================================================================
//  Vector2  -  a point on your SCREEN (x, y)
// =====================================================================
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

    // How long this vector is
    float Length() const { return sqrtf(x * x + y * y); }

    // How far away another point is
    float Distance(const Vector2& other) const { return (*this - other).Length(); }
};

// =====================================================================
//  Vector3  -  a position in the 3D GAME WORLD (x, y, z)
// =====================================================================
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

    float Length() const { return sqrtf(x * x + y * y + z * z); }
    float Distance(const Vector3& other) const { return (*this - other).Length(); }
};

struct Matrix4x4
{
    float m[4][4];
};


// =====================================================================
//  Colors
// =====================================================================

// Make your own color: Color(red, green, blue, alpha)  - each from 0 to 255
inline ImU32 Color(int r, int g, int b, int a = 255)
{
    return IM_COL32(r, g, b, a);
}

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

// =====================================================================
//  Drawing functions  -  call these inside YOUR drawing function
// =====================================================================

// Draws a line from one point to another
inline void DrawLine(Vector2 from, Vector2 to, ImU32 color = Colors::White, float thickness = 1.0f)
{
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(from.x, from.y), ImVec2(to.x, to.y), color, thickness);
}

// Draws an empty box. 'topLeft' is the top-left corner of the box.
inline void DrawBox(Vector2 topLeft, float width, float height, ImU32 color = Colors::White, float thickness = 1.0f)
{
    ImGui::GetBackgroundDrawList()->AddRect(
        ImVec2(topLeft.x, topLeft.y),
        ImVec2(topLeft.x + width, topLeft.y + height),
        color, 0.0f, 0, thickness);
}

// Draws a box filled with color (tip: use a low alpha like Color(0,0,0,100) for see-through)
inline void DrawFilledBox(Vector2 topLeft, float width, float height, ImU32 color = Colors::White)
{
    ImGui::GetBackgroundDrawList()->AddRectFilled(
        ImVec2(topLeft.x, topLeft.y),
        ImVec2(topLeft.x + width, topLeft.y + height),
        color);
}

// Draws an empty circle
inline void DrawCircle(Vector2 center, float radius, ImU32 color = Colors::White, float thickness = 1.0f)
{
    ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(center.x, center.y), radius, color, 0, thickness);
}

// Draws text on the screen
// (named DrawString because Windows already uses the name "DrawText")
inline void DrawString(Vector2 position, const char* text, ImU32 color = Colors::White)
{
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(position.x, position.y), color, text);
}

// Returns the size of your screen, e.g. (1920, 1080)
inline Vector2 GetScreenSize()
{
    return Vector2((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN));
}