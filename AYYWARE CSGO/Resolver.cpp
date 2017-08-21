#include "Resolver.h"
int Globals::Shots = 0;
bool Globals::change;
CUserCmd* Globals::UserCmd;
int Globals::TargetID;
IClientEntity* Globals::Target;
/*
void CResolver::Resolve(IClientEntity* ent) {
	auto cur = m_arrInfos.at(ent->GetIndex()).m_sRecords;
	if (cur.size() < 2)
		return;
	
		if (Backtracking->CanBacktrackShot(GetShotRecord(ent)))
			m_arrInfos.at(ent->entindex()).m_sRecords.erase(std::remove(m_arrInfos.at(ent->entindex()).m_sRecords.begin(), m_arrInfos.at(ent->entindex()).m_sRecords.end(), GetShotRecord(ent)), m_arrInfos.at(ent->entindex()).m_sRecords.end());
		else if (IsEntityMoving(ent))
			ent->m_angEyeAngles().y = ent->GetLowerBodyYawTarget();
		else if (HasStaticRealAngle(cur))
			ent->m_angEyeAngles().y = (cur.front().m_flLowerBodyYawTarget) + (M::RandomFloat(0.f, 1.f) > 0.5f ? 10 : -10);
		else if (HasStaticYawDifference(cur))
			ent->m_angEyeAngles().y = ent->GetEyeAngles().y - (cur.front().m_angEyeAngles.y - cur.front().m_flLowerBodyYawTarget);
		else if (HasSteadyDifference(cur)) {
			float tickdif = static_cast<float>(cur.front().tickcount - cur.at(1).tickcount);
			float lbydif = GetDelta(cur.front().m_flLowerBodyYawTarget, cur.at(1).m_flLowerBodyYawTarget);
			float ntickdif = static_cast<float>(G::UserCmd->tick_count - cur.front().tickcount);
			ent->m_angEyeAngles().y = (lbydif / tickdif) * ntickdif;
		}
		else if (DeltaKeepsChanging(cur))
			ent->m_angEyeAngles().y = ent->GetEyeAngles().y - GetDeltaByComparingTicks(cur);
		else if (LBYKeepsChanging(cur))
			ent->m_angEyeAngles().y = GetLBYByComparingTicks(cur);
		else
			ent->m_angEyeAngles().y = ent->GetEyeAngles().y + 180;
	
}


bool& CResolver::LowerBodyYawChanged(IClientEntity* ent) {
	return m_arrInfos.at(ent->GetIndex()).m_bLowerBodyYawChanged;
}

void CResolver::StoreVars(IClientEntity* ent) {
	if (m_arrInfos.at(ent->GetIndex()).m_sRecords.size() >= static_cast<size_t>(Config->GetValue<int>("Resolver", "Ticks")))
		m_arrInfos.at(ent->GetIndex()).m_sRecords.pop_back();
	m_arrInfos.at(ent->GetIndex()).m_sRecords.push_front(CTickRecord(ent));
}

void CResolver::StoreVars(IClientEntity* ent, QAngle ang, float lby, float simtime, float tick) {
	if (m_arrInfos.at(ent->GetIndex()).m_sRecords.size() >= static_cast<size_t>(Config->GetValue<int>("Resolver", "Ticks")))
		m_arrInfos.at(ent->GetIndex()).m_sRecords.pop_back();
	m_arrInfos.at(ent->GetIndex()).m_sRecords.push_front(CTickRecord(ent));
}

bool& CResolver::BacktrackThisTick(IClientEntity* ent) {
	return m_arrInfos.at(ent->GetIndex()).m_bBacktrackThisTick;
}

CTickRecord CResolver::GetShotRecord(IClientEntity* ent) {
	for (auto cur : m_arrInfos[ent->entindex()].m_sRecords) {
		if (cur.validtick)
			return CTickRecord(cur);
	}
	return CTickRecord();
}

bool CResolver::HasStaticRealAngle(const deque<CTickRecord>& l, float tolerance) {
	auto minmax = std::minmax_element(std::begin(l), std::end(l), [](const CTickRecord& t1, const CTickRecord& t2) { return t1.m_flLowerBodyYawTarget < t2.m_flLowerBodyYawTarget; });
	return (fabs(minmax.first->m_flLowerBodyYawTarget - minmax.second->m_flLowerBodyYawTarget) <= tolerance);
}

bool CResolver::HasStaticRealAngle(int index, float tolerance) {
	return HasStaticRealAngle(m_arrInfos[index].m_sRecords, tolerance);
}

bool CResolver::HasStaticYawDifference(const deque<CTickRecord>& l, float tolerance) {
	for (auto i = l.begin(); i < l.end() - 1;) {
		if (GetDelta(LBYDelta(*i), LBYDelta(*++i)) > tolerance)
			return false;
	}
	return true;
}

bool CResolver::HasSteadyDifference(const deque<CTickRecord>& l, float tolerance) {
	size_t misses = 0;
	for (size_t i = 0; i < l.size() - 1; i++) {
		float tickdif = static_cast<float>(l.at(i).m_flSimulationTime - l.at(i + 1).tickcount);
		float lbydif = GetDelta(l.at(i).m_flLowerBodyYawTarget, l.at(i + 1).m_flLowerBodyYawTarget);
		float ntickdif = static_cast<float>(G::UserCmd->tick_count - l.at(i).tickcount);
		if (((lbydif / tickdif) * ntickdif) > tolerance) misses++;
	}
	return (misses <= (l.size() / 3));
}

int CResolver::GetDifferentDeltas(const sdeque<CTickRecord>& l, float tolerance) {
	std::vector<float> vec;
	for (auto var : l) {
		float curdelta = LBYDelta(var);
		bool add = true;
		for (auto fl : vec) {
			if (!IsDifferent(curdelta, fl, tolerance))
				add = false;
		}
		if (add)
			vec.push_back(curdelta);
	}
	return vec.size();
}

int CResolver::GetDifferentLBYs(const deque<CTickRecord>& l, float tolerance) {
	std::vector<float> vec;
	for (auto var : l) {
		float curyaw = var.m_flLowerBodyYawTarget;
		bool add = true;
		for (auto fl : vec) {
			if (!IsDifferent(curyaw, fl, tolerance))
				add = false;
		}
		if (add)
			vec.push_back(curyaw);
	}
	return vec.size();
}

float CResolver::GetLBYByComparingTicks(const deque<CTickRecord>& l) {
	int modulo = Config->GetValue<int>("Resolver", "Modulo");
	int difangles = GetDifferentLBYs(l);
	int inc = modulo * difangles;
	for (auto var : l) {
		for (int lasttick = var.tickcount; lasttick < G::UserCmd->tick_count; lasttick += inc) {
			if (lasttick == G::UserCmd->tick_count)
				return var.m_flLowerBodyYawTarget;
		}
	}
	return 0.f;
}

float CResolver::GetDeltaByComparingTicks(const deque<CTickRecord>& l) {
	int modulo = Config->GetValue<int>("Resolver", "Modulo");
	int difangles = GetDifferentDeltas(l);
	int inc = modulo * difangles;
	for (auto var : l) {
		for (int lasttick = var.tickcount; lasttick < G::UserCmd->tick_count; lasttick += inc) {
			if (lasttick == G::UserCmd->tick_count)
				return LBYDelta(var);
		}
	}
	return 0.f;
}

bool CResolver::DeltaKeepsChanging(const deque<CTickRecord>& cur, float tolerance) {
	return (GetDifferentDeltas(cur) >(int) cur.size() / 2);
}

bool CResolver::LBYKeepsChanging(const deque<CTickRecord>& cur, float tolerance) {
	return (GetDifferentLBYs(cur, tolerance) > (int)cur.size() / 2);
}

bool CResolver::IsEntityMoving(IClientEntity* ent) {
	return (ent->GetVelocity().Length2D() > 0.1f && ent->GetFlags() & FL_ONGROUND);
} */