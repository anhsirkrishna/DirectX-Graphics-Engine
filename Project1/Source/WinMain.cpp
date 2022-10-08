#include "App.h"

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, INT) {

	/* Create a App Class */
	
	App app;

	int ret_code = app.Run();

	return ret_code;
}