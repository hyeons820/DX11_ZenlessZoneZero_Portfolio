#pragma once
#include "Component.h"
#include "Model_Format.h"

NS_BEGIN(Engine)

class CMesh;
class CMaterial;
class CShader;

struct ANIM_SYNC_INFO
{
    _uint iAnimationIndex = {};          // 현재 재생 중인 애니메이션 인덱스
    _float fTrackPosition = {};          // 현재 애니메이션 재생 위치
    _float fDuration = {};               // 현재 애니메이션 전체 길이
    _float fNormalizedTime = {};         // 현재 재생 위치를 0~1로 정규화한 값
    _bool bBlending = { false };         // 애니메이션 블렌딩 중인지
};

struct ANIMATION_UPDATE_RESULT
{
    _bool bFinished = { false };          // 이번 프레임에 애니메이션이 끝났는지
    _bool bLooped = { false };            // 이번 프레임에 루프가 한 번 돌았는지
};

struct OVERLAY_ANIM_STATE
{
    _bool  bActive = false;               // 오버레이 애니메이션이 활성화되어 있는지
    _uint  iAnimationIndex = 0;           // 오버레이로 재생할 애니메이션 인덱스

    _float fTime = 0.f;                   // 오버레이 재생 누적 시간
    _float fDuration = 0.f;               // 오버레이 유지 시간

    _float fWeight = 0.f;                 // 현재 오버레이 가중치
    _float fMaxWeight = 0.8f;              // 최대 오버레이 가중치

    _bool  bLoop = false;                 // 오버레이 애니메이션 반복 여부
};


class ENGINE_DLL CModel final : public CComponent
{
private:
    CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CModel(const CModel& Prototype);
    virtual ~CModel() = default;

public:
    const _uint     Get_NumMeshes() const { return m_Meshes.size(); }                           // 모델이 가진 메쉬 개수 반환
    const _int      Get_NodeIndex(const _char* pNodeName) const;                                // 노드 이름으로 노드 인덱스 검색
    const _float3&  Get_RootMotionDeltaLocal() const { return m_vRootMotionDeltaLocal; }        // 이번 프레임 루트모션 로컬 이동량 반환
    const _float3&  Get_DebugRootBeforeStripLocal() const { return m_fRootBeforeStrip; }        // 루트모션 제거 전 루트 위치 디버그 값
    const _float3&  Get_DebugRootAfterStripLocal() const { return m_fRootAfterStrip; }          // 루트모션 제거 후 루트 위치 디버그 값
    const _char*    Get_AnimationName(_uint iIndex) const;                                      // 애니메이션 인덱스에 해당하는 이름 반환
    HRESULT         Get_AnimationNames(vector<const _char*>& OutNames) const;                   // 전체 애니메이션 이름 목록 반환

    // 에디터용
    _uint           Get_CurrentAnimationIndex() const { return m_iCurrentAnimationIndex; }
    _uint           Get_NextAnimationIndex() const { return m_iNextAnimationIndex; }
    _uint           Get_AnimationCount() const { return static_cast<_uint>(m_Animations.size()); }

    _bool           Is_Loop() const { return m_bLoop; }
    _bool           Is_Blending() const { return m_bBlending; }
    _bool           Is_UsingRootMotion() const { return m_bUseRootMotion; }

    _float          Get_AnimationSpeed() const { return static_cast<_float>(m_fAnimationSpeed); }
    _float          Get_BlendDuration() const { return static_cast<float>(m_fBlendDuration); }
    _float          Get_BlendAccTime() const { return static_cast<float>(m_fBlendAccTime); }
    _float          Get_BlendRatio() const
    {
        if (!m_bBlending || m_fBlendDuration <= 0.f)
            return 0.f;

        return min(1.f, static_cast<_float>(m_fBlendAccTime / m_fBlendDuration));
    }

public:
    void    Set_Animation(_uint iIndex) { m_iCurrentAnimationIndex = iIndex; }              // 현재 애니메이션 인덱스를 직접 설정
    void    Set_UseRootMotion(_bool bUse) { m_bUseRootMotion = bUse; }                      // 루트모션 사용 여부 설정
    void    Set_AnimationSpeed(_float fSpeed) { m_fAnimationSpeed = max(0.f, fSpeed); }     // 애니메이션 재생 속도 설정
    void    Set_Loop(_bool bLoop) { m_bLoop = bLoop; }                                      // 현재 애니메이션 반복 여부 설정

    HRESULT Get_CurrentAnimSyncInfo(ANIM_SYNC_INFO& OutInfo) const;                         // 현재 애니메이션 동기화 정보 반환

    HRESULT Play_AnimationImmediate(_uint iIndex, _bool bLoop);                             // 즉시 재생 + 트랙 초기화 + 루트모션 기준 리셋
    HRESULT Reset_CurrentAnimationTrack();                                                  // 현재 클립을 처음부터 다시 보기

public:
    HRESULT Play_OverlayAnimation(_uint iIndex, _float fDuration, _float fWeight = 1.f);    // 현재 애니메이션 위에 오버레이 애니메이션 재생
    void    Stop_OverlayAnimation();                                                        // 현재 오버레이 애니메이션 중지

public:
    HRESULT Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
    virtual HRESULT Initialize(const COMPONENT_DESC* pDesc) override;

public:
    HRESULT Render_All(CShader* pShader, _uint iPassIndex = 0);                              // 모든 메쉬 렌더
    HRESULT Render_All_Except(CShader* pShader, _uint iPassIndex, _int iExceptMeshIndex);    // 특정 메쉬를 제외하고 렌더
    HRESULT Render_Mesh(CShader* pShader, _uint iMeshIndex, _uint iPassIndex);               // 지정한 메쉬 하나만 렌더
    HRESULT Render_MeshRange(CShader* pShader, _uint iStartMeshIndex, _uint iEndMeshIndex, _uint iPassIndex);   // 지정 범위 메쉬 렌더
    HRESULT Render_All_NoMaterial(CShader* pShader, _uint iPassIndex = 0);
    HRESULT Intersect_Ray(_fmatrix WorldMatrix, _fvector vRayOriginWorld, _fvector vRayDirWorld, _float& fOutDistance) const;   // 월드 레이를 모델 로컬로 변환해 메쉬 충돌 검사

    HRESULT Reset_NodeLocalMatrices();                                              // 노드 로컬 행렬을 원본 로컬 행렬로 초기화
    HRESULT Update_NodeCombinedMatrices();                                          // 부모 계층을 반영한 노드 Combined 행렬 갱신
    HRESULT Update_FinalBoneMatrices();                                             // 스키닝에 사용할 최종 본 행렬 갱신

    ANIMATION_UPDATE_RESULT Play_Animation(_float fTimeDelta);                      // 현재 애니메이션을 진행하고 본 행렬 갱신
    HRESULT Change_Animation(_uint iNextIndex, _double fBlendTime, _bool bLoop);    // 다음 애니메이션으로 블렌딩 전환
    HRESULT Blend_NodeLocalMatrices(const vector<_float4x4>& SourMat, const vector<_float4x4>& DestMat, _float fWeight, vector<_float4x4>& OutMat); // 두 노드 로컬 포즈를 가중치로 보간

    HRESULT Find_RootNode();                                                        // 루트모션 기준 루트 노드 탐색
    HRESULT Find_CamNode();                                                         // 카메라 연출용 노드 탐색
    HRESULT Consume_RootMotion();                                                   // 루트 노드 이동량을 추출하고 모델 포즈에서는 제거


private:
    // 공유 데이터
    MODEL m_eModelType = { MODEL::END };                    // 정적/애니메이션 모델 타입
    MODEL_HEADER m_Header = {};                             // modelbin 헤더 정보
    _float4x4 m_PreTransformMatrix = {};                    // 모델 로드 시 적용한 사전 변환 행렬

    vector<CMaterial*> m_Materials;                         // 모델 머티리얼 목록
    vector<CMesh*> m_Meshes;                                // 모델 메쉬 목록
    
