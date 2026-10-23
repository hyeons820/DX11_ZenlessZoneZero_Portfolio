// 포트폴리오용 코드 발췌: 편집 데이터를 엔진 오브젝트와 연결하고 가장 가까운 피킹 대상을 선택
// 원본: Editor/Private/Editor_Bridge.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 111~185행: Get_EditLayerSaveData
HRESULT CEditor_Bridge::Get_EditLayerSaveData(_uint iLevelIndex, vector<LEVEL_OBJECT_SAVE_DATA>& OutObjectSaveDatas) const
{
    OutObjectSaveDatas.clear();

    if (m_pGameInstance == nullptr)
        return E_FAIL;

    vector<_udlong> ObjectIDs;
    if (FAILED(m_pGameInstance->Get_GameObjectIDs(iLevelIndex, TEXT("Layer_Edit"), ObjectIDs)))
        return E_FAIL;

    for (auto ObjectID : ObjectIDs)
    {
        CGameObject* pGameObject = m_pGameInstance->Find_GameObject_ByID(iLevelIndex, ObjectID);
        if (pGameObject == nullptr)
            continue;

        const GAMEOBJECT_META_DATA& MetaData = pGameObject->Get_MetaData();

        CTransform* pTransform = pGameObject->Get_Transform();
        if (pTransform == nullptr)
            continue;

        _float4 vPosition = {};
        XMStoreFloat4(&vPosition, pTransform->Get_State(STATE::POSITION));

        _float3 vScale = pTransform->Get_Scaled();
        
        LEVEL_OBJECT_SAVE_DATA SaveData = {};
        SaveData.GameObjectSaveData.strPrototypeTag = MetaData.strPrototypeTag;
        SaveData.GameObjectSaveData.strRunTimeLayerTag = MetaData.strRunTimeLayerTag;
        memcpy(SaveData.vPosition, &vPosition, sizeof (_float3));
        memcpy(SaveData.vScale, &vScale, sizeof (_float3));

        CSpawnMark* pSpawnMark = dynamic_cast<CSpawnMark*>(pGameObject);
        if (pSpawnMark != nullptr)
        {
            SaveData.bIsMonsterSpawn = true;
            SaveData.GameObjectSaveData.strRunTimeLayerTag = TEXT("Layer_SpawnMarker");

            SaveData.strMonsterPrototypeTag = pSpawnMark->Get_MonsterPrototypeTag();
            SaveData.iMonsterPrototypeLevelIndex = pSpawnMark->Get_PrototypeLevelIndex();
            SaveData.iNavigationLevelIndex = pSpawnMark->Get_NavigationLevelIndex();
            SaveData.strNavigationPrototypeTag = pSpawnMark->Get_NavigationPrototypeTag();
            SaveData.iSpawnGroupID = pSpawnMark->Get_SpawnGroupID();
            SaveData.fTriggerRadius = pSpawnMark->Get_TriggerRadius();
            SaveData.bStartDormant = pSpawnMark->Is_StartDormant();
            SaveData.bPlayBornAnimation = pSpawnMark->Should_PlayBornAnimation();
        }

        CPlayerSpawnMark* pPlayerSpawnMark = dynamic_cast<CPlayerSpawnMark*>(pGameObject);
        if (pPlayerSpawnMark != nullptr)
        {
            SaveData.bIsPlayerSpawn = true;
            SaveData.GameObjectSaveData.strRunTimeLayerTag = TEXT("Layer_PlayerSpawnMarker");

            SaveData.strPlayerPrototypeTag = pPlayerSpawnMark->Get_PlayerPrototypeTag();
            SaveData.iPlayerPrototypeLevelIndex = pPlayerSpawnMark->Get_PlayerPrototypeLevelIndex();
            SaveData.iNavigationLevelIndex = pPlayerSpawnMark->Get_NavigationLevelIndex();
            SaveData.strNavigationPrototypeTag = pPlayerSpawnMark->Get_NavigationPrototypeTag();
        }

        OutObjectSaveDatas.push_back(SaveData);

        char szDebug[256] = {};
        sprintf_s(szDebug, "SaveData Push : ID=%llu\n", ObjectID);
        OutputDebugStringA(szDebug);
    }

    char szDebug[128] = {};
    sprintf_s(szDebug, "Layer_Edit Count : %zu\n", ObjectIDs.size());
    OutputDebugStringA(szDebug);

    return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 244~315행: Spawn_EditLayerObjects
HRESULT CEditor_Bridge::Spawn_EditLayerObjects(_uint iLevelIndex, const vector<LEVEL_OBJECT_SAVE_DATA>& ObjectDatas)
{
    if (m_pGameInstance == nullptr)
        return E_FAIL;

    for (const auto& ObjectData : ObjectDatas)
    {
        if (ObjectData.GameObjectSaveData.strPrototypeTag.empty())
            continue;

        GAMEOBJECT_DESC Desc = {};
        Desc.strRunTimeLayerTag = ObjectData.GameObjectSaveData.strRunTimeLayerTag;

        CGameObject* pCreatedObject = nullptr;

        if (ObjectData.bIsMonsterSpawn)
        {
            CSpawnMark::SPAWN_MARK_DESC Desc = {};
            Desc.strRunTimeLayerTag = ObjectData.GameObjectSaveData.strRunTimeLayerTag;

            Desc.strMonsterPrototypeTag = ObjectData.strMonsterPrototypeTag;
            Desc.iPrototypeLevelIndex = ObjectData.iMonsterPrototypeLevelIndex;
            Desc.iNavigationLevelIndex = ObjectData.iNavigationLevelIndex;
            Desc.strNavigationPrototypeTag = ObjectData.strNavigationPrototypeTag;
            Desc.iSpawnGroupID = ObjectData.iSpawnGroupID;
            Desc.fTriggerRadius = ObjectData.fTriggerRadius;
            Desc.bStartDormant = ObjectData.bStartDormant;
            Desc.bPlayBornAnimation = ObjectData.bPlayBornAnimation;

            if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, ObjectData.GameObjectSaveData.strPrototypeTag, iLevelIndex, TEXT("Layer_Edit"), &pCreatedObject, &Desc))) 
                return E_FAIL;
        }
        else if (ObjectData.bIsPlayerSpawn)
        {
            CPlayerSpawnMark::PLAYER_SPAWN_MARK_DESC Desc = {};
            Desc.strRunTimeLayerTag = ObjectData.GameObjectSaveData.strRunTimeLayerTag;
            Desc.strPlayerPrototypeTag = ObjectData.strPlayerPrototypeTag;
            Desc.iPlayerPrototypeLevelIndex = ObjectData.iPlayerPrototypeLevelIndex;
            Desc.iNavigationLevelIndex = ObjectData.iNavigationLevelIndex;
            Desc.strNavigationPrototypeTag = ObjectData.strNavigationPrototypeTag;

            if (FAILED(m_pGameInstance->Add_GameObject(
                iLevelIndex,
                ObjectData.GameObjectSaveData.strPrototypeTag,
                iLevelIndex,
                TEXT("Layer_Edit"),
                &pCreatedObject,
                &Desc)))
                return E_FAIL;
        }
        else
        {
            GAMEOBJECT_DESC Desc = {};
            Desc.strRunTimeLayerTag = ObjectData.GameObjectSaveData.strRunTimeLayerTag;

            if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, ObjectData.GameObjectSaveData.strPrototypeTag, iLevelIndex, TEXT("Layer_Edit"), &pCreatedObject, &Desc))) 
                return E_FAIL;
        }

        if (pCreatedObject == nullptr)
            return E_FAIL;

        CTransform* pTransform = pCreatedObject->Get_Transform();
        if (pTransform == nullptr)
            return E_FAIL;

        pTransform->Set_State(STATE::POSITION, XMVectorSet(ObjectData.vPosition[0], ObjectData.vPosition[1], ObjectData.vPosition[2], 1.f));
        pTransform->Set_Scale(ObjectData.vScale[0], ObjectData.vScale[1], ObjectData.vScale[2]);
    }

    return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 429~446행: Apply_Transform
HRESULT CEditor_Bridge::Apply_Transform(_uint iLevelIndex, ObjectID ID, const TRANSFORM_DATA& TransformData)
{
    if (m_pGameInstance == nullptr || ID == INVALID_EDITOR_OBJECT_ID)
        return E_FAIL;

    CGameObject* pGameObject = m_pGameInstance->Find_GameObject_ByID(iLevelIndex, ID);
    if (pGameObject == nullptr)
        return E_FAIL;

    CTransform* pTransform = pGameObject->Get_Transform();
    if (pTransform == nullptr)
        return E_FAIL;

    pTransform->Set_State(STATE::POSITION, XMVectorSet(TransformData.vPosition[0], TransformData.vPosition[1], TransformData.vPosition[2], 1.f));
    pTransform->Set_Scale(TransformData.vScale[0], TransformData.vScale[1], TransformData.vScale[2]);

    return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 487~524행: Pick_ObjectInLayer
HRESULT CEditor_Bridge::Pick_ObjectInLayer(_uint iLevelIndex, const _wstring& strLayerTag, _int iMouseX, _int iMouseY, _float fViewportX, _float fViewportY, _float fViewportWidth, _float fViewportHeight, ObjectID& ID) const
{
    ID = INVALID_EDITOR_OBJECT_ID;

    if (m_pGameInstance == nullptr)
        return E_FAIL;

    PICKING_RAY Ray = {};
    if (FAILED(m_pGameInstance->Make_Ray(iMouseX, iMouseY, fViewportX, fViewportY, fViewportWidth, fViewportHeight, Ray)))
        return E_FAIL;

    vector<_udlong> ObjectIDs;
    if (FAILED(m_pGameInstance->Get_GameObjectIDs(iLevelIndex, strLayerTag, ObjectIDs)))
        return E_FAIL;

    _float fClosestDist = { FLT_MAX };
    _bool  bHit = { false };

    for (auto& ObjectID : ObjectIDs)
    {
        CGameObject* pGameObject = m_pGameInstance->Find_GameObject_ByID(iLevelIndex, ObjectID);
        if (pGameObject == nullptr)
            continue;

        _float fHitDist = {};
        if (FAILED(pGameObject->Intersect_Ray(Ray.vOrigin, Ray.vDirection, fHitDist)))
            continue;

        if (fHitDist < fClosestDist)
        {
            fClosestDist = fHitDist;
            ID = ObjectID;
            bHit = true;
        }
    }

    return bHit ? S_OK : E_FAIL;
}

// [중략] 원본의 다른 함수 및 선언.
