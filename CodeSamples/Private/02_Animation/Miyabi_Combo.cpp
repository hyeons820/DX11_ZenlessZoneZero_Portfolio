// 포트폴리오용 코드 발췌: 입력 버퍼와 애니메이션 진행률을 이용한 일반 공격 콤보 연결
// 원본: Client/Private/Miyabi.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.
// 읽는 흐름: Update → 입력 문맥·우선순위 → 공격 입력·버퍼 → 상태 전환 요청. 이동·회피·차지·특수기 개별 처리와 상태별 애니메이션 매핑은 중략했습니다.

// 원본 669~747행: Update
void CMiyabi::Update(_float fTimeDelta)
{
    _float fPlayerDelta = m_pGameInstance->Get_ScaledDelta(TIME_LAYER::PLAYER, fTimeDelta);

    const _bool bAllowPlayerInput = m_pGameInstance->Get_RunMode() != RUN_MODE::EDITOR_SCENE;

    Update_PendingVoice(fTimeDelta);

    if (bAllowPlayerInput && m_pGameInstance->Get_KeyDown('P'))
        m_fCurSpecialGauge = 100.f;

    if (bAllowPlayerInput && m_pGameInstance->Get_KeyDown('O'))
        m_fCurDecibel = 3000.f;

    if (bAllowPlayerInput && m_pGameInstance->Get_KeyDown('I'))
        m_iCurChargeStack = m_iMaxChargeStack;

    Update_SpecialGauge(fTimeDelta);    

    if (bAllowPlayerInput)
    {
        Handle_StateInput(fTimeDelta);
    }
    else
    {
        // 편집 입력이 이동이나 공격으로 이어지지 않도록 입력과 예약 공격을 비운다.
        m_InputContext = {};
        Clear_MoveDirection();
        m_bAttackBuffered = false;
        m_fCurAttackBufferTime = 0.f;
        m_bCounterBuffered = false;
        m_bChargeHolding = false;
        m_fCurChargeTime = 0.f;
        Update_ActionTimers(fTimeDelta);
    }

    if (m_pStateCom->Get_ActionIndex() != ETOUI(MIYABI_ACTION::NORMAL_ATTACK_5))
    {
        m_bNormalAttack5ChargeStackGranted = false;
    }

    if (m_pStateCom->Get_MainState() != MAIN_STATE::ATTACK)
    {
        m_iLastNormalAttackSoundID = UINT_MAX;
    }

    Update_AttackBuffer(fTimeDelta);

    __super::Update(fPlayerDelta);

    if (bAllowPlayerInput)
        Update_ChargeFlow(fTimeDelta);

    if (m_bQuestStartPlaying)
    {
        ANIM_SYNC_INFO AnimInfo{};
        if (SUCCEEDED(m_pModelCom->Get_CurrentAnimSyncInfo(AnimInfo)))
        {
            if (AnimInfo.fNormalizedTime >= 0.98f)
            {
                m_bQuestStartPlaying = false;

                CState::STATE_REQUEST Request{};
                Request.bValid = true;
                Request.eMainState = MAIN_STATE::IDLE;
                Request.eMoveState = MOVE_SUBSTATE::NONE;
                Request.eAttackState = ATTACK_SUBSTATE::NONE;
                Request.eDirState = DIR_SUBSTATE::NONE;
                Request.eStatePhase = STATE_PHASE::LOOP;

                m_pStateCom->Request_State(Request);
            }
        }

        return;
    }

    Update_CameraFocus(fTimeDelta); 
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 1560~1586행: Handle_StateInput
void CMiyabi::Handle_StateInput(_float fTimeDelta)
{
    Update_ActionTimers(fTimeDelta);
    Update_InputContext();
    Update_ActionSyncInfo();

    Handle_ChargeInput(fTimeDelta);

    bool bActionRequested = false;

    if (!bActionRequested)
        bActionRequested = Handle_UltimateInput();
    
    if (!bActionRequested)
        bActionRequested = Handle_ParryInput();

    if (!bActionRequested)
        bActionRequested = Handle_EvadeInput(fTimeDelta);

    if (!bActionRequested)
        bActionRequested = Handle_SpecialInput();

    if (!bActionRequested)
        bActionRequested = Handle_AttackInput();

    Handle_MoveInput(fTimeDelta);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 1650~1710행: Update_InputContext
void CMiyabi::Update_InputContext()
{
    m_InputContext = {};

    _matrix fCameraMat = XMLoadFloat4x4(m_pGameInstance->Get_Transform_Inverse(D3DTS::VIEW));

    _vector vCamLook = fCameraMat.r[ETOUI(STATE::LOOK)];
    _vector vCamRight = fCameraMat.r[ETOUI(STATE::RIGHT)];

    vCamLook = XMVector3Normalize(XMVectorSetY(vCamLook, 0.f));
    vCamRight = XMVector3Normalize(XMVectorSetY(vCamRight, 0.f));

    _vector vInputDir = XMVectorZero();

    if (m_pGameInstance->Get_KeyState('W') & 0x80)
    {
        vInputDir += vCamLook;
    }
    if (m_pGameInstance->Get_KeyState('S') & 0x80)
    {
        vInputDir -= vCamLook;
    }
    if (m_pGameInstance->Get_KeyState('D') & 0x80)
    {
        vInputDir += vCamRight;
    }
    if (m_pGameInstance->Get_KeyState('A') & 0x80)
    {
        vInputDir -= vCamRight;
    } 

    if (XMVectorGetX(XMVector3LengthSq(vInputDir)) > 0.f)
    {
        vInputDir = XMVector3Normalize(vInputDir);

        m_InputContext.bHasMoveInput = true;
        XMStoreFloat3(&m_InputContext.vMoveDir, vInputDir);
    }

    const _bool bShiftDown = m_pGameInstance->Get_KeyDown(VK_LSHIFT) || m_pGameInstance->Get_KeyDown(VK_SHIFT);
    const _bool bShiftHeld = (m_pGameInstance->Get_KeyState(VK_LSHIFT) & 0x80) || (m_pGameInstance->Get_KeyState(VK_SHIFT) & 0x80);
    const _bool bShiftUp   = m_pGameInstance->Get_KeyUp(VK_LSHIFT) || m_pGameInstance->Get_KeyUp(VK_SHIFT);

    const _bool bRButtonDown = m_pGameInstance->Get_MouseBtnDown(RIMB::RBUTTON);
    const _bool bRButtonHeld = (m_pGameInstance->Get_MouseBtnState(RIMB::RBUTTON) & 0x80);
    const _bool bRButtonUp   = m_pGameInstance->Get_MouseBtnUp(RIMB::RBUTTON);

    m_InputContext.bEvadePressed  = bShiftDown || bRButtonDown;
    m_InputContext.bEvadeHeld     = bShiftHeld || bRButtonHeld;
    m_InputContext.bEvadeReleased = bShiftUp || bRButtonUp;

    m_InputContext.bAttackPressed  = m_pGameInstance->Get_MouseBtnDown(RIMB::LBUTTON);
    m_InputContext.bAttackHeld     = (m_pGameInstance->Get_MouseBtnState(RIMB::LBUTTON) & 0x80);
    m_InputContext.bAttackReleased = m_pGameInstance->Get_MouseBtnUp(RIMB::LBUTTON);

    m_InputContext.bSpecialPressed = m_pGameInstance->Get_KeyDown('E');

    m_InputContext.bUltimatePressed = m_pGameInstance->Get_KeyDown('Q');

    m_InputContext.bParryPressed = m_pGameInstance->Get_KeyDown(VK_SPACE);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 1916~1986행: Update_ActionSyncInfo
void CMiyabi::Update_ActionSyncInfo()
{
    if (m_pStateCom->Get_MainState() == MAIN_STATE::HIT)
        return;

    CState::ACTION_SYNC_INFO Info = m_pStateCom->Get_ActionInfo();

    switch (m_pStateCom->Get_AttackSubState())
    {
    case ATTACK_SUBSTATE::COMBO5:
        Info.fAttackMotionEndTime = 0.88f;
        break;

    default:
        Info.fAttackMotionEndTime = 0.75f;
        break;
    }

    Info.fAttackEndFinishTime = 0.88f;

    switch (static_cast<MIYABI_ACTION>(m_pStateCom->Get_ActionIndex()))
    {
    case MIYABI_ACTION::SPECIAL_ATTACK_1:
        Info.fSpecialMotionEndTime = 0.72f;
        Info.fSpecialEndFinishTime = 0.82f;
        break;

    case MIYABI_ACTION::SPECIAL_ATTACK_2:
        Info.fSpecialMotionEndTime = 0.78f;
        Info.fSpecialEndFinishTime = 0.88f;
        break;

    case MIYABI_ACTION::CHARGE_END_1:
        Info.fChargeMotionEndTime = 0.88f;
        Info.fChargeEndFinishTime = 0.88f;
        break;

    case MIYABI_ACTION::CHARGE_END_2:
        Info.fChargeMotionEndTime = 0.88f;
        Info.fChargeEndFinishTime = 0.88f;
        break;

    case MIYABI_ACTION::CHARGE_END_3:
        Info.fChargeMotionEndTime = 0.88f;
        Info.fChargeEndFinishTime = 0.88f;
        break;
    }

    if (m_pStateCom->Get_MainState() == MAIN_STATE::ULTIMATE)
    {
        switch (m_pStateCom->Get_StatePhase())
        {
        case STATE_PHASE::START:
            Info.fUltimateMotionEndTime = 1.f;
            Info.fUltimateEndFinishTime = 1.f;
            break;

        case STATE_PHASE::ACTIVE:
            Info.fUltimateMotionEndTime = 0.95f;
            Info.fUltimateEndFinishTime = 0.98f;
            break;

        case STATE_PHASE::END:
            Info.fUltimateMotionEndTime = 0.35f;
            Info.fUltimateEndFinishTime = 0.92f;
            break;
        }
    }

    m_pStateCom->Set_ActionInfo(Info);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 1755~1797행: Handle_AttackInput
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

// 원본 1878~1914행: Update_AttackBuffer
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

// 원본 1988~2076행: Start_NormalAttack
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

    if (!m_pStateCom->Request_State(Request))
        return false;

    m_eCameraFocusMode = CAMERA_FOCUS_MODE::DEFAULT;

    EVENT_PLAY_EFFECT Event{};
    XMStoreFloat3(&Event.vPosition, m_pTransformCom->Get_State(STATE::POSITION));
    XMStoreFloat3(&Event.vDirection, XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)));
    Event.pFollowTransform = m_pTransformCom;

    switch (eAttackState)
    {
    case ATTACK_SUBSTATE::COMBO1:
        Event.eEffectID = EFFECT_ID::PLAYER_NORMAL1_FX;
        break;

    case ATTACK_SUBSTATE::COMBO2:
        Event.eEffectID = EFFECT_ID::PLAYER_NORMAL2_FX;
        break;

    case ATTACK_SUBSTATE::COMBO3:
        Event.eEffectID = EFFECT_ID::PLAYER_NORMAL3_FX;
        break;

    case ATTACK_SUBSTATE::COMBO4:
        Event.eEffectID = EFFECT_ID::PLAYER_NORMAL4_FX;
        Event.pFollowTransform = nullptr;
        break;

    case ATTACK_SUBSTATE::COMBO5:
        Event.eEffectID = EFFECT_ID::PLAYER_NORMAL5_FX;
        break;
    }

    if (Event.eEffectID != EFFECT_ID::END)
        m_pGameInstance->Publish_Event<EVENT_PLAY_EFFECT>(Event);

    const _uint iSoundID = ETOUI(Convert_AttackToAction(eAttackState));

    if (m_iLastNormalAttackSoundID != iSoundID)
    {
        m_iLastNormalAttackSoundID = iSoundID;

        Play_RandomNormalVoice();

        switch (eAttackState)
        {
        case ATTACK_SUBSTATE::COMBO1:
            m_pGameInstance->PlaySFX(L"Normal_Attack01_SFX", SOUND_CHANNEL::PLAYER, 0.8f);
            break;

        case ATTACK_SUBSTATE::COMBO2:
            m_pGameInstance->PlaySFX(L"Normal_Attack02_SFX", SOUND_CHANNEL::PLAYER, 0.8f);
            break;

        case ATTACK_SUBSTATE::COMBO3:
            m_pGameInstance->PlaySFX(L"Normal_Attack03_SFX", SOUND_CHANNEL::PLAYER, 0.8f);
            break;

        case ATTACK_SUBSTATE::COMBO4:
            m_pGameInstance->PlaySFX(L"Normal_Attack04_SFX", SOUND_CHANNEL::PLAYER, 0.8f);
            break;

        case ATTACK_SUBSTATE::COMBO5:
            m_pGameInstance->PlaySFX(L"Normal_Attack05_SFX", SOUND_CHANNEL::PLAYER, 0.8f);
            break;
        }
    }

    return true;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 3071~3091행: Get_NextCombo
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

// 원본 3093~3115행: Convert_AttackToAction
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