    vector<BONE_INFO> m_Bones;                              // 스키닝 본 정보 목록
    vector<NODE_INFO> m_Nodes;                              // 전체 노드 계층 정보 목록

    unordered_map<string, uint32_t> m_NodeIndexMap;         // 노드 이름으로 인덱스를 찾기 위한 맵

    // 공유 X
    vector<class CAnimation*> m_Animations;                 // 모델이 가진 애니메이션 클립 목록

    vector<_float4x4> m_NodeLocalMatrices;                  // 현재 프레임 노드 로컬 행렬
    vector<_float4x4> m_NodeCombinedMatrices;               // 부모 계층이 누적된 노드 행렬
    vector<_float4x4> m_FinalBoneMatrices;                  // 셰이더에 전달할 최종 본 행렬

    _uint   m_iCurrentAnimationIndex = { 0 };               // 현재 재생 중인 애니메이션 인덱스
    _double m_fAnimationSpeed = { 1.f };                    // 애니메이션 재생 속도
    _bool   m_bLoop = { false };                            // 현재 애니메이션 반복 여부

    // 애니메이션 블렌드
    _bool   m_bBlending = { false };                        // 진행 중인지
    _uint   m_iNextAnimationIndex = {};                     // 넘어갈 애니메이션
    _float  m_fBlendDuration = {};                          // 총 블렌딩 시간
    _double m_fBlendAccTime = {};                           // 현재까지 지난 블렌딩 시간
    _bool   m_bNextLoop = { false }; 

    vector<_float4x4> m_BlendSourNodeLocalMatrices;         // 블렌딩 시작 애니메이션 포즈
    vector<_float4x4> m_BlendDestNodeLocalMatrices;         // 블렌딩 대상 애니메이션 포즈

    // 루트 노드
    _int    m_iRootNodeIndex = { -1 };                      // 루트모션 기준 루트 노드 인덱스
    _bool   m_bUseRootMotion = { false };                   // 루트모션 사용 여부
    _bool   m_bRootMotionInitialized = { false };           // 루트모션 이전 위치가 초기화됐는지

    // 카메라 노드
    _int    m_iCamNodeIndex = { -1 };                       // 카메라 연출 기준 노드 인덱스

    // 루트 노드 포지션 적용
    _float3 m_vPrevRootMotions = {};                        // 이전 프레임 루트 노드 위치
    _float3 m_vRootMotionDeltaLocal = {};                   // 이번 프레임 루트모션 이동량
    _float3 m_fRootBeforeStrip = {};                        // 루트 위치 제거 전 디버그 값
    _float3 m_fRootAfterStrip = {};                         // 루트 위치 제거 후 디버그 값

private:
    OVERLAY_ANIM_STATE m_OverlayAnim = {};                  // 현재 오버레이 애니메이션 상태
    vector<_float4x4>  m_OverlayNodeLocalMatrices;          // 오버레이 애니메이션 포즈 행렬


private:
    HRESULT Read_ModelFile(const _char* pModelFilePath, _fmatrix PreTransformMatrix);
    HRESULT Read_Header(ifstream& ifs);
    HRESULT Read_Materials(ifstream& ifs);
    HRESULT Read_Meshes(ifstream& ifs, _fmatrix PreTransformMatrix);
    HRESULT Read_Bones(ifstream& ifs);
    HRESULT Read_Nodes(ifstream& ifs);
    HRESULT Read_Animations(ifstream& ifs, CModel* pModel);

    HRESULT Bind_Material_Default(class CShader* pShader, _uint iMeshIndex);
    HRESULT Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, _uint iMeshIndex);

private:
    HRESULT Update_OverlayAnimation(_float fTimeDelta);
    HRESULT Apply_OverlayToNodeLocalMatrices(const vector<_float4x4>& OverlayNodeLocalMatrices, _float fWeight);

public:
    static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
    virtual CComponent* Clone(const COMPONENT_DESC* pDesc) override;
    virtual void Free() override;
};

NS_END