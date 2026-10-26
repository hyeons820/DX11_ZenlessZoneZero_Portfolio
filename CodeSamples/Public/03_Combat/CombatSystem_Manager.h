#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Collision_Manager.h"

NS_BEGIN(Client)

class CMonster;
struct HITBOX_ACTIVE_DESC;

class CCombatSystem_Manager final : public CBase
{
public:
    // 이미 맞은 기록
    struct HIT_RECORD
    {
        _udlong dlAttackerID = {};
        _udlong dlTargetID = {};

        _uint iAttackID = {};
        _uint iHitBoxID = {};

        _uint iHitCount = {};
        _float fHitTimer = {};
    };

    struct DEFENDED_HITBOX_RECORD
    {
        _udlong dlAttackerID = {};
        _uint iAttackID = {};
        _uint iHitBoxID = {};
    };

private:
    CCombatSystem_Manager();
    virtual ~CCombatSystem_Manager() = default;

public:
    HRESULT     Initialize();
    void        Process_Collisions(const vector<CCollision_Manager::COLLISION_RESULT>& Results, _float fTimeDelta);
    void        Clear_HitRecords(_udlong dlAttackerID);
    void        Clear_HitRecords(_udlong dlAttackerID, _uint iAttackID);

private:
    void        Update_HitRecords(_float fTimeDelta);
    HIT_RECORD* Find_HitRecord(_udlong dlAttackerID, _udlong dlTargetID, _uint iAttackID, _uint iHitBoxID);
    void        Add_HitRecord(_udlong dlAttackerID, _udlong dlTargetID, _uint iAttackID, _uint iHitBoxID);
    void        Resolve_BodyCollision(const CCollision_Manager::COLLISION_RESULT& Result);
    void        Resolve_HitCollision(const CCollision_Manager::COLLISION_RESULT& Result);
    void        Resolve_JustEvadeCollision(const CCollision_Manager::COLLISION_RESULT& Result);
    void        Resolve_ParryCollision(const CCollision_Manager::COLLISION_RESULT& Result);
    void        Resolve_ParryImpactCollision(const CCollision_Manager::COLLISION_RESULT& Result);
    void        Apply_Damage(const CCollision_Manager::COLLISION_RESULT& Result, const HITBOX_ACTIVE_DESC& HitBoxDesc, _uint iDamageRateIndex);
    void        Add_DefendedHitBox(_udlong dlAttackerID, _uint iAttackID, _uint iHitBoxID);
    void        Clear_DefendedHitBoxes(_udlong dlAttackerID, _uint iAttackID);
    _float      Get_HitDamageRatePercent(const HITBOX_ACTIVE_DESC& HitBoxDesc, _uint iDamageRateIndex) const;

private:
    _float      Calculate_PlayerDamage(const CCollision_Manager::COLLISION_RESULT& Result, const HITBOX_ACTIVE_DESC& HitBoxDesc, _uint iDamageRateIndex) const;
    _float      Calculate_MonsterDamage(const CCollision_Manager::COLLISION_RESULT& Result, const HITBOX_ACTIVE_DESC& HitBoxDesc, _uint iDamageRateIndex) const;
    _float      Calculate_MonsterDefenseCoeff(CMonster* pMonster) const;

private:
    vector<HIT_RECORD> m_HitRecords;
    vector<DEFENDED_HITBOX_RECORD> m_DefendedHitBoxRecords;
    class CGameInstance* m_pGameInstance = { nullptr };

private:
    _bool   Is_BodyCollision(const CCollision_Manager::COLLISION_RESULT& Result) const;
    _bool   Is_PlayerHitMonster(const CCollision_Manager::COLLISION_RESULT& Result) const;
    _bool   Is_MonsterHitPlayerDodgeBox(const CCollision_Manager::COLLISION_RESULT& Result) const;
    _bool   Is_MonsterHitPlayerParryBox(const CCollision_Manager::COLLISION_RESULT& Result) const;
    _bool   Is_MonsterWarningPlayerDodgeBox(const CCollision_Manager::COLLISION_RESULT& Result) const;
    _bool   Is_MonsterWarningPlayerParryBox(const CCollision_Manager::COLLISION_RESULT& Result) const;
    _bool   Is_DefendedHitBox(_udlong dlAttackerID, _uint iAttackID, _uint iHitBoxID) const;

public:
    static CCombatSystem_Manager* Create();
    virtual void Free() override;
};

NS_END