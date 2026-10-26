// 포트폴리오용 코드 발췌: 충돌 분류, 중복·다단 피격 기록, 방어 판정과 피해 처리
// 원본: Client/Private/CombatSystem_Manager.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 19~29행: Initialize
HRESULT CCombatSystem_Manager::Initialize()
{
    m_pGameInstance->Subscribe_Event<EVENT_ATTACK_END>(
        this, [this](const EVENT_ATTACK_END& Event)
        {
            Clear_HitRecords(Event.dlAttackerID, Event.iAttackID);
            Clear_DefendedHitBoxes(Event.dlAttackerID, Event.iAttackID);
        });

    return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 31~35행: Update_HitRecords
void CCombatSystem_Manager::Update_HitRecords(_float fTimeDelta)
{
    for (auto& Record : m_HitRecords)
        Record.fHitTimer += fTimeDelta;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 37~74행: Process_Collisions
void CCombatSystem_Manager::Process_Collisions(const vector<CCollision_Manager::COLLISION_RESULT>& Results, _float fTimeDelta)
{
    Update_HitRecords(fTimeDelta);

    for (const auto& Result : Results)
    {
        if (Is_BodyCollision(Result))
        {
            Resolve_BodyCollision(Result);
            continue;
        }
        if (Is_MonsterWarningPlayerDodgeBox(Result))
        {
            Resolve_JustEvadeCollision(Result);
            continue;
        }
        if (Is_MonsterWarningPlayerParryBox(Result))
        {
            Resolve_ParryCollision(Result);
            continue;
        }
        if (Is_MonsterHitPlayerParryBox(Result))
        {
            Resolve_ParryImpactCollision(Result);
            continue;
        }
        if (Is_MonsterHitPlayerDodgeBox(Result))
        {
            Resolve_JustEvadeCollision(Result);
            continue;
        }
        if (Is_PlayerHitMonster(Result))
        {
            Resolve_HitCollision(Result);
            continue;
        }
    }
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 76~85행: Clear_HitRecords
void CCombatSystem_Manager::Clear_HitRecords(_udlong dlAttackerID)
{
    for (auto iter = m_HitRecords.begin(); iter != m_HitRecords.end();)
    {
        if (iter->dlAttackerID == dlAttackerID)
            iter = m_HitRecords.erase(iter);
        else
            ++iter;
    }
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 87~101행: Clear_HitRecords
void CCombatSystem_Manager::Clear_HitRecords(_udlong dlAttackerID, _uint iAttackID)
{
    for (auto iter = m_HitRecords.begin(); iter != m_HitRecords.end();)
    {
        if (iter->dlAttackerID == dlAttackerID &&
            iter->iAttackID == iAttackID)
        {
            iter = m_HitRecords.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 103~109행: Is_BodyCollision
_bool CCombatSystem_Manager::Is_BodyCollision(const CCollision_Manager::COLLISION_RESULT& Result) const
{
    const _uint Src = Result.iSrcGroup;
    const _uint Dst = Result.iDstGroup;

    return Src == ETOUI(COLLISION_GROUP::PLAYER_BODY) && Dst == ETOUI(COLLISION_GROUP::MONSTER_BODY);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 111~120행: Is_PlayerHitMonster
_bool CCombatSystem_Manager::Is_PlayerHitMonster(const CCollision_Manager::COLLISION_RESULT& Result) const
{
    const _uint Src = Result.iSrcGroup;
    const _uint Dst = Result.iDstGroup;
    
    _bool bMosterHit =  Src == ETOUI(COLLISION_GROUP::PLAYER_HITBOX) && Dst == ETOUI(COLLISION_GROUP::MONSTER_HURTBOX);
    _bool bPlayerHit =  Src == ETOUI(COLLISION_GROUP::MONSTER_HITBOX) && Dst == ETOUI(COLLISION_GROUP::PLAYER_HURTBOX);

    return bMosterHit || bPlayerHit;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 122~125행: Is_MonsterHitPlayerDodgeBox
_bool CCombatSystem_Manager::Is_MonsterHitPlayerDodgeBox(const CCollision_Manager::COLLISION_RESULT& Result) const
{
    return Result.iSrcGroup == ETOUI(COLLISION_GROUP::MONSTER_HITBOX) && Result.iDstGroup == ETOUI(COLLISION_GROUP::PLAYER_DODGEBOX);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 127~130행: Is_MonsterHitPlayerParryBox
_bool CCombatSystem_Manager::Is_MonsterHitPlayerParryBox(const CCollision_Manager::COLLISION_RESULT& Result) const
{
    return Result.iSrcGroup == ETOUI(COLLISION_GROUP::MONSTER_HITBOX) && Result.iDstGroup == ETOUI(COLLISION_GROUP::PLAYER_PARRYBOX);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 132~135행: Is_MonsterWarningPlayerDodgeBox
_bool CCombatSystem_Manager::Is_MonsterWarningPlayerDodgeBox(const CCollision_Manager::COLLISION_RESULT& Result) const
{
    return Result.iSrcGroup == ETOUI(COLLISION_GROUP::MONSTER_WARNINGBOX) && Result.iDstGroup == ETOUI(COLLISION_GROUP::PLAYER_DODGEBOX);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 137~140행: Is_MonsterWarningPlayerParryBox
_bool CCombatSystem_Manager::Is_MonsterWarningPlayerParryBox(const CCollision_Manager::COLLISION_RESULT& Result) const
{
    return Result.iSrcGroup == ETOUI(COLLISION_GROUP::MONSTER_WARNINGBOX) && Result.iDstGroup == ETOUI(COLLISION_GROUP::PLAYER_PARRYBOX);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 142~155행: Is_DefendedHitBox
_bool CCombatSystem_Manager::Is_DefendedHitBox(_udlong dlAttackerID, _uint iAttackID, _uint iHitBoxID) const
{
    for (const auto& Record : m_DefendedHitBoxRecords)
    {
        if (Record.dlAttackerID == dlAttackerID &&
            Record.iAttackID == iAttackID &&
            Record.iHitBoxID == iHitBoxID)
        {
            return true;
        }
    }

    return false;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 157~171행: Find_HitRecord
CCombatSystem_Manager::HIT_RECORD* CCombatSystem_Manager::Find_HitRecord(_udlong dlAttackerID, _udlong dlTargetID, _uint iAttackID, _uint iHitBoxID)
{
    for (auto& Record : m_HitRecords)
    {
        if (Record.dlAttackerID == dlAttackerID &&
            Record.dlTargetID == dlTargetID &&
            Record.iAttackID == iAttackID &&
            Record.iHitBoxID == iHitBoxID)
        {
            return &Record;
        }
    }

    return nullptr;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 172~183행: Add_HitRecord
void CCombatSystem_Manager::Add_HitRecord(_udlong dlAttackerID, _udlong dlTargetID, _uint iAttackID, _uint iHitBoxID)
{
    HIT_RECORD Record = {};
    Record.dlAttackerID = dlAttackerID;
    Record.dlTargetID = dlTargetID;
    Record.iAttackID = iAttackID;
    Record.iHitBoxID = iHitBoxID;
    Record.iHitCount = 1;
    Record.fHitTimer = 0.f;

    m_HitRecords.push_back(Record);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 185~222행: Resolve_BodyCollision
void CCombatSystem_Manager::Resolve_BodyCollision(const CCollision_Manager::COLLISION_RESULT& Result)
{
    CGameObject* pSrc = Result.pSrcOwner;
    CGameObject* pDst = Result.pDstOwner;

    if (pSrc == nullptr || pDst == nullptr)
        return;

    CCharacter* pSrcCharacter = dynamic_cast<CCharacter*>(pSrc);
    CCharacter* pDstCharacter = dynamic_cast<CCharacter*>(pDst);

    if (pSrcCharacter != nullptr && pSrcCharacter->Should_IgnoreBodyCollision())
        return;

    if (pDstCharacter != nullptr && pDstCharacter->Should_IgnoreBodyCollision())
        return;

    CTransform* pSrcTransform = pSrc->Get_Transform();
    CTransform* pDstTransform = pDst->Get_Transform();

    if (pSrcTransform == nullptr || pDstTransform == nullptr)
        return;

    _vector vSrcPos = pSrcTransform->Get_State(STATE::POSITION);
    _vector vDstPos = pDstTransform->Get_State(STATE::POSITION);

    _vector vPushDir = vSrcPos - vDstPos;
    vPushDir = XMVectorSetY(vPushDir, 0.f);

    if (XMVectorGetX(XMVector3LengthSq(vPushDir)) < 0.0001f)
        return;

    vPushDir = XMVector3Normalize(vPushDir);

    const _float fPushPower = 0.01f;

    pSrcTransform->Set_State(STATE::POSITION, vSrcPos + vPushDir * fPushPower);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 224~269행: Resolve_HitCollision
void CCombatSystem_Manager::Resolve_HitCollision(const CCollision_Manager::COLLISION_RESULT& Result)
{
    if (Result.pSrcOwner == nullptr || Result.pDstOwner == nullptr)
        return;

    const _udlong dlAttackerID = Result.pSrcOwner->Get_ObjectID();
    const _udlong dlTargetID = Result.pDstOwner->Get_ObjectID();

    const _uint iAttackID = Result.iSrcAttackID;
    const _uint iHitBoxID = Result.iSrcHitBoxID;

    // 패링/저스트 회피로 이미 처리된 히트박스는 일반 피격 처리 금지.
    if (Is_DefendedHitBox(dlAttackerID, iAttackID, iHitBoxID))
        return;

    HITBOX_ACTIVE_DESC HitBoxDesc{};
    IHitBoxProvider* pProvider = dynamic_cast<IHitBoxProvider*>(Result.pSrcOwner);
    if (pProvider == nullptr)
        return;

    if (!pProvider->Get_HitBoxInfo(iAttackID, iHitBoxID, HitBoxDesc))
        return;

    HIT_RECORD* pRecord = Find_HitRecord(dlAttackerID, dlTargetID, iAttackID, iHitBoxID);
    if (pRecord == nullptr)
    {
        Add_HitRecord(dlAttackerID, dlTargetID, iAttackID, iHitBoxID);

        Apply_Damage(Result, HitBoxDesc, 0);
        return;
    }

    if (!HitBoxDesc.bMultiHit)
        return;

    if (pRecord->iHitCount >= HitBoxDesc.iMaxHitCount)
        return;

    if (pRecord->fHitTimer < HitBoxDesc.fHitInterval)
        return;

    pRecord->fHitTimer = 0.f;
    ++pRecord->iHitCount;

    Apply_Damage(Result, HitBoxDesc, pRecord->iHitCount - 1);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 271~295행: Resolve_JustEvadeCollision
void CCombatSystem_Manager::Resolve_JustEvadeCollision(const CCollision_Manager::COLLISION_RESULT& Result)
{
    if (Result.pSrcOwner == nullptr || Result.pDstOwner == nullptr)
        return;
    const _udlong dlAttackerID = Result.pSrcOwner->Get_ObjectID();

    if (Is_DefendedHitBox(dlAttackerID, Result.iSrcAttackID, Result.iSrcHitBoxID))
        return;

    DAMAGE_DESC DamageDesc{};
    DamageDesc.pAttacker = Result.pSrcOwner;
    DamageDesc.iAttackID = Result.iSrcAttackID;
    DamageDesc.iHitBoxID = Result.iSrcHitBoxID;
    DamageDesc.iColliderID = Result.iSrcColliderID;

    IDefensiveResponder* pResponder = dynamic_cast<IDefensiveResponder*>(Result.pDstOwner);

    if (pResponder == nullptr)
        return;

    if (pResponder->Try_JustEvade(DamageDesc))
    {
        Add_DefendedHitBox(dlAttackerID, Result.iSrcAttackID, Result.iSrcHitBoxID);
    }
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 297~329행: Resolve_ParryCollision
void CCombatSystem_Manager::Resolve_ParryCollision(const CCollision_Manager::COLLISION_RESULT& Result)
{
    if (Result.pSrcOwner == nullptr || Result.pDstOwner == nullptr)
        return;

    DAMAGE_DESC DamageDesc{};
    DamageDesc.pAttacker = Result.pSrcOwner;
    DamageDesc.iAttackID = Result.iSrcAttackID;
    DamageDesc.iHitBoxID = Result.iSrcHitBoxID;
    DamageDesc.iColliderID = Result.iSrcColliderID;

    CCharacter* pAttacker = dynamic_cast<CCharacter*>(Result.pSrcOwner);
    if (pAttacker != nullptr)
    {
        HITBOX_ACTIVE_DESC WarningDesc{};
        if (pAttacker->Get_WarningBoxInfo(Result.iSrcAttackID, Result.iSrcHitBoxID, WarningDesc))
        {
            if (!WarningDesc.bParryable)
                return;

            DamageDesc.eReactionType = WarningDesc.eReactionType;
            DamageDesc.bConsecutiveParry = WarningDesc.bConsecutiveParry;
            DamageDesc.bCancelOwnerOnParry = WarningDesc.bCancelOwnerOnParry;
        }
    }

    IDefensiveResponder* pResponder = dynamic_cast<IDefensiveResponder*>(Result.pDstOwner);

    if (pResponder == nullptr)
        return;

    pResponder->Try_Parry(DamageDesc);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 331~375행: Resolve_ParryImpactCollision
void CCombatSystem_Manager::Resolve_ParryImpactCollision(const CCollision_Manager::COLLISION_RESULT& Result)
{
    if (Result.pSrcOwner == nullptr || Result.pDstOwner == nullptr)
        return;

    const _udlong dlAttackerID = Result.pSrcOwner->Get_ObjectID();

    if (Is_DefendedHitBox(dlAttackerID, Result.iSrcAttackID, Result.iSrcHitBoxID))
        return;

    DAMAGE_DESC DamageDesc{};
    DamageDesc.pAttacker = Result.pSrcOwner;
    DamageDesc.iAttackID = Result.iSrcAttackID;
    DamageDesc.iHitBoxID = Result.iSrcHitBoxID;
    DamageDesc.iColliderID = Result.iSrcColliderID;

    CCharacter* pAttacker = dynamic_cast<CCharacter*>(Result.pSrcOwner);
    if (pAttacker != nullptr)
    {
        HITBOX_ACTIVE_DESC HitBoxDesc{};
        if (pAttacker->Get_HitBoxInfo(Result.iSrcAttackID, Result.iSrcHitBoxID, HitBoxDesc))
        {
            if (!HitBoxDesc.bParryable)
                return;

            DamageDesc.eReactionType = HitBoxDesc.eReactionType;
            DamageDesc.bConsecutiveParry = HitBoxDesc.bConsecutiveParry;
            DamageDesc.bCancelOwnerOnParry = HitBoxDesc.bCancelOwnerOnParry;
        }
    }

    IDefensiveResponder* pResponder = dynamic_cast<IDefensiveResponder*>(Result.pDstOwner);

    if (pResponder == nullptr)
        return;

    if (pResponder->Try_ParryImpact(DamageDesc))
    {
        Add_DefendedHitBox(dlAttackerID, Result.iSrcAttackID, Result.iSrcHitBoxID);

        CMonster* pMonster = dynamic_cast<CMonster*>(Result.pSrcOwner);
        if (pMonster != nullptr)
            pMonster->On_Parried(DamageDesc);
    }
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 377~471행: Apply_Damage
void CCombatSystem_Manager::Apply_Damage(const CCollision_Manager::COLLISION_RESULT& Result, const HITBOX_ACTIVE_DESC& HitBoxDesc, _uint iDamageRateIndex)
{
    if (Result.pSrcOwner == nullptr || Result.pDstOwner == nullptr)
        return;

    IDamageable* pDamageable = dynamic_cast<IDamageable*>(Result.pDstOwner);
    if (pDamageable == nullptr)
        return;

    DAMAGE_DESC DamageDesc = {};
    DamageDesc.pAttacker = Result.pSrcOwner;

    if (Result.iSrcGroup == ETOUI(COLLISION_GROUP::PLAYER_HITBOX) &&
        Result.iDstGroup == ETOUI(COLLISION_GROUP::MONSTER_HURTBOX))
    {
        DamageDesc.fDamage = Calculate_PlayerDamage(Result, HitBoxDesc, iDamageRateIndex);
    }
    else
    {
        DamageDesc.fDamage = Calculate_MonsterDamage(Result, HitBoxDesc, iDamageRateIndex);
    }

    DamageDesc.iAttackID = HitBoxDesc.iAttackID;
    DamageDesc.iHitBoxID = HitBoxDesc.iHitBoxID;
    DamageDesc.iColliderID = Result.iSrcColliderID;
    DamageDesc.bCanStagger = HitBoxDesc.bCanStagger;
    DamageDesc.eReactionType = HitBoxDesc.eReactionType;
    DamageDesc.eAttackType = HitBoxDesc.eAttackType;

    CTransform* pSrcTransform = Result.pSrcOwner->Get_Transform();
    CTransform* pDstTransform = Result.pDstOwner->Get_Transform();

    if (pSrcTransform != nullptr && pDstTransform != nullptr)
    {
        _vector vSrcPos = pSrcTransform->Get_State(STATE::POSITION);
        _vector vDstPos = pDstTransform->Get_State(STATE::POSITION);

        _vector vHitDir = vSrcPos - vDstPos;
        vHitDir = XMVectorSetY(vHitDir, 0.f);

        if (XMVectorGetX(XMVector3LengthSq(vHitDir)) > 0.0001f)
        {
            vHitDir = XMVector3Normalize(vHitDir);

            _vector vDstLook = pDstTransform->Get_State(STATE::LOOK);
            vDstLook = XMVector3Normalize(XMVectorSetY(vDstLook, 0.f));

            const _float fDot = XMVectorGetX(XMVector3Dot(vDstLook, vHitDir));

            DamageDesc.eHitDir = fDot >= 0.f ? DIR_SUBSTATE::FRONT : DIR_SUBSTATE::BACK;
        }
    }

    CMiyabi* pMiyabi = dynamic_cast<CMiyabi*>(Result.pSrcOwner);
    if (pMiyabi != nullptr)
    {
        if (HitBoxDesc.eAttackType == ATTACK_DAMAGE_TYPE::NORMAL || 
            HitBoxDesc.eAttackType == ATTACK_DAMAGE_TYPE::COUNTER ||
            HitBoxDesc.eAttackType == ATTACK_DAMAGE_TYPE::CHARGE)
            pMiyabi->Add_SpecialGauge(0.5f);
    }

    pDamageable->Take_Damage(DamageDesc);

    EVENT_DAMAGE_TEXT EventDesc = {};
    EventDesc.fDamage = DamageDesc.fDamage;
    EventDesc.dlTargetID = Result.pDstOwner->Get_ObjectID();

    if (Result.iSrcGroup == ETOUI(COLLISION_GROUP::PLAYER_HITBOX) && 
        Result.iDstGroup == ETOUI(COLLISION_GROUP::MONSTER_HURTBOX))
    {
        EventDesc.eType = DAMAGE_TEXT_TYPE::PLAYER_TO_MONSTER;
        if (pMiyabi != nullptr)
            pMiyabi->On_AttackHit_ForChargeStack(HitBoxDesc.iAttackID);
    }
    else if (Result.iSrcGroup == ETOUI(COLLISION_GROUP::MONSTER_HITBOX) &&
             Result.iDstGroup == ETOUI(COLLISION_GROUP::PLAYER_HURTBOX))
    {
        EventDesc.eType = DAMAGE_TEXT_TYPE::MONSTER_TO_PLAYER;
    }
    else
    {
        return;
    }

    if (pDstTransform != nullptr)
    {
        _vector vPos = pDstTransform->Get_State(STATE::POSITION);
        vPos += XMVectorSet(0.f, 1.5f, 0.f, 0.f);

        XMStoreFloat3(&EventDesc.vWorldPos, vPos);
    }

    m_pGameInstance->Publish_Event<EVENT_DAMAGE_TEXT>(EventDesc);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 473~484행: Add_DefendedHitBox
void CCombatSystem_Manager::Add_DefendedHitBox(_udlong dlAttackerID, _uint iAttackID, _uint iHitBoxID)
{
    if (Is_DefendedHitBox(dlAttackerID, iAttackID, iHitBoxID))
        return;

    DEFENDED_HITBOX_RECORD Record = {};
    Record.dlAttackerID = dlAttackerID;
    Record.iAttackID = iAttackID;
    Record.iHitBoxID = iHitBoxID;

    m_DefendedHitBoxRecords.push_back(Record);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 486~500행: Clear_DefendedHitBoxes
void CCombatSystem_Manager::Clear_DefendedHitBoxes(_udlong dlAttackerID, _uint iAttackID)
{
    for (auto iter = m_DefendedHitBoxRecords.begin(); iter != m_DefendedHitBoxRecords.end();)
    {
        if (iter->dlAttackerID == dlAttackerID &&
            iter->iAttackID == iAttackID)
        {
            iter = m_DefendedHitBoxRecords.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 502~513행: Get_HitDamageRatePercent
_float CCombatSystem_Manager::Get_HitDamageRatePercent(const HITBOX_ACTIVE_DESC& HitBoxDesc, _uint iDamageRateIndex) const
{
    if (!HitBoxDesc.DamageRatePercents.empty())
    {
        if (iDamageRateIndex < HitBoxDesc.DamageRatePercents.size())
            return HitBoxDesc.DamageRatePercents[iDamageRateIndex];

        return HitBoxDesc.DamageRatePercents.back();
    }

    return 100.f;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 515~531행: Calculate_PlayerDamage
_float CCombatSystem_Manager::Calculate_PlayerDamage(const CCollision_Manager::COLLISION_RESULT& Result, const HITBOX_ACTIVE_DESC& HitBoxDesc, _uint iDamageRateIndex) const
{
    CMiyabi* pMiyabi = dynamic_cast<CMiyabi*>(Result.pSrcOwner);
    CMonster* pMonster = dynamic_cast<CMonster*>(Result.pDstOwner);

    if (pMiyabi == nullptr || pMonster == nullptr)
        return 10.f;

    const _float fAttackPower = pMiyabi->Get_AttackPower();
    const _float fRatePercent = Get_HitDamageRatePercent(HitBoxDesc, iDamageRateIndex);
    const _float fBaseDamage = fAttackPower * (fRatePercent * 0.01f);

    const _float fDefenseCoeff = Calculate_MonsterDefenseCoeff(pMonster);
    const _float fGroggyMultiplier = pMonster->Is_Groggy() ? 1.5f : 1.f;

    return fBaseDamage * fDefenseCoeff * fGroggyMultiplier * 5.f;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 533~544행: Calculate_MonsterDamage
_float CCombatSystem_Manager::Calculate_MonsterDamage(const CCollision_Manager::COLLISION_RESULT& Result, const HITBOX_ACTIVE_DESC& HitBoxDesc, _uint iDamageRateIndex) const
{
    CMonster* pMonster = dynamic_cast<CMonster*>(Result.pSrcOwner);

    if (pMonster == nullptr)
        return 10.f;

    const _float fAttackPower = pMonster->Get_AttackPower();
    const _float fRatePercent = Get_HitDamageRatePercent(HitBoxDesc, iDamageRateIndex);

    return fAttackPower * (fRatePercent * 0.01f);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 546~565행: Calculate_MonsterDefenseCoeff
_float CCombatSystem_Manager::Calculate_MonsterDefenseCoeff(CMonster* pMonster) const
{
    if (pMonster == nullptr)
        return 1.f;

    switch (pMonster->Get_MonsterRank())
    {
    case CMonster::MONSTER_RANK::NORMAL:
        return 1.f;

    case CMonster::MONSTER_RANK::ELITE:
        return 0.8f;

    case CMonster::MONSTER_RANK::BOSS:
        return 0.6f;

    default:
        return 1.f;
    }
}

// [중략] 원본의 다른 함수 및 선언.
