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

#include "Hooks.h"
#include "Hacks.h"
#include "Chams.h"
#include "Menu.h"
#include <intrin.h>
#include "Controls.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "MiscHacks.h"
#include "CRC32.h"
#include "Resolver.h"
#include "Utilities.h"
#include "Backtrack.h"
#include <string>

#define TICK_INTERVAL			( Interfaces::Globals->interval_per_tick )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )



/*
class CBacktracking
{
private:
	struct BacktrackData_t
	{
		int     iEntityIndex = 0;
		int     iTickCount = 0;
		float   flSimulationTime = 0.f;
		float   flDamage = 0.f;
		Vector  vecBestPoint = Vector(0, 0, 0);
	} Data_t;

public:
	void StoreData(IClientEntity* pEntity, CUserCmd* pCmd, Vector vecAimPos, float flPredictedDamage) {
		Data_t.iEntityIndex = pEntity->GetIndex();
		Data_t.iTickCount = pCmd->tick_count;
		Data_t.flSimulationTime = pEntity->GetSimulationTime();
		Data_t.flDamage = flPredictedDamage;
		Data_t.vecBestPoint = vecAimPos;

		if (m_vData[Data_t.iEntityIndex].size() == 10)
			m_vData[Data_t.iEntityIndex].erase(m_vData[Data_t.iEntityIndex].begin());

		m_vData[Data_t.iEntityIndex].push_back(Data_t);
	}

	void RestoreData(IClientEntity* pEntity, CUserCmd* pCmd, int TicksToBacktrack = 10) {
		pEntity->GetSimulationTime = GetBacktrackData(pEntity, pCmd, TicksToBacktrack).flSimulationTime;
		pCmd->tick_count = GetBacktrackData(pEntity, pCmd, TicksToBacktrack).iTickCount;
	}

	BacktrackData_t GetBacktrackData(IClientEntity* pEntity, CUserCmd* pCmd, int TicksToBacktrack = 10) {
		return m_vData[pEntity->GetIndex].at(11 - TicksToBacktrack);
	}

private:
	std::array< std::vector< BacktrackData_t >, 64 > m_vData;
};


class CBacktracking
{
public:
	void StoreData(IClientEntity* pEntity, CUserCmd* pCmd, float flDamageGiven)
	{
		CurTickCount = pCmd->tick_count;
		if (pEntity) {
			m_BacktrackData[pEntity->GetIndex()][CurTickCount % 12].iTickCount = pCmd->tick_count;
			m_BacktrackData[pEntity->GetIndex()][CurTickCount % 12].flDamage = flDamageGiven;
			m_BacktrackData[pEntity->GetIndex()][CurTickCount % 12].flSimulationTime = pEntity->GetSimulationTime();
			m_BacktrackData[pEntity->GetIndex()][CurTickCount % 12].vecHeadPos = pEntity->GetHeadPos();
		}
	}

	void RestoreData(IClientEntity* pEntity, CUserCmd* pCmd, int TicksBack)
	{
		if (pEntity) {
			pEntity->GetSimulationTime = m_BacktrackData[pEntity->GetIndex()][(CurTickCount - TicksBack) % 12].flSimulationTime;
			pCmd->tick_count = m_BacktrackData[pEntity->GetIndex()][(CurTickCount - TicksBack - 1) % 12].iTickCount;
		}
	}
private:
	struct BacktrackData_t
	{
		int iTickCount = 0;
		float flDamage = 0.f;
		float flSimulationTime = 0.f;
		Vector vecHeadPos = Vector(0, 0, 0);
	} m_BacktrackData[64][12];

public:
	BacktrackData_t GetBacktrackData(IClientEntity* pEntity, int TicksBack) const
	{
		if (pEntity)
			return m_BacktrackData[pEntity->GetIndex()][(CurTickCount - TicksBack - 1) % 12];

		return m_BacktrackData[0][0];
	}
private:
	int CurTickCount;
};
 */

DamageEventListener* DamageListener;
DamageEventListener* KillListener;
DamageEventListener* roundstart;
DamageEventListener* round_end;
DamageEventListener* Resolvertest;


Vector LastAngleAA;
#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr) + (DWORD)(addValue))
// Funtion Typedefs
typedef void(__thiscall* DrawModelEx_)(void*, void*, void*, const ModelRenderInfo_t&, matrix3x4*);
typedef void(__thiscall* PaintTraverse_)(PVOID, unsigned int, bool, bool);
typedef bool(__thiscall* InPrediction_)(PVOID);
typedef void(__stdcall *FrameStageNotifyFn)(ClientFrameStage_t);
typedef void(__thiscall* RenderViewFn)(void*, CViewSetup&, CViewSetup&, int, int);

using OverrideViewFn = void(__fastcall*)(void*, void*, CViewSetup*);
typedef float(__stdcall *oGetViewModelFOV)();



// Function Pointers to the originals
PaintTraverse_ oPaintTraverse;
DrawModelEx_ oDrawModelExecute;
FrameStageNotifyFn oFrameStageNotify;
OverrideViewFn oOverrideView;
RenderViewFn oRenderView;

static bool resolverMemeTest;

// Hook function prototypes
void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
bool __stdcall Hooked_InPrediction();
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);
bool __stdcall CreateMoveClient_Hooked(/*void* self, int edx,*/ float frametime, CUserCmd* pCmd);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
float __stdcall GGetViewModelFOV();
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw);

static bool roundStartTest;
static float myLBYTimer;
static int roundStartTimer;

// VMT Managers
namespace Hooks
{
	// VMT Managers
	Utilities::Memory::VMTManager VMTPanel; // Hooking drawing functions
	Utilities::Memory::VMTManager VMTClient; // Maybe CreateMove
	Utilities::Memory::VMTManager VMTClientMode; // CreateMove for functionality
	Utilities::Memory::VMTManager VMTModelRender; // DrawModelEx for chams
	Utilities::Memory::VMTManager VMTPrediction; // InPrediction for no vis recoil
	Utilities::Memory::VMTManager VMTPlaySound; // Autoaccept 
	Utilities::Memory::VMTManager VMTRenderView;
};

/*// Initialise all our hooks
void Hooks::Initialise()
{
// Panel hooks for drawing to the screen via surface functions
VMTPanel.Initialise((DWORD*)Interfaces::Panels);
oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);
//Utilities::Log("Paint Traverse Hooked");

// No Visual Recoil
VMTPrediction.Initialise((DWORD*)Interfaces::Prediction);
VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);
//Utilities::Log("InPrediction Hooked");

// Chams
VMTModelRender.Initialise((DWORD*)Interfaces::ModelRender);
oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);
//Utilities::Log("DrawModelExecute Hooked");

// Setup ClientMode Hooks
//VMTClientMode.Initialise((DWORD*)Interfaces::ClientMode);
//VMTClientMode.HookMethod((DWORD)&CreateMoveClient_Hooked, 24);
//Utilities::Log("ClientMode CreateMove Hooked");

// Setup client hooks
VMTClient.Initialise((DWORD*)Interfaces::Client);
oCreateMove = (CreateMoveFn)VMTClient.HookMethod((DWORD)&hkCreateMove, 21);
}*/



#include "GameEventManager.h"
static float saveLastHeadshotFloat[65];
static float saveLastBaimFloat[65];
static float saveLastBaim30Float[65];

static float saveLastBaim10Float[65];

int hitmarkertime = 0;

static float testtimeToTick;
static float testServerTick;
static float testTickCount64 = 1;


static int hittedLogHits;
static int missedLogHits;

void imfinnarunuponya(IGameEvent* pEvent)
{
	/*
	int iAttacker = Interfaces::Engine->GetPlayerForUserID(pEvent->GetInt("attacker", false));
	int iVictim = Interfaces::Engine->GetPlayerForUserID(pEvent->GetInt("userid", false));
	int iDamage = pEvent->GetInt("dmg_health", false);

	if (iAttacker == Interfaces::Engine->GetLocalPlayer() && iVictim != Interfaces::Engine->GetLocalPlayer())
	{
		Interfaces::Engine->
	}
	*/
	int attackerid = pEvent->GetInt("attacker");
	int userid = pEvent->GetInt("userid");
	int dmg = pEvent->GetInt("dmg_health");
	int hitgroup = pEvent->GetInt("hitgroup");

	if (Interfaces::Engine->GetPlayerForUserID(userid) == Interfaces::Engine->GetLocalPlayer()) {

		// If we got hitted in our Head Sideswitch
		if (hitgroup == 1) {
			//SwitchAntiAimSide
			switchAntiAimSide = true;
		}
		else {
			switchAntiAimSide = false;
		}
	}


	if (Interfaces::Engine->GetPlayerForUserID(attackerid) == Interfaces::Engine->GetLocalPlayer())
	{
		hitmarkertime = 255;
		PlaySoundA("C:\\Hitmarker.wav", NULL, SND_FILENAME | SND_ASYNC);
		
		// WE have hitted someone is pretty good or???
		hittedLogHits++;

		IClientEntity* pEntity = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetPlayerForUserID(userid));
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

		if (!pEntity || !pLocal || pEntity == pLocal)
			return;

		if (pEntity->GetVelocity().Length2D() == 0) {
			if (hitgroup == 1) {
				saveLastHeadshotFloat[pEntity->GetIndex()] = pEntity->GetEyeAngles().y;

			}
			else if (dmg >= 50) {
				saveLastBaimFloat[pEntity->GetIndex()] = pEntity->GetEyeAngles().y;
			}
			else if (dmg >= 30) {
				saveLastBaim30Float[pEntity->GetIndex()] = pEntity->GetEyeAngles().y;
			}
		}

		
	}
	else {
		// we missed ?????? log it...
		missedLogHits++;
		
	}

	/*
	int entityid = Interfaces::Engine->GetPlayerForUserID(attackerid);
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (!pEntity || pEntity->IsDormant() || pEntity == pLocal)
			continue;
		Vector* eyeAngles = pEntity->GetEyeAnglesPointer();
		player_info_t pTemp;

		if (!Interfaces::Engine->GetPlayerInfo(i, &pTemp))
			continue;


			if (entityid == Interfaces::Engine->GetLocalPlayer())
			{
				

				hitmarkertime = 255;
				PlaySoundA("C:\\Hitmarker.wav", NULL, SND_FILENAME | SND_ASYNC);
				if (pEntity->GetVelocity().Length2D() == 0) {

				int nUserID = pEvent->GetInt("attacker");
				int nDead = pEvent->GetInt("userid");
				int dmg = pEvent->GetInt("dmg_health");

				if (nUserID || nDead)
				{
					int headshot = pEvent->GetInt("hitgroup");

					if (headshot == 1)
					{
						saveLastHeadshotFloat[pEntity->GetIndex()] = pEntity->GetTargetYaw();
					}
					else {
						if (dmg > 50) {
							saveLastBaimFloat[pEntity->GetIndex()] = pEntity->GetTargetYaw();
						

						}
						else if (dmg > 30) {

							saveLastBaim30Float[pEntity->GetIndex()] = pEntity->GetTargetYaw();
							

						}
						else if (dmg > 10) {

							saveLastBaim10Float[pEntity->GetIndex()] = pEntity->GetTargetYaw();
							

						}



					}
				}



				}

			}

		

	}
	*/
	
	


}
player_info_t GetInfo(int Index) {
	player_info_t Info;
	Interfaces::Engine->GetPlayerInfo(Index, &Info);
	return Info;
}

void testResolver(IGameEvent* pEvent) {

	// Reset lby timer
	roundStartTest = true;
	roundStartTimer++;
	shotsfired = 0;
}

void imfinnakillyou(IGameEvent* pEvent)
{
	// DEATH
}

void imfinnamemeu(IGameEvent* pEvent)
{

	

}

