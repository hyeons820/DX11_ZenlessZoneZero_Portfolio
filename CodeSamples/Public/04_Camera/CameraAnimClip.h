#pragma once
#include "Client_Defines.h"

NS_BEGIN(Client)

struct CAMERA_ANIM_KEY
{
	_float  fTime = {};
	_float3 vPosition = {};
	_float4 vRotation = { 0.f, 0.f, 0.f, 1.f };
	_float  fFov = XMConvertToRadians(60.f);
};

class CCameraAnimClip final : public CBase
{
private:
	struct CAMERA_ANIM_HEADER
	{
		_uint  iMagic = {};
		_uint  iVersion = {};
		_float fDuration = {};
		_float fSamplerate = {};
		_uint  iNumKeys = {};
	};

private:
	CCameraAnimClip() = default;
	virtual ~CCameraAnimClip() = default;

public:
	_float Get_Duration() const { return m_fDuration; }
	_bool  Is_Loaded() const { return !m_Keys.empty(); }

public:
	HRESULT Load_FromBin(const char* pPath);
	void	Sample(_float fTime, CAMERA_ANIM_KEY& OutKey) const;

private:
	_float m_fDuration = {};
	_float m_fSampleRate = 60.f;
	vector<CAMERA_ANIM_KEY> m_Keys;

public:
	static CCameraAnimClip* Create(const char* pPath);
	virtual void Free() override;
};

NS_END