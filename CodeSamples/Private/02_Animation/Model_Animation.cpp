// 포트폴리오용 코드 발췌: 애니메이션 전환·보간과 루트 이동량 추출
// 원본: Engine/Private/Model.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.
// 읽는 흐름: 애니메이션 변경 요청 → 재생·블렌딩 → 루트 이동 추출 → 계층 행렬·최종 본 행렬 반영. 오버레이와 개별 채널의 키프레임 샘플링은 중략했습니다.

// 원본 85~103행: Get_CurrentAnimSyncInfo
HRESULT CModel::Get_CurrentAnimSyncInfo(ANIM_SYNC_INFO& OutInfo) const
{
	OutInfo = {};

	if (m_Animations.empty() || m_iCurrentAnimationIndex >= m_Animations.size())
		return E_FAIL;

	CAnimation* pAnim = m_Animations[m_iCurrentAnimationIndex];
	if (pAnim == nullptr)
		return E_FAIL;

	OutInfo.iAnimationIndex = m_iCurrentAnimationIndex;
	OutInfo.fTrackPosition = pAnim->Get_CurrentTrackPosition();
	OutInfo.fDuration = pAnim->Get_Duration();
	OutInfo.fNormalizedTime = pAnim->Get_NormalizedTime();
	OutInfo.bBlending = m_bBlending;

	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 445~505행: Change_Animation
HRESULT CModel::Change_Animation(_uint iNextIndex, _double fBlendTime, _bool bLoop)
{
	if (iNextIndex >= m_Animations.size())
		return E_FAIL;

	// 이미 블렌딩 중이라면
	if (m_bBlending)
	{
		// 같은 애니로 바꾸려고 할 때
		if (iNextIndex == m_iNextAnimationIndex)
		{
			m_bLoop = bLoop;
			return S_OK;
		}

		// 블렌딩 중인데 다시 현재 애니메이션으로 돌아가려 할 때
		// Idle -> Start 블렌딩 중 Idle
		if (iNextIndex == m_iCurrentAnimationIndex)
		{
			m_bBlending = false;

			m_iNextAnimationIndex = m_iCurrentAnimationIndex;

			m_bLoop = bLoop;
			m_bNextLoop = bLoop;

			m_fBlendDuration = 0.f;
			m_fBlendAccTime = 0.f;

			return S_OK;
		}
	}
	else
	{
		if (iNextIndex == m_iCurrentAnimationIndex)
		{
			m_bLoop = bLoop;
			return S_OK;
		}
	}
	

	m_iNextAnimationIndex = iNextIndex;
	m_bNextLoop = bLoop;
	m_fBlendDuration = max(0.f, fBlendTime);
	m_fBlendAccTime = 0.0f;
	m_bBlending = (m_fBlendDuration > 0.f);

	m_Animations[m_iNextAnimationIndex]->Reset_TrackPosition();
	m_bRootMotionInitialized = false;
	m_vPrevRootMotions = _float3(0.f, 0.f, 0.f);
	m_vRootMotionDeltaLocal = _float3(0.f, 0.f, 0.f);

	if (!m_bBlending)
	{
		m_iCurrentAnimationIndex = iNextIndex;
		m_bLoop = bLoop;
	}

	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 381~443행: Play_Animation
ANIMATION_UPDATE_RESULT CModel::Play_Animation(_float fTimeDelta)
{
	ANIMATION_UPDATE_RESULT Result = {};

	if (m_Animations.empty() || m_iCurrentAnimationIndex >= m_Animations.size())
		return Result;

	if (FAILED(Reset_NodeLocalMatrices()))
		return Result;

	_bool bLooped = false;
	_bool bFinished = false;

	if (m_bBlending == false)
	{
		bFinished = m_Animations[m_iCurrentAnimationIndex]->Update_TransformationMatrices(m_NodeLocalMatrices, fTimeDelta * m_fAnimationSpeed, m_bLoop, &bLooped);
	}
	else
	{
		m_BlendSourNodeLocalMatrices = m_NodeLocalMatrices;
		m_Animations[m_iCurrentAnimationIndex]->Update_TransformationMatrices(m_BlendSourNodeLocalMatrices, fTimeDelta * m_fAnimationSpeed, m_bLoop, &bLooped);

		m_BlendDestNodeLocalMatrices = m_NodeLocalMatrices;
		m_Animations[m_iNextAnimationIndex]->Update_TransformationMatrices(m_BlendDestNodeLocalMatrices, fTimeDelta * m_fAnimationSpeed, m_bNextLoop, &bLooped);

		m_fBlendAccTime += fTimeDelta;
		_float fWeight = (_float)min(1.f, m_fBlendAccTime / m_fBlendDuration);

		if (FAILED(Blend_NodeLocalMatrices(m_BlendSourNodeLocalMatrices, m_BlendDestNodeLocalMatrices, fWeight, m_NodeLocalMatrices)))
			return Result;

		if (fWeight >= 1.f)
		{
			m_bBlending = false;
			m_iCurrentAnimationIndex = m_iNextAnimationIndex;
			m_bLoop = m_bNextLoop;
		}
	}

	if (bLooped)
	{
		m_bRootMotionInitialized = false;
		m_vPrevRootMotions = _float3(0.f, 0.f, 0.f);
		m_vRootMotionDeltaLocal = _float3(0.f, 0.f, 0.f);
	}

	if (FAILED(Update_OverlayAnimation(fTimeDelta)))
		return Result;

	if (FAILED(Consume_RootMotion()))
		return Result;

	if (FAILED(Update_NodeCombinedMatrices()))
		return Result;

	if (FAILED(Update_FinalBoneMatrices()))
		return Result;

	Result.bFinished = bFinished;
	Result.bLooped = bLooped;

	return Result;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 664~675행: Reset_NodeLocalMatrices
HRESULT CModel::Reset_NodeLocalMatrices()
{
	if (m_NodeLocalMatrices.size() != m_Nodes.size())
		m_NodeLocalMatrices.resize(m_Nodes.size());

	for (size_t i = 0; i < m_Nodes.size(); i++)
	{
		memcpy(&m_NodeLocalMatrices[i], &m_Nodes[i].fLocalMatrix, sizeof(_float4x4));
	}

	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 508~537행: Blend_NodeLocalMatrices
HRESULT CModel::Blend_NodeLocalMatrices(const vector<_float4x4>& SourMat, const vector<_float4x4>& DestMat, _float fWeight, vector<_float4x4>& OutMat)
{
	if (SourMat.size() != DestMat.size())
		return E_FAIL;

	if (OutMat.size() != DestMat.size())
		return E_FAIL;

	for (size_t i = 0; i < SourMat.size(); i++)
	{
		_vector vSourScale, vSourRot, vSourPos;
		_vector vDestScale, vDestRot, vDestPos;

		_matrix Sour = XMLoadFloat4x4(&SourMat[i]);
		_matrix Dest = XMLoadFloat4x4(&DestMat[i]);

		XMMatrixDecompose(&vSourScale, &vSourRot, &vSourPos, Sour);
		XMMatrixDecompose(&vDestScale, &vDestRot, &vDestPos, Dest);

		_vector vBlendScale = XMVectorLerp(vSourScale, vDestScale, fWeight);
		_vector vBlendRot = XMQuaternionSlerp(vSourRot, vDestRot, fWeight);
		_vector vBlendPos = XMVectorLerp(vSourPos, vDestPos, fWeight);

		_matrix Blend = XMMatrixAffineTransformation(vBlendScale, XMVectorZero(), vBlendRot, vBlendPos);

		XMStoreFloat4x4(&OutMat[i], Blend);
	}

	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 605~661행: Consume_RootMotion
HRESULT CModel::Consume_RootMotion()
{
	// 이번 프레임의 이동량 초기화
	m_vRootMotionDeltaLocal = _float3(0.f, 0.f, 0.f);

	if (m_iRootNodeIndex >= m_Nodes.size() || m_iRootNodeIndex < 0)
		return E_FAIL;

	_vector vScale = {};
	_vector vRotation = {};
	_vector vTranslation = {};

	// 루트 노드 행렬
	_matrix RootLocal = XMLoadFloat4x4(&m_NodeLocalMatrices[m_iRootNodeIndex]);

	if (!XMMatrixDecompose(&vScale, &vRotation, &vTranslation, RootLocal))
		return E_FAIL;

	_float3 vCurrentPos = {};
	XMStoreFloat3(&vCurrentPos, vTranslation);

	if (m_bUseRootMotion)
	{
		if (!m_bRootMotionInitialized)
		{
			// 첫 프레임이면 현재 위치 저장만 하고 통과
			m_vPrevRootMotions = vCurrentPos;

			m_bRootMotionInitialized = true;
		}
		else
		{
			// (현재 애니메이션 위치 - 이전 프레임 애니메이션 위치) = 실제 이동해야 할 거리
			m_vRootMotionDeltaLocal.x = vCurrentPos.x - m_vPrevRootMotions.x;
			m_vRootMotionDeltaLocal.y = 0.f; // Y값은 0
			m_vRootMotionDeltaLocal.z = vCurrentPos.z - m_vPrevRootMotions.z;
			
			// 현재 위치 저장
			m_vPrevRootMotions = vCurrentPos;
		}
	}
	else
	{
		m_bRootMotionInitialized = false;
		m_vPrevRootMotions = _float3(0.f, 0.f, 0.f);
	}


	// Y값만 남긴 새로운 위치 벡터 생성
	_vector vStrippedTranslation = XMVectorSet(0.f, XMVectorGetY(vTranslation), 0.f, 1.f);
	// 크기, 회전은 그대로 유지하고 위치만 XZ가 제거된 상태로 행렬 재조립
	_matrix StrippedLocal = XMMatrixAffineTransformation(vScale, XMVectorZero(), vRotation, vStrippedTranslation);
	// 재조립된 행렬을 루트 노드에 다시 저장
	XMStoreFloat4x4(&m_NodeLocalMatrices[m_iRootNodeIndex], StrippedLocal);

	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 678~706행: Update_NodeCombinedMatrices
HRESULT CModel::Update_NodeCombinedMatrices()
{
	if (m_NodeCombinedMatrices.size() != m_Nodes.size())
		m_NodeCombinedMatrices.resize(m_Nodes.size());

	for (size_t i = 0; i < m_Nodes.size(); i++)
	{
		_matrix Local = XMLoadFloat4x4(&m_NodeLocalMatrices[i]);
		_matrix Combined = XMMatrixIdentity();

		_int iParentIndex = m_Nodes[i].iParentIndex;

		// 부모가 없으면 루트
		if (iParentIndex == -1)
		{
			 // 루트는 m_PreTransformMatrix를 포함해서 combined 계산
			Combined = XMLoadFloat4x4(&m_PreTransformMatrix) * Local;
		}
		else
		{
			// 자식은 내 local * 부모 combined
			Combined = Local * XMLoadFloat4x4(&m_NodeCombinedMatrices[iParentIndex]);
		}

		XMStoreFloat4x4(&m_NodeCombinedMatrices[i], Combined);
	}

	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 709~733행: Update_FinalBoneMatrices
HRESULT CModel::Update_FinalBoneMatrices()
{
	if (m_FinalBoneMatrices.size() != m_Bones.size())
		m_FinalBoneMatrices.resize(m_Bones.size());

	for (size_t i = 0; i < m_Bones.size(); i++)
	{
		const BONE_INFO& Bone = m_Bones[i];

		if (Bone.iNodeIndex < 0 || Bone.iNodeIndex >= m_NodeCombinedMatrices.size())
			return E_FAIL;

		// 메시 정점을 bone 기준으로 바꾸는 행렬
		_matrix Offset = XMLoadFloat4x4(reinterpret_cast<const _float4x4*>(Bone.fOffsetMatrix));

		// 현재 애니메이션으로 움직인 뼈대
		_matrix Combined = XMLoadFloat4x4(&m_NodeCombinedMatrices[Bone.iNodeIndex]);

		// 셰이더가 실제 스키닝에 쓸 행렬
		_matrix Final = Offset * Combined;
		XMStoreFloat4x4(&m_FinalBoneMatrices[i], Final);
	}

	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.
