// 포트폴리오용 코드 발췌: 단계형 툰 음영과 재질 버퍼를 이용한 최종 합성
// 원본: Engine/Bin/ShaderFiles/Shader_Deferred.hlsl
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 25~34행: ToonStep
float ToonStep(float fNdotL)
{
    if (fNdotL < 0.28f)
        return 0.78f;

    if (fNdotL < 0.60f)
        return 0.91f;

    return 1.f;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 89~135행: PS_MAIN_DIRECTIONAL
PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 500.f;
    
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    float fSpecMask = vNormalDesc.a;
    
    float3 vLightDir = normalize(g_vLightDir.xyz) * -1.f;
    float3 vNormalDir = normalize(vNormal.xyz);
    
    float fNdotL = saturate(dot(vLightDir, vNormalDir));
    float fToonShade = ToonStep(fNdotL);
    
    float3 vAmbient = g_vLightAmbient.rgb * g_vMtrlAmbient.rgb;

    Out.vShade = float4(/*max(vAmbient,*/ g_vLightDiffuse.rgb * fToonShade/*)*/, 1.f);

    vector vReflect = reflect(normalize(g_vLightDir), normalize(vNormal));

    vector vWorldPos;
    
    /* 투영공간상의 위치 */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;

    /* 뷰스페이스 상의 위치 */
    vWorldPos *= fViewZ;
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);

    /* 월드스페이스 상의 위치 */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);

    vector vLook = normalize(vWorldPos - g_vCamPosition);
    
    float fSpec = pow(saturate(dot(vReflect * -1.f, vLook)), 24.f);

    float3 vSpecular = g_vLightSpecular.rgb * fSpec * fSpecMask * 0.22f;
    Out.vSpecular = float4(vSpecular, 1.f);

    return Out;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 182~205행: PS_MAIN_COMBINED
PS_OUT_BACKBUFFER PS_MAIN_COMBINED(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.f == vDiffuse.a)
        discard;
    
    vector vShade = g_ShadeTexture.Sample(PointSampler, In.vTexcoord);
    vector vSpecular = g_SpecularTexture.Sample(PointSampler, In.vTexcoord);
    vector vMaterial = g_MaterialTexture.Sample(PointSampler, In.vTexcoord);
    
    float fFaceLight = vMaterial.a;

    float3 vFaceMinShade = float3(0.96f, 0.90f, 0.84f);
    float3 vFaceShade = lerp(vShade.rgb, max(vShade.rgb, vFaceMinShade), fFaceLight * 0.60f);

    Out.vBackBuffer =
      float4(vDiffuse.rgb * vFaceShade, vDiffuse.a)
      + vSpecular
      + float4(vMaterial.rgb, 0.f);
    
    return Out;
}

// [중략] 원본의 다른 함수 및 선언.
