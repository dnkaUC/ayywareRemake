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

#pragma once

#include "Hacks.h"
extern int hittedLogHits[65];
extern int missedLogHits[65];
extern int ResolverMode;
extern float enemyLBYTimer[65];
extern float enemysLastProxyTimer[65];
extern float enemyLBYDelta[65];
extern float lineLBY;
extern float lineRealAngle;
extern float lineFakeAngle;
extern int ResolverStage[65];
extern float autowalldmgtest[65];
class CEsp : public CHack
{
public:
	void Init();
	void Draw();
	void Move(CUserCmd *pCmd, bool &bSendPacket);
	std::vector<IMaterial*> SkyTextures;
	std::vector<IMaterial*> WorldTextures;
private:

	// Other shit
	IClientEntity *BombCarrier;

	struct ESPBox
	{
		int x, y, w, h;
	};

	// Draw a player
	void NoSky();
	void DoFOV();
	void DrawPlayer(IClientEntity* pEntity, player_info_t pinfo);

	// Get player info
	Color GetPlayerColor(IClientEntity* pEntity);
	void DrawLinesAA(Color color);
	bool GetBox(IClientEntity* pEntity, ESPBox &result);
	

	// Draw shit about player
	void SpecList();
	void DrawGlow(IClientEntity *pEntity, int r, int g, int b, int a);
	void DrawBox(ESPBox size, Color color);
	void DrawName(player_info_t pinfo, ESPBox size, IClientEntity* pEntity);
	void DrawGrenades(IClientEntity * pEntity);
	void DrawHealth(IClientEntity* pEntity, ESPBox size);
	void DrawArmor(IClientEntity* pEntity, CEsp::ESPBox size);
	void DrawDistanse(IClientEntity* pEntity, CEsp::ESPBox size);
	float DistanceTo(Vector vecDst, Vector vecSrc);
	void DrawHealth2(IClientEntity* pEntity, ESPBox size);
	void DrawInfo(IClientEntity* pEntity, ESPBox size);
	void DrawCross(IClientEntity* pEntity);
	void DrawSkeleton(IClientEntity* pEntity);

	void DrawChicken(IClientEntity* pEntity, ClientClass* cClass);
	void DrawDrop(IClientEntity* pEntity, ClientClass* cClass);
	void DrawBombPlanted(IClientEntity* pEntity, ClientClass* cClass);
	void DrawBomb(IClientEntity* pEntity, ClientClass* cClass);
};

