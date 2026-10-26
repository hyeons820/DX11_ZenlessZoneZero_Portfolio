// 포트폴리오용 코드 발췌: 패링 진입·연속 패링·저스트 회피와 시간 배율 적용
// 원본: Client/Private/Miyabi.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 2640~2705행: Start_Parry
_bool CMiyabi::Start_Parry(CGameObject* pTarget, HIT_REACTION_TYPE eReactionType)
{
    if (pTarget == nullptr || m_pTransformCom == nullptr)
        return false;

    CTransform* pTargetTransform = pTarget->Get_Transform();
    if (pTargetTransform == nullptr)
        return false;

    _vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vTargetPos = pTargetTransform->Get_State(STATE::POSITION);

    _vector vToTarget = XMVectorSetY(vTargetPos - vMyPos, 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vToTarget)) <= 0.0001f)
        return false;

    vToTarget = XMVector3Normalize(vToTarget);

    Look_MoveDirection(vToTarget);

    MIYABI_ACTION eParryAction = MIYABI_ACTION::PARRY_L;

    if (eReactionType == HIT_REACTION_TYPE::LAUNCH || eReactionType == HIT_REACTION_TYPE::HEAVY)
        eParryAction = MIYABI_ACTION::PARRY_H;

    CState::STATE_REQUEST Request = {};
    Request.bValid = true;
    Request.eMainState = MAIN_STATE::PARRY;
    Request.eMoveState = MOVE_SUBSTATE::NONE;
    Request.eAttackState = ATTACK_SUBSTATE::NONE;
    Request.eDirState = DIR_SUBSTATE::FRONT;
    Request.eStatePhase = STATE_PHASE::START;
    Request.bHasActionDir = true;
    Request.iActionIndex = ETOUI(eParryAction);

    XMStoreFloat3(&Request.vActionDir, vToTarget);

    if (!m_pStateCom->Request_State(Request))
        return false;

    // 너무 멀면 몬스터 앞 패링 보정 위치로 당김.
    _float fParryTargetDistance = m_fParryTargetDistance;

    CMonster* pMonster = dynamic_cast<CMonster*>(pTarget);
    if (pMonster != nullptr)
        fParryTargetDistance = pMonster->Get_ParryTargetDistance();

    _vector vParryPos = vTargetPos - vToTarget * fParryTargetDistance;

    vParryPos = XMVectorSetY(vParryPos, XMVectorGetY(vMyPos));

    m_pTransformCom->Set_State(STATE::POSITION, vParryPos);

    m_pCombatTarget = pTarget;

    m_bParryWarningBoxActive = false;
    m_fParryWarningBoxTime = 0.f;

    m_bParryChainActive = false;
    m_iParryChainAttackID = UINT_MAX;
    m_pParryChainTarget = nullptr;
    
    Start_ParryImpactBox();

    return true;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 2849~2860행: Try_JustEvade
_bool CMiyabi::Try_JustEvade(const DAMAGE_DESC& DamageDesc)
{
    if (m_pStateCom->Get_MainState() != MAIN_STATE::EVADE)
        return false;

    if (m_bJustEvadeTriggered)
        return false;

    Start_JustEvade(DamageDesc);

    return true;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 2862~2878행: Try_Parry
_bool CMiyabi::Try_Parry(const DAMAGE_DESC& DamageDesc)
{
    if (!m_bParryWarningBoxActive)
        return false;

    if (DamageDesc.pAttacker == nullptr)
        return false;

    if (!Start_Parry(DamageDesc.pAttacker, DamageDesc.eReactionType))
        return false;

    // 여기서 parrybox를 바로 끄지 않는 게 핵심.
    // 실제 HITBOX와 맞부딪힐 때까지 유지해야 함.
    m_pCombatTarget = DamageDesc.pAttacker;

    return true;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 2880~2970행: Try_ParryImpact
_bool CMiyabi::Try_ParryImpact(const DAMAGE_DESC& DamageDesc)
{
    if (!m_bParryImpactBoxActive)
        return false;

    if (m_pStateCom->Get_MainState() != MAIN_STATE::PARRY)
        return false;

    if (DamageDesc.pAttacker == nullptr)
        return false;

    if (DamageDesc.pAttacker != m_pCombatTarget)
        return false;

    const STATE_PHASE ePhase = m_pStateCom->Get_StatePhase();

    const _bool bFirstImpact = ePhase == STATE_PHASE::START;

    const _bool bChainImpact =
        m_bParryChainActive &&
        DamageDesc.pAttacker == m_pParryChainTarget &&
        DamageDesc.iAttackID == m_iParryChainAttackID &&
        (ePhase == STATE_PHASE::ACTIVE || ePhase == STATE_PHASE::END);

    if (!bFirstImpact && !bChainImpact)
        return false;

    CState::STATE_REQUEST Request = {};
    Request.bValid = true;
    Request.eMainState = MAIN_STATE::PARRY;
    Request.eMoveState = MOVE_SUBSTATE::NONE;
    Request.eAttackState = ATTACK_SUBSTATE::NONE;
    Request.eDirState = DIR_SUBSTATE::FRONT;
    Request.eStatePhase = STATE_PHASE::ACTIVE;
    Request.iActionIndex = m_pStateCom->Get_ActionIndex();

    if (m_pStateCom->Has_ActionDir())
    {
        Request.bHasActionDir = true;
        Request.vActionDir = m_pStateCom->Get_ActionDir();
    }

    if (!m_pStateCom->Request_State(Request))
        return false;

    m_pGameInstance->PlaySFX(L"Parry_SFX", SOUND_CHANNEL::PLAYER, 1.f);

    m_eLastParryReactionType = DamageDesc.eReactionType;

    if (DamageDesc.eReactionType == HIT_REACTION_TYPE::HEAVY ||
        DamageDesc.eReactionType == HIT_REACTION_TYPE::KNOCKBACK ||
        DamageDesc.eReactionType == HIT_REACTION_TYPE::LAUNCH)
    {
        m_fCounterRushDistanceScale = 2.f;
    }
    else
    {
        m_fCounterRushDistanceScale = 1.f;
    }

    if (DamageDesc.bConsecutiveParry)
    {
        // 이 HitBox를 패링한 뒤 다음 HitBox를 계속 기다림.
        m_bParryChainActive = true;
        m_iParryChainAttackID = DamageDesc.iAttackID;
        m_pParryChainTarget = DamageDesc.pAttacker;

        m_bParryImpactBoxActive = true;
        m_fParryImpactBoxTime = 2.5f;

        m_bCounterReady = false;
        m_bCounterBuffered = false;

        return true;
    }

    // 여기서 연속 패링 종료.
    m_bParryChainActive = false;
    m_iParryChainAttackID = UINT_MAX;
    m_pParryChainTarget = nullptr;

    m_bParryImpactBoxActive = false;
    m_fParryImpactBoxTime = 0.f;

    m_bCounterReady = true;
    m_bCounterBuffered = false;
    m_pCombatTarget = DamageDesc.pAttacker;
    m_fCounterReadyTime = m_fCounterReadyDuration;

    return true;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 2972~2987행: Start_JustEvade
void CMiyabi::Start_JustEvade(const DAMAGE_DESC& DamageDesc)
{
    m_bJustEvadeTriggered = true;

    m_pGameInstance->Set_TimeScale(TIME_LAYER::MONSTER, 0.15f, 1.8f);
    m_pGameInstance->Set_TimeScale(TIME_LAYER::PLAYER, 0.15f, 0.4f);

#ifdef _DEBUG
    OutputDebugStringW(L"[Just Evade]\n");
#endif

    // TODO:
    // 1. 플레이어 제외 몬스터 슬로우
    // 2. 화면 이펙트
    // 3. 저스트 회피 전용 액션/카운터 가능 상태
}

// [중략] 원본의 다른 함수 및 선언.
