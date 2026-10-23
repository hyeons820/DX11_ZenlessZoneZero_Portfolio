#pragma once
#include "Editor_Defines.h"
#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
struct GAMEOBJECT_DESC;
NS_END

NS_BEGIN(Editor)

class CEditor_Bridge final : public CBase
{
private:
	CEditor_Bridge() = default;
	virtual ~CEditor_Bridge() = default;

public:
	_int		 Get_CurrentLevelIndex() const;
	HRESULT		 Get_PrototypeTags(_uint iLevelIndex, vector<_wstring>& Tags) const;
	HRESULT		 Get_TransformData(_uint iLevelIndex, ObjectID ID, TRANSFORM_DATA& OutData) const;
	HRESULT		 Get_HierarchyItems(_uint iLevelIndex, vector<HIERARCHY_ITEM>& OutItems) const;
	HRESULT		 Get_EditLayerSaveData(_uint iLevelIndex, vector<LEVEL_OBJECT_SAVE_DATA>& OutObjectSaveDatas) const;

	HRESULT		 Get_ObjectRuntimeLayerTag(_uint iLevelIndex, ObjectID ID, _wstring& OutTag) const;
	HRESULT		 Set_ObjectRuntimeLayerTag(_uint iLevelIndex, ObjectID ID, const _wstring& strTag);

	HRESULT		 Clear_EditLayerObjects(_uint iLevelIndex);
	HRESULT		 Spawn_EditLayerObjects(_uint iLevelIndex, const vector<LEVEL_OBJECT_SAVE_DATA>& ObjectDatas);
	
	HRESULT		 Get_AnimationInspectorData(_uint iLevelIndex, ObjectID ID, ANIMATION_DEBUG_DATA& OutData) const;
	HRESULT		 Apply_AnimationInspectorData(_uint iLevelIndex, ObjectID ID, const ANIMATION_DEBUG_DATA& InData);

	HRESULT		 Set_AnimationByIndex(_uint iLevelIndex, ObjectID ID, _int iIndex, _bool bLoop);
	HRESULT		 Blend_AnimationByIndex(_uint iLevelIndex, ObjectID ID, _int iIndex, _float fBlendTime, _bool bLoop);
	HRESULT		 Reset_AnimationTrack(_uint iLevelIndex, ObjectID ID);

	HRESULT		 Get_AnimationNames(_uint iLevelIndex, ObjectID ID, vector<const _char*>& OutNames) const;

public:
	HRESULT		 Initialize();
	HRESULT      Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, ObjectID* pOutObjectID, const GAMEOBJECT_DESC* pDesc);
	HRESULT		 Remove_GameObject(_uint iLevelIndex, ObjectID ObjectID);
	CGameObject* Find_GameObject_ByID(_uint iLevelIndex, ObjectID ID);
	HRESULT		 Apply_Transform(_uint iLevelIndex, ObjectID ID, const TRANSFORM_DATA& TransformData);
	HRESULT		 Pick_Object(_uint iLevelIndex,_int iMouseX, _int iMouseY, _float fViewportX, _float fViewportY, _float fViewportWidth, _float fViewportHeight, ObjectID& ID) const;
	HRESULT		 Pick_ObjectInLayer(_uint iLevelIndex, const _wstring& strLayerTag, _int iMouseX, _int iMouseY, _float fViewportX, _float fViewportY, _float fViewportWidth, _float fViewportHeight, ObjectID& ID) const;
	HRESULT		 Pick_WorldPosition(_uint iLevelIndex, _int iMouseX, _int iMouseY, _float fViewportX, _float fViewportY, _float fViewportWidth, _float fViewportHeight, _float3& OutHitPosition, ObjectID* ID = nullptr) const;

private:
	CGameInstance* m_pGameInstance = { nullptr };

public:
	static CEditor_Bridge* Create();
	virtual void Free() override;
};

NS_END