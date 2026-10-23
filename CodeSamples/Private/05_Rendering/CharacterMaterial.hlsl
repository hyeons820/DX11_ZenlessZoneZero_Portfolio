// 포트폴리오용 코드 발췌: MatCap 하이라이트와 얼굴 라이트맵 값 계산
// 원본: Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 142~160행: GetWorldNormal
float3 GetWorldNormal(PS_IN In)
{
    float3 vNormal = normalize(In.vNormal.xyz);

    if (g_iUseNormalTexture == 0)
        return vNormal;

    float fNormalStrength = 0.55f;

    float3 vNormalTex = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).xyz;
    vNormalTex = vNormalTex * 2.f - 1.f;

    vNormalTex.xy *= fNormalStrength;
    vNormalTex.z = sqrt(saturate(1.f - dot(vNormalTex.xy, vNormalTex.xy)));

    float3x3 TBN = float3x3(normalize(In.vTangent), normalize(In.vBinormal * -1.f), normalize(vNormal));

    return normalize(mul(vNormalTex, TBN));
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 162~170행: GetMatCapUV
float2 GetMatCapUV(float3 vWorldNormal)
{
    float3 vViewNormal = normalize(mul(float4(vWorldNormal, 0.f), g_ViewMatrix).xyz);

    float2 vMatCapUV = vViewNormal.xy * 0.5f + 0.5f;
    vMatCapUV.y = 1.f - vMatCapUV.y;

    return saturate(vMatCapUV);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 172~178행: ExtractMatCapHighlight
float3 ExtractMatCapHighlight(float3 vMatCapColor, float fPower, float fIntensity)
{
    float fLuma = dot(vMatCapColor, float3(0.299f, 0.587f, 0.114f));
    float fMask = pow(saturate(fLuma), fPower);

    return vMatCapColor * fMask * fIntensity;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 180~222행: BuildMaterialAdditive
float3 BuildMaterialAdditive(float3 vWorldNormal, float2 vTexcoord)
{
    float3 vAdd = 0.f;

    float2 vMatCapUV = GetMatCapUV(vWorldNormal);
    if (g_iUseMatCapTexture != 0)
    {
        float3 vMatCap = g_MatCapTexture.Sample(LinearSampler, vMatCapUV).rgb;
        vAdd += ExtractMatCapHighlight(vMatCap, 10.f, 0.10f);
    }
    
    if (g_iUseMatCapTexture2 != 0)
    {
        float3 vMatCap = g_MatCapTexture2.Sample(LinearSampler, vMatCapUV).rgb;
        vAdd += ExtractMatCapHighlight(vMatCap, 10.f, 0.07f);
    }

    if (g_iUseMatCapTexture3 != 0)
    {
        float3 vMatCap = g_MatCapTexture3.Sample(LinearSampler, vMatCapUV).rgb;
        vAdd += ExtractMatCapHighlight(vMatCap, 10.f, 0.08f);
    }

    if (g_iUseMatCapTexture4 != 0)
    {
        float3 vMatCap = g_MatCapTexture4.Sample(LinearSampler, vMatCapUV).rgb;
        vAdd += ExtractMatCapHighlight(vMatCap, 10.f, 0.08f);
    }

    if (g_iUseMatCapTexture5 != 0)
    {
        float3 vMatCap = g_MatCapTexture5.Sample(LinearSampler, vMatCapUV).rgb;
        vAdd += ExtractMatCapHighlight(vMatCap, 10.f, 0.08f);
    }

    if (g_iUseEyeColorTexture != 0)
    {
        float4 vEye = g_EyeColorTexture.Sample(LinearSampler, vTexcoord);
        vAdd += vEye.rgb * 0.35f;
    }

    return saturate(vAdd);
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 224~237행: GetFaceLightFactor
float GetFaceLightFactor(float2 vTexcoord)
{
    if (g_iUseFaceLightMapTexture == 0)
        return 0.f;

    float3 vFaceLight = g_FaceLightMapTexture.Sample(LinearSampler, vTexcoord).rgb;

      // r 하나만 쓰면 특정 부위가 띠처럼 튈 수 있어서 평균으로 부드럽게 사용
    float fFaceLight = dot(vFaceLight, float3(0.333f, 0.333f, 0.333f));

      // 너무 강한 대비 제거
    fFaceLight = smoothstep(0.10f, 0.75f, fFaceLight);
    return fFaceLight;
}

// [중략] 원본의 다른 함수 및 선언.
