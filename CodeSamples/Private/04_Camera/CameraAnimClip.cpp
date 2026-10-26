// 포트폴리오용 코드 발췌: 카메라 키프레임의 위치·회전을 시간에 따라 보간
// 원본: Client/Private/CameraAnimClip.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 31~70행: Sample
void CCameraAnimClip::Sample(_float fTime, CAMERA_ANIM_KEY& OutKey) const
{
	if (m_Keys.empty())
		return;

	if (fTime <= 0.f)
	{
		OutKey = m_Keys.front();
		return;
	}

	if (fTime >= m_fDuration)
	{
		OutKey = m_Keys.back();
		return;
	}

	_float fFrame = fTime * m_fSampleRate;

	_uint i0 = static_cast<_uint>(floorf(fFrame));
	_uint i1 = min(i0 + 1, static_cast<_uint>(m_Keys.size() - 1));

	_float fRatio = fFrame - i0;

	const CAMERA_ANIM_KEY& A = m_Keys[i0];
	const CAMERA_ANIM_KEY& B = m_Keys[i1];

	OutKey.fTime = fTime;

	XMStoreFloat3(&OutKey.vPosition, XMVectorLerp(XMLoadFloat3(&A.vPosition), XMLoadFloat3(&B.vPosition), fRatio));
	_vector qA = XMLoadFloat4(&A.vRotation);
	_vector qB = XMLoadFloat4(&B.vRotation);

	_vector q = XMQuaternionSlerp(qA, qB, fRatio);
	q = XMQuaternionNormalize(q);

	XMStoreFloat4(&OutKey.vRotation, q);

	OutKey.fFov = A.fFov + (B.fFov - A.fFov) * fRatio;
}

// [중략] 원본의 다른 함수 및 선언.
