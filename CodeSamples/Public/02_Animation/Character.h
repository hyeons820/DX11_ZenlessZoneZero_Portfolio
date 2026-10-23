#pragma once
#include "Client_Defines.h"
#include "GameObject.h"
#include "HitBoxProvider.h"
#include "Damageable.h"
#include "State.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CNavigation;
class CState;
class CCollider;
NS_END

NS_BEGIN(Client)

struct CHARACTER_SPAWN_DESC : public GAMEOBJECT_DESC
{
	_uint iNavigationLevelIndex = 0;
	_wstring strNavigationPrototypeTag;

	_float3 vSpawnPosition = _float3(0.f, 0.f, 0.f);
	_float3 vSpawnLook = _float3(0.f, 0.f, 1.f);
};

class CCharacter abstract : public CGameObject, public IHitBoxProvider, public IDamageable
{
protected:
	CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCharacter(const CCharacter& Prototype);

#ifdef _DEBUG
public:
	virtual ~CCharacter() = default;
#else
protected:
	virtual ~CCharacter() = default;
#endif

public:
	virtual _bool Should_IgnoreBodyCollision() const { return false; }

public:
	_float Get_CurHP() const { return m_fCurHP; }
	_float Get_MaxHP() const { return m_fMaxHP; }

	_float Get_HPRatio() const
	{
		if (m_fMaxHP <= 0.f)
			return 0.f;

		return max(0.f, min(1.f, m_fCurHP / m_fMaxHP));
	}

	_bool Is_Dead() const { return m_bDead; }


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(const GAMEOBJECT_DESC* pDesc);
	virtual void	Priority_Update(_float fTimeDelta);
	virtual void	Update(_float fTimeDelta);
	virtual void	Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	virtual	HRESULT Intersect_Ray(_fvector vRayOriginWorld, _fvector vRayDirWorld, _float& fOutDistance);

	virtual	 _bool		Get_HitBoxInfo(_uint iAttackID, _uint iHitBoxID, HITBOX_ACTIVE_DESC& OutDesc) const override;
	virtual  _bool		Get_WarningBoxInfo(_uint iAttackID, _uint iHitBoxID, HITBOX_ACTIVE_DESC& OutDesc) const;

	virtual  CAMERA_FOCUS_INFO  Get_CameraForcusInfo() const;
	
protected:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CNavigation* m_pNavigationCom = { nullptr };
	CState* m_pStateCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };


protected:
	_float m_fMaxHP = {};
	_float m_fCurHP = {};

	_bool  m_bDead = { false };

	_float3 m_vMoveDir = {};		// 현재 이동 입력 방향
	_float3 m_vVelocity = {};		// 현재 이동 속도 벡터

	_float	m_fTargetSpeed = {};	// 현재 이동 목표 속도
	_float	m_fCurrentSpeed = {};	// 현재 이동 실제 속도

	_float	m_fWalkSpeed = { 6.f };
	_float	m_fRunSpeed = { 9.f };

	_float  m_fAcceleration = { 35.f };		// 목표 속도까지 증가하는 가속도
	_float  m_fDeceleration = { 45.f };		// 목표 속도까지 감소하는 감속도

	_float m_fRotationSpeed = { XMConvertToRadians(1080.f) };

	MOVE_SUBSTATE m_eRequestedMoveState = {};	// 외부 컨트롤러가 요청한 이동 상태

	_bool m_bWasAttackActionState = { false };
	_bool m_bRotateToMoveDirection = { true };
	_uint m_iPrevAttackActionID = {};

protected:
	_uint    m_iNavigationLevelIndex = 0;
	_wstring m_strNavigationPrototypeTag;

protected:
	virtual  HRESULT	Ready_Components();
	virtual  void		Apply_StateToAnimation();

	virtual	 _bool		Get_CurrentHitBoxDescs(vector<HITBOX_ACTIVE_DESC>& OutDescs) const;
	virtual	 _bool		Get_CurrentWarningBoxDescs(vector<HITBOX_ACTIVE_DESC>& OutDescs) const;
	
	virtual	 _bool		Should_UseRootMotion(const STATE_DESC& StateDesc) const;
	virtual _vector		Filter_RootMotionDelta(_fvector vDeltaWorld);
	
	HRESULT Bind_ShaderResources();

	// 애니메이션 루트모션 이동량을 월드 이동으로 적용
	void	Apply_RootMotion();
	// 목표 속도에 맞춰 현재 속도와 velocity 계산
	void	Update_MoveVelocity(_float fTimeDelta);

protected:
	// 이동 방향과 이동 상태를 입력받아 목표 속도 설정
	void	Set_MoveInput(_fvector vMoveDir, MOVE_SUBSTATE eMoveState);
	// 이동 입력, 목표 속도, velocity 초기화
	void	Clear_MoveDirection();
	// 현재 유효한 이동 방향이 있는지 확인
	_bool	Has_MoveDirection() const;

	// 현재 velocity를 이용해 네비게이션 위에서 실제 이동
	void	Apply_Move(_float fTimeDelta);
	// 지정 방향을 즉시 바라보도록 Transform 회전
	void	Look_MoveDirection(_fvector vMoveDir);

	_bool	Is_AttackActionState() const;
	void	Update_AttackEndEvent();
	void	Apply_SpawnPosition(_fvector vSpawnPosition);
public:
	virtual CGameObject* Clone(const GAMEOBJECT_DESC* pDesc) = 0;
	virtual void Free();
};

NS_END