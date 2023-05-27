#pragma once
#include <string>
#include <d3d9.h>
#include <Windows.h>

namespace gui {
	constexpr int WIDTH = 200;
	constexpr int HEIGHT = 300;

	inline bool isRunning = true;

	inline HWND window = nullptr;
	inline WNDCLASSEX windowClass = { };

	inline POINTS position = { };
	
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
