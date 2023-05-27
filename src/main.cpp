#include "gui.h"

#include <thread>

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE previousInstance, PWSTR arguments, int commandShow) {

	//create gui
	gui::CreateHWindow(L"TempHz", L"TempHz");
	gui::CreateDevice();
	gui::CreateImGui();


	while (gui::isRunning) {
		gui::BeginRender();


		gui::Render();



		gui::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	}

	//destroy gui
	gui::DestroyImGui();
	gui::DestroyDevice();
	gui::DestroyWindow();

	PostQuitMessage(1);

	return EXIT_SUCCESS;
}