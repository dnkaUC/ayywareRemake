/*
Rest In Peace ApocalypseCheats
*/

#pragma once

#include "Hacks.h"
extern bool islbyupdate;
extern float ProxyLBYtime;
extern int LBYBreakerTimer;
extern float fsnLBY;

extern bool switchInverse;

extern int shotsfired;

extern bool switchAntiAimSide;
extern bool antiAimSide;

extern float autowalldmgtest[65];
extern float lineFakeAngle;

class CRageBot : public CHack
{
public:
	void Init();
	void Draw();
	void Move(CUserCmd *pCmd, bool &bSendPacket);
private:
	// Targetting
	int GetTargetCrosshair();
	int GetTargetDistance();
	int GetTargetHealth();
	void AtTarget(CUserCmd *pCmd);
	bool TargetMeetsRequirements(IClientEntity* pEntity);
	float FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int HitBox);
	int HitScan(IClientEntity* pEntity);
	bool AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket);

	// Functionality
	void DoAimbot(CUserCmd *pCmd, bool &bSendPacket);
	void DoNoRecoil(CUserCmd *pCmd);
	void PositionAdjustment(CUserCmd* pCmd);

	// AntiAim
	void DoAntiAim(CUserCmd *pCmd, bool&bSendPacket);
	//bool EdgeAntiAim(IClientEntity* pLocalBaseEntity, CUserCmd* cmd, float flWall, float flCornor);
	//bool bEdge;

	// AimStep
	bool IsAimStepping;
	Vector LastAimstepAngle;
	Vector LastAngle;

	// Aimbot
	bool IsLocked;
	int TargetID;
	int HitBox;
	Vector AimPoint;
};