#pragma comment(lib, "dxgi.lib")
#include "gui.h"
#include "../resource.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

static LPDIRECT3D9              d3d = NULL;
static LPDIRECT3DDEVICE9        device = NULL;
static D3DPRESENT_PARAMETERS    presentParameters = {};
ImGuiIO* pIo;


std::string outputMessage = "Refresh rate changer, set any refresh rate and the original one will be restored on close.";


struct MonitorData {
	int monitorIndex;
	int originalRefreshRate;
	int selectedIndex;
	std::vector<int> possibleRefreshRates;

	MonitorData(int index, int originalRefreshRate, int selectedIndex)
		: monitorIndex(index), originalRefreshRate(originalRefreshRate), selectedIndex(selectedIndex) {}
};

std::vector<MonitorData> monitorDataList;

bool ChangeRefreshRate(int desiredRefreshRate, int monitorIndex) {
	DISPLAY_DEVICE displayDevice;
	ZeroMemory(&displayDevice, sizeof(displayDevice));
	displayDevice.cb = sizeof(displayDevice);

	if (EnumDisplayDevices(NULL, monitorIndex, &displayDevice, 0)) {
		DEVMODE dm;
		ZeroMemory(&dm, sizeof(dm));
		dm.dmSize = sizeof(dm);

		if (EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &dm) != 0) {
			int originalRefreshRate = dm.dmDisplayFrequency;
			dm.dmDisplayFrequency = desiredRefreshRate;  // Set the desired refresh rate

			if (ChangeDisplaySettingsEx(displayDevice.DeviceName, &dm, NULL, CDS_UPDATEREGISTRY, NULL) == DISP_CHANGE_SUCCESSFUL) {
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));
				return true;  // Refresh rate changed successfully
			}
		}
	}

	return false;  // Failed to change the refresh rate
}


std::vector<int> ListSupportedRefreshRates(int monitorIndex) {
	std::vector<int> list;

	DISPLAY_DEVICE displayDevice;
	ZeroMemory(&displayDevice, sizeof(displayDevice));
	displayDevice.cb = sizeof(displayDevice);

	if (EnumDisplayDevices(NULL, monitorIndex, &displayDevice, 0))
	{
		DEVMODE dm;
		ZeroMemory(&dm, sizeof(dm));
		dm.dmSize = sizeof(dm);

		int modeIndex = 0;
		while (EnumDisplaySettings(displayDevice.DeviceName, modeIndex, &dm) != 0)
		{
			int refreshRate = dm.dmDisplayFrequency;

			// Check if the refresh rate is already in the list
			if (std::find(list.begin(), list.end(), refreshRate) == list.end()) {
				list.emplace_back(refreshRate);
			}

			++modeIndex;
		}
	}
	std::sort(list.rbegin(), list.rend());
	return list;
}




extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter) {
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message) {
	case WM_CLOSE: {
		PostQuitMessage(0);
		gui::isRunning = false;
	}return 0;

	case WM_DESTROY: {
		PostQuitMessage(0);
		gui::isRunning = false;
	}return 0;

	case WM_SIZE: {
		if (device && wideParameter != SIZE_MINIMIZED) {
			presentParameters.BackBufferWidth = LOWORD(longParameter);
			presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT aplication menu
			return 0;

	}break;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter); //set click points
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON) {
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{};

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 &&
				gui::position.y <= 19) {
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left, rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
			}

		}
	}return 0;

	default:
		break;
	}

	return DefWindowProcW(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(LPCWSTR windowName, LPCWSTR className) noexcept {
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0L;
	windowClass.cbWndExtra = 0L;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = LoadIcon(windowClass.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = className;
	windowClass.hIconSm = NULL;

	RegisterClassEx(&windowClass);

	window = CreateWindow( windowClass.lpszClassName, windowName, WS_POPUP, 100, 100, WIDTH, HEIGHT, 0, 0, windowClass.hInstance, 0);
	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyWindow() noexcept {
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

}

bool gui::CreateDevice() noexcept {
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d) return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0) {
		return false;
	}

	return true;
}

void gui::ResetDevice() noexcept {
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL) {
		IM_ASSERT(0);
	}
	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept {
	if (device) {
		device->Release();
		device = nullptr;
	}
	if (d3d) {
		d3d->Release();
		d3d = nullptr;
	}

}


void gui::CreateImGui() noexcept {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO(); (void)io;
	pIo = &io;

	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	/////////////////////////////////////////////////////////////////////////////////
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);

	int monitorIndex = 0;

	for (int monitorIndex = 0; monitorIndex < GetSystemMetrics(SM_CMONITORS); ++monitorIndex) {
		DISPLAY_DEVICE displayDevice;
		ZeroMemory(&displayDevice, sizeof(displayDevice));
		displayDevice.cb = sizeof(displayDevice);

		if (EnumDisplayDevices(NULL, monitorIndex, &displayDevice, 0)) {
			DEVMODE dm;
			ZeroMemory(&dm, sizeof(dm));
			dm.dmSize = sizeof(dm);

			if (EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &dm) != 0) {
				int originalRefreshRate = dm.dmDisplayFrequency;

				monitorDataList.push_back(MonitorData(monitorIndex, originalRefreshRate, 0));
				monitorDataList.back().possibleRefreshRates = ListSupportedRefreshRates(monitorIndex);

				for (int i = 0; i < monitorDataList.back().possibleRefreshRates.size(); i++) {
					if (monitorDataList.back().possibleRefreshRates[i] == originalRefreshRate) {
						monitorDataList.back().selectedIndex = i;
						break;
					}
				}

			}
			else {
				outputMessage = "ERROR: Cannot iterate over devices;";
			}
		}
		else {
			outputMessage = "ERROR: Cannot iterate over devices;";
		}


	}


}

