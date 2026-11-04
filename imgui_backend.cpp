#include "headers/imgui_backend.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <d3d11.h>
#include <stdexcept>
#include <string>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

namespace
{
	constexpr wchar_t WindowClassName[] = L"AcCheatImGuiWindow";
}

ImGuiBackend::ImGuiBackend(int width, int height, std::wstring_view title)
{
	CreateWindowClass(title);

	std::wstring windowTitle(title);
	window_ = CreateWindowExW(
		WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
		MAKEINTRESOURCEW(windowClass_),
		windowTitle.c_str(),
		WS_POPUP,
		0,
		0,
		width,
		height,
		nullptr,
		nullptr,
		GetModuleHandleW(nullptr),
		this);

	if (!window_) {
		throw std::runtime_error("Failed to create ImGui window.");
	}

	if (!SetLayeredWindowAttributes(window_, RGB(0, 0, 0), 0, LWA_COLORKEY)) {
		throw std::runtime_error("Failed to configure transparent overlay window.");
	}

	CreateDevice();
	CreateRenderTarget();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	if (!ImGui_ImplWin32_Init(window_) || !ImGui_ImplDX11_Init(device_.Get(), context_.Get())) {
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		throw std::runtime_error("Failed to initialize ImGui backends.");
	}

	RegisterHotKey(window_, 1, MOD_NOREPEAT, VK_F1);
	SetOverlayInteraction(show_menu);

	ShowWindow(window_, SW_SHOWDEFAULT);
	UpdateWindow(window_);
}

ImGuiBackend::~ImGuiBackend()
{
	UnregisterHotKey(window_, 1);

	if (ImGui::GetCurrentContext()) {
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	CleanupRenderTarget();
	swapChain_.Reset();
	context_.Reset();
	device_.Reset();

	if (window_) {
		DestroyWindow(window_);
	}

	if (windowClass_) {
		UnregisterClassW(MAKEINTRESOURCEW(windowClass_), GetModuleHandleW(nullptr));
	}
}

bool ImGuiBackend::ProcessMessages()
{
	MSG message{};
	while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);

		if (message.message == WM_HOTKEY && message.wParam == 1) {
			show_menu = !show_menu;
			SetOverlayInteraction(show_menu);
		}

		if (message.message == WM_QUIT) {
			closeRequested_ = true;
		}
	}

	return !closeRequested_;
}

void ImGuiBackend::NewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiBackend::Render()
{
	ImGui::Render();

	constexpr float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	context_->OMSetRenderTargets(1, renderTarget_.GetAddressOf(), nullptr);
	context_->ClearRenderTargetView(renderTarget_.Get(), clearColor);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	swapChain_->Present(1, 0);
}

LRESULT CALLBACK ImGuiBackend::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto* backend = reinterpret_cast<ImGuiBackend*>(GetWindowLongPtrW(window, GWLP_USERDATA));

	if (message == WM_NCCREATE) {
		auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
		backend = static_cast<ImGuiBackend*>(createStruct->lpCreateParams);
		SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(backend));
	}

	if (message == WM_NCHITTEST && !show_menu) {
		return HTTRANSPARENT;
	}

	if (ImGui::GetCurrentContext() && ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam)) {
		return true;
	}

	if (backend) {
		switch (message) {
		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED) {
				backend->Resize(LOWORD(lParam), HIWORD(lParam));
			}
			return 0;
		case WM_CLOSE:
			backend->closeRequested_ = true;
			DestroyWindow(window);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			break;
		}
	}

	return DefWindowProcW(window, message, wParam, lParam);
}

void ImGuiBackend::CreateWindowClass(std::wstring_view title)
{
	(void)title;

	WNDCLASSEXW windowClass{
		sizeof(WNDCLASSEXW),
		CS_CLASSDC,
		WindowProc,
		0,
		0,
		GetModuleHandleW(nullptr),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		WindowClassName,
		nullptr
	};

	windowClass_ = RegisterClassExW(&windowClass);
	if (!windowClass_) {
		throw std::runtime_error("Failed to register ImGui window class.");
	}
}

void ImGuiBackend::CreateDevice()
{
	DXGI_SWAP_CHAIN_DESC swapChainDescription{};
	swapChainDescription.BufferCount = 2;
	swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescription.OutputWindow = window_;
	swapChainDescription.SampleDesc.Count = 1;
	swapChainDescription.Windowed = TRUE;
	swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	constexpr D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};

	D3D_FEATURE_LEVEL featureLevel{};
	const HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&swapChainDescription,
		swapChain_.GetAddressOf(),
		device_.GetAddressOf(),
		&featureLevel,
		context_.GetAddressOf());

	if (FAILED(result)) {
		throw std::runtime_error("Failed to create DirectX 11 device and swap chain.");
	}
}

void ImGuiBackend::CreateRenderTarget()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()))) ||
		FAILED(device_->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTarget_.GetAddressOf()))) {
		throw std::runtime_error("Failed to create DirectX 11 render target.");
	}
}

void ImGuiBackend::CleanupRenderTarget()
{
	renderTarget_.Reset();
}

void ImGuiBackend::Resize(UINT width, UINT height)
{
	if (!swapChain_ || width == 0 || height == 0) {
		return;
	}

	CleanupRenderTarget();
	context_->OMSetRenderTargets(0, nullptr, nullptr);
	swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	CreateRenderTarget();
}

void ImGuiBackend::SetOverlayInteraction(bool interactive)
{
	LONG_PTR exStyle = GetWindowLongPtrW(window_, GWL_EXSTYLE);

	if (interactive)
	{
		exStyle &= ~WS_EX_TRANSPARENT;
		exStyle &= ~WS_EX_NOACTIVATE;
	}
	else
	{
		exStyle |= WS_EX_TRANSPARENT;
		exStyle |= WS_EX_NOACTIVATE;
	}

	SetWindowLongPtrW(window_, GWL_EXSTYLE, exStyle);

	SetWindowPos(
		window_,
		HWND_TOPMOST,
		0, 0, 0, 0,
		SWP_NOMOVE |
		SWP_NOSIZE |
		SWP_FRAMECHANGED |
		(interactive ? 0 : SWP_NOACTIVATE)
	);

	if (interactive)
		SetForegroundWindow(window_);
}
