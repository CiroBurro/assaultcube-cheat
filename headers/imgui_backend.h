#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <string_view>

extern bool show_menu;

class ImGuiBackend
{
public:
	ImGuiBackend(int width, int height, std::wstring_view title);
	~ImGuiBackend();

	ImGuiBackend(const ImGuiBackend&) = delete;
	ImGuiBackend& operator=(const ImGuiBackend&) = delete;

	bool ProcessMessages();
	void NewFrame();
	void Render();

private:
	static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

	void CreateWindowClass(std::wstring_view title);
	void CreateDevice();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	void Resize(UINT width, UINT height);
	void SetOverlayInteraction(bool interactive);

	HWND window_ = nullptr;
	ATOM windowClass_ = 0;
	bool closeRequested_ = false;

	Microsoft::WRL::ComPtr<ID3D11Device> device_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget_;
};
