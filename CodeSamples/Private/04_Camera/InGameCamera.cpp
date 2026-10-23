// 포트폴리오용 코드 발췌: 추적 카메라에서 궁극기 카메라 애니메이션으로 전환한 뒤 복귀
// 원본: Client/Private/InGameCamera.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 19~26행: Request_Mode
void CInGameCamera::Request_Mode(INGAME_CAMERA_MODE eMode)
{
	if (eMode == m_eMode)
		return;

	m_eMode = eMode;
	m_fModeTime = 0.f;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 37~53행: Request_Ultimate
void CInGameCamera::Request_Ultimate(CTransform* pActor)
{
	if (pActor == nullptr)
		return;

	if (m_pUltCamClip == nullptr || !m_pUltCamClip->Is_Loaded())
		return;

	m_pLookTarget = pActor;

	XMStoreFloat4x4(&m_ScriptBaseWorld, XMLoadFloat4x4(pActor->Get_WorldMatrixPtr()));

	m_fScriptTime = 0.f;
	m_fScriptDuration = m_pUltCamClip->Get_Duration();

	Request_Mode(INGAME_CAMERA_MODE::ULTIMATE);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 102~122행: Late_Update
void CInGameCamera::Late_Update(_float fTimeDelta)
{
	switch (m_eMode)
	{
	case INGAME_CAMERA_MODE::FOLLOW:
		Update_Follow(fTimeDelta);
		break;

	case INGAME_CAMERA_MODE::LOCK_ON:
		break;

	case INGAME_CAMERA_MODE::ULTIMATE:
		Update_Ultimate(fTimeDelta);
		break;

	case INGAME_CAMERA_MODE::SCRIPTED:
		break;
	}

	__super::Update(fTimeDelta);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 129~157행: Update_Follow
void CInGameCamera::Update_Follow(_float fTimeDelta)
{
	if (m_pFollowTarget == nullptr)
		return;

 	CAMERA_FOCUS_INFO Info = m_pFollowTarget->Get_CameraForcusInfo();

 	_float fFollowSpeed = Info.fFollowSpeed;
	if (fFollowSpeed <= 0.f)
		fFollowSpeed = m_fFollowLerpSpeed;

	_vector vTarget = XMLoadFloat3(&Info.vFocusPosition) + XMVectorSet(0.f, m_fHeight, 0.f, 0.f);
	_vector vLook   = XMVector3Normalize(XMVector3Rotate(XMVectorSet(0.f, 0.f, 1.f, 0.f), XMLoadFloat4(&m_vQuaternion)));

	_float fRatio = min(1.f, fFollowSpeed * fTimeDelta);
	XMStoreFloat4(&m_vLerpPos, XMVectorLerp(XMLoadFloat4(&m_vLerpPos), vTarget, fRatio));

	_vector vEye	= XMVectorSubtract(XMLoadFloat4(&m_vLerpPos), XMVectorScale(vLook, m_fDistance));

	// World Up 기준 
	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vLook));
	_vector vUp = XMVector3Cross(vLook, vRight);

	m_pTransformCom->Set_State(STATE::RIGHT, vRight);
	m_pTransformCom->Set_State(STATE::UP, vUp);
	m_pTransformCom->Set_State(STATE::LOOK, vLook);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vEye, 1.f));
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 159~177행: Update_Input
void CInGameCamera::Update_Input(_float fTimeDelta)
{
	_long MouseMoveX = m_pGameInstance->Get_MouseDelta(RIMM::X);
	_long MouseMoveY = m_pGameInstance->Get_MouseDelta(RIMM::Y);

	// 좌우 회전 각도 -> 마우스 X 이동량 더하기
	m_fYaw += MouseMoveX * m_fMouseSensor;

	// 상하 회전 각도 -> 마우스 Y 이동량 더하기
	m_fPitch += MouseMoveY * m_fMouseSensor;
	// 각도 제한
	m_fPitch = clamp(m_fPitch, m_fPitchMin, m_fPitchMax);

	_vector qYaw = XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fYaw);
	_vector qPitch = XMQuaternionRotationAxis(XMVectorSet(1.f, 0.f, 0.f, 0.f), m_fPitch);
	_vector q = XMQuaternionMultiply(qPitch, qYaw);

	XMStoreFloat4(&m_vQuaternion, q);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 179~210행: Update_Ultimate
void CInGameCamera::Update_Ultimate(_float fTimeDelta)
{
	if (!m_pUltCamClip->Is_Loaded())
	{
		End_Ultimate();
		return;
	}

	m_fScriptTime += fTimeDelta;

	if (m_fScriptTime >= m_fScriptDuration)
	{
		End_Ultimate();
		return;
	}

	CAMERA_ANIM_KEY Key = {};
	m_pUltCamClip->Sample(m_fScriptTime, Key);

	_matrix LocalRot = XMMatrixRotationQuaternion(XMLoadFloat4(&Key.vRotation));
	_matrix LocalTrans = XMMatrixTranslation(Key.vPosition.x, Key.vPosition.y, Key.vPosition.z);

	_matrix LocalCamera = LocalRot * LocalTrans;
	_matrix BaseWorld = XMLoadFloat4x4(&m_ScriptBaseWorld);

	_matrix CameraWorld = LocalCamera * BaseWorld;

	m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSetW(CameraWorld.r[0], 0.f));
	m_pTransformCom->Set_State(STATE::UP, XMVectorSetW(CameraWorld.r[1], 0.f));
	m_pTransformCom->Set_State(STATE::LOOK, XMVectorSetW(CameraWorld.r[2], 0.f));
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(CameraWorld.r[3], 1.f));
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 212~226행: End_Ultimate
void CInGameCamera::End_Ultimate()
{
	m_fScriptTime = 0.f;
	m_fScriptDuration = 0.f;

	if (m_pFollowTarget != nullptr)
	{
		CAMERA_FOCUS_INFO Info = m_pFollowTarget->Get_CameraForcusInfo();
		_vector vTarget = XMLoadFloat3(&Info.vFocusPosition) + XMVectorSet(0.f, m_fHeight, 0.f, 0.f);

		XMStoreFloat4(&m_vLerpPos, vTarget);
	}

	Request_Mode(INGAME_CAMERA_MODE::FOLLOW);
}

// [중략] 원본의 다른 함수 및 선언.
