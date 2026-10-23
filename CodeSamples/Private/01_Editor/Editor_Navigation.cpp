// 포트폴리오용 코드 발췌: 내비게이션 셀의 정점 스냅·유효성 검사·이웃 연결·저장
// 원본: Editor/Private/Editor_Navigation.cpp
// [중략] include·클래스/자료형 선언·초기화 및 관련 보조 구현은 전체 프로젝트에 있습니다.
// 아래 함수 본문은 원본 그대로이며, 이 파일만으로 빌드하거나 실행할 수 없습니다.

// 원본 8~19행: IsDegenerateTriangleXZ
_bool CEditor_Navigation::IsDegenerateTriangleXZ(const _float3* pPoints, _float fEpsilon)
{
	const _float3& A = pPoints[0];
	const _float3& B = pPoints[1];
	const _float3& C = pPoints[2];

	const _float fSignedArea =
		(B.x - A.x) * (C.z - A.z) -
		(B.z - A.z) * (C.x - A.x);

	return fabsf(fSignedArea) <= fEpsilon;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 21~26행: IsSamePointXZ
_bool CEditor_Navigation::IsSamePointXZ(const _float3& A, const _float3& B, _float fEpsilon)
{
	return fabsf(A.x - B.x) <= fEpsilon &&
		fabsf(A.z - B.z) <= fEpsilon &&
		fabsf(A.y - B.y) <= 0.05f;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 28~32행: IsSameEdge
_bool CEditor_Navigation::IsSameEdge(const _float3& A0, const _float3& A1, const _float3& B0, const _float3& B1)
{
	return (IsSamePointXZ(A0, B0) && IsSamePointXZ(A1, B1)) ||
		(IsSamePointXZ(A0, B1) && IsSamePointXZ(A1, B0));
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 34~62행: TrySnapToExistingPoint
_bool CEditor_Navigation::TrySnapToExistingPoint(_fvector vPickedPos, _float3& OutSnappedPoint)
{
	// 거리 제곱
	const _float fSnapDistSq = m_NavBuildData.fSnapDistance * m_NavBuildData.fSnapDistance;

	// 모든 셀 순회
	for (const auto& Cell : m_NavBuildData.Cells)
	{
		// 각 점 순회
		for (int i = 0; i < 3; i++)
		{
			// 클릭 위치 - 점 위치
			_vector vPoint = XMLoadFloat3(&Cell.vPoints[i]);
			_vector vDiff = vPickedPos - vPoint;

			// y 차이는 0
			vDiff = XMVectorSetY(vDiff, 0.f);

			// 스냅 범위 안이면
			if (XMVectorGetX(XMVector3LengthSq(vDiff)) <= fSnapDistSq)
			{
				OutSnappedPoint = Cell.vPoints[i];
				return true;
			}
		}
	}

	return false;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 64~78행: FixClockWiseXZ
void CEditor_Navigation::FixClockWiseXZ(_float3* pPoints)
{
	const _float3& A = pPoints[0];
	const _float3& B = pPoints[1];
	const _float3& C = pPoints[2];

	// 행렬식 계산 -> 두 벡터 사이의 간격이나 벌어진 정도 계산
	// 결과값은 두 벡터가 만드는 평행사변형의 넓이(방향 포함)
	// -> 음수면 오른쪽 방향 (시계), 양수면 왼쪽 방향(반시계)
	const _float fSignedArea = (B.x - A.x) * (C.z - A.z) - (B.z - A.z) * (C.x - A.x);

	if (fSignedArea > 0.f)
		std::swap(pPoints[1], pPoints[2]);

}

// [중략] 원본의 다른 함수 및 선언.

// 원본 81~124행: RebuildNeighbors
void CEditor_Navigation::RebuildNeighbors()
{
	// 초기화
	for (auto& Cell : m_NavBuildData.Cells)
	{
		Cell.iNeighborIndices[0] = -1;
		Cell.iNeighborIndices[1] = -1;
		Cell.iNeighborIndices[2] = -1;
	}

	for (size_t i = 0; i < m_NavBuildData.Cells.size(); i++)
	{
		auto& Sour = m_NavBuildData.Cells[i];

		for (size_t j = 0; j < m_NavBuildData.Cells.size(); j++)
		{
			if (i == j)
				continue;

			auto& Dest = m_NavBuildData.Cells[j];

			if (IsSameEdge(Sour.vPoints[0], Sour.vPoints[1], Dest.vPoints[0], Dest.vPoints[1]) ||
				IsSameEdge(Sour.vPoints[0], Sour.vPoints[1], Dest.vPoints[1], Dest.vPoints[2]) ||
				IsSameEdge(Sour.vPoints[0], Sour.vPoints[1], Dest.vPoints[2], Dest.vPoints[0]))
			{
				Sour.iNeighborIndices[0] = static_cast<_int>(j);
			}

			if (IsSameEdge(Sour.vPoints[1], Sour.vPoints[2], Dest.vPoints[0], Dest.vPoints[1]) ||
				IsSameEdge(Sour.vPoints[1], Sour.vPoints[2], Dest.vPoints[1], Dest.vPoints[2]) ||
				IsSameEdge(Sour.vPoints[1], Sour.vPoints[2], Dest.vPoints[2], Dest.vPoints[0]))
			{
				Sour.iNeighborIndices[1] = static_cast<_int>(j);
			}

			if (IsSameEdge(Sour.vPoints[2], Sour.vPoints[0], Dest.vPoints[0], Dest.vPoints[1]) ||
				IsSameEdge(Sour.vPoints[2], Sour.vPoints[0], Dest.vPoints[1], Dest.vPoints[2]) ||
				IsSameEdge(Sour.vPoints[2], Sour.vPoints[0], Dest.vPoints[2], Dest.vPoints[0]))
			{
				Sour.iNeighborIndices[2] = static_cast<_int>(j);
			}
		}
	}
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 126~155행: Add_Cell
HRESULT CEditor_Navigation::Add_Cell(_float3 vFinalPoint)
{
	m_NavBuildData.CurrentPickedPoints.push_back(vFinalPoint);

	if (m_NavBuildData.CurrentPickedPoints.size() < 3)
		return E_FAIL;

	_float3 vPoints[3] =
	{
		m_NavBuildData.CurrentPickedPoints[0],
		m_NavBuildData.CurrentPickedPoints[1],
		m_NavBuildData.CurrentPickedPoints[2]
	};

	if (IsDegenerateTriangleXZ(vPoints))
	{
		m_NavBuildData.CurrentPickedPoints.clear();
		return E_FAIL;
	}

	FixClockWiseXZ(vPoints);

	EDITOR_CELL_DATA NewCell = {};
	NewCell.vPoints[0] = vPoints[0];
	NewCell.vPoints[1] = vPoints[1];
	NewCell.vPoints[2] = vPoints[2];

	m_NavBuildData.Cells.push_back(NewCell);
	return S_OK;
}

// [중략] 원본의 다른 함수 및 선언.

// 원본 195~212행: Save_NavMesh_ToFile
HRESULT CEditor_Navigation::Save_NavMesh_ToFile(const char* pFilePath)
{
	ofstream ofs(pFilePath, ios::binary);
	if (!ofs.is_open())
		return E_FAIL;

	NAV_FILE_HEADER Header = {};
	Header.iCellCount = static_cast<_uint>(m_NavBuildData.Cells.size());

	ofs.write(reinterpret_cast<const char*>(&Header), sizeof(Header));

	for (const auto& Cell : m_NavBuildData.Cells)
	{
		ofs.write(reinterpret_cast<const char*>(&Cell), sizeof(EDITOR_CELL_DATA));
	}

	return ofs.good() ? S_OK : E_FAIL;
}

// [중략] 원본의 다른 함수 및 선언.
