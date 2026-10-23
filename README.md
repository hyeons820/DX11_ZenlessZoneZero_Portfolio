# Zenless Zone Zero · DirectX 11 코드 샘플

게임 구현을 설명하기 위해 핵심 함수만 발췌한 자료입니다. 전체 프로젝트와 리소스는 포함하지 않았으며, 이 저장소만으로 빌드하거나 실행할 수 없습니다.

## 읽는 순서

| 영역 | 확인할 흐름 |
| --- | --- |
| 에디터 | Controller의 편집 요청 → Bridge의 오브젝트 접근·피킹 → 내비게이션 셀 제작 |
| 애니메이션·조작 | Model의 블렌딩·루트 이동량 → Character의 이동 반영 → Miyabi의 입력 버퍼·콤보 |
| 전투 | 충돌 분류 → 공격별 피격 기록 → 패링·회피 판정 → 시간 배율 제어 |
| 카메라 | 추적 → 궁극기 카메라 키프레임 보간 → 추적 복귀 |
| 렌더링 | G-buffer → 툰 조명 → 캐릭터 재질을 반영한 합성 |

구현 파일(cpp, hlsl)은 `CodeSamples/Private`, 헤더(h)는 `CodeSamples/Public`에 배치하고, 양쪽에 동일한 01~05 기능별 폴더를 유지합니다.

## 샘플 목록

| 파일 | 발췌 내용 | 원본 위치 |
| --- | --- | --- |
| [01_Editor/Editor_Controller.cpp](CodeSamples/Private/01_Editor/Editor_Controller.cpp) | 오브젝트 배치·편집과 레벨 저장·로드의 호출 흐름 | `Editor/Private/Editor_Controller.cpp` |
| [01_Editor/Editor_Bridge.cpp](CodeSamples/Private/01_Editor/Editor_Bridge.cpp) | 편집 데이터를 엔진 오브젝트와 연결하고 가장 가까운 피킹 대상을 선택 | `Editor/Private/Editor_Bridge.cpp` |
| [01_Editor/Editor_Navigation.cpp](CodeSamples/Private/01_Editor/Editor_Navigation.cpp) | 내비게이션 셀의 정점 스냅·유효성 검사·이웃 연결·저장 | `Editor/Private/Editor_Navigation.cpp` |
| [02_Animation/Model_Animation.cpp](CodeSamples/Private/02_Animation/Model_Animation.cpp) | 애니메이션 전환·보간과 루트 이동량 추출 | `Engine/Private/Model.cpp` |
| [02_Animation/Character_RootMotion.cpp](CodeSamples/Private/02_Animation/Character_RootMotion.cpp) | 애니메이션 갱신 결과를 캐릭터 이동에 반영하고 공격 종료를 알림 | `Client/Private/Character.cpp` |
| [02_Animation/Miyabi_Combo.cpp](CodeSamples/Private/02_Animation/Miyabi_Combo.cpp) | 입력 버퍼와 애니메이션 진행률을 이용한 일반 공격 콤보 연결 | `Client/Private/Miyabi.cpp` |
| [03_Combat/CombatSystem_Manager.cpp](CodeSamples/Private/03_Combat/CombatSystem_Manager.cpp) | 충돌 분류, 중복·다단 피격 기록, 방어 판정과 피해 처리 | `Client/Private/CombatSystem_Manager.cpp` |
| [03_Combat/Miyabi_Defense.cpp](CodeSamples/Private/03_Combat/Miyabi_Defense.cpp) | 패링 진입·연속 패링·저스트 회피와 시간 배율 적용 | `Client/Private/Miyabi.cpp` |
| [03_Combat/TimeScale_Manager.cpp](CodeSamples/Private/03_Combat/TimeScale_Manager.cpp) | 플레이어·몬스터 등 시간 레이어별 배율과 지속 시간 관리 | `Engine/Private/TimeScale_Manager.cpp` |
| [04_Camera/InGameCamera.cpp](CodeSamples/Private/04_Camera/InGameCamera.cpp) | 추적 카메라에서 궁극기 카메라 애니메이션으로 전환한 뒤 복귀 | `Client/Private/InGameCamera.cpp` |
| [04_Camera/CameraAnimClip.cpp](CodeSamples/Private/04_Camera/CameraAnimClip.cpp) | 카메라 키프레임의 위치·회전을 시간에 따라 보간 | `Client/Private/CameraAnimClip.cpp` |
| [05_Rendering/Renderer.cpp](CodeSamples/Private/05_Rendering/Renderer.cpp) | G-buffer 렌더링, 조명 누적, 최종 합성의 연결 | `Engine/Private/Renderer.cpp` |
| [05_Rendering/ToonLighting.hlsl](CodeSamples/Private/05_Rendering/ToonLighting.hlsl) | 단계형 툰 음영과 재질 버퍼를 이용한 최종 합성 | `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl` |
| [05_Rendering/CharacterMaterial.hlsl](CodeSamples/Private/05_Rendering/CharacterMaterial.hlsl) | MatCap 하이라이트와 얼굴 라이트맵 값 계산 | `Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl` |

