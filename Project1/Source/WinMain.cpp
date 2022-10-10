#include "App.h"
#include <stdio.h>

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, INT) {

	/* Create a App Class */
	bool ret_val;
	ret_val = AllocConsole();
	FILE* newstdin = nullptr;
	FILE* newstdout = nullptr;
	FILE* newstderr = nullptr;
	freopen_s(&newstdin, "CONIN$", "r", stdin);
	freopen_s(&newstdout, "CONOUT$", "w", stdout);
	freopen_s(&newstderr, "CONOUT$", "w", stderr);

	printf("opened console");
	App app;

	int ret_code = app.Run();

	return ret_code;
}