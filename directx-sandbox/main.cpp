//#include "systemclass.h";
#include <Windows.h>
#include <stdio.h>

#include "System.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	// Create the system object.
	System* pSystem = new System();
	if (!pSystem)
	{
		printf("ERROR: pSystem null after 'new System()'\n");
		return -5;
	}

	bool result = pSystem->Init();
	if (result)
	{
		printf("System Init() successful, now calling Run()\n");
		pSystem->Run();
	}
	else {
		printf("System::Init failed. Too bad!\n");
	}

	// Shutdown and release the system object.
	pSystem->Shutdown();
	delete pSystem;
	pSystem = NULL;

	return 0;
}

int main() {
	printf("Ages ago, life was born in the primitive sea.\n\n");

	return WinMain(GetModuleHandle(NULL), NULL, NULL, 0);
}