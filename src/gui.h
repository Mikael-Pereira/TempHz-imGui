#pragma once
#include <string>
#include <d3d9.h>
#include <Windows.h>
#include "DX9Viewport.cpp"
#include "RenderPrimitives.cpp"



namespace gui {
	constexpr int WIDTH = 200;
	constexpr int HEIGHT = 300;

	inline bool isRunning = true;

	inline HWND window = nullptr;
	inline WNDCLASSEX windowClass = { };

	inline POINTS position = { };

	inline PDIRECT3D9 d3d = nullptr;
	inline LPDIRECT3DDEVICE9 device = nullptr;
	inline D3DPRESENT_PARAMETERS presentParameters = { };

	inline DX9Viewport* dxViewport = nullptr;



	void CreateHWindow(LPCWSTR windowName, LPCWSTR className) noexcept;
	void DestroyWindow() noexcept;

	bool CreateDevice() noexcept;
	void ResetDevice() noexcept;
	void DestroyDevice() noexcept;

	void CreateImGui() noexcept;
	void DestroyImGui() noexcept;

	void DXRender() noexcept;

	void BeginRender() noexcept;
	void Render() noexcept;
	void EndRender() noexcept;

}
