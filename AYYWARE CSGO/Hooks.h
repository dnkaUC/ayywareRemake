/*
Rest In Peace ApocalypseCheats
*/

#pragma once

// It's actually in DLLMain but w/e
extern bool DoUnload;

#include "Utilities.h"
#include "Hacks.h"
extern bool islbyupdate;
extern float ProxyLBYtime;
extern int LBYBreakerTimer;
extern float enemysLastProxyTimer[65];
extern float fsnLBY;
extern float enemyLBYTimer[65];

extern bool switchAntiAimSide;

extern int shotsfired;
//Resolver
extern int ResolverStage[65];

extern float testFloat1;
extern float testFloat2;
extern float testFloat3;
extern float testFloat4;
extern bool antiAimSide;

// Console Vars for LBY TESTS
extern float consoleProxyLbyLASTUpdateTime; // This is in ProxyLBY AntiAim.cpp
extern float consoleFsnMyPredictTime; // This is in FSN POSTSTART UPDATE
extern float consoleWhenLBYCalledOnAntiAim; // this is in CreateMove


extern float lineLBY;
extern float lineRealAngle;
extern float lineFakeAngle;

namespace Hooks
{
	void Initialise();
	void UndoHooks();
	
	class HooksXD;
	// VMT Managers
	extern Utilities::Memory::VMTManager VMTPanel; // Hooking drawing functions
	extern Utilities::Memory::VMTManager VMTClient; // Maybe CreateMove
	extern Utilities::Memory::VMTManager VMTClientMode; // CreateMove for functionality
	extern Utilities::Memory::VMTManager VMTModelRender; // DrawModelEx for chams
	extern Utilities::Memory::VMTManager VMTPrediction; // InPrediction for no vis recoil
	extern Utilities::Memory::VMTManager VMTPlaySound; // Autoaccept shit
	extern Utilities::Memory::VMTManager VMTRenderView;
	
};
class HooksXD
{
public:
	static void XD3();
	static int night;
	static int night2;
};
//void XD3();


