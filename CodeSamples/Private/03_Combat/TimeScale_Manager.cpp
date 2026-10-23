// 포트폴리오용 코드 발췌: 플레이어·몬스터 등 시간 레이어별 배율과 지속 시간 관리
// 원본: Engine/Private/TimeScale_Manager.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 3~23행: Update
void CTimeScale_Manager::Update(_float fTimeDelta)
{
    for (_uint i = 0; i < ETOUI(TIME_LAYER::END); ++i)
    {
        auto& Desc = m_TimeScales[i];

        if (Desc.fRemainTime <= 0.f)
        {
            Desc.fScale = 1.f;
            continue;
        }

        Desc.fRemainTime -= fTimeDelta;

        if (Desc.fRemainTime <= 0.f)
        {
            Desc.fRemainTime = 0.f;
            Desc.fScale = 1.f;
        }
    }
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 25~31행: Set_TimeScale
void CTimeScale_Manager::Set_TimeScale(TIME_LAYER eLayer, _float fScale, _float fDuration)
{
    auto& Desc = m_TimeScales[ETOUI(eLayer)];

    Desc.fScale = fScale;
    Desc.fRemainTime = max(Desc.fRemainTime, fDuration);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 33~36행: Get_ScaledDelta
_float CTimeScale_Manager::Get_ScaledDelta(TIME_LAYER eLayer, _float fTimeDelta) const
{
    return fTimeDelta * m_TimeScales[ETOUI(eLayer)].fScale;
}

// [중략] 원본의 다른 함수 및 선언.
