#pragma once
#include "Client_Defines.h"
#include "Camera.h"

NS_BEGIN(Client)

enum INGAME_CAMERA_MODE
{
	FOLLOW,
	LOCK_ON,
	ULTIMATE,
	SCRIPTED,
	OPNEING,
	WIPEOUT
};

struct INGAMECAMERA_DESC final : public CAMERA_DESC
{
	INGAME_CAMERA_MODE	eMode = INGAME_CAMERA_MODE::FOLLOW;
	class CCharacter* pFollowTarget = { nullptr };
	CTransform* pLookTarget = { nullptr };
	_float fHeight;
	_float fDistance;
	_float fMouseSensor;
};

class CInGameCamera final : public CCamera
{
private:
	CInGameCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CInGameCamera(const CInGameCamera& Prototype);
	virtual ~CInGameCamera() = default;

public:
	void	Request_Mode(INGAME_CAMERA_MODE eMode);
	void	Request_Follow(class CCharacter* pTarget);
	void	Request_LockOn(CTransform* pPlayer, CTransform* pEnemy);
	void	Request_Ultimate(CTransform* pActor);

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(const GAMEOBJECT_DESC* pArg) override;
	virtual void			Priority_Update(_float fTimeDelta) override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;


private:
	INGAME_CAMERA_MODE	m_eMode = INGAME_CAMERA_MODE::FOLLOW;

	class CCharacter*	m_pFollowTarget = { nullptr };
	CTransform*			m_pLookTarget = { nullptr };

	_float				m_fYaw = 0.f;
	_float				m_fPitch = XMConvertToRadians(15.f);
	_float				m_fPitchMin = XMConvertToRadians(-20.f);
	_float				m_fPitchMax = XMConvertToRadians(60.f);
	_float				m_fDistance = {};
	_float				m_fHeight = {};
	_float				m_fMouseSensor = {};
	_float				m_fModeTime = {};

	_float4				m_vQuaternion = {};
	_float4				m_vLerpPos = {};

	_float				m_fFollowLerpSpeed = 12.f;

private:
	_float4x4 m_ScriptBaseWorld = {};
	_float    m_fScriptTime = {};
	_float    m_fScriptDuration = {};

	class CCameraAnimClip* m_pUltCamClip = { nullptr };

private:
	void	Update_Follow(_float fTimeDelta);
	void	Update_Input(_float fTimeDelta);

	void	Update_Ultimate(_float fTimeDelta);
	void	End_Ultimate();

public:
	static	CInGameCamera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(const GAMEOBJECT_DESC* pArg) override;
	virtual void			Free() override;
};

NS_END