void gui::DestroyImGui() noexcept {

	for (auto& monitor : monitorDataList) {
		ChangeRefreshRate(monitor.originalRefreshRate, monitor.monitorIndex);
	}
	outputMessage = "DONE!";
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept {
	MSG message;
	while (::PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		::TranslateMessage(&message);
		::DispatchMessage(&message);
	}

		//Start ImGui frame
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

}

void gui::Render() noexcept {
	
	ImGui::SetNextWindowPos({0,0});
	ImGui::SetNextWindowSize({WIDTH, HEIGHT});
	DWORD dwFlag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin("Refresh Change", &isRunning);
	// Display refresh rate slider and apply button for each monitor
	for (auto& monitorData : monitorDataList) {
		ImGui::Separator();
		ImGui::Text("Monitor %d", monitorData.monitorIndex + 1);

		std::string comboLabel = "Hz##" + std::to_string(monitorData.monitorIndex);
		if (ImGui::BeginCombo(comboLabel.c_str(), std::to_string(monitorData.possibleRefreshRates[monitorData.selectedIndex]).c_str()))
		{
			for (int i = 0; i < monitorData.possibleRefreshRates.size(); ++i) 
			{
				bool isSelected = (monitorData.selectedIndex == i);
				if (ImGui::Selectable(std::to_string(monitorData.possibleRefreshRates[i]).c_str(), isSelected))
				{
					monitorData.selectedIndex = i;  // Update the selected item index
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();  // Set the selected item as the default focus
				}
			}

			ImGui::EndCombo();
		}


		if (ImGui::Button(("Apply##" + std::to_string(monitorData.monitorIndex)).c_str())) {

			if (ChangeRefreshRate(monitorData.possibleRefreshRates[monitorData.selectedIndex], monitorData.monitorIndex)) {
				outputMessage = "Changed refresh rate for Monitor: " + std::to_string(monitorData.monitorIndex + 1) + " to: " + std::to_string(monitorData.possibleRefreshRates[monitorData.selectedIndex]) + "Hz.";
			}
			else {
				outputMessage = "Failed to change refresh rate for Monitor " + std::to_string(monitorData.monitorIndex + 1) + " to " + std::to_string(monitorData.possibleRefreshRates[monitorData.selectedIndex]) + "Hz.";
			}

		}


	}
	

	ImGui::Separator();
	ImGui::Separator();

	if (ImGui::Button("Revert all and close")) {
		outputMessage = "Closing...";
		isRunning = false;
	}

	ImGui::Separator();
	ImGui::Separator();

	ImGui::TextWrapped(outputMessage.c_str());


	ImGui::End();
	// Dockspace END
}

void gui::EndRender() noexcept {
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, false);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	if (pIo->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	HRESULT result = device->Present(NULL, NULL, NULL, NULL);
	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
		

}