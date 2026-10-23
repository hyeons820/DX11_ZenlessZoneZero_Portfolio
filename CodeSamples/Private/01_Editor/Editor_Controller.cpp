// 포트폴리오용 코드 발췌: 오브젝트 배치·편집과 레벨 저장·로드의 호출 흐름
// 원본: Editor/Private/Editor_Controller.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 271~300행: Add_SelectedPrototypeToLayer
HRESULT CEditor_Controller::Add_SelectedPrototypeToLayer(_uint iLayerLevelIndex, const _wstring& strLayerTag, ObjectID* pOutObjectID, const GAMEOBJECT_DESC* pDesc)
{
	if (m_pContext == nullptr || m_pBridge == nullptr)
		return E_FAIL;

	if (pOutObjectID != nullptr)
		*pOutObjectID = INVALID_EDITOR_OBJECT_ID;

	const _int iPrototypeLevelIndex = m_pContext->Get_PaletteTargetLevelIndex();
	if (iPrototypeLevelIndex < 0)
		return E_FAIL;

	const _wstring& strPrototypeTag = m_pContext->Get_SelectedPrototypeTag();
	if (strPrototypeTag.empty())
		return E_FAIL;

	ObjectID pCreatedObjectID = INVALID_EDITOR_OBJECT_ID;

	if (FAILED(m_pBridge->Add_GameObject(static_cast<_uint>(iPrototypeLevelIndex), strPrototypeTag, iLayerLevelIndex, strLayerTag, &pCreatedObjectID, pDesc)))
	{
		return E_FAIL;
	}

	m_pContext->Set_SelectedObjectID(pCreatedObjectID);

	if (pOutObjectID != nullptr)
		*pOutObjectID = pCreatedObjectID;

	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 302~316행: Apply_SelectedObjectTransform
HRESULT CEditor_Controller::Apply_SelectedObjectTransform(const TRANSFORM_DATA& TransformData)
{
	if (m_pContext == nullptr || m_pBridge == nullptr)
		return E_FAIL;

	const ObjectID ID = m_pContext->Get_SelectedObjectID();
	if (ID == INVALID_EDITOR_OBJECT_ID)
		return E_FAIL;

	const _int iLevelIndex = m_pBridge->Get_CurrentLevelIndex();
	if (iLevelIndex < 0)
		return E_FAIL;

	return m_pBridge->Apply_Transform(static_cast<_uint>(iLevelIndex), ID, TransformData);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 337~354행: Pick_EditObjectAtMouse
HRESULT CEditor_Controller::Pick_EditObjectAtMouse(_int iMouseX, _int iMouseY, _float fViewportX, _float fViewportY, _float fViewportWidth, _float fViewportHeight)
{
	if (m_pContext == nullptr || m_pBridge == nullptr)
		return E_FAIL;

	const _int iLevelIndex = m_pContext->Get_CurrentLevelIndex();
	if (iLevelIndex < 0)
		return E_FAIL;

	ObjectID PickedID = INVALID_EDITOR_OBJECT_ID;

	if (FAILED(m_pBridge->Pick_ObjectInLayer(static_cast<_uint>(iLevelIndex),TEXT("Layer_Edit"), iMouseX, iMouseY, fViewportX, fViewportY, fViewportWidth, fViewportHeight, PickedID)))
		return E_FAIL;

	m_pContext->Set_SelectedObjectID(PickedID);

	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 386~404행: Save_CurrentLevel_EditObjects
HRESULT CEditor_Controller::Save_CurrentLevel_EditObjects(const wstring& strFilePath)
{
	if (m_pContext == nullptr || m_pBridge == nullptr)
		return E_FAIL;

	const _int iLevelIndex = m_pContext->Get_CurrentLevelIndex();
	if (iLevelIndex < 0)
		return E_FAIL;

	vector<LEVEL_OBJECT_SAVE_DATA> ObjectSaveData;
	if (FAILED(m_pBridge->Get_EditLayerSaveData(static_cast<_uint>(iLevelIndex), ObjectSaveData)))
		return E_FAIL;

	LEVEL_SAVE_DATA LevelSaveData = {};
	LevelSaveData.iLevelIndex = iLevelIndex;
	LevelSaveData.LevelSaveData = ObjectSaveData;

	return Save_LevelData_ToJsonFile(LevelSaveData, strFilePath);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 406~427행: Load_CurrentLevel_EditObjects
HRESULT CEditor_Controller::Load_CurrentLevel_EditObjects(const wstring& strFilePath)
{
	if (m_pContext == nullptr || m_pBridge == nullptr)
		return E_FAIL;

	const _int iLevelIndex = m_pContext->Get_CurrentLevelIndex();
	if (iLevelIndex < 0)
		return E_FAIL;

	LEVEL_SAVE_DATA LevelSaveData = {};
	if (FAILED(Load_LevelData_FromJsonFile(strFilePath, LevelSaveData)))
		return E_FAIL;

	if (FAILED(m_pBridge->Clear_EditLayerObjects(static_cast<_uint>(iLevelIndex))))
		return E_FAIL;

	if (FAILED(m_pBridge->Spawn_EditLayerObjects(static_cast<_uint>(iLevelIndex), LevelSaveData.LevelSaveData)))
		return E_FAIL;

	m_pContext->Clear_SelectedObjectID();
	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.