// Undo our hooks
void Hooks::UndoHooks()
{
	VMTPanel.RestoreOriginal();
	VMTPrediction.RestoreOriginal();
	VMTModelRender.RestoreOriginal();
	VMTClientMode.RestoreOriginal();
	Interfaces::GameEventManager->RemoveListener(DamageListener);
	delete DamageListener;
}


// Initialise all our hooks
void Hooks::Initialise()
{
	// Panel hooks for drawing to the screen via surface functions
	VMTPanel.Initialise((DWORD*)Interfaces::Panels);
	oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);
	//Utilities::Log("Paint Traverse Hooked");

	// No Visual Recoi	l
	VMTPrediction.Initialise((DWORD*)Interfaces::Prediction);
	VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);
	//Utilities::Log("InPrediction Hooked");

	// Chams
	VMTModelRender.Initialise((DWORD*)Interfaces::ModelRender);
	oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);
	//Utilities::Log("DrawModelExecute Hooked");

	// Setup ClientMode Hooks
	VMTClientMode.Initialise((DWORD*)Interfaces::ClientMode);
	VMTClientMode.HookMethod((DWORD)CreateMoveClient_Hooked, 24);

	oOverrideView = (OverrideViewFn)VMTClientMode.HookMethod((DWORD)&Hooked_OverrideView, 18);
	VMTClientMode.HookMethod((DWORD)&GGetViewModelFOV, 35);

	// Setup client hooks
	VMTClient.Initialise((DWORD*)Interfaces::Client);
	oFrameStageNotify = (FrameStageNotifyFn)VMTClient.HookMethod((DWORD)&Hooked_FrameStageNotify, 36);


	DamageListener = new DamageEventListener(imfinnarunuponya);
	// ...In injection thread
	Interfaces::GameEventManager->AddListener(DamageListener, "player_hurt", false);





	KillListener = new DamageEventListener(imfinnakillyou);
	// ...In injection thread
	Interfaces::GameEventManager->AddListener(KillListener, "player_death", false);


	Resolvertest = new DamageEventListener(testResolver);
	// ...In injection thread
	Interfaces::GameEventManager->AddListener(Resolvertest, "round_start", false);

	//Interfaces::GameEventManager->FireEvent();

	//IGameEvent* pEvent = Interfaces::GameEventManager->CreateEvent("game_end");

	//Resolvertest = new DamageEventListener(testResolver);
	// ...In injection thread
	//Interfaces::GameEventManager->FireEvent(Resolvertest, false);

	//roundstart = new DamageEventListener(resolvermeme);
	// ...In injection thread
	//Interfaces::GameEventManager->AddListener(roundstart, "player_hurt", false);

	

	round_end = new DamageEventListener(imfinnamemeu);
	// ...In injection thread
	Interfaces::GameEventManager->AddListener(round_end, "player_death", false);

}

void MovementCorrection(CUserCmd* pCmd)
{

}

//---------------------------------------------------------------------------------------------------------
//                                         Hooked Functions
//---------------------------------------------------------------------------------------------------------

void SetClanTag(const char* tag, const char* name)//190% paste
{
	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(((DWORD)Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15\x00\x00\x00\x00\x6A\x24\x8B\xC8\x8B\x30", "xxxxxxxxx????xxxxxx")));
	pSetClanTag(tag, name);
}
void NoClantag()
{
	SetClanTag("", "");
}

void ClanTag()
{
	static int counter = 0;
	switch (Menu::Window.MiscTab.OtherClantag.GetIndex())
	{
	case 0:
		SetClanTag("", "");
		break;
	case 1:
	{
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2.5;

		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 19;
		switch (value) {
		case 0:SetClanTag("          ", "apocalypse"); break;
		case 1:SetClanTag("         a", "apocalypse"); break;
		case 2:SetClanTag("        ap", "apocalypse"); break;
		case 3:SetClanTag("       apo", "apocalypse"); break;
		case 4:SetClanTag("      apoc", "apocalypse"); break;
		case 5:SetClanTag("     apoca", "apocalypse"); break;
		case 6:SetClanTag("    apocal", "apocalypse"); break;
		case 7:SetClanTag("   apocaly", "apocalypse"); break;
		case 8:SetClanTag("  apocalyp", "apocalypse"); break;
		case 9:SetClanTag(" apocalyps", "apocalypse"); break;
		case 10:SetClanTag("apocalypse", "apocalypse"); break;
		case 11:SetClanTag("pocalypse ", "apocalypse"); break;
		case 12:SetClanTag("ocalypse  ", "apocalypse"); break;
		case 13:SetClanTag("calypse   ", "apocalypse"); break;
		case 14:SetClanTag("alypse    ", "apocalypse"); break;
		case 15:SetClanTag("lypse     ", "apocalypse"); break;
		case 16:SetClanTag("ypse      ", "apocalypse"); break;
		case 17:SetClanTag("pse       ", "apocalypse"); break;
		case 18:SetClanTag("se        ", "apocalypse"); break;
		case 19:SetClanTag("e         ", "apocalypse"); break;
		}
		counter++;
	}
	break;
	case 2:
	{
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 3;

		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 6;
		switch (value) {
		case 0:SetClanTag("Best LBY", "LBY BREAKER"); break;
		case 1:SetClanTag("LBY BREAKER", "LBY"); break;
		case 2:SetClanTag("UnknownCheats", "UnknownCheats"); break;
		case 3:SetClanTag("fag", "fag"); break;
		case 4:SetClanTag("Pro Baimer", "Pro Baimer"); break;
		case 5:SetClanTag("Incredible sad", "Incredible sad"); break;
		case 6:SetClanTag("me > you", "me > you"); break;
		}
		counter++;
	}
	break;
	case 3:
		// stainless
		SetClanTag("\r", "\r");
		break;
	case 4:
		SetClanTag("[VALV\xE1\xB4\xB1]", "Valve");
		break;
	case 5:
		SetClanTag("splashgang", "splashgang");
		break;
	case 6:
		SetClanTag("STAINLESS", "STAINLESS");
		break;
	}
}
HooksXD lmao;
int HooksXD::night;
int HooksXD::night2;
#include "Entities.h"
void HooksXD::XD3()
{
	CSGOClassID::CMaterialModifyControl;
	//int HooksXD::night;
	//int HooksXD::night2;
	 
	 HooksXD::night = 1;
	 HooksXD::night2 = 1;
};


bool __stdcall CreateMoveClient_Hooked(/*void* self, int edx,*/ float frametime, CUserCmd* pCmd)
{
	
	if (!pCmd->command_number)
		return true;

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		//pCmd->tick_count -= 12;
		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;
		
		if (Menu::Window.MiscTab.OtherClantag.GetIndex() > 0)
			ClanTag();

		//	CUserCmd* cmdlist = *(CUserCmd**)((DWORD)Interfaces::pInput + 0xEC);
		//	CUserCmd* pCmd = &cmdlist[sequence_number % 150];

		// (CurTickCount - TicksBack - 1)
		
		printf("Createmove\n");
		//CBacktracking RestoreData();
		// Backup for safety
		Vector origView = pCmd->viewangles;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup;
		Vector qAimAngles;
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);

		// Do da hacks
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal && pLocal->IsAlive())
			Hacks::MoveHacks(pCmd, bSendPacket);

		//testFloat3 = pCmd->tick_count;
		static int TESTERRR;
		// $updateTickCount - $tickCount < 1 && $updateTickCount - $tickCount > -1
		if (TESTERRR != LBYBreakerTimer) {
			TESTERRR = LBYBreakerTimer;
			//pCmd->viewangles.y -= 180;
			consoleWhenLBYCalledOnAntiAim = pLocal->GetTickBase();
		}

		//Movement Fix
		//GameUtils::CL_FixMove(pCmd, origView);
		qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f); // if pCmd->viewangles.x > 89, set pCmd->viewangles.x instead of 0.0f on first
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
		Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
		Vector vRightNorm;			Normalize(viewright, vRightNorm);
		Vector vUpNorm;				Normalize(viewup, vUpNorm);
		

		//if (abs(testFloat4 - pCmd->tick_count) < 5 && abs(testFloat4 - pCmd->tick_count) > 0) {
		//	pCmd->viewangles.y -= 180;
		//}

		// Original shit for movement correction
		float forward = pCmd->forwardmove;
		float right = pCmd->sidemove;
		float up = pCmd->upmove;
		if (forward > 450) forward = 450;
		if (right > 450) right = 450;
		if (up > 450) up = 450;
		if (forward < -450) forward = -450;
		if (right < -450) right = -450;
		if (up < -450) up = -450;
		pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
		pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
		pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);


		

		// Angle normalisation
		if (Menu::Window.MiscTab.OtherSafeMode.GetState())
		{
			GameUtils::NormaliseViewAngle(pCmd->viewangles);


			if (pCmd->viewangles.z != 0.0f)
			{
				pCmd->viewangles.z = 0.00;
			}

			if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
			{
				Utilities::Log("Having to re-normalise!");
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
				Beep(750, 800); // Why does it do this
				if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
				{
					pCmd->viewangles = origView;
					pCmd->sidemove = right;
					pCmd->forwardmove = forward;
				}
			}
		}

		if (pCmd->viewangles.x > 90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		if (pCmd->viewangles.x < -90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		

		
			
		
		if (bSendPacket)
			LastAngleAA = pCmd->viewangles;



		lineLBY = pLocal->GetLowerBodyYaw();
		if (bSendPacket == true) {
			lineFakeAngle = pCmd->viewangles.y; 
		}
		else {
			lineRealAngle = pCmd->viewangles.y;
		}

		static Vector vecAngles;
		Interfaces::Engine->GetViewAngles(vecAngles);

		if (Menu::Window.MiscTab.OtherThirdperson.GetState() && pLocal) {

			if (Menu::Window.MiscTab.OtherThirdpersonFake.GetState()) { // SHow FAKE if checked 

				if (!Interfaces::pInput->m_fCameraInThirdPerson)
				{

					Interfaces::pInput->m_fCameraInThirdPerson = true;
					Interfaces::pInput->m_vecCameraOffset = Vector(LastAngleAA.x, LastAngleAA.y, 100);
					//Interfaces::pInput->m_vecCameraOffset = LastAngleAA;

				}

			}
			else {

				if (!Interfaces::pInput->m_fCameraInThirdPerson)
				{

					Interfaces::pInput->m_fCameraInThirdPerson = true;
					Interfaces::pInput->m_vecCameraOffset = Vector(vecAngles.x, vecAngles.y, 100);
					//Interfaces::pInput->m_vecCameraOffset = LastAngleAA;
				}

			}

		}
		
		else {

			Interfaces::pInput->m_fCameraInThirdPerson = false;
			Interfaces::pInput->m_vecCameraOffset = Vector(vecAngles.x, vecAngles.y, 0);

		}

		printf("Createmove END\n");
		//pCmd->aimdirection -= 12;

		//pCmd->tick_count -= 12;
	}
	
	return false;
}




// Paint Traverse Hooked function
void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);

	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;
	

	if (!FoundPanel)
	{
		PCHAR szPanelName = (PCHAR)Interfaces::Panels->GetName(vguiPanel);
		if (strstr(szPanelName, "MatSystemTopPanel"))
		{
			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}
	}
	else if (FocusOverlayPanel == vguiPanel)
	{
		
		
		if (hitmarkertime > 0)
		{

			RECT View = Render::GetViewport();
			int MidX = View.right / 2;
			int MidY = View.bottom / 2;

			float alpha = hitmarkertime;
			IGameEvent* pEvent;

			Render::Line(MidX - 15, MidY - 10, MidX + 10, MidY + 15, Color(255, 0, 0, alpha));
			Render::Line(MidX + 15, MidY - 10, MidX - 10, MidY + 15, Color(255, 0, 0, alpha));

			hitmarkertime -= 2;
		}
		
		if (islbyupdate == true) {
			//char buffer1[64];
			//sprintf_s(buffer1, "LBY Breaker:  %f", testFloat1);
			Render::Text(70, 420, Color(0, 255, 0, 255), Render::Fonts::LBY, "LBY");
		}
		else {
			//char buffer1[64];
			//sprintf_s(buffer1, "LBY Breaker:  %f", testFloat1);
			Render::Text(70, 420, Color(255, 0, 0, 150), Render::Fonts::LBY, "LBY ");
		}

		if (antiAimSide == true) {
			// HEAD RIGHT
			Render::Text(500, 130, Color(14, 134, 9, 255), Render::Fonts::LBY, "<- HEAD LEFT");
		}
		else {
			// HEAD LEFT       HEAD RIGHT ->
			Render::Text(500, 130, Color(14, 134, 9, 255), Render::Fonts::LBY, "HEAD RIGHT ->");
		}

		
		

		char buffer3[64];
		sprintf_s(buffer3, "ShotsFired:  %i", shotsfired);
		Render::Text(70, 140, Color(0, 178, 255, 255), Render::Fonts::MenuBold, buffer3);

		// 3547


		//static float test5;
		//test5 = testFloat4 - testFloat3;
		

		
		char buffer7[64];
		sprintf_s(buffer7, "LBY Breaker Timer:  %i", LBYBreakerTimer);
		Render::Text(70, 200, Color(0, 255, 0, 255), Render::Fonts::MenuBold, buffer7);

		char buffer8[64];
		sprintf_s(buffer8, "Prediction Next LBY Update:  %f", myLBYTimer);
		Render::Text(70, 220, Color(0, 255, 0, 255), Render::Fonts::MenuBold, buffer8);

		char buffer9[64];
		sprintf_s(buffer9, "Proxy LBY Time:  %f", testFloat2);
		Render::Text(70, 240, Color(0, 255, 0, 255), Render::Fonts::MenuBold, buffer9);

		char buffer33[64];
		sprintf_s(buffer33, "Round Timer:  %i", roundStartTimer);
		Render::Text(70, 260, Color(0, 255, 0, 255), Render::Fonts::MenuBold, buffer33);

		static float test9394;
		if (testServerTick == 128) {
			test9394 = testServerTick / 128;
		}
		else {
			test9394 = testServerTick / 64;
		}

		char buffer333[64];
		sprintf_s(buffer333, "1 Tick:  %.1f", test9394); // testtimeToTick
		Render::Text(70, 280, Color(0, 178, 0, 255), Render::Fonts::MenuBold, buffer333);


		char buffer33333[64];
		sprintf_s(buffer33333, "Server Tick:  %f", testServerTick);
		Render::Text(70, 300, Color(0, 178, 0, 255), Render::Fonts::MenuBold, buffer33333);

		char bufferLineLBY[64];
		sprintf_s(bufferLineLBY, "LowerBodyYaw:  %.1f", lineLBY);
		Render::Text(70, 320, Color(249, 151, 54, 255), Render::Fonts::MenuBold, bufferLineLBY);

		char bufferlineRealAngle[64];
		sprintf_s(bufferlineRealAngle, "Real Angle:  %.1f", lineRealAngle);
		Render::Text(70, 340, Color(0, 255, 0, 255), Render::Fonts::MenuBold, bufferlineRealAngle);

		char bufferlineFakeAngle[64];
		sprintf_s(bufferlineFakeAngle, "Fake Angle:  %.1f", lineFakeAngle);
		Render::Text(70, 360, Color(202, 43, 43, 255), Render::Fonts::MenuBold, bufferlineFakeAngle);

		char buffermissedLoghits[64];
		sprintf_s(buffermissedLoghits, "Missed Shots:  %i", missedLogHits);
		Render::Text(70, 380, Color(202, 43, 43, 255), Render::Fonts::MenuBold, buffermissedLoghits);

		char bufferhittedhits[64];
		sprintf_s(bufferhittedhits, "Hitted Shots:  %i", hittedLogHits);
		Render::Text(70, 400, Color(202, 43, 43, 255), Render::Fonts::MenuBold, bufferhittedhits);


		


		if (Menu::Window.MiscTab.OtherWatermark.GetState())
		{
			//RECT scrn = Render::GetViewport();
			//Render::GradientV(8, 8, 160, 18, Color(0, 0, 0, 0), Color(7, 39, 17, 255));
			//RECT TextSize = Render::GetTextSize(Render::Fonts::ESP, "ApocalypseCheats");
			//Render::Text(scrn.right - TextSize.right - 20, 10, Color(Menu::Window.ColorTab.MenuInnerR.GetValue(), Menu::Window.ColorTab.MenuInnerG.GetValue(), Menu::Window.ColorTab.MenuInnerB.GetValue(), 255), Render::Fonts::MenuBold, "ApocalypseCheats");
			//Render::Text(10, 10, Color(Menu::Window.ColorTab.MenuInnerR.GetValue(), Menu::Window.ColorTab.MenuInnerG.GetValue(), Menu::Window.ColorTab.MenuInnerB.GetValue(), 255), Render::Fonts::MenuBold, "Private");
		}
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
			Hacks::DrawHacks();

		// Update and draw the menu
		Menu::DoUIFrame();
	}
}

// InPrediction Hooked Function
bool __stdcall Hooked_InPrediction()
{
	bool result;
	static InPrediction_ origFunc = (InPrediction_)Hooks::VMTPrediction.GetOriginalFunction(14);
	static DWORD *ecxVal = Interfaces::Prediction;
	result = origFunc(ecxVal);

	// If we are in the right place where the player view is calculated
	// Calculate the change in the view and get rid of it
	if (Menu::Window.VisualsTab.OtherNoVisualRecoil.GetState() && (DWORD)(_ReturnAddress()) == Offsets::Functions::dwCalcPlayerView)
	{
		IClientEntity* pLocalEntity = NULL;

		float* m_LocalViewAngles = NULL;

		__asm
		{
			MOV pLocalEntity, ESI
			MOV m_LocalViewAngles, EBX
		}

		Vector viewPunch = pLocalEntity->localPlayerExclusive()->GetViewPunchAngle();
		Vector aimPunch = pLocalEntity->localPlayerExclusive()->GetAimPunchAngle();

		m_LocalViewAngles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
		m_LocalViewAngles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
		m_LocalViewAngles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		return true;
	}

	return result;
}

// DrawModelExec for chams and shit
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{
	Color color;
	float flColor[3] = { 0.f };
	static IMaterial* CoveredLit = CreateMaterial(true);
	static IMaterial* OpenLit = CreateMaterial(false);
	static IMaterial* CoveredFlat = CreateMaterial(true, false);
	static IMaterial* OpenFlat = CreateMaterial(false, false);
	bool DontDraw = false;

	const char* ModelName = Interfaces::ModelInfo->GetModelName((model_t*)pInfo.pModel);
	IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Menu::Window.VisualsTab.Active.GetState())
	{
		// Player Chams
		int ChamsStyle = Menu::Window.VisualsTab.OptionsChams.GetIndex();
		int HandsStyle = Menu::Window.VisualsTab.OtherNoHands.GetIndex();


		//MADE BY CyclesProgramming
		//

		//terrorist red
		float TR = Menu::Window.ColorTab.TVisColorR.GetValue();
		float TNVR = Menu::Window.ColorTab.TNVisColorR.GetValue();
		//terrorist green
		float TG = Menu::Window.ColorTab.TVisColorG.GetValue();
		float TNVG = Menu::Window.ColorTab.TNVisColorG.GetValue();
		//terrorist green
		float TB = Menu::Window.ColorTab.TVisColorB.GetValue();
		float TNVB = Menu::Window.ColorTab.TNVisColorB.GetValue();

		//ct red
		float CTR = Menu::Window.ColorTab.CTVisColorR.GetValue();
		float CTNVR = Menu::Window.ColorTab.CTNVisColorR.GetValue();
		//ct green
		float CTG = Menu::Window.ColorTab.CTVisColorG.GetValue();
		float CTNVG = Menu::Window.ColorTab.CTNVisColorG.GetValue();
		//ct blue
		float CTB = Menu::Window.ColorTab.CTVisColorB.GetValue();
		float CTNVB = Menu::Window.ColorTab.CTNVisColorB.GetValue();



		if (ChamsStyle != 0 && Menu::Window.VisualsTab.FiltersPlayers.GetState() && strstr(ModelName, "models/player"))
		{
			if (pLocal/* && (!Menu::Window.VisualsTab.FiltersEnemiesOnly.GetState() || pModelEntity->GetTeamNum() != pLocal->GetTeamNum())*/)
			{
				IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
				IMaterial *open = ChamsStyle == 1 ? OpenLit : OpenFlat;

				IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
				if (pModelEntity)
				{
					IClientEntity *local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
					if (local)
					{
						if (pModelEntity->IsAlive() && pModelEntity->GetHealth() > 0 /*&& pModelEntity->GetTeamNum() != local->GetTeamNum()*/)
						{
							int alpharaw = Menu::Window.ColorTab.CharmsA.GetValue();

							float alpha2 = alpharaw / 255;
							float alpha;

							if (pModelEntity->HasGunGameImmunity())
								alpha2 = 0.5f;

							
							if (pModelEntity->GetTeamNum() == 2)
							{
								flColor[0] = TNVR / 255.f;
								flColor[1] = TNVG / 255.f;
								flColor[2] = TNVB / 255.f;
								if (ChamsStyle == 3 || ChamsStyle == 4)
								{
									alpha = 0;
								}
								else
								{
									alpha = alpha2;
								}

							}
							else
							{
								flColor[0] = CTNVR / 255.f;
								flColor[1] = CTNVG / 255.f;
								flColor[2] = CTNVB / 255.f;
								if (ChamsStyle == 3 || ChamsStyle == 4)
								{
									alpha = 0;
								}
								else
								{
									alpha = alpha2;
								}
							}

							Interfaces::RenderView->SetColorModulation(flColor);
							Interfaces::RenderView->SetBlend(alpha);
							Interfaces::ModelRender->ForcedMaterialOverride(covered);
							oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

							if (pModelEntity->GetTeamNum() == 2)
							{
								flColor[0] = TR / 255.f;
								flColor[1] = TG / 255.f;
								flColor[2] = TB / 255.f;
							}
							else
							{
								flColor[0] = CTR / 255.f;
								flColor[1] = CTG / 255.f;
								flColor[2] = CTB / 255.f;
							}

							Interfaces::RenderView->SetColorModulation(flColor);
							Interfaces::RenderView->SetBlend(alpha);
							Interfaces::ModelRender->ForcedMaterialOverride(open);
						}
						else
						{
							color.SetColor(255, 255, 255, 255);
							ForceMaterial(color, open);
						}
					}
				}
			}
		}
		else if (HandsStyle != 0 && strstr(ModelName, "arms"))
		{
			if (HandsStyle == 1)
			{
				DontDraw = true;
			}
			else if (HandsStyle == 2)
			{
				Interfaces::RenderView->SetBlend(0.3);
			}
			else if (HandsStyle == 3)
			{
				IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
				IMaterial *open = ChamsStyle == 1 ? OpenLit : OpenFlat;
				if (pLocal)
				{
					if (pLocal->IsAlive())
					{
						int alpha = pLocal->HasGunGameImmunity() ? 150 : 255;

						if (pLocal->GetTeamNum() == 2)
							color.SetColor(240, 30, 35, alpha);
						else
							color.SetColor(63, 72, 205, alpha);

						ForceMaterial(color, covered);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

						if (pLocal->GetTeamNum() == 2)
							color.SetColor(247, 180, 20, alpha);
						else
							color.SetColor(32, 180, 57, alpha);
					}
					else
					{
						color.SetColor(255, 255, 255, 255);
					}

					ForceMaterial(color, open);
				}
			}
			else
			{
				static int counter = 0;
				static float colors[3] = { 1.f, 0.f, 0.f };

				if (colors[counter] >= 1.0f)
				{
					colors[counter] = 1.0f;
					counter += 1;
					if (counter > 2)
						counter = 0;
				}
				else
				{
					int prev = counter - 1;
					if (prev < 0) prev = 2;
					colors[prev] -= 0.05f;
					colors[counter] += 0.05f;
				}

				Interfaces::RenderView->SetColorModulation(colors);
				Interfaces::RenderView->SetBlend(0.3);
				Interfaces::ModelRender->ForcedMaterialOverride(OpenLit);
			}
		}
		else if (ChamsStyle != 0 && Menu::Window.VisualsTab.FiltersWeapons.GetState() && strstr(ModelName, "_dropped.mdl"))
		{
			IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
			color.SetColor(255, 255, 255, 255);
			ForceMaterial(color, covered);
		}
	}

	if (!DontDraw)
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	Interfaces::ModelRender->ForcedMaterialOverride(NULL);
}