## 헤더 대응 관계

| 샘플 폴더 | 포함한 헤더 |
| --- | --- |
| 01_Editor | [Editor_Controller.h](CodeSamples/Public/01_Editor/Editor_Controller.h), [Editor_Bridge.h](CodeSamples/Public/01_Editor/Editor_Bridge.h), [Editor_Navigation.h](CodeSamples/Public/01_Editor/Editor_Navigation.h) |
| 02_Animation | [Model.h](CodeSamples/Public/02_Animation/Model.h), [Character.h](CodeSamples/Public/02_Animation/Character.h), [Miyabi.h](CodeSamples/Public/02_Animation/Miyabi.h) |
| 03_Combat | [CombatSystem_Manager.h](CodeSamples/Public/03_Combat/CombatSystem_Manager.h), [Miyabi.h](CodeSamples/Public/03_Combat/Miyabi.h), [TimeScale_Manager.h](CodeSamples/Public/03_Combat/TimeScale_Manager.h) |
| 04_Camera | [InGameCamera.h](CodeSamples/Public/04_Camera/InGameCamera.h), [CameraAnimClip.h](CodeSamples/Public/04_Camera/CameraAnimClip.h) |
| 05_Rendering | [Renderer.h](CodeSamples/Public/05_Rendering/Renderer.h) |

`Model_Animation.cpp`는 `Model.h`, `Character_RootMotion.cpp`는 `Character.h`에 대응합니다. `Miyabi_Combo.cpp`와 `Miyabi_Defense.cpp`는 같은 클래스의 발췌이므로 두 폴더에 동일한 `Miyabi.h`를 넣었습니다. 헤더에는 발췌하지 않은 함수의 선언도 포함되어 있습니다. HLSL 샘플의 공통 셰이더 정의 파일은 이번 C++ 클래스 헤더 추가 범위에 포함하지 않았습니다.

## 발췌 기준과 범위

- 함수 본문은 원본 그대로 유지했습니다. 생략한 부분은 `[중략]`으로 표시하고, 각 함수 위에 발췌 당시 원본 행 번호를 적었습니다.
- 대응하는 원본 클래스 헤더를 각 샘플 폴더에 포함했습니다. 헤더 내용은 원본 그대로이며 UTF-8로 저장했습니다. 공통 의존 헤더, 초기화 코드와 일부 보조 구현은 포함하지 않아 단독 빌드는 지원하지 않습니다.
- 파일명은 설명할 기능에 맞춰 나눴습니다. `Miyabi_Combo.cpp`와 `Miyabi_Defense.cpp`는 모두 원본 `Miyabi.cpp`의 일부입니다.
- `Model_Animation.cpp`의 오버레이 갱신·본 행렬 갱신 등 보조 함수는 생략했습니다. 이 샘플의 설명 범위는 블렌딩과 루트 이동량 추출입니다.
- 셰이더는 함수만 발췌했습니다. 전역 변수·입출력 구조체·렌더 상태·패스 정의는 포함하지 않았습니다.
- 저스트 회피 샘플은 현재 판정과 시간 배율 적용까지를 보여줍니다. 원본의 후속 화면 효과 TODO는 그대로 남겼습니다.
- 본문이 비어 있는 락온 요청 함수와 위치가 확인되지 않은 소스 템플릿 자동 생성 기능은 제외했습니다.
- 외부 라이브러리 소스, 모델·텍스처·음원, 실행 파일과 빌드 산출물은 포함하지 않았습니다.

현재 문서는 코드 샘플 안내입니다. 최종 포트폴리오 소개에는 개발 기간, 시연 GIF와 함께 직접 구현·확장한 범위를 정리할 예정입니다.