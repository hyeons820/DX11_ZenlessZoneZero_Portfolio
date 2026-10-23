// 포트폴리오용 코드 발췌: 입력 버퍼와 애니메이션 진행률을 이용한 일반 공격 콤보 연결
// 원본: Client/Private/Miyabi.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 1439~1481행: Handle_AttackInput
_bool CMiyabi::Handle_AttackInput()
{
    const MAIN_STATE eMain = m_pStateCom->Get_MainState();
    const STATE_PHASE ePhase = m_pStateCom->Get_StatePhase();

    if (m_bCounterReady && eMain == MAIN_STATE::PARRY)
    {
        if (m_InputContext.bAttackPressed)
            m_bCounterBuffered = true;

        if (ePhase == STATE_PHASE::END && (m_bCounterBuffered || m_InputContext.bAttackPressed || m_InputContext.bAttackHeld))
        {
            if (Start_CounterAttack())
                return true;
        }
    }

    if (!m_InputContext.bAttackPressed)
        return false;

    // Idle / Movement에서는 기본 공격 1타 먼저
    if (eMain == MAIN_STATE::IDLE || eMain == MAIN_STATE::MOVEMENT || 
        (eMain == MAIN_STATE::COUNTER && ePhase == STATE_PHASE::END))
    {
        return Start_NormalAttack(ATTACK_SUBSTATE::COMBO1);
    }

    // 공격 중이면 기존 콤보 처리 유지
    if (eMain == MAIN_STATE::ATTACK)
    {
        if (ePhase == STATE_PHASE::END)
        {
           Start_NormalAttack(ATTACK_SUBSTATE::COMBO1);
           return true;
        }

        m_bAttackBuffered = true;
        m_fCurAttackBufferTime = m_fMaxAttackBufferTime;
        return true;
    }

    return true;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 1562~1598행: Update_AttackBuffer
void CMiyabi::Update_AttackBuffer(_float fTimeDelta)
{
    if (!m_bAttackBuffered)
        return;

    m_fCurAttackBufferTime -= fTimeDelta;

    if (m_fCurAttackBufferTime <= 0.f)
    {
        m_bAttackBuffered = false;
        m_fCurAttackBufferTime = 0.f;
        return;
    }

    if (m_pStateCom->Get_MainState() != MAIN_STATE::ATTACK)
        return;

    if (m_pStateCom->Get_StatePhase() != STATE_PHASE::ACTIVE)
        return;

    ANIM_SYNC_INFO AnimInfo = {};
    m_pModelCom->Get_CurrentAnimSyncInfo(AnimInfo);

    if (AnimInfo.fNormalizedTime < 0.35f || AnimInfo.fNormalizedTime > 0.85f)
        return;

    ATTACK_SUBSTATE eCurrent = m_pStateCom->Get_AttackSubState();
    ATTACK_SUBSTATE eNext = Get_NextCombo(eCurrent);

    if (eNext == ATTACK_SUBSTATE::NONE)
        return;

    Start_NormalAttack(eNext);

    m_bAttackBuffered = false;
    m_fCurAttackBufferTime = 0.f;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 1672~1707행: Start_NormalAttack
_bool CMiyabi::Start_NormalAttack(ATTACK_SUBSTATE eAttackState)
{
    CState::STATE_REQUEST Request = {};
    Request.bValid = true;
    Request.eMainState = MAIN_STATE::ATTACK;
    Request.eMoveState = MOVE_SUBSTATE::NONE;
    Request.eAttackState = eAttackState;
    Request.eDirState = DIR_SUBSTATE::FRONT;
    Request.eStatePhase = STATE_PHASE::ACTIVE;
    Request.iActionIndex = ETOUI(Convert_AttackToAction(eAttackState));

    _vector vAttackDir = Resolve_AttackDir();

    Request.bHasActionDir = true;
    XMStoreFloat3(&Request.vActionDir, vAttackDir);

    Setup_AttackRootTarget(false);

    if (eAttackState == ATTACK_SUBSTATE::COMBO4)
    {
        m_eCameraFocusMode = CAMERA_FOCUS_MODE::LERP;

        _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
        _float fDistance = Compute_NonPierceFocusDistance(vAttackDir, 2.5f);
        Begin_AttackCameraFocus(vPos, vAttackDir, fDistance, 0.55f, 4.5f);
    }
    else
    {
        m_eCameraFocusMode = CAMERA_FOCUS_MODE::DEFAULT;
    }

    if (!m_pStateCom->Request_State(Request))
        return false;

    return true;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 2654~2674행: Get_NextCombo
ATTACK_SUBSTATE CMiyabi::Get_NextCombo(ATTACK_SUBSTATE eAttackState) const
{
    switch (eAttackState)
    {
    case ATTACK_SUBSTATE::COMBO1:
        return ATTACK_SUBSTATE::COMBO2;

    case ATTACK_SUBSTATE::COMBO2:
        return ATTACK_SUBSTATE::COMBO3;

    case ATTACK_SUBSTATE::COMBO3:
        return ATTACK_SUBSTATE::COMBO4;

    case ATTACK_SUBSTATE::COMBO4:
        return ATTACK_SUBSTATE::COMBO5;

    case ATTACK_SUBSTATE::COMBO5:
    default:
        return ATTACK_SUBSTATE::NONE;
    }
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 2676~2698행: Convert_AttackToAction
CMiyabi::MIYABI_ACTION CMiyabi::Convert_AttackToAction(ATTACK_SUBSTATE eAttackState) const
{
    switch (eAttackState)
    {
    case ATTACK_SUBSTATE::COMBO1:
        return MIYABI_ACTION::NORMAL_ATTACK_1;

    case ATTACK_SUBSTATE::COMBO2:
        return MIYABI_ACTION::NORMAL_ATTACK_2;

    case ATTACK_SUBSTATE::COMBO3:
        return MIYABI_ACTION::NORMAL_ATTACK_3;

    case ATTACK_SUBSTATE::COMBO4:
        return MIYABI_ACTION::NORMAL_ATTACK_4;

    case ATTACK_SUBSTATE::COMBO5:
        return MIYABI_ACTION::NORMAL_ATTACK_5;

    default:
        return MIYABI_ACTION::NONE;
    }
}

// [중략] 원본의 다른 함수 및 선언.