template <typename T, std::size_t N> T* end_(T(&arr)[N]) { return arr + N; }
template <typename T, std::size_t N> T* begin_(T(&arr)[N]) { return arr; }



// Hooked FrameStageNotify for removing visual recoil
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	
	DWORD eyeangles = NetVar.GetNetVar(0xBFEA4E7B);
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	
	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_RENDER_START)
	{

		if (pLocal->IsAlive())
		{
			if (*(bool*)((DWORD)Interfaces::pInput + 0xA5))
				*(Vector*)((DWORD)pLocal + 0x31C8) = LastAngleAA;
		}

		/*
		if (Menu::Window.VisualsTab.NightMode.GetState()) {
			{
				for (MaterialHandle_t i = Interfaces::MaterialSystem->FirstMaterial(); i != Interfaces::MaterialSystem->InvalidMaterial(); i = Interfaces::MaterialSystem->NextMaterial(i))
				{
					IMaterial *pMaterial = Interfaces::MaterialSystem->GetMaterial(i);

					if (!pMaterial)
						continue;

					if (strstr(pMaterial->GetTextureGroupName(), "World")) {
						//	pMaterial->AlphaModulate(0 / 255);
						pMaterial->ColorModulate(0.1, 0.1, 0.1);
					}

				}
			}
		} */


		if ((Menu::Window.MiscTab.OtherThirdperson.GetState()) || Menu::Window.VisualsTab.OtherAsus.GetState() || Menu::Window.VisualsTab.OtherWireframe.GetState() || Menu::Window.VisualsTab.OtherNoSmoke.GetState())
		{
			
			static bool rekt = false;
			if (!rekt)
			{
				ConVar* sv_cheats = Interfaces::CVar->FindVar("sv_cheats");
				SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
				sv_cheats_spoofed->SetInt(1);
				rekt = true;
			}
		}


		static bool wireframe;
		if (Menu::Window.VisualsTab.OtherWireframe.GetState())
		{
			if (!wireframe)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("r_drawothermodels 2");
				wireframe = true;
			}
		}
		else if (!Menu::Window.VisualsTab.OtherWireframe.GetState())
		{
			if (wireframe)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("r_drawothermodels 1");
				wireframe = false;
			}
			
		}
		static bool nosmoke;
		if (Menu::Window.VisualsTab.OtherNoSmoke.GetState())
		{

			if (!nosmoke) {
				Interfaces::Engine->ClientCmd_Unrestricted("r_drawparticles 0");
				Interfaces::Engine->ClientCmd_Unrestricted("fog_enable 0");
				nosmoke = true;
			}

		}
		else if (!Menu::Window.VisualsTab.OtherNoSmoke.GetState())
		{
			if (nosmoke)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("r_drawparticles 1");
				Interfaces::Engine->ClientCmd_Unrestricted("fog_enable 1");
				nosmoke = false;
			}
			
		}
		static bool xd;
		if (Menu::Window.VisualsTab.OtherNoScope.GetState() && hackManager.pLocal()) {
			if (hackManager.pLocal()->IsScoped())
			{
				int width, height;
				Interfaces::Engine->GetScreenSize(width, height);
				Render::Line(width / 2, 0, width / 2, height, Color(0, 0, 0, 255));
				Render::Line(0, height / 2, width, height / 2, Color(0, 0, 0, 255));

				ConVar* drawhud = Interfaces::CVar->FindVar("cl_DrawHud");
				*(int*)((DWORD)&drawhud->fnChangeCallback + 0xC) = 0;
				drawhud->SetValue(0);
			}
			else
			{
				ConVar* drawhud = Interfaces::CVar->FindVar("cl_DrawHud");
				drawhud->SetValue(1);
			}
		}
	}

	if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
		//Utilities::Log("APPLY SKIN APPLY SKIN");
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
		printf("HOOKS POSTUPDATE\n");
		//AllocConsole();
		//freopen("CONIN$", "r", stdin);
		//freopen("CONOUT$", "w", stdout);
		//freopen("CONOUT$", "w", stderr);

		//SetConsoleTitle("LBY Prediction");

		static float myOldLby;
		static float myoldTime;
		testtimeToTick = TIME_TO_TICKS(0.1);
		testServerTick = TIME_TO_TICKS(1);
		//testTickCount64 = Interfaces::Globals->tickcount;

		static int timerino;

		static float oneTickMinues;
		if (testServerTick == 128) {
			oneTickMinues = testServerTick / 128;
		}
		else {
			oneTickMinues = testServerTick / 64;
		}


		

		
		static float nextLBYUpdate;
		float serverTime = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		
		static float oldlby[65];
		static float oldtime[65];
		static bool isPredicted[65];

		
	

		static int startRound;
		
		// 1. Time on Server LBY Update Predicten
		if (consoleProxyLbyLASTUpdateTime <= 0 || startRound != roundStartTest) { // Everytime on each Server its at the Start 0
			// Predict each Round LBY NEW... 

			startRound = roundStartTest;
			nextLBYUpdate = 0;
			// Set nextUpdate to 0
			nextLBYUpdate = testFloat2;
			testFloat4 = 0;
			//memset(enemyLBYTimer, 0, sizeof(enemyLBYTimer));
			myLBYTimer = nextLBYUpdate;
			std::fill(begin_(enemyLBYTimer), end_(enemyLBYTimer), 0);
			std::fill(begin_(saveLastHeadshotFloat), end_(saveLastHeadshotFloat), 0);
			std::fill(begin_(saveLastBaimFloat), end_(saveLastBaimFloat), 0);
			std::fill(begin_(saveLastBaim30Float), end_(saveLastBaim30Float), 0);
			std::fill(begin_(saveLastBaim10Float), end_(saveLastBaim10Float), 0);
			
		}
		else {

			
			if (pLocal->IsAlive()) {

				if (pLocal->GetVelocity().Length2D() > 0.1f && pLocal->GetFlags() & FL_ONGROUND) {

					if (Interfaces::Globals->curtime >= nextLBYUpdate) {
						nextLBYUpdate = Interfaces::Globals->curtime + 0.22;  
						
					}
					

				}
				else {

					if (Interfaces::Globals->curtime >= nextLBYUpdate && pLocal->GetFlags() & FL_ONGROUND) {

						nextLBYUpdate = Interfaces::Globals->curtime + 1.1; 
						LBYBreakerTimer++;
						
						// TEST LBY PREDICT.... 
						//consoleFsnMyPredictTime = nextLBYUpdate;

						// BreakingLBY
						//testFloat4 = (int)floorf(TIME_TO_TICKS(nextLBYUpdate));
					}
					

				}
			}
			
		}
			
			myLBYTimer = nextLBYUpdate;

			static int startTickBase;
			static int timerxd;
			static float oldlbyyy[65];
			static float oldtimer[65];
			static bool isLBYPredictited[65];
			

			for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
			{
				IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
				if (!pEntity || pEntity->IsDormant() || !pEntity->IsAlive())
					continue;
				if (pEntity->GetTeamNum() == pLocal->GetTeamNum() || !pLocal->IsAlive())
					continue;
				Vector* eyeAngles = pEntity->GetEyeAnglesPointer();
				player_info_t pTemp;

				if (!Interfaces::Engine->GetPlayerInfo(i, &pTemp))
					continue;


				// We dont use this just for deco
				if (oldlbyyy[pEntity->GetIndex()] == pEntity->GetLowerBodyYaw()) {
					if (oldtimer[pEntity->GetIndex()] + 1.1f >= Interfaces::Globals->curtime) {
						oldtimer[pEntity->GetIndex()] = Interfaces::Globals->curtime;
						isLBYPredictited[pEntity->GetIndex()] = true;

					}
					else {
						isLBYPredictited[pEntity->GetIndex()] = false;
					}
				}
				else if (pEntity->IsDormant() || !pEntity->IsAlive()) {
					oldtimer[pEntity->GetIndex()] = -1;
				}
				else {
					oldlbyyy[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
					oldtimer[pEntity->GetIndex()] = Interfaces::Globals->curtime - nci->GetAvgLatency(FLOW_OUTGOING);
					isLBYPredictited[pEntity->GetIndex()] = false;
				}


				//.... Delta
				float deltadif = abs(pEntity->GetEyeAngles().y - pEntity->GetLowerBodyYaw());


				static float oldlowerbodyyaw;
				static float lbyproxytime;
				static int bullets;

				

					if (pEntity->GetVelocity().Length2D() > 0.1f) {

						ResolverStage[pEntity->GetIndex()] = 1;
						eyeAngles->y = pEntity->GetLowerBodyYaw();

					}
					else {


						if (shotsfired >= 4 && shotsfired <= 6) {
							if (saveLastHeadshotFloat[pEntity->GetIndex()] != 0) {
								ResolverStage[pEntity->GetIndex()] = 5;
								eyeAngles->y = saveLastHeadshotFloat[pEntity->GetIndex()];

							}
							else if (saveLastBaimFloat[pEntity->GetIndex()] != 0) {
								ResolverStage[pEntity->GetIndex()] = 5;

								eyeAngles->y = saveLastBaimFloat[pEntity->GetIndex()];
							}
							else if (saveLastBaim30Float[pEntity->GetIndex()] != 0) {
								ResolverStage[pEntity->GetIndex()] = 5;
								eyeAngles->y = saveLastBaim30Float[pEntity->GetIndex()];
							}
							else {
								// Just aimbot kek
								ResolverStage[pEntity->GetIndex()] = 6;
							}


						}
						else {

							lbyproxytime = enemysLastProxyTimer[pEntity->GetIndex()] + 0.15f;

							if (lbyproxytime != enemyLBYTimer[pEntity->GetIndex()] && abs(lbyproxytime - enemyLBYTimer[pEntity->GetIndex()]) > 0.6f) {



								ResolverStage[pEntity->GetIndex()] = 2;
								enemyLBYTimer[pEntity->GetIndex()] = lbyproxytime;
								eyeAngles->y = pEntity->GetLowerBodyYaw();
								oldlowerbodyyaw = pEntity->GetLowerBodyYaw();
							}


							else {

								ResolverStage[pEntity->GetIndex()] = 3;
								eyeAngles->y = oldlowerbodyyaw + deltadif;

							}

						}


					}


			


			}
					 // FOR SCHLEIFE END 

			



		
		

		int iBayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
		int iButterfly = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
		int iFlip = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
		int iGut = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
		int iKarambit = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
		int iM9Bayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
		int iHuntsman = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
		int iFalchion = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
		int iDagger = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
		int iBowie = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");

		int iGunGame = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gg.mdl");

		for (int i = 0; i <= Interfaces::EntList->GetHighestEntityIndex(); i++) // CHANGE
		{
			IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);

			if (pEntity)
			{
				ULONG hOwnerEntity = *(PULONG)((DWORD)pEntity + 0x148);

				IClientEntity* pOwner = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)hOwnerEntity);

				if (pOwner)
				{
					if (pOwner == pLocal)
					{
						std::string sWeapon = Interfaces::ModelInfo->GetModelName(pEntity->GetModel());

						if (!(sWeapon.find("models/weapons", 0) != std::string::npos))
							continue;

						if (sWeapon.find("c4_planted", 0) != std::string::npos)
							continue;

						if (sWeapon.find("thrown", 0) != std::string::npos)
							continue;

						if (sWeapon.find("smokegrenade", 0) != std::string::npos)
							continue;

						if (sWeapon.find("flashbang", 0) != std::string::npos)
							continue;

						if (sWeapon.find("fraggrenade", 0) != std::string::npos)
							continue;

						if (sWeapon.find("molotov", 0) != std::string::npos)
							continue;

						if (sWeapon.find("decoy", 0) != std::string::npos)
							continue;

						if (sWeapon.find("incendiarygrenade", 0) != std::string::npos)
							continue;

						if (sWeapon.find("ied", 0) != std::string::npos)
							continue;

						if (sWeapon.find("w_eq_", 0) != std::string::npos)
							continue;

						CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)pEntity;

						ClientClass *pClass = Interfaces::Client->GetAllClasses();

						if (Menu::Window.SkinchangerTab.SkinEnable.GetState())
						{
							int Model = Menu::Window.SkinchangerTab.KnifeModel.GetIndex();

							int M41S = Menu::Window.SkinchangerTab.M41SSkin.GetIndex();
							int M4A4 = Menu::Window.SkinchangerTab.M4A4Skin.GetIndex();
							int AK47 = Menu::Window.SkinchangerTab.AK47Skin.GetIndex();
							int AWP = Menu::Window.SkinchangerTab.AWPSkin.GetIndex();
							int GLOCK = Menu::Window.SkinchangerTab.GLOCKSkin.GetIndex();
							int USPS = Menu::Window.SkinchangerTab.USPSSkin.GetIndex();
							int DEAGLE = Menu::Window.SkinchangerTab.DEAGLESkin.GetIndex();
							int FIVE7 = Menu::Window.SkinchangerTab.FIVESEVENSkin.GetIndex();
							int AUG = Menu::Window.SkinchangerTab.AUGSkin.GetIndex();
							int FAMAS = Menu::Window.SkinchangerTab.FAMASSkin.GetIndex();
							int G3SG1 = Menu::Window.SkinchangerTab.G3SG1Skin.GetIndex();
							int Galil = Menu::Window.SkinchangerTab.GALILSkin.GetIndex();
							int M249 = Menu::Window.SkinchangerTab.M249Skin.GetIndex();
							int MAC10 = Menu::Window.MiscTab.MAC10Skin.GetIndex();
							int P90 = Menu::Window.MiscTab.P90Skin.GetIndex();
							int UMP45 = Menu::Window.MiscTab.UMP45Skin.GetIndex();
							int XM1014 = Menu::Window.SkinchangerTab.XM1014Skin.GetIndex();
							int BIZON = Menu::Window.MiscTab.BIZONSkin.GetIndex();
							int MAG7 = Menu::Window.SkinchangerTab.MAG7Skin.GetIndex();
							int NEGEV = Menu::Window.SkinchangerTab.NEGEVSkin.GetIndex();
							int SAWEDOFF = Menu::Window.SkinchangerTab.SAWEDOFFSkin.GetIndex();
							int TEC9 = Menu::Window.SkinchangerTab.TECNINESkin.GetIndex();
							int P2000 = Menu::Window.SkinchangerTab.P2000Skin.GetIndex();
							int MP7 = Menu::Window.MiscTab.MP7Skin.GetIndex();
							int MP9 = Menu::Window.MiscTab.MP9Skin.GetIndex();
							int NOVA = Menu::Window.SkinchangerTab.NOVASkin.GetIndex();
							int P250 = Menu::Window.SkinchangerTab.P250Skin.GetIndex();
							int SCAR20 = Menu::Window.SkinchangerTab.SCAR20Skin.GetIndex();
							int SG553 = Menu::Window.SkinchangerTab.SG553Skin.GetIndex();
							int SSG08 = Menu::Window.SkinchangerTab.SSG08Skin.GetIndex();
							int Magnum = Menu::Window.SkinchangerTab.DEAGLESkin.GetIndex();



							int weapon = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();

							switch (weapon)
							{
							case 7: // AK47 
							{
								switch (AK47)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 341;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 14;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 44;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 172;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 180;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 394;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 300;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 226;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 282;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 302;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 316;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 340;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 380;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 422;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 456;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 474;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 490;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 506;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 524;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 600;
									break;
								default:
									break;
								}
							}
							break;
							case 16: // M4A4
							{
								switch (M4A4)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 155;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 187;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 255;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 309;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 215;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 336;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 384;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 400;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 449;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 471;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 480;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 512;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 533;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 588;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 632;
									break;
								default:
									break;
								}
							}
							break;
							case 60: // M4A1
							{
								switch (M41S)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 60;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 430;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 77;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 254;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 189;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 301;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 217;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 257;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 321;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 326;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 360;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 383;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 440;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 445;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 497;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 548;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 587;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 631;
									break;
								default:
									break;
								}
							}
							break;
							case 9: // AWP
							{
								switch (AWP)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 174;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 344;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 84;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 30;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 51;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 72;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 181;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 259;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 395;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 212;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 227;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 251;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 279;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 424;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 446;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 451;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 475;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 525;
									break;
								default:
									break;
								}
							}
							break;
							case 61: // USP
							{
								switch (USPS)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 25;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 60;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 183;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 339;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 217;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 221;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 236;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 277;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 290;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 313;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 318;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 332;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 364;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 454;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 489;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 504;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 540;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 637;
									break;
								default:
									break;
								}
							}
							break;
							case 4: // Glock
							{
								switch (GLOCK)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 2;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 3;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 38;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 40;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 48;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 437;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 99;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 159;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 399;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 208;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 230;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 278;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 293;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 353;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 367;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 381;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 479;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 495;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 532;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 607;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 586;
									break;
								default:
									break;
								}
							}
							break;
							case 1: // Deagle
							{
								switch (DEAGLE)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 37;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 347;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 468;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 469;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 5;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 232;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 17;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 40;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 61;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 90;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 185;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 231;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 237;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 397;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 328;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 273;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 296;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 351;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 425;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 470;
									break;
								case 21:
									*pWeapon->FallbackPaintKit() = 509;
									break;
								case 22:
									*pWeapon->FallbackPaintKit() = 527;
									break;
								default:
									break;
								}
							}
							break;
							case 3: // Five Seven
							{
								switch (FIVE7)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 3;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 27;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 44;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 46;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 78;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 141;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 151;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 254;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 248;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 210;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 223;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 252;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 265;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 274;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 464;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 352;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 377;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 387;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 427;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 510;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 530;
									break;
								default:
									break;
								}
							}
							break;
							case 8: // AUG
							{
								switch (AUG)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 9;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 33;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 280;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 305;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 375;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 442;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 444;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 455;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 507;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 541;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 601;
									break;
								default:
									break;
								}
							}
							break;
							case 10: // Famas
							{
								switch (FAMAS)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 22;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 47;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 92;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 429;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 154;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 178;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 194;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 244;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 218;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 260;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 288;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 371;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 477;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 492;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 529;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 604;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 626;
									break;
								default:
									break;
								}
							}
							break;
							case 11: // G3SG1
							{
								switch (G3SG1)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 8;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 6;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 27;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 46;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 72;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 74;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 147;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 170;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 195;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 229;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 294;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 465;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 464;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 382;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 438;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 493;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 511;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 545;
									break;
								default:
									break;
								}
							}
							break;
							case 13: // Galil
							{
								switch (Galil)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 5;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 22;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 83;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 428;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 76;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 119;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 398;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 192;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 308;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 216;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 237;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 241;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 264;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 297;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 379;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 460;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 478;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 494;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 546;
									break;
								default:
									break;
								}
							}
							break;
							case 14: // M249
							{
								switch (M249)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 22;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 75;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 202;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 243;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 266;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 401;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 452;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 472;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 496;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 547;
									break;
								default:
									break;
								}
							}
							break;
							case 17: // Mac 10
							{
								switch (MAC10)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 101;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 3;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 32;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 5;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 17;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 38;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 433;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 98;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 157;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 188;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 337;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 246;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 284;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 310;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 333;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 343;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 372;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 402;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 498;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 534;
									break;
								default:
									break;
								}
							}
							break;
							case 19: // P90
							{
								switch (P90)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 342;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 20;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 22;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 100;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 67;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 111;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 124;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 156;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 234;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 169;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 175;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 182;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 244;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 228;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 283;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 311;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 335;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 359;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 486;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 516;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 636;
									break;
								default:
									break;
								}
							}
							break;
							case 24: // UMP-45
							{
								switch (UMP45)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 37;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 5;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 15;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 17;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 436;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 70;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 93;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 169;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 175;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 193;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 392;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 281;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 333;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 362;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 441;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 488;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 556;
									break;
								default:
									break;
								}
							}
							break;
							case 25: // XM1014
							{
								switch (XM1014)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 166;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 238;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 27;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 42;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 96;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 95;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 135;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 151;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 169;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 205;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 240;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 251;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 393;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 320;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 314;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 348;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 370;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 407;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 505;
									break;
								case 21:
									*pWeapon->FallbackPaintKit() = 521;
									break;
								case 22:
									*pWeapon->FallbackPaintKit() = 557;
									break;
								default:
									break;
								}
							}
							break;
							case 26: // Bizon
							{
								switch (BIZON)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 13;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 164;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 25;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 27;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 70;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 148;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 149;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 159;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 171;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 203;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 224;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 236;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 267;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 306;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 323;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 349;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 376;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 457;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 459;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 508;
									break;
								case 21:
									*pWeapon->FallbackPaintKit() = 526;
									break;
								case 22:
									*pWeapon->FallbackPaintKit() = 542;
									break;
								default:
									break;
								}
							}
							break;
							case 27: // Mag 7
							{
								switch (MAG7)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 462;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 34;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 32;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 100;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 39;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 431;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 99;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 171;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 177;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 198;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 291;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 385;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 473;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 499;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 535;
									break;
								default:
									break;
								}
							}
							break;
							case 28: // Negev
							{
								switch (NEGEV)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 28;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 432;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 157;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 201;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 240;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 285;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 298;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 317;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 355;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 369;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 483;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 514;
									break;
								default:
									break;
								}
							}
							break;
							case 29: // Sawed Off
							{
								switch (SAWEDOFF)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 345;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 5;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 22;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 30;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 83;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 38;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 41;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 434;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 119;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 171;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 204;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 405;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 246;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 250;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 390;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 256;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 323;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 458;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 459;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 517;
									break;
								case 21:
									*pWeapon->FallbackPaintKit() = 552;
									break;
								case 22:
									*pWeapon->FallbackPaintKit() = 638;
									break;
								default:
									break;
								}
							}
							break;
							case 30: // Tec 9
							{
								switch (TEC9)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 101;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 2;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 5;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 463;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 17;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 36;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 439;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 159;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 179;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 248;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 206;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 216;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 242;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 272;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 289;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 303;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 374;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 459;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 520;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 539;
									break;
								case 21:
									*pWeapon->FallbackPaintKit() = 555;
									break;
								case 22:
									*pWeapon->FallbackPaintKit() = 614;
									break;
								default:
									break;
								}
							}
							break;
							case 32: // P2000
							{
								switch (P2000)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 104;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 32;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 21;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 25;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 36;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 485;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 38;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 71;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 95;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 184;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 211;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 338;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 246;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 275;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 327;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 346;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 357;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 389;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 442;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 443;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 515;
									break;
								case 21:
									*pWeapon->FallbackPaintKit() = 550;
									break;
								case 22:
									*pWeapon->FallbackPaintKit() = 591;
									break;
								default:
									break;
								}
							}
							break;
							case 33: // MP7
							{
								switch (MP7)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 2;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 102;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 5;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 28;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 11;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 15;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 22;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 27;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 36;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 141;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 235;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 245;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 209;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 213;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 250;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 354;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 365;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 423;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 442;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 481;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 500;
									break;
								case 21:
									*pWeapon->FallbackPaintKit() = 536;
									break;
								default:
									break;
								}
							}
							break;
							case 34: // MP9
							{
								switch (MP9)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 482;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 27;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 33;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 100;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 39;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 61;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 148;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 141;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 199;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 329;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 262;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 366;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 368;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 386;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 403;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 448;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 549;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 609;
									break;
								default:
									break;
								}
							}
							break;
							case 35: // Nova
							{
								switch (NOVA)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 3;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 166;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 164;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 25;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 62;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 99;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 107;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 158;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 170;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 191;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 214;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 225;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 263;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 286;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 294;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 299;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 356;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 450;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 484;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 537;
									break;
								default:
									break;
								}
							}
							break;
							case 36: // P250
							{
								switch (P250)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 102;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 34;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 162;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 15;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 164;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 27;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 77;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 99;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 168;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 258;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 207;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 219;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 404;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 230;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 271;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 295;
									break;
								case 16:
									*pWeapon->FallbackPaintKit() = 464;
									break;
								case 17:
									*pWeapon->FallbackPaintKit() = 358;
									break;
								case 18:
									*pWeapon->FallbackPaintKit() = 373;
									break;
								case 19:
									*pWeapon->FallbackPaintKit() = 388;
									break;
								case 20:
									*pWeapon->FallbackPaintKit() = 426;
									break;
								case 21:
									*pWeapon->FallbackPaintKit() = 466;
									break;
								case 22:
									*pWeapon->FallbackPaintKit() = 467;
									break;
								case 23:
									*pWeapon->FallbackPaintKit() = 501;
									break;
								case 24:
									*pWeapon->FallbackPaintKit() = 551;
									break;
								default:
									break;
								}
							}
							break;
							case 38: // Scar 20
							{
								switch (SCAR20)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 165;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 100;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 46;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 70;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 116;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 157;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 196;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 232;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 391;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 298;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 312;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 406;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 453;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 502;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 518;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 597;
									break;
								default:
									break;
								}
							}
							break;
							case 39: // SG553
							{
								switch (SG553)
								{
								case 1:
									*pWeapon->FallbackPaintKit() = 39;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 98;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 410;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 347;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 287;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 298;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 363;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 378;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 487;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 519;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 553;
									break;
								default:
									break;
								}
							}
							break;
							case 40: // SSG08
							{
								switch (SSG08)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 26;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 60;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 96;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 99;
									break;
								case 4:
									*pWeapon->FallbackPaintKit() = 157;
									break;
								case 5:
									*pWeapon->FallbackPaintKit() = 200;
									break;
								case 6:
									*pWeapon->FallbackPaintKit() = 222;
									break;
								case 7:
									*pWeapon->FallbackPaintKit() = 233;
									break;
								case 8:
									*pWeapon->FallbackPaintKit() = 253;
									break;
								case 9:
									*pWeapon->FallbackPaintKit() = 304;
									break;
								case 10:
									*pWeapon->FallbackPaintKit() = 319;
									break;
								case 11:
									*pWeapon->FallbackPaintKit() = 361;
									break;
								case 12:
									*pWeapon->FallbackPaintKit() = 503;
									break;
								case 13:
									*pWeapon->FallbackPaintKit() = 538;
									break;
								case 14:
									*pWeapon->FallbackPaintKit() = 554;
									break;
								case 15:
									*pWeapon->FallbackPaintKit() = 624;
									break;
								default:
									break;
								}
							}
							break;
							case 64: // Revolver
							{
								switch (Magnum)
								{
								case 0:
									*pWeapon->FallbackPaintKit() = 27;
									break;
								case 1:
									*pWeapon->FallbackPaintKit() = 12;
									break;
								case 2:
									*pWeapon->FallbackPaintKit() = 522;
									break;
								case 3:
									*pWeapon->FallbackPaintKit() = 523;
									break;
								default:
									break;
								}
							}
							break;
							default:
								break;
							}



							if (pEntity->GetClientClass()->m_ClassID == (int)CSGOClassID::CKnife)
							{
								auto pCustomName1 = MakePtr(char*, pWeapon, 0x301C);


								if (Model == 0) // Bayonet
								{
									*pWeapon->ModelIndex() = iBayonet; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iBayonet;
									*pWeapon->WorldModelIndex() = iBayonet + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 500;

									int Skin = Menu::Window.SkinchangerTab.KnifeSkin.GetIndex();

									if (Skin == 0)
									{
										*pWeapon->FallbackPaintKit() = 0; // Forest DDPAT
									}
									else if (Skin == 1)
									{
										*pWeapon->FallbackPaintKit() = 12; // Crimson Web
									}
									else if (Skin == 2)
									{
										*pWeapon->FallbackPaintKit() = 27; // Bone Mask
									}
									else if (Skin == 3)
									{
										*pWeapon->FallbackPaintKit() = 38; // Fade
									}
									else if (Skin == 4)
									{
										*pWeapon->FallbackPaintKit() = 40; // Night
									}
									else if (Skin == 5)
									{
										*pWeapon->FallbackPaintKit() = 42; // Blue Steel
									}
									else if (Skin == 6)
									{
										*pWeapon->FallbackPaintKit() = 43; // Stained
									}
									else if (Skin == 7)
									{
										*pWeapon->FallbackPaintKit() = 44; // Case Hardened
									}
									else if (Skin == 8)
									{
										*pWeapon->FallbackPaintKit() = 59; // Slaughter
									}
									else if (Skin == 9)
									{
										*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
									}
									else if (Skin == 10)
									{
										*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
									}
									else if (Skin == 11)
									{
										*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
									}
									else if (Skin == 12)
									{
										*pWeapon->FallbackPaintKit() = 143; // Urban Masked
									}
									else if (Skin == 13)
									{
										*pWeapon->FallbackPaintKit() = 175; // Scorched
									}
									else if (Skin == 14)
									{
										*pWeapon->FallbackPaintKit() = 323; // Rust Coat
									}
									else if (Skin == 15)
									{
										*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
									}
									else if (Skin == 16)
									{
										*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
									}
									else if (Skin == 17)
									{
										*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
									}
									else if (Skin == 18)
									{
										*pWeapon->FallbackPaintKit() = 413; // Marble Fade
									}
									else if (Skin == 19)
									{
										*pWeapon->FallbackPaintKit() = 414; // Rust Coat
									}
									else if (Skin == 20)
									{
										*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
									}
									else if (Skin == 21)
									{
										*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
									}
									else if (Skin == 22)
									{
										*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
									}
									else if (Skin == 23)
									{
										*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
									}
									else if (Skin == 24)
									{
										*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
									}
									else if (Skin == 25)
									{
										*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
									}
									else if (Skin == 26)
									{
										*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
									}
									else if (Skin == 29)
									{
										*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
									}
									else if (Skin == 30)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
									}
									else if (Skin == 31)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
									}
									else if (Skin == 32)
									{
										*pWeapon->FallbackPaintKit() = 558; // Lore
									}
								}
								else if (Model == 1) // Bowie Knife
								{
									*pWeapon->ModelIndex() = iBowie; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iBowie;
									*pWeapon->WorldModelIndex() = iBowie + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 514;

									int Skin = Menu::Window.SkinchangerTab.KnifeSkin.GetIndex();

									if (Skin == 0)
									{
										*pWeapon->FallbackPaintKit() = 0; // Forest DDPAT
									}
									else if (Skin == 1)
									{
										*pWeapon->FallbackPaintKit() = 12; // Crimson Web
									}
									else if (Skin == 2)
									{
										*pWeapon->FallbackPaintKit() = 27; // Bone Mask
									}
									else if (Skin == 3)
									{
										*pWeapon->FallbackPaintKit() = 38; // Fade
									}
									else if (Skin == 4)
									{
										*pWeapon->FallbackPaintKit() = 40; // Night
									}
									else if (Skin == 5)
									{
										*pWeapon->FallbackPaintKit() = 42; // Blue Steel
									}
									else if (Skin == 6)
									{
										*pWeapon->FallbackPaintKit() = 43; // Stained
									}
									else if (Skin == 7)
									{
										*pWeapon->FallbackPaintKit() = 44; // Case Hardened
									}
									else if (Skin == 8)
									{
										*pWeapon->FallbackPaintKit() = 59; // Slaughter
									}
									else if (Skin == 9)
									{
										*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
									}
									else if (Skin == 10)
									{
										*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
									}
									else if (Skin == 11)
									{
										*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
									}
									else if (Skin == 12)
									{
										*pWeapon->FallbackPaintKit() = 143; // Urban Masked
									}
									else if (Skin == 13)
									{
										*pWeapon->FallbackPaintKit() = 175; // Scorched
									}
									else if (Skin == 14)
									{
										*pWeapon->FallbackPaintKit() = 323; // Rust Coat
									}
									else if (Skin == 15)
									{
										*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
									}
									else if (Skin == 16)
									{
										*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
									}
									else if (Skin == 17)
									{
										*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
									}
									else if (Skin == 18)
									{
										*pWeapon->FallbackPaintKit() = 413; // Marble Fade
									}
									else if (Skin == 19)
									{
										*pWeapon->FallbackPaintKit() = 414; // Rust Coat
									}
									else if (Skin == 20)
									{
										*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
									}
									else if (Skin == 21)
									{
										*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
									}
									else if (Skin == 22)
									{
										*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
									}
									else if (Skin == 23)
									{
										*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
									}
									else if (Skin == 24)
									{
										*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
									}
									else if (Skin == 25)
									{
										*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
									}
									else if (Skin == 26)
									{
										*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
									}
									else if (Skin == 29)
									{
										*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
									}
									else if (Skin == 30)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
									}
									else if (Skin == 31)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
									}
									else if (Skin == 32)
									{
										*pWeapon->FallbackPaintKit() = 558; // Lore
									}

								}
								else if (Model == 2) // Butterfly Knife
								{
									*pWeapon->ModelIndex() = iButterfly; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iButterfly;
									*pWeapon->WorldModelIndex() = iButterfly + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 515;

									int Skin = Menu::Window.SkinchangerTab.KnifeSkin.GetIndex();

									if (Skin == 0)
									{
										*pWeapon->FallbackPaintKit() = 0; // Forest DDPAT
									}
									else if (Skin == 1)
									{
										*pWeapon->FallbackPaintKit() = 12; // Crimson Web
									}
									else if (Skin == 2)
									{
										*pWeapon->FallbackPaintKit() = 27; // Bone Mask
									}
									else if (Skin == 3)
									{
										*pWeapon->FallbackPaintKit() = 38; // Fade
									}
									else if (Skin == 4)
									{
										*pWeapon->FallbackPaintKit() = 40; // Night
									}
									else if (Skin == 5)
									{
										*pWeapon->FallbackPaintKit() = 42; // Blue Steel
									}
									else if (Skin == 6)
									{
										*pWeapon->FallbackPaintKit() = 43; // Stained
									}
									else if (Skin == 7)
									{
										*pWeapon->FallbackPaintKit() = 44; // Case Hardened
									}
									else if (Skin == 8)
									{
										*pWeapon->FallbackPaintKit() = 59; // Slaughter
									}
									else if (Skin == 9)
									{
										*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
									}
									else if (Skin == 10)
									{
										*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
									}
									else if (Skin == 11)
									{
										*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
									}
									else if (Skin == 12)
									{
										*pWeapon->FallbackPaintKit() = 143; // Urban Masked
									}
									else if (Skin == 13)
									{
										*pWeapon->FallbackPaintKit() = 175; // Scorched
									}
									else if (Skin == 14)
									{
										*pWeapon->FallbackPaintKit() = 323; // Rust Coat
									}
									else if (Skin == 15)
									{
										*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
									}
									else if (Skin == 16)
									{
										*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
									}
									else if (Skin == 17)
									{
										*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
									}
									else if (Skin == 18)
									{
										*pWeapon->FallbackPaintKit() = 413; // Marble Fade
									}
									else if (Skin == 19)
									{
										*pWeapon->FallbackPaintKit() = 414; // Rust Coat
									}
									else if (Skin == 20)
									{
										*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
									}
									else if (Skin == 21)
									{
										*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
									}
									else if (Skin == 22)
									{
										*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
									}
									else if (Skin == 23)
									{
										*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
									}
									else if (Skin == 24)
									{
										*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
									}
									else if (Skin == 25)
									{
										*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
									}
									else if (Skin == 26)
									{
										*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
									}
									else if (Skin == 29)
									{
										*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
									}
									else if (Skin == 30)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
									}
									else if (Skin == 31)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
									}
									else if (Skin == 32)
									{
										*pWeapon->FallbackPaintKit() = 558; // Lore
									}

								}
								else if (Model == 3) // Falchion Knife
								{
									*pWeapon->ModelIndex() = iFalchion; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iFalchion;
									*pWeapon->WorldModelIndex() = iFalchion + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 512;

									int Skin = Menu::Window.SkinchangerTab.KnifeSkin.GetIndex();

									if (Skin == 0)
									{
										*pWeapon->FallbackPaintKit() = 0; // Forest DDPAT
									}
									else if (Skin == 1)
									{
										*pWeapon->FallbackPaintKit() = 12; // Crimson Web
									}
									else if (Skin == 2)
									{
										*pWeapon->FallbackPaintKit() = 27; // Bone Mask
									}
									else if (Skin == 3)
									{
										*pWeapon->FallbackPaintKit() = 38; // Fade
									}
									else if (Skin == 4)
									{
										*pWeapon->FallbackPaintKit() = 40; // Night
									}
									else if (Skin == 5)
									{
										*pWeapon->FallbackPaintKit() = 42; // Blue Steel
									}
									else if (Skin == 6)
									{
										*pWeapon->FallbackPaintKit() = 43; // Stained
									}
									else if (Skin == 7)
									{
										*pWeapon->FallbackPaintKit() = 44; // Case Hardened
									}
									else if (Skin == 8)
									{
										*pWeapon->FallbackPaintKit() = 59; // Slaughter
									}
									else if (Skin == 9)
									{
										*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
									}
									else if (Skin == 10)
									{
										*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
									}
									else if (Skin == 11)
									{
										*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
									}
									else if (Skin == 12)
									{
										*pWeapon->FallbackPaintKit() = 143; // Urban Masked
									}
									else if (Skin == 13)
									{
										*pWeapon->FallbackPaintKit() = 175; // Scorched
									}
									else if (Skin == 14)
									{
										*pWeapon->FallbackPaintKit() = 323; // Rust Coat
									}
									else if (Skin == 15)
									{
										*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
									}
									else if (Skin == 16)
									{
										*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
									}
									else if (Skin == 17)
									{
										*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
									}
									else if (Skin == 18)
									{
										*pWeapon->FallbackPaintKit() = 413; // Marble Fade
									}
									else if (Skin == 19)
									{
										*pWeapon->FallbackPaintKit() = 414; // Rust Coat
									}
									else if (Skin == 20)
									{
										*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
									}
									else if (Skin == 21)
									{
										*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
									}
									else if (Skin == 22)
									{
										*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
									}
									else if (Skin == 23)
									{
										*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
									}
									else if (Skin == 24)
									{
										*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
									}
									else if (Skin == 25)
									{
										*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
									}
									else if (Skin == 26)
									{
										*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
									}
									else if (Skin == 29)
									{
										*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
									}
									else if (Skin == 30)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
									}
									else if (Skin == 31)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
									}
									else if (Skin == 32)
									{
										*pWeapon->FallbackPaintKit() = 558; // Lore
									}
								}
								else if (Model == 4) // Flip Knife
								{
									*pWeapon->ModelIndex() = iFlip; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iFlip;
									*pWeapon->WorldModelIndex() = iFlip + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 505;

									int Skin = Menu::Window.SkinchangerTab.KnifeSkin.GetIndex();

									if (Skin == 0)
									{
										*pWeapon->FallbackPaintKit() = 0; // Forest DDPAT
									}
									else if (Skin == 1)
									{
										*pWeapon->FallbackPaintKit() = 12; // Crimson Web
									}
									else if (Skin == 2)
									{
										*pWeapon->FallbackPaintKit() = 27; // Bone Mask
									}
									else if (Skin == 3)
									{
										*pWeapon->FallbackPaintKit() = 38; // Fade
									}
									else if (Skin == 4)
									{
										*pWeapon->FallbackPaintKit() = 40; // Night
									}
									else if (Skin == 5)
									{
										*pWeapon->FallbackPaintKit() = 42; // Blue Steel
									}
									else if (Skin == 6)
									{
										*pWeapon->FallbackPaintKit() = 43; // Stained
									}
									else if (Skin == 7)
									{
										*pWeapon->FallbackPaintKit() = 44; // Case Hardened
									}
									else if (Skin == 8)
									{
										*pWeapon->FallbackPaintKit() = 59; // Slaughter
									}
									else if (Skin == 9)
									{
										*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
									}
									else if (Skin == 10)
									{
										*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
									}
									else if (Skin == 11)
									{
										*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
									}
									else if (Skin == 12)
									{
										*pWeapon->FallbackPaintKit() = 143; // Urban Masked
									}
									else if (Skin == 13)
									{
										*pWeapon->FallbackPaintKit() = 175; // Scorched
									}
									else if (Skin == 14)
									{
										*pWeapon->FallbackPaintKit() = 323; // Rust Coat
									}
									else if (Skin == 15)
									{
										*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
									}
									else if (Skin == 16)
									{
										*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
									}
									else if (Skin == 17)
									{
										*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
									}
									else if (Skin == 18)
									{
										*pWeapon->FallbackPaintKit() = 413; // Marble Fade
									}
									else if (Skin == 19)
									{
										*pWeapon->FallbackPaintKit() = 414; // Rust Coat
									}
									else if (Skin == 20)
									{
										*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
									}
									else if (Skin == 21)
									{
										*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
									}
									else if (Skin == 22)
									{
										*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
									}
									else if (Skin == 23)
									{
										*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
									}
									else if (Skin == 24)
									{
										*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
									}
									else if (Skin == 25)
									{
										*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
									}
									else if (Skin == 26)
									{
										*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
									}
									else if (Skin == 29)
									{
										*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
									}
									else if (Skin == 30)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
									}
									else if (Skin == 31)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
									}
									else if (Skin == 32)
									{
										*pWeapon->FallbackPaintKit() = 559; // Lore
									}

								}
								else if (Model == 5) // Gut Knife
								{
									*pWeapon->ModelIndex() = iGut; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iGut;
									*pWeapon->WorldModelIndex() = iGut + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 506;

									int Skin = Menu::Window.SkinchangerTab.KnifeSkin.GetIndex();

									if (Skin == 0)
									{
										*pWeapon->FallbackPaintKit() = 0; // Forest DDPAT
									}
									else if (Skin == 1)
									{
										*pWeapon->FallbackPaintKit() = 12; // Crimson Web
									}
									else if (Skin == 2)
									{
										*pWeapon->FallbackPaintKit() = 27; // Bone Mask
									}
									else if (Skin == 3)
									{
										*pWeapon->FallbackPaintKit() = 38; // Fade
									}
									else if (Skin == 4)
									{
										*pWeapon->FallbackPaintKit() = 40; // Night
									}
									else if (Skin == 5)
									{
										*pWeapon->FallbackPaintKit() = 42; // Blue Steel
									}
									else if (Skin == 6)
									{
										*pWeapon->FallbackPaintKit() = 43; // Stained
									}
									else if (Skin == 7)
									{
										*pWeapon->FallbackPaintKit() = 44; // Case Hardened
									}
									else if (Skin == 8)
									{
										*pWeapon->FallbackPaintKit() = 59; // Slaughter
									}
									else if (Skin == 9)
									{
										*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
									}
									else if (Skin == 10)
									{
										*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
									}
									else if (Skin == 11)
									{
										*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
									}
									else if (Skin == 12)
									{
										*pWeapon->FallbackPaintKit() = 143; // Urban Masked
									}
									else if (Skin == 13)
									{
										*pWeapon->FallbackPaintKit() = 175; // Scorched
									}
									else if (Skin == 14)
									{
										*pWeapon->FallbackPaintKit() = 323; // Rust Coat
									}
									else if (Skin == 15)
									{
										*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
									}
									else if (Skin == 16)
									{
										*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
									}
									else if (Skin == 17)
									{
										*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
									}
									else if (Skin == 18)
									{
										*pWeapon->FallbackPaintKit() = 413; // Marble Fade
									}
									else if (Skin == 19)
									{
										*pWeapon->FallbackPaintKit() = 414; // Rust Coat
									}
									else if (Skin == 20)
									{
										*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
									}
									else if (Skin == 21)
									{
										*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
									}
									else if (Skin == 22)
									{
										*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
									}
									else if (Skin == 23)
									{
										*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
									}
									else if (Skin == 24)
									{
										*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
									}
									else if (Skin == 25)
									{
										*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
									}
									else if (Skin == 26)
									{
										*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
									}
									else if (Skin == 29)
									{
										*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
									}
									else if (Skin == 30)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
									}
									else if (Skin == 31)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
									}
									else if (Skin == 32)
									{
										*pWeapon->FallbackPaintKit() = 560; // Lore
									}


								}
								else if (Model == 6) // Huntsman Knife
								{
									*pWeapon->ModelIndex() = iHuntsman; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iHuntsman;
									*pWeapon->WorldModelIndex() = iHuntsman + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 509;

									int Skin = Menu::Window.SkinchangerTab.KnifeSkin.GetIndex();

									if (Skin == 0)
									{
										*pWeapon->FallbackPaintKit() = 0; // Forest DDPAT
									}
									else if (Skin == 1)
									{
										*pWeapon->FallbackPaintKit() = 12; // Crimson Web
									}
									else if (Skin == 2)
									{
										*pWeapon->FallbackPaintKit() = 27; // Bone Mask
									}
									else if (Skin == 3)
									{
										*pWeapon->FallbackPaintKit() = 38; // Fade
									}
									else if (Skin == 4)
									{
										*pWeapon->FallbackPaintKit() = 40; // Night
									}
									else if (Skin == 5)
									{
										*pWeapon->FallbackPaintKit() = 42; // Blue Steel
									}
									else if (Skin == 6)
									{
										*pWeapon->FallbackPaintKit() = 43; // Stained
									}
									else if (Skin == 7)
									{
										*pWeapon->FallbackPaintKit() = 44; // Case Hardened
									}
									else if (Skin == 8)
									{
										*pWeapon->FallbackPaintKit() = 59; // Slaughter
									}
									else if (Skin == 9)
									{
										*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
									}
									else if (Skin == 10)
									{
										*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
									}
									else if (Skin == 11)
									{
										*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
									}
									else if (Skin == 12)
									{
										*pWeapon->FallbackPaintKit() = 143; // Urban Masked
									}
									else if (Skin == 13)
									{
										*pWeapon->FallbackPaintKit() = 175; // Scorched
									}
									else if (Skin == 14)
									{
										*pWeapon->FallbackPaintKit() = 323; // Rust Coat
									}
									else if (Skin == 15)
									{
										*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
									}
									else if (Skin == 16)
									{
										*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
									}
									else if (Skin == 17)
									{
										*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
									}
									else if (Skin == 18)
									{
										*pWeapon->FallbackPaintKit() = 413; // Marble Fade
									}
									else if (Skin == 19)
									{
										*pWeapon->FallbackPaintKit() = 414; // Rust Coat
									}
									else if (Skin == 20)
									{
										*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
									}
									else if (Skin == 21)
									{
										*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
									}
									else if (Skin == 22)
									{
										*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
									}
									else if (Skin == 23)
									{
										*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
									}
									else if (Skin == 24)
									{
										*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
									}
									else if (Skin == 25)
									{
										*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
									}
									else if (Skin == 26)
									{
										*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
									}
									else if (Skin == 29)
									{
										*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
									}
									else if (Skin == 30)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
									}
									else if (Skin == 31)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
									}
									else if (Skin == 32)
									{
										*pWeapon->FallbackPaintKit() = 559; // Lore
									}


								}
								else if (Model == 7) // Karambit
								{
									*pWeapon->ModelIndex() = iKarambit; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iKarambit;
									*pWeapon->WorldModelIndex() = iKarambit + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 507;

									int Skin = Menu::Window.SkinchangerTab.KnifeSkin.GetIndex();

									if (Skin == 0)
									{
										*pWeapon->FallbackPaintKit() = 0; // Forest DDPAT
									}
									else if (Skin == 1)
									{
										*pWeapon->FallbackPaintKit() = 12; // Crimson Web
									}
									else if (Skin == 2)
									{
										*pWeapon->FallbackPaintKit() = 27; // Bone Mask
									}
									else if (Skin == 3)
									{
										*pWeapon->FallbackPaintKit() = 38; // Fade
									}
									else if (Skin == 4)
									{
										*pWeapon->FallbackPaintKit() = 40; // Night
									}
									else if (Skin == 5)
									{
										*pWeapon->FallbackPaintKit() = 42; // Blue Steel
									}
									else if (Skin == 6)
									{
										*pWeapon->FallbackPaintKit() = 43; // Stained
									}
									else if (Skin == 7)
									{
										*pWeapon->FallbackPaintKit() = 44; // Case Hardened
									}
									else if (Skin == 8)
									{
										*pWeapon->FallbackPaintKit() = 59; // Slaughter
									}
									else if (Skin == 9)
									{
										*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
									}
									else if (Skin == 10)
									{
										*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
									}
									else if (Skin == 11)
									{
										*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
									}
									else if (Skin == 12)
									{
										*pWeapon->FallbackPaintKit() = 143; // Urban Masked
									}
									else if (Skin == 13)
									{
										*pWeapon->FallbackPaintKit() = 175; // Scorched
									}
									else if (Skin == 14)
									{
										*pWeapon->FallbackPaintKit() = 323; // Rust Coat
									}
									else if (Skin == 15)
									{
										*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
									}
									else if (Skin == 16)
									{
										*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
									}
									else if (Skin == 17)
									{
										*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
									}
									else if (Skin == 18)
									{
										*pWeapon->FallbackPaintKit() = 413; // Marble Fade
									}
									else if (Skin == 19)
									{
										*pWeapon->FallbackPaintKit() = 414; // Rust Coat
									}
									else if (Skin == 20)
									{
										*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
									}
									else if (Skin == 21)
									{
										*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
									}
									else if (Skin == 22)
									{
										*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
									}
									else if (Skin == 23)
									{
										*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
									}
									else if (Skin == 24)
									{
										*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
									}
									else if (Skin == 25)
									{
										*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
									}
									else if (Skin == 26)
									{
										*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 570; // Doppler Phase 4
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 568; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
									}
									else if (Skin == 29)
									{
										*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
									}
									else if (Skin == 30)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
									}
									else if (Skin == 31)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
									}
									else if (Skin == 32)
									{
										*pWeapon->FallbackPaintKit() = 561; // Lore
									}

								}
								else if (Model == 8) // M9 Bayonet
								{
									*pWeapon->ModelIndex() = iM9Bayonet; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iM9Bayonet;
									*pWeapon->WorldModelIndex() = iM9Bayonet + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 508;

									int Skin = Menu::Window.SkinchangerTab.KnifeSkin.GetIndex();

									if (Skin == 0)
									{
										*pWeapon->FallbackPaintKit() = 0; // Forest DDPAT
									}
									else if (Skin == 1)
									{
										*pWeapon->FallbackPaintKit() = 12; // Crimson Web
									}
									else if (Skin == 2)
									{
										*pWeapon->FallbackPaintKit() = 27; // Bone Mask
									}
									else if (Skin == 3)
									{
										*pWeapon->FallbackPaintKit() = 38; // Fade
									}
									else if (Skin == 4)
									{
										*pWeapon->FallbackPaintKit() = 40; // Night
									}
									else if (Skin == 5)
									{
										*pWeapon->FallbackPaintKit() = 42; // Blue Steel
									}
									else if (Skin == 6)
									{
										*pWeapon->FallbackPaintKit() = 43; // Stained
									}
									else if (Skin == 7)
									{
										*pWeapon->FallbackPaintKit() = 44; // Case Hardened
									}
									else if (Skin == 8)
									{
										*pWeapon->FallbackPaintKit() = 59; // Slaughter
									}
									else if (Skin == 9)
									{
										*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
									}
									else if (Skin == 10)
									{
										*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
									}
									else if (Skin == 11)
									{
										*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
									}
									else if (Skin == 12)
									{
										*pWeapon->FallbackPaintKit() = 143; // Urban Masked
									}
									else if (Skin == 13)
									{
										*pWeapon->FallbackPaintKit() = 175; // Scorched
									}
									else if (Skin == 14)
									{
										*pWeapon->FallbackPaintKit() = 323; // Rust Coat
									}
									else if (Skin == 15)
									{
										*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
									}
									else if (Skin == 16)
									{
										*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
									}
									else if (Skin == 17)
									{
										*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
									}
									else if (Skin == 18)
									{
										*pWeapon->FallbackPaintKit() = 413; // Marble Fade
									}
									else if (Skin == 19)
									{
										*pWeapon->FallbackPaintKit() = 414; // Rust Coat
									}
									else if (Skin == 20)
									{
										*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
									}
									else if (Skin == 21)
									{
										*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
									}
									else if (Skin == 22)
									{
										*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
									}
									else if (Skin == 23)
									{
										*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
									}
									else if (Skin == 24)
									{
										*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
									}
									else if (Skin == 25)
									{
										*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
									}
									else if (Skin == 26)
									{
										*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 570; // Doppler Phase 4
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 568; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
									}
									else if (Skin == 29)
									{
										*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
									}
									else if (Skin == 30)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
									}
									else if (Skin == 31)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
									}
									else if (Skin == 32)
									{
										*pWeapon->FallbackPaintKit() = 562; // Lore
									}

								}


								else if (Model == 10) // Shadow Daggers
								{
									*pWeapon->ModelIndex() = iDagger; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iDagger;
									*pWeapon->WorldModelIndex() = iDagger + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 516;

									int Skin = Menu::Window.SkinchangerTab.KnifeSkin.GetIndex();

									if (Skin == 0)
									{
										*pWeapon->FallbackPaintKit() = 5; // Forest DDPAT
									}
									else if (Skin == 1)
									{
										*pWeapon->FallbackPaintKit() = 12; // Crimson Web
									}
									else if (Skin == 2)
									{
										*pWeapon->FallbackPaintKit() = 27; // Bone Mask
									}
									else if (Skin == 3)
									{
										*pWeapon->FallbackPaintKit() = 38; // Fade
									}
									else if (Skin == 4)
									{
										*pWeapon->FallbackPaintKit() = 40; // Night
									}
									else if (Skin == 5)
									{
										*pWeapon->FallbackPaintKit() = 42; // Blue Steel
									}
									else if (Skin == 6)
									{
										*pWeapon->FallbackPaintKit() = 43; // Stained
									}
									else if (Skin == 7)
									{
										*pWeapon->FallbackPaintKit() = 44; // Case Hardened
									}
									else if (Skin == 8)
									{
										*pWeapon->FallbackPaintKit() = 59; // Slaughter
									}
									else if (Skin == 9)
									{
										*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
									}
									else if (Skin == 10)
									{
										*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
									}
									else if (Skin == 11)
									{
										*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
									}
									else if (Skin == 12)
									{
										*pWeapon->FallbackPaintKit() = 143; // Urban Masked
									}
									else if (Skin == 13)
									{
										*pWeapon->FallbackPaintKit() = 175; // Scorched
									}
									else if (Skin == 14)
									{
										*pWeapon->FallbackPaintKit() = 323; // Rust Coat
									}
									else if (Skin == 15)
									{
										*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
									}
									else if (Skin == 16)
									{
										*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
									}
									else if (Skin == 17)
									{
										*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
									}
									else if (Skin == 18)
									{
										*pWeapon->FallbackPaintKit() = 413; // Marble Fade
									}
									else if (Skin == 19)
									{
										*pWeapon->FallbackPaintKit() = 414; // Rust Coat
									}
									else if (Skin == 20)
									{
										*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
									}
									else if (Skin == 21)
									{
										*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
									}
									else if (Skin == 22)
									{
										*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
									}
									else if (Skin == 23)
									{
										*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
									}
									else if (Skin == 24)
									{
										*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
									}
									else if (Skin == 25)
									{
										*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
									}
									else if (Skin == 26)
									{
										*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
									}
									else if (Skin == 27)
									{
										*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
									}
									else if (Skin == 28)
									{
										*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
									}
									else if (Skin == 29)
									{
										*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
									}
									else if (Skin == 30)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
									}
									else if (Skin == 31)
									{
										*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
									}
									else if (Skin == 32)
									{
										*pWeapon->FallbackPaintKit() = 561; // Lore
									}
								}
							}

							*pWeapon->OwnerXuidLow() = 0;
							*pWeapon->OwnerXuidHigh() = 0;
							*pWeapon->FallbackWear() = 0.001f;
							*pWeapon->m_AttributeManager()->m_Item()->ItemIDHigh() = 1;

						}
					}
				}

			}
		}
		

		
		if (curStage == FRAME_NET_UPDATE_END) {
			//CBacktracking StoreData();
		}
		

	}

	oFrameStageNotify(curStage);
}

void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		if (Menu::Window.VisualsTab.Active.GetState() && pLocal->IsAlive() && !pLocal->IsScoped())
		{
			if (pSetup->fov = 90)
				pSetup->fov = Menu::Window.VisualsTab.OtherFOV.GetValue();
		}

		oOverrideView(ecx, edx, pSetup);
	}

}

void GetViewModelFOV(float& fov)
{
	IClientEntity* localplayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{

		if (!localplayer)
			return;


		if (Menu::Window.VisualsTab.Active.GetState())
			fov += Menu::Window.VisualsTab.OtherViewmodelFOV.GetValue();
	}
}

float __stdcall GGetViewModelFOV()
{
	float fov = Hooks::VMTClientMode.GetMethod<oGetViewModelFOV>(35)();

	GetViewModelFOV(fov);

	return fov;
}

void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static DWORD oRenderView = Hooks::VMTRenderView.GetOriginalFunction(6);

	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	__asm
	{
		PUSH whatToDraw
		PUSH nClearFlags
		PUSH hudViewSetup
		PUSH setup
		MOV ECX, ecx
		CALL oRenderView
	}
} //hooked for no reason yay