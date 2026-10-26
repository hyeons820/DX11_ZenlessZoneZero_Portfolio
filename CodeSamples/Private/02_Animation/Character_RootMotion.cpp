// 포트폴리오용 코드 발췌: 애니메이션 갱신 결과를 캐릭터 이동에 반영하고 공격 종료를 알림
// 원본: Client/Private/Character.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.
// 읽는 흐름: Update → 상태·이동 갱신 → Model::Play_Animation → 루트 이동 반영 → 공격 종료 이벤트. Apply_StateToAnimation은 파생 클래스에서 구현하며 상태별 애니메이션 번호 매핑은 중략했습니다.

// 원본 60~119행: Update
void CCharacter::Update(_float fTimeDelta)
{
    Update_MoveVelocity(fTimeDelta);

    CState::MOVE_SYNC_INFO MoveInfo = {};
    MoveInfo.bHasMoveInput = m_eRequestedMoveState != MOVE_SUBSTATE::NONE && m_fTargetSpeed > 0.f;
    MoveInfo.fCurrentSpeed = m_fCurrentSpeed;
    MoveInfo.fTargetSpeed = m_fTargetSpeed;

    switch (m_eRequestedMoveState)
    {
    case MOVE_SUBSTATE::RUN:
        MoveInfo.fMaxSpeed = m_fRunSpeed;
        MoveInfo.eRequestedMoveState = MOVE_SUBSTATE::RUN;
        break;

    case MOVE_SUBSTATE::WALK:
        MoveInfo.fMaxSpeed = m_fWalkSpeed;
        MoveInfo.eRequestedMoveState = MOVE_SUBSTATE::WALK;
        break;

    case MOVE_SUBSTATE::NONE:
    default:
        MoveInfo.fMaxSpeed = m_fWalkSpeed;
        MoveInfo.eRequestedMoveState = MOVE_SUBSTATE::NONE;
        break;
    }

    m_pStateCom->Set_MoveInfo(MoveInfo);

    ANIM_SYNC_INFO Animinfo = {};
    m_pModelCom->Get_CurrentAnimSyncInfo(Animinfo);

    m_pStateCom->Tick(fTimeDelta, Animinfo);

    const STATE_DESC& StateDesc = m_pStateCom->Get_StateDesc();
    m_pModelCom->Set_UseRootMotion(Should_UseRootMotion(StateDesc));

    Update_AttackEndEvent();

    if (m_pStateCom->Has_PendingAnimChange())
    {
        // 상태가 어떤지 보고 실제 애니메이션 번호로 바꾸기
        Apply_StateToAnimation();

        m_pStateCom->Clear_PendingAnimChange();
    }

    Apply_Move(fTimeDelta);

    ANIMATION_UPDATE_RESULT AnimResult = m_pModelCom->Play_Animation(fTimeDelta);

    if (AnimResult.bFinished)
        m_pStateCom->On_AnimationFinished();

    if (m_pModelCom->Is_UsingRootMotion())
        Apply_RootMotion();

    m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 257~287행: Update_MoveVelocity
void CCharacter::Update_MoveVelocity(_float fTimeDelta)
{
    _float fSpeedDelta = 0.f;

    if (m_fTargetSpeed > m_fCurrentSpeed)
    {
        fSpeedDelta = m_fAcceleration * fTimeDelta;
        m_fCurrentSpeed = min(m_fCurrentSpeed + fSpeedDelta, m_fTargetSpeed);
    }
    else
    {
        fSpeedDelta = m_fDeceleration * fTimeDelta;
        m_fCurrentSpeed = max(m_fCurrentSpeed - fSpeedDelta, m_fTargetSpeed);
    }

    _vector vDir = XMLoadFloat3(&m_vMoveDir);

    if (m_fCurrentSpeed <= 0.01f)
    {
        m_fCurrentSpeed = 0.f;
        m_vVelocity = _float3(0.f, 0.f, 0.f);
        return;
    }
    if (XMVectorGetX(XMVector3LengthSq(vDir)) <= 0.f)
    {
        m_vVelocity = _float3(0.f, 0.f, 0.f);
        return;
    }

    XMStoreFloat3(&m_vVelocity, XMVector3Normalize(vDir) * m_fCurrentSpeed);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 333~356행: Apply_Move
void CCharacter::Apply_Move(_float fTimeDelta)
{
    const STATE_DESC& StateDesc = m_pStateCom->Get_StateDesc();

    if (!StateDesc.bCanMove)
        return;

    if (!Has_MoveDirection())
        return;

    _vector vVelocity = XMLoadFloat3(&m_vVelocity);
    if (XMVectorGetX(XMVector3LengthSq(vVelocity)) <= 0.f)
        return;

    _vector vMoveDir = XMVector3Normalize(vVelocity);

    if (StateDesc.bCanRotate && m_bRotateToMoveDirection)
       m_pTransformCom->Rotate_ToDirection(vMoveDir, m_fRotationSpeed, fTimeDelta);

    _vector vDelta = vVelocity * fTimeDelta;

    m_pTransformCom->Add_Pos(vDelta, m_pNavigationCom);
    m_pTransformCom->Set_State(STATE::POSITION, m_pNavigationCom->Compute_OnNavigation(m_pTransformCom));
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 211~214행: Should_UseRootMotion
_bool CCharacter::Should_UseRootMotion(const STATE_DESC& StateDesc) const
{
    return StateDesc.bUseRootMotion;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 232~255행: Apply_RootMotion
void CCharacter::Apply_RootMotion()
{
    _float3 vDeltaLocal = m_pModelCom->Get_RootMotionDeltaLocal();

    _vector vRight = m_pTransformCom->Get_State(STATE::RIGHT);
    _vector vLook = m_pTransformCom->Get_State(STATE::LOOK);

    vRight = XMVector3Normalize(XMVectorSetY(vRight, 0.f));
    vLook = XMVector3Normalize(XMVectorSetY(vLook, 0.f));

    _vector vDeltaWorld = vRight * vDeltaLocal.x  - vLook * vDeltaLocal.z;

    vDeltaWorld = XMVectorSet(XMVectorGetX(vDeltaWorld), 0.f, XMVectorGetZ(vDeltaWorld), 0.f);

    vDeltaWorld = Filter_RootMotionDelta(vDeltaWorld);

    if (XMVectorGetX(XMVector3LengthSq(vDeltaWorld)) <= 0.000001f)
        return;

    _bool bUseNavigation = m_pStateCom->Get_StateDesc().bUseNavigation;
    m_pTransformCom->Add_Pos(vDeltaWorld, m_pNavigationCom, bUseNavigation);

    m_pTransformCom->Set_State(STATE::POSITION, m_pNavigationCom->Compute_OnNavigation(m_pTransformCom));
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 216~219행: Filter_RootMotionDelta
_vector CCharacter::Filter_RootMotionDelta(_fvector vDeltaWorld)
{
    return vDeltaWorld;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 375~394행: Update_AttackEndEvent
void CCharacter::Update_AttackEndEvent()
{
    const _bool bIsAttackActionState = Is_AttackActionState();
    const _uint iCurAttackID = m_pStateCom->Get_ActionIndex();

    if (m_bWasAttackActionState)
    {
        if (!bIsAttackActionState || iCurAttackID != m_iPrevAttackActionID)
        {
            EVENT_ATTACK_END Event = {};
            Event.dlAttackerID = Get_ObjectID();
            Event.iAttackID = m_iPrevAttackActionID;

            m_pGameInstance->Enqueue_Event(Event);
        }
    }

    m_bWasAttackActionState = bIsAttackActionState;
    m_iPrevAttackActionID = iCurAttackID;
}

// [중략] 원본의 다른 함수 및 선언.
