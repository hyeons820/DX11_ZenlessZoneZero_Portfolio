#pragma once

#include "Editor_Context.h"
#include "Editor_Bridge.h"
#include "Editor_Types.h"

NS_BEGIN(Engine)
struct GAMEOBJECT_DESC;
NS_END


NS_BEGIN(Editor)

class CEditor_Controller final : public CBase
{
private:
	CEditor_Controller() = default;
	virtual ~CEditor_Controller() = default;

public:
	_int			Get_CurrentLevelIndex() const;
	_int			Get_PaletteTargetLevelIndex() const;
	HRESULT			Get_PrototypeTags(vector<_wstring>& Tags) const;
	const _wstring& Get_SelectedPrototypeTag() const;
	ObjectID		Get_SelectedObjectID() const;
	HRESULT			Get_SelectedObjectTransform(TRANSFORM_DATA& OutData) const;
	HRESULT			Get_CurrentLevelHierarchyItems(vector<HIERARCHY_ITEM>& OutItems) const;

	HRESULT			Get_SelectedObjectRuntimeLayerTag(_wstring& OutTag) const;
	HRESULT			Apply_SelectedObjectRuntimeLayerTag(const _wstring& strTag);

	HRESULT			Get_SelectedObjectAnimationData(const ObjectID ID, ANIMATION_DEBUG_DATA& OutData) const;
	HRESULT			Apply_SelectedObjectAnimationData(const ObjectID ID, const ANIMATION_DEBUG_DATA& InData);

	HRESULT			Set_SelectedObjectAnimation(const ObjectID ID, _int iIndex, _bool bLoop);
	HRESULT			Blend_SelectedObjectAnimation(const ObjectID ID, _int iIndex, _float fBlendTime, _bool bLoop);
	HRESULT			Reset_SelectedObjectAnimationTrack(const ObjectID ID);

	HRESULT			Get_SelectedObjectAnimationNames(const ObjectID ID, vector<const _char*>& OutNames) const;

	void			Set_SelectedPrototypeTag(const _wstring& strPrototypeTag);
	void			Set_PaletteTargetLevelIndex(_int iLevelIndex);
	void			Set_SelectedObjectID(ObjectID ID);
	
	_bool			Has_SelectedObject() const;

public:
	HRESULT			Initialize();
	HRESULT			Sync_CurrentLevelFromEngine();
	HRESULT			Add_SelectedPrototypeToLayer(_uint iLayerLevelIndex,const _wstring& strLayerTag, ObjectID* pOutObjectID, const GAMEOBJECT_DESC* pDesc = nullptr);
	HRESULT			Apply_SelectedObjectTransform(const TRANSFORM_DATA& TransformData);
	HRESULT			Pick_ObjectAtMouse(_int iMouseX, _int iMouseY, _float fViewportX, _float fViewportY, _float fViewportWidth, _float fViewportHeight);
	HRESULT			Pick_EditObjectAtMouse(_int iMouseX, _int iMouseY, _float fViewportX, _float fViewportY, _float fViewportWidth, _float fViewportHeight);
	HRESULT			Pick_WorldPositionAtMouse(_int iMouseX, _int iMouseY, _float fViewportX, _float fViewportY, _float fViewportWidth, _float fViewportHeight, _float3& OutHitPosition, ObjectID* pOutObjectID = nullptr);
	HRESULT			Remove_GameObject(_udlong ObjectID);

	HRESULT			Save_CurrentLevel_EditObjects(const wstring& strFilePath);
	HRESULT			Load_CurrentLevel_EditObjects(const wstring& strFilePath);

	void			Clear_SelectedObjectID();
	void			Clear_SelectedPrototypeTag();

private:
	// 에디터 선택 상태
	class CEditor_Context* m_pContext = { nullptr };
	// 에디터 엔진 연결
	class CEditor_Bridge* m_pBridge = { nullptr };

public:
	static CEditor_Controller* Create();
	virtual void Free() override;
};

NS_END