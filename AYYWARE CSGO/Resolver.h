#pragma once

#include "Interfaces.h"
#include "Hacks.h"

namespace Globals
{
	extern CUserCmd* UserCmd;
	extern IClientEntity* Target;
	extern int Shots;
	extern bool change;
	extern int TargetID;
}

/*
struct CTickRecord;

struct CValidTick {
	explicit operator CTickRecord() const;

	explicit operator bool() const noexcept {
		return m_flSimulationTime > 0.f;
	}

	float m_flPitch = 0.f;
	float m_flYaw = 0.f;
	float m_flSimulationTime = 0.f;
	IClientEntity* m_pEntity = nullptr;
};

struct CTickRecord {
	CTickRecord() {}
	CTickRecord(IClientEntity* ent) {
		m_flLowerBodyYawTarget = ent->GetLowerBodyYaw();
		m_angEyeAngles = ent->GetEyeAngles();
		m_flCycle = ent->getCycle();
		m_flSimulationTime = ent->GetSimulationTime();
		m_nSequence = ent->getSequence();
		m_vecOrigin = ent->GetOrigin();
		m_vecVelocity = ent->GetVelocity();
		m_flPoseParameter = ent->getPoseParams();

		m_angAbsAngles = ent->GetAbsAngles();
		m_vecAbsOrigin = ent->GetAbsOrigin();
		tickcount = 0;
	}

	explicit operator bool() const noexcept {
		return m_flSimulationTime > 0.f;
	}

	bool operator>(const CTickRecord& others) {
		return (m_flSimulationTime > others.m_flSimulationTime);
	}
	bool operator>=(const CTickRecord& others) {
		return (m_flSimulationTime >= others.m_flSimulationTime);
	}
	bool operator<(const CTickRecord& others) {
		return (m_flSimulationTime < others.m_flSimulationTime);
	}
	bool operator<=(const CTickRecord& others) {
		return (m_flSimulationTime <= others.m_flSimulationTime);
	}
	bool operator==(const CTickRecord& others) {
		return (m_flSimulationTime == others.m_flSimulationTime);
	}

	float m_flLowerBodyYawTarget = 0.f;
	QAngle m_angEyeAngles = QAngle(0, 0, 0);
	float m_flCycle = 0.f;
	float m_flSimulationTime = 0.f;
	int m_nSequence = 0;
	Vector m_vecOrigin = Vector(0, 0, 0);
	Vector m_vecAbsOrigin = Vector(0, 0, 0);
	Vector m_vecVelocity = Vector(0, 0, 0);
	std::array<float, 24> m_flPoseParameter = {};
	QAngle m_angAbsAngles = QAngle(0, 0, 0);
	CValidTick validtick;
	int tickcount = 0;
};

inline CValidTick::operator CTickRecord() const {
	CTickRecord rec(m_pEntity);
	rec.m_angEyeAngles.x = this->m_flPitch;
	rec.m_angEyeAngles.y = this->m_flYaw;
	rec.m_flSimulationTime = this->m_flSimulationTime;
	return rec;
}

template<

	class T,
	class Allocator = std::allocator<T>
> class deque;

#pragma once

enum class ResolveMode : int {
	OFF = 0,
	FORCE,
	DELTA,
	STEADY,
	TICKMODULO,
	ALL
};

class CResolveInfo {
	friend class CResolver;
protected:
	deque<CTickRecord> m_sRecords;
	bool	m_bEnemyShot; //priority
	bool	m_bLowerBodyYawChanged;
	bool	m_bBacktrackThisTick;
};

#define Resolver CResolver::GetInstance()

class CResolver {
	friend class CLagcompensation;
	friend class CBacktracking;

	//IMPLEMENT_SINGLETON(CResolver);

public:
	void StoreVars(IClientEntity* Entity);
	void StoreVars(IClientEntity* Entity, QAngle ang, float lby, float simtime, float tick);

	void Resolve(IClientEntity* ent);

	bool& LowerBodyYawChanged(IClientEntity* ent);
	bool& BacktrackThisTick(IClientEntity* ent);

private:
	CTickRecord GetShotRecord(IClientEntity*);
	bool HasStaticRealAngle(int index, float tolerance = 15.f);
	bool HasStaticRealAngle(const deque<CTickRecord>& l, float tolerance = 15.f);
	bool HasStaticYawDifference(const deque<CTickRecord>& l, float tolerance = 15.f);
	bool HasSteadyDifference(const deque<CTickRecord>& l, float tolerance = 15.f);
	int GetDifferentDeltas(const deque<CTickRecord>& l, float tolerance = 15.f);
	int GetDifferentLBYs(const deque<CTickRecord>& l, float tolerance = 15.f);
	float GetLBYByComparingTicks(const deque<CTickRecord>& l);
	float GetDeltaByComparingTicks(const deque<CTickRecord>& l);
	bool DeltaKeepsChanging(const deque<CTickRecord>& cur, float tolerance = 15.f);
	bool LBYKeepsChanging(const deque<CTickRecord>& cur, float tolerance = 15.f);
	bool IsEntityMoving(IClientEntity* ent);

private:
	std::array<CResolveInfo, 32> m_arrInfos;
};


const inline float GetDelta(float a, float b) {

	while (a < -180.0f)
		a += 360.0f;

	while (a > 180.0f)
		a -= 360.0f;

	while (b < -180.0f)
		b += 360.0f;

	while (b > 180.0f)
		b -= 360.0f;

	return abs(a - b);
}

const inline float LBYDelta(const CTickRecord& v) {
	return v.m_angEyeAngles.y - v.m_flLowerBodyYawTarget;
}

const inline bool IsDifferent(float a, float b, float tolerance = 10.f) {
	return (GetDelta(a, b) > tolerance);
} */