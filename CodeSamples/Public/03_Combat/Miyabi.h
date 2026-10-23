#pragma once
#include "Client_Defines.h"
#include "Character.h"
#include "DefensiveResponder.h"

NS_BEGIN(Client)

class CMiyabi final : public CCharacter, public IDefensiveResponder
{
private:
	struct ACTION_INPUT_CONTEXT
	{
		_bool bHasMoveInput = { false };			// 이번 프레임에 이동 입력이 있는지

		_bool bEvadePressed = { false };			// 이번 프레임에 회피 버튼을 눌렀는지
		_bool bEvadeHeld = { false };				// 현재 회피 버튼을 누르고 있는지
		_bool bEvadeReleased = { false };			// 이번 프레임에 회피 버튼을 뗐는지

		_bool bAttackPressed = { false };			// 이번 프레임에 공격 버튼을 눌렀는지
		_bool bAttackHeld = { false };
		_bool bAttackReleased = { false };

		_bool bSpecialPressed = { false };

		_bool bUltimatePressed = { false };

		_bool bParryPressed = { false };


		_float3 vMoveDir = {};						// 카메라 기준으로 계산된 월드 이동 방향
	};

	enum class CAMERA_FOCUS_MODE
	{
		DEFAULT,
		LERP,
		FIXED,
	};

	enum class MIYABI_ACTION
	{
		NONE = 0,

		EVADE_BACK_DEFAULT,
		EVADE_FRONT,
		EVADE_FRONT_LEFT,
		EVADE_FRONT_RIGHT,
		EVADE_BACK,
		EVADE_BACK_LEFT,
		EVADE_BACK_RIGHT,
		EVADE_STRONG,

		NORMAL_ATTACK_1,
		NORMAL_ATTACK_2,
		NORMAL_ATTACK_3,
		NORMAL_ATTACK_4,
		NORMAL_ATTACK_5,

		SPECIAL_ATTACK_1,
		SPECIAL_ATTACK_2,

		CHARGE_READY_1,

		CHARGE_ATTACK_1,
		CHARGE_ATTACK_2,
		CHARGE_ATTACK_3,

		CHARGE_END_1,
		CHARGE_END_2,
		CHARGE_END_3,

		ULTIMATE,

		PARRY_L,
		PARRY_H,
		COUNTER_ATTACK,
	};

private:
	CMiyabi(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMiyabi(const CMiyabi& Prototype);
	virtual ~CMiyabi() = default;

public:
	virtual _bool	Get_CurrentHitBoxDescs(vector<HITBOX_ACTIVE_DESC>& OutDescs) const override;

	virtual _bool	Try_JustEvade(const DAMAGE_DESC& DamageDesc) override;
	virtual _bool	Try_Parry(const DAMAGE_DESC& DamageDesc) override;
	virtual _bool	Try_ParryImpact(const DAMAGE_DESC& DamageDesc) override;
	virtual _bool	Should_IgnoreBodyCollision() const override
	{
		if (m_pStateCom == nullptr)
			return false;

		return m_pStateCom->Get_MainState() == MAIN_STATE::EVADE;
	}

public:
	_float Get_SPRatio() const
	{
		if (m_fMaxSpecialGauge <= 0.f)
			return 0.f;

		return max(0.f, min(1.f, m_fCurSpecialGauge / m_fMaxSpecialGauge));
	}

	void Add_SpecialGauge(_float fAmount)
	{
		m_fCurSpecialGauge += fAmount;

		if (m_fCurSpecialGauge > m_fMaxSpecialGauge)
			m_fCurSpecialGauge = m_fMaxSpecialGauge;
	}

public:
	_float Get_DecibelRatio() const
	{
		if (m_fMaxDecibel <= 0.f)
			return 0.f;

		return max(0.f, min(1.f, m_fCurDecibel / m_fMaxDecibel));
	}

	_bool Can_UseSpecialUI() const
	{
		return m_fCurSpecialGauge >= m_fSpecialCost;
	}

	_bool Can_UseUltimateUI() const
	{
		return m_fCurDecibel >= m_fUltimateCost;
	}

	_uint Get_CurEvadeStack() const
	{
		return m_iCurEvadeStack;
	}

	_uint Get_MaxEvadeStack() const
	{
		return m_iMaxEvadeStack;
	}

	_float Get_EvadeRechargeRatio() const
	{
		if (m_fMaxEvadeRechargeTime <= 0.f)
			return 1.f;

		return max(0.f, min(1.f, m_fCurEvadeRechargeTime / m_fMaxEvadeRechargeTime));
	}

	_bool Is_ParryAssistActiveUI() const
	{
		if (m_pStateCom == nullptr)
			return false;

		const MAIN_STATE eMain = m_pStateCom->Get_MainState();

		return eMain == MAIN_STATE::PARRY || eMain == MAIN_STATE::COUNTER;
	}

	_bool Is_EvadingUI() const
	{
		if (m_pStateCom == nullptr)
			return false;

		return m_pStateCom->Get_MainState() == MAIN_STATE::EVADE;
	}

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(const GAMEOBJECT_DESC* pDesc) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render(RENDER_PASS ePass) override;

	virtual CAMERA_FOCUS_INFO Get_CameraForcusInfo() const override;

private:
	vector<CCollider*> m_pHitBoxSpheres;
	CCollider* m_pDodgeColliderCom = { nullptr };
	CCollider* m_pParryColliderCom = { nullptr };

	ACTION_INPUT_CONTEXT m_InputContext = {};						// 이번 프레임의 입력 해석 결과

	_float m_fWalkMaintainTime = {};								// 걷기를 유지한 누적 시간
	_bool  m_bForceRunAfterDirectionalEvade = { false };			// 방향성 회피 후 이동 입력 유지 시 Run으로 복귀시키는 플래그

#pragma region 회피
	_uint  m_iCurEvadeStack = { 2 };								// 현재 사용 가능한 회피 스택 수
	_uint  m_iMaxEvadeStack = { 2 };								// 최대 회피 스택 수

	_float m_fCurEvadeRechargeTime = {};							// 회피 스택 회복 누적 시간
	_float m_fMaxEvadeRechargeTime = { 0.5f };						// 회피 스택 1개 회복에 필요한 시간

	_bool  m_bEvadeButtonHolding = { false };						// 회피 버튼 홀드 판정 진행 중인지
	_bool  m_bEvadeHoldConsumed = { false };						// 홀드 회피가 이미 실행되었는지

	_float m_fEvadeHoldTime = {};									// 회피 버튼을 누르고 있던 시간
	_float m_fEvadeHoldThreshold = { 0.14f };						// 탭/홀드 회피를 구분하는 기준 시간

	_bool m_bJustEvadeTriggered = { false };

	_float m_fJustEvadeStartRatio = { 0.12f };
	_float m_fJustEvadeEndRatio = { 0.55f };
#pragma endregion


#pragma region 공격
	_bool	m_bAttackBuffered = { false };							// 다음 기본 공격 입력이 예약되어 있는지
	_float  m_fCurAttackBufferTime = {};							// 공격 입력 버퍼 남은 시간
	_float  m_fMaxAttackBufferTime = { 0.25f };						// 공격 입력을 저장해둘 시간

	CGameObject* m_pCombatTarget = { nullptr };

	_bool  m_bPierceAttack = { false };
	_float m_fNonPierceStopDistance = { 1.35f };
	_float m_fAttackTargetSearchRange = { 6.f };
#pragma endregion


#pragma region 특수기
	_float m_fCurSpecialGauge = { 100.f };
	_float m_fMaxSpecialGauge = { 100.f };
	_float m_fSpecialCost = { 30.f };
	_float m_fSpecialRegenPerSec = { 0.5f };
#pragma endregion


#pragma region 궁극기
	_float	m_fCurDecibel = {};
	_float	m_fMaxDecibel = { 3000.f };
	_float	m_fUltimateCost = { 3000.f };
	_bool   m_bUltimateHitAreaReady = { false };
	_float3 m_vUltimateHitCenter = {};
	_float  m_fUltimateHitRadius = { 2.f };
#pragma endregion


#pragma region 차지 공격
	_uint  m_iCurChargeStack = {};
	_uint  m_iMaxChargeStack = { 6 };
	_uint  m_iChargeCost = { 2 };

	_bool  m_bChargeHolding = { false };			// 공격 버튼을 누르고 있는 차지 흐름 중인지

	_float m_fCurChargeTime = {};					// Ready 이전 내부 차지 시간
	_float m_fChargeThreshold = { 0.2f };			// Ready 진입 조건 시간

	_uint  m_iCurChargeHitIndex = {};				// 현재 몇 번째 차지 공격까지 사용했는지
	_uint  m_iMaxChargeHitIndex = { 3 };			// 최대 3타
	_uint  m_iPendingChargeHitIndex = {};

	_bool   m_bNeedFix = false;
	_bool   m_bChargeEnd3LookFixed = false;
	_float3 m_vChargeEnd3FinalLook = {};

	_bool   m_bChargeHitAreaReady = { false };
	_float3 m_vChargeHitCenter = {};
	_float  m_fChargeHitRadius = { 1.f };
#pragma endregion


#pragma region 카메라
	CAMERA_FOCUS_MODE m_eCameraFocusMode = CAMERA_FOCUS_MODE::DEFAULT;

	_float3 m_vCameraFocusStart = {};
	_float3 m_vCameraFocusEnd = {};	
	_float3 m_vCameraFocus = {};

	_float  m_fCameraFocusTime = {};
	_float  m_fCameraFocusDuration = {};
	_float  m_fCameraFollowSpeed = { 12.f };
#pragma endregion


#pragma region 피격
	HIT_REACTION_TYPE m_eLastHitReactionType = HIT_REACTION_TYPE::LIGHT;
#pragma endregion


#pragma region 패링
	_bool  m_bParryWarningBoxActive = { false };
	_float m_fParryWarningBoxTime = {};

	_bool  m_bParryImpactBoxActive = { false };
	_float m_fParryImpactBoxTime = {};

	_float m_fParryAssistRange = { 6.f };
	_float m_fParryTargetDistance = { 2.0f };

	_bool m_bParryChainActive = { false };
	_int m_iParryChainAttackID = {};
	CGameObject* m_pParryChainTarget = { nullptr };
#pragma endregion


#pragma region 반격
	_bool m_bCounterBuffered = { false };

	_bool m_bCounterReady = { false };

	_float m_fCounterReadyTime = {};
	_float m_fCounterReadyDuration = { 1.f };
	_float m_fCounterRushDistanceScale = { 1.f };

	HIT_REACTION_TYPE m_eLastParryReactionType = HIT_REACTION_TYPE::LIGHT;
#pragma endregion



private:
	HRESULT Render_Default();
	HRESULT Render_EyeShadow();

private:
	virtual HRESULT Ready_Components() override;
	virtual void	Apply_StateToAnimation() override;
	virtual void	Take_Damage(const DAMAGE_DESC& DamageDesc) override;
	virtual _vector Filter_RootMotionDelta(_fvector vDeltaWorld) override;

	CGameObject*	Find_NearestMonster(_float fRange) const;

	// 회피 스택 충전 등 액션 타이머 갱신
	void	Update_ActionTimers(_float fTimeDelta);
	// 키보드/마우스 입력을 이동/회피/공격 컨텍스트로 변환
	void	Update_InputContext();
	void	Update_ActionSyncInfo();

#pragma region HANDLE
	//이번 프레임 입력을 보고 즉시 실행할 수 있으면 실행, 타이밍을 기다려야 하면 버퍼에 저장
	
	// 입력 컨텍스트 갱신 후 회피/공격/이동 입력 처리
	void	Handle_StateInput(_float fTimeDelta);
	// 회피 버튼 입력을 탭/홀드 회피로 해석하고 회피 요청
	_bool	Handle_EvadeInput(_float fTimeDelta);
	// 기본 공격 입력 처리
	_bool	Handle_AttackInput();
	// 마우스 홀드 상태만 관리
	_bool	Handle_ChargeInput(_float fTimeDelta);
	// 이동 입력에 따라 걷기/달리기 이동 요청 설정
	_bool	Handle_MoveInput(_float fTimeDelta);
	// 특수 공격 입력 처리
	_bool	Handle_SpecialInput();
	// 궁극기 입력 처리
	_bool	Handle_UltimateInput();
	// 패링 입력 처리
	_bool	Handle_ParryInput();
#pragma endregion


#pragma region UPDATE_BUFFER
	// 이전에 저장된 입력을 매 프레임 검사해서,	실행 가능한 타이밍이 오면 실제 액션을 시작
	// 기본 공격 
	void	Update_AttackBuffer(_float fTimeDelta);
	_bool	Start_NormalAttack(ATTACK_SUBSTATE eAttackState);

	void	Setup_AttackRootTarget(_bool bPierceAttack);
	void	Clear_AttackRootTarget();
	_bool	Is_PierceAttack(MIYABI_ACTION eAction) const;

	// 특수기
	void	Update_SpecialGauge(_float fTimeDelta);
	_bool   Can_ChainSpecial() const;
	_bool   Can_StartSpecial() const;
	_bool   Start_SpecialAttack(MIYABI_ACTION eAction);

	// 차지 공격
	void	Update_ChargeFlow(_float fTimeDelta);
	void	Update_ChargeReadyFlow();	
	// 조건 보고 다시 Ready로 갈지 End로 갈지 판단하고 전환
	_bool	Start_ChargeReady(_uint iChargeHitIndex);
	void	Update_ChargeAttackFlow();
	_float	Get_ChargeFinishTime() const;
	_bool	Can_StartChargeReady() const;
	_bool	Start_ChargeAttack(_uint iChargeHitIndex);
	_bool	Start_ChargeEnd();
	void	Fix_TransformAfterChargeEnd3();

	// 궁극기
	_bool   Can_StartUltimate() const;
	_bool   Start_Ultimate();

	// 패링
	_bool	Start_Parry(CGameObject* pTarget, HIT_REACTION_TYPE eReactionType);
	void	Start_ParryWarningBox();
	void	Start_ParryImpactBox();

	// 반격
	_bool	Start_CounterAttack();

	void	Ready_HitArea(_fvector vLook, _float3& vOut, _bool& bFlag, _float fDistance);

	// 이동 입력 시간과 회피 후 플래그를 기준으로 Walk/Run 결정
	MOVE_SUBSTATE   Resolve_MoveSubState(_float fTimeDelta);
	// 회피 스택을 검사하고 State에 회피 요청을 보냄
	_bool			Try_StartEvade(MIYABI_ACTION eEvadeAction);
	void			Start_JustEvade(const DAMAGE_DESC& DamageDesc);

	// 입력 방향과 현재 Look 기준으로 회피 액션 종류 결정
	MIYABI_ACTION   Resolve_MiyabiDirEvadeAction();
	_vector			Resolve_AttackDir();

	ATTACK_SUBSTATE Get_NextCombo(ATTACK_SUBSTATE eAttackState) const;
	MIYABI_ACTION	Convert_AttackToAction(ATTACK_SUBSTATE eAttackState) const;

	// 카메라 조절
	void	Begin_AttackCameraFocus(_fvector vStartPos, _fvector vAttackDir, _float fDistance, _float fDuration, _float fFollowSpeed);
	_float  Compute_NonPierceFocusDistance(_fvector vAttackDir, _float fDesiredDistance) const;
	void	Update_CameraFocus(_float fTimeDelta);
	void	Reset_CameraFocus();

	// 피격
	_float	Get_HitFinishTime(HIT_REACTION_TYPE eReactionType) const;
#pragma endregion

public:
	static CMiyabi* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(const GAMEOBJECT_DESC* pDesc) override;
	virtual void Free();
};

NS_END