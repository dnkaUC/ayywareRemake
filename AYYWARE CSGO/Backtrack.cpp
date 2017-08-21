/*#include "Backtrack.h"

LagCompensation::LagCompensation()
{

}

void LagCompensation::logEntity(IClientEntity *player)
{
	int idx = player->GetIndex();
	LagRecord *m_LagRecords = this->m_LagRecord[idx];

	if (!player || !player->GetHealth() > 0)
	{
		for (int i = 0; i < 11; i++)
		{
			m_LagRecords[i].m_fSimulationTime = 0.0f;
		}
	}

	float simTime = player->GetSimulationTime();

	if (!isValidTick(simTime))
		return;

	int highestRecordIdx = -1;
	float highestSimTime = 0.0f;

	for (int i = 0; i < 11; i++)
	{
		if (m_LagRecords[i].m_fSimulationTime > simTime)
			m_LagRecords[i].m_fSimulationTime = 0.0f;

		if (m_LagRecords[i].m_fSimulationTime == 0.0f)
			continue;

		if (m_LagRecords[i].m_fSimulationTime == simTime)
			return;

		if (m_LagRecords[i].m_fSimulationTime > highestSimTime)
		{
			highestRecordIdx = i;
			highestSimTime = m_LagRecords[i].m_fSimulationTime;
		}
	}

	highestRecordIdx++;
	highestRecordIdx = highestRecordIdx % 11;

	m_LagRecords[highestRecordIdx].m_bIsFixed = false;
	m_LagRecords[highestRecordIdx].m_iTargetID = idx;

	m_LagRecords[highestRecordIdx].headSpot = player->GetHeadPos();

	m_LagRecords[highestRecordIdx].m_vAbsOrigin = player->GetAbsOrigin();
	m_LagRecords[highestRecordIdx].m_vecVelocity = player->GetVelocity();
	m_LagRecords[highestRecordIdx].m_fSimulationTime = player->GetSimulationTime();
	m_LagRecords[highestRecordIdx].m_vecAngles = player->GetAbsAngles();
	m_LagRecords[highestRecordIdx].m_flCycle = player->getCycle();
	m_LagRecords[highestRecordIdx].m_nSequence = player->getSequence();
	m_LagRecords[highestRecordIdx].flags = player->GetFlags();
	m_LagRecords[highestRecordIdx].m_flLowerBodyYawTarget = player->GetLowerBodyYaw();
	
	for (int i = 0; i < 24; i++)
		m_LagRecords[highestRecordIdx].m_flPoseParameter[i] = player->getPoseParams(i);
}

void LagCompensation::logCurrentEnt(IClientEntity *player)
{
	int idx = player->GetIndex();

	this->m_PrevRecords[idx].m_vAbsOrigin = player->GetAbsOrigin();
	this->m_PrevRecords[idx].m_vecVelocity = player->GetVelocity();
	this->m_PrevRecords[idx].m_fSimulationTime = player->GetSimulationTime();
	this->m_PrevRecords[idx].m_vecAngles = player->GetAbsAngles();
	this->m_PrevRecords[idx].m_flCycle = player->getCycle();
	this->m_PrevRecords[idx].m_nSequence = player->getSequence();
	this->m_PrevRecords[idx].flags = player->GetFlags();
	this->m_PrevRecords[idx].m_flLowerBodyYawTarget = player->GetLowerBodyYaw();

	for (int i = 0; i < 24; i++)
		this->m_PrevRecords[idx].m_flPoseParameter[i] = player->getPoseParams(i);
}

void LagCompensation::setEntity(IClientEntity *player, LagRecord record)
{
	if (!player || record.m_fSimulationTime == 0.0f)
	{
		record.m_bIsFixed = false;
		return;
	}

	this->logCurrentEnt(player);

	player->setAbsOriginal(record.m_vAbsOrigin);
	*(Vector*)((DWORD)player + 0x40C1CA24) = record.m_vecVelocity;  // offs.vekVel
	*(float*)((DWORD)player + 0xC4560E44) = record.m_fSimulationTime; // simulTime
	player->setAbsAechse(record.m_vecAngles);
	*(float*)((DWORD)player + 0xA14) = record.m_flCycle; // cycle
	*(int*)((DWORD)player + 0x28AC) = record.m_nSequence; // sequence
	*(int*)((DWORD)player + 0xE456D580) = record.flags; // flags
	*(float*)((DWORD)player + 0xE6996CCF) = record.m_flLowerBodyYawTarget; // LBY

	for (int i = 0; i < 24; i++)
		*(float*)((DWORD)player + 0x2764 + sizeof(float) * i) = record.m_flPoseParameter[i]; // getposeparams

	record.m_iWantedTick = record.m_fSimulationTime;
	record.m_bIsFixed = true;

	player->updateClientSideAnimation();

	unsigned long g_iModelBoneCounter = **(unsigned long**)(offs.invalKostCache + 10);

	//*(int*)((DWORD)player + offs.upisivaKost) = 0;
	*(unsigned int*)((DWORD)player + 0x2914) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*(unsigned int*)((DWORD)player + 0x2680) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}

void LagCompensation::setCurrentEnt(IClientEntity *player)
{
	int idx = player->getIdx();

	player->setAbsOriginal(m_PrevRecords[idx].m_vAbsOrigin);
	*(Vector3*)((DWORD)player + offs.vekVel) = m_PrevRecords[idx].m_vecVelocity;
	*(float*)((DWORD)player + offs.simulTime) = m_PrevRecords[idx].m_fSimulationTime;
	player->setAbsAechse(m_PrevRecords[idx].m_vecAngles);
	*(float*)((DWORD)player + offs.cycle) = m_PrevRecords[idx].m_flCycle;
	*(int*)((DWORD)player + offs.sequence) = m_PrevRecords[idx].m_nSequence;
	*(int*)((DWORD)player + offs.flags) = m_PrevRecords[idx].flags;
	*(float*)((DWORD)player + offs.lowBodYtarg) = m_PrevRecords[idx].m_flLowerBodyYawTarget;

	for (int i = 0; i < 24; i++)
		*(float*)((DWORD)player + offs.poseParams + sizeof(float) * i) = m_PrevRecords[idx].m_flPoseParameter[i];

	m_PrevRecords[idx].m_iWantedTick = m_PrevRecords[idx].m_fSimulationTime;

	player->updateClientSideAnimation();

	unsigned long g_iModelBoneCounter = **(unsigned long**)(offs.invalKostCache + 10);

	//*(int*)((DWORD)player + offs.upisivaKost) = 0;
	*(unsigned int*)((DWORD)player + 0x2914) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*(unsigned int*)((DWORD)player + 0x2680) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}

template<class T, class U>
T LagCompensation::clamp(T in, U low, U high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}

float LagCompensation::lerpTime()
{
	int ud_rate = p_Console->FindVar("cl_updaterate")->GetInt();
	ConVar *min_ud_rate = p_Console->FindVar("sv_minupdaterate");
	ConVar *max_ud_rate = p_Console->FindVar("sv_maxupdaterate");

	if (min_ud_rate && max_ud_rate)
		ud_rate = max_ud_rate->GetInt();

	float ratio = p_Console->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
		ratio = 1.0f;

	float lerp = p_Console->FindVar("cl_interp")->GetFloat();
	ConVar *c_min_ratio = p_Console->FindVar("sv_client_min_interp_ratio");
	ConVar *c_max_ratio = p_Console->FindVar("sv_client_max_interp_ratio");

	if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
		ratio = clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

	return max(lerp, (ratio / ud_rate));
}

int LagCompensation::fakeLagFix(IClientEntity *player, int historyIdx)
{
	int idx = player->getIdx();

	LagRecord *m_LagRecords = this->m_LagRecord[idx];

	LagRecord recentLR = m_LagRecords[historyIdx];
	LagRecord prevLR;
	if (historyIdx == 0)
		prevLR = m_LagRecords[8];
	else
		prevLR = m_LagRecords[historyIdx - 1];

	if (recentLR.m_fSimulationTime == 0.0f)
		return -1;

	INetChannelInfo *nci = p_Engine->getNetChannelInfo();
	int predCmdArrivTick = Global::userCMD->tick_count + 1 + TIME_TO_TICKS(nci->GetAvgLatency(FLOW_INCOMING) + nci->GetAvgLatency(FLOW_OUTGOING)); // (c) n0xius @ uc

	int iLerpTicks = TIME_TO_TICKS(lerpTime());
	int iTargetTickCount = TIME_TO_TICKS(recentLR.m_fSimulationTime) + iLerpTicks;

	ConVar *sv_max_usercmd_future_ticks = p_Console->FindVar("sv_max_usercmd_future_ticks");

	float flCorrect = clamp(lerpTime() + nci->GetLatency(FLOW_OUTGOING), 0.f, 1.f) - TICKS_TO_TIME(predCmdArrivTick + TIME_TO_TICKS(lerpTime()) - iTargetTickCount); // (c) n0xius @ uc

	if (fabs(flCorrect) > 0.2f) // Too big deltatime, can't lagfix here
	{
		//p_Console->ConsoleColorPrintf(Color::Red(), "StartLagCompensation: delta too big (%.3f)\n", flCorrect);
		return -1;
	}

	if ((recentLR.m_vAbsOrigin - prevLR.m_vAbsOrigin).lengthSqr() > 4096.f)
	{
		for (auto i = 0; i <= player->getChockedTicks(); i++)
		{
			// ~ deleted parts cause pub release

			rebGameMovement->FullWalkMove(player); // resimulate
		}
		recentLR.m_bIsBreakingLagComp = true;
		//p_Console->ConsoleColorPrintf(Color::Green(), "m_bIsBreakingLagComp: true\n");

		return -1;
	}
	else
	{
		setEntity(player, recentLR);
	}

	return iTargetTickCount;
}

void LagCompensation::initLagRecord()
{
	for (int i = 0; i <= 32; i++)
	{
		for (int j = 0; j < 11; j++)
		{
			m_LagRecord[i][j].m_fSimulationTime = 0.0f;
		}
	}
}

int LagCompensation::fixTickcount(IClientEntity *player)
{
	int idx = player->getIdx();

	LagRecord *m_LagRecords = this->m_LagRecord[idx];

	LagRecord recentLR = m_LagRecords[mVars.historyIdx];

	if (recentLR.m_fSimulationTime == 0.0f)
		return TIME_TO_TICKS(player->getSimulTime() + lerpTime()) + 1;

	int iLerpTicks = TIME_TO_TICKS(lerpTime());
	int iTargetTickCount = TIME_TO_TICKS(recentLR.m_fSimulationTime) + iLerpTicks;

	return iTargetTickCount + 1;
}

bool LagCompensation::isValidTick(float simTime)
{
	INetChannelInfo *nci = p_Engine->getNetChannelInfo();

	if (!nci)
		return false;

	auto LerpTicks = TIME_TO_TICKS(lerpTime());

	int predCmdArrivTick = p_GlobVars->tickcount + 1 + TIME_TO_TICKS(nci->GetAvgLatency(FLOW_INCOMING) + nci->GetAvgLatency(FLOW_OUTGOING));

	float flCorrect = clamp(lerpTime() + nci->GetLatency(FLOW_OUTGOING), 0.f, 1.f) - TICKS_TO_TIME(predCmdArrivTick + LerpTicks - (TIME_TO_TICKS(simTime) + TIME_TO_TICKS(lerpTime())));

	return abs(flCorrect) <= 0.2f;
}

void LagCompensation::log(ClientFrameStage_t stage)
{
	if (!p_Engine->isIG())
		return;

	if (!Global::locPlayer || Global::locPlayer == NULL)
		return;

	if (mVars.bEnableLagComp)
	{
		if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_END)
		{
			for (int i = 1; i < p_GlobVars->maxClients; i++)
			{
				Entity *player = reinterpret_cast<Entity*>(p_ClientEntList->GetClientEntity(i));

				if (!player || player == nullptr)
					continue;

				if (player == Global::locPlayer)
					continue;

				if (player->isStationary())
					continue;

				if (player->isImmune())
					continue;

				if (player->getLeben() <= 0)
					continue;

				if (player->getTeam() == Global::locPlayer->getTeam())
					continue;

				this->logEntity(player);
			}
		}
	}
}

*/