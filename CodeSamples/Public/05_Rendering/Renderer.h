#pragma once
#include "Base.h"
#include "GameObject.h"

NS_BEGIN(Engine)

struct RENDER_OBJECT
{
	CGameObject* pGameObject = nullptr;
	RENDER_PASS ePass = RENDER_PASS::DEFAULT;
};

class CRenderer final : public CBase
{
private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;

public:
	HRESULT Initialize();
	void	Add_RenderGroup(RENDERID eGroupID, class CGameObject* pGameObject, RENDER_PASS ePass);
	HRESULT Draw();

#ifdef _DEBUG
	void Set_Debug() { m_bDebug == false ? m_bDebug = true : m_bDebug = false; }
	void Add_DebugComponent(class CComponent* pComponent);
#endif

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

private:
	list<RENDER_OBJECT>			m_RenderObjects[ETOUI(RENDERID::END)];
	class CGameInstance*		m_pGameInstance = { nullptr };

	class CShader*				m_pShader = { nullptr };
	class CVIBuffer_Rect*		m_pVIBuffer = { nullptr };
	_float4x4					m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};

#ifdef _DEBUG
	_bool						m_bDebug = { true };
	list<class CComponent*>		m_DebugComponents;
#endif

private:
	HRESULT Render_Priority();
	HRESULT Render_NonBlend();
	HRESULT Render_Lights();
	HRESULT Render_Combined();
	HRESULT Render_NonLight();
	HRESULT Render_Blend();
	HRESULT Render_UI();

#ifdef _DEBUG
private:
	HRESULT Render_Debug();
#endif

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END