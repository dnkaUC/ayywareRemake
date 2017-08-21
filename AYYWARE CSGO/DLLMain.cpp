/*
ApocalypseCheats
------------------------------
Contributors:
XBoom
Ma$$A$
madlifer
control1337
CyclesPrograming
FZCat1337
UC Community <3
*/
//#define AC_DEBUG

// General shit
#include "DLLMain.h"
#include "Utilities.h"

// Injection stuff
#include "INJ/ReflectiveLoader.h"

// Stuff to initialise
#include "Offsets.h"
#include "Interfaces.h"
#include "Hooks.h"
#include "RenderManager.h"
#include "Hacks.h"
#include "Hooks.h"
#include "Menu.h"
#include "AntiAntiAim.h"

#include "Dumping.h"





// COnsole Tests VArs for LBY
float consoleProxyLbyLASTUpdateTime;
float consoleFsnMyPredictTime;
float consoleWhenLBYCalledOnAntiAim;


// Used as part of the reflective DLL injection
extern HINSTANCE hAppInstance;

// Our DLL Instance
HINSTANCE HThisModule;
bool DoUnload;
bool islbyupdate;
float ProxyLBYtime;
int LBYBreakerTimer;
float fsnLBY;
bool switchInverse;

float testFloat1;
float testFloat2;
float testFloat3;
float testFloat4;
bool antiAimSide;
float enemyLBYTimer[65];
int ResolverMode;
bool pEntityLBYUpdate[65];
float pEntityLastUpdateTime[65];
float enemyLBYDelta[65];
int ResolverStage[65];
int shotsfired;
bool switchAntiAimSide;
//GameEventManager2 *p_GameEventManager2;
float enemysLastProxyTimer[65];
float lineLBY;
float lineRealAngle;
float lineFakeAngle;
float autowalldmgtest[65];


int hittedLogHits[65];
int missedLogHits[65];

// Our thread we use to setup everything we need
// Everything appart from code in hooks get's called from inside 
// here.

int InitialThread()
{
#ifdef AC_DEBUG
	Utilities::OpenConsole("Logic");
#endif

	//AllocConsole();
	//freopen("CONIN$", "r", stdin);
	//freopen("CONOUT$", "w", stdout);
	//freopen("CONOUT$", "w", stderr);
	//SetConsoleTitle("DEBUG GAY SHIT");
	// Intro banner with info
	PrintMetaHeader();

	//---------------------------------------------------------
	// Initialise all our shit
	Offsets::Initialise(); // Set our VMT offsets and do any pattern scans
	Interfaces::Initialise(); // Get pointers to the valve classes
	NetVar.RetrieveClasses(); // Setup our NetVar manager (thanks shad0w bby)
	NetvarManager::Instance()->CreateDatabase();
	Render::Initialise();
	Hacks::SetupHacks();
	Menu::SetupMenu();
	Hooks::Initialise();
	ApplyAAAHooks();



	HooksXD::XD3();
	//GUI.LoadWindowState(&Menu::Window, "config.xml");

	// Dumping
	//Dump::DumpClassIds();

	//---------------------------------------------------------
	Utilities::Log("Ready");
	
	
	//MessageBox(NULL, "Cheat loaded succesfully in CSGO.", "Apocalypse", MB_OK);
	// While our cheat is running
	while (DoUnload == false)
	{
		Sleep(1000);
	}

	Hooks::UndoHooks();
	Sleep(2000); // Make sure none of our hooks are running
	FreeLibraryAndExitThread(HThisModule, 0);

	return 0;
}

// DllMain
// Entry point for our module
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
	)
{
	switch (fdwReason)
	{
	case DLL_QUERY_HMODULE:
		if (lpvReserved != NULL)
			*(HMODULE *)lpvReserved = hAppInstance;
		break;
	case DLL_PROCESS_ATTACH:
		HThisModule = hinstDLL;
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)InitialThread, NULL, NULL, NULL);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}