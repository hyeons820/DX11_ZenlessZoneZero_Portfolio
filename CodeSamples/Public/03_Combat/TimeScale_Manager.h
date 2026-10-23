#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTimeScale_Manager final : public CBase
{
private:
    CTimeScale_Manager() = default;
    virtual ~CTimeScale_Manager() = default;


public:
    void Update(_float fTimeDelta);

    void Set_TimeScale(TIME_LAYER eLayer, _float fScale, _float fDuration);
    _float Get_ScaledDelta(TIME_LAYER eLayer, _float fTimeDelta) const;

private:
    struct TIME_SCALE_DESC
    {
        _float fScale = { 1.f };
        _float fRemainTime = {};
    };

    TIME_SCALE_DESC m_TimeScales[ETOUI(TIME_LAYER::END)];

public:
    static CTimeScale_Manager* Create();
    virtual void Free() override;
};

NS_END