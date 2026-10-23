#pragma once
#include "Editor_Defines.h"
#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Editor)

struct NAV_FILE_HEADER
{
	_uint iMagic = 0x56414E45; // 'ENAV' 같은 식
	_uint iVersion = 1;
	_uint iCellCount = 0;
};

struct EDITOR_CELL_DATA
{
	_float3 vPoints[3] = {};
	_int	iNeighborIndices[3] = { -1, -1, -1 };
};

struct EDITOR_NAV_BUILD_DATA
{
	vector<EDITOR_CELL_DATA> Cells;
	vector<_float3> CurrentPickedPoints;
	_float fSnapDistance = 0.25f;
};

class CEditor_Navigation : public CBase
{
private:
	CEditor_Navigation() = default;
	~CEditor_Navigation() = default;


public:
	const EDITOR_NAV_BUILD_DATA& Get_NavData() const { return m_NavBuildData; }

public:
	HRESULT		 Initialize();

	_bool		 IsDegenerateTriangleXZ(const _float3* pPoints, _float fEpsilon = 0.001f);
	_bool		 IsSamePointXZ(const _float3& A, const _float3& B, _float fEpsilon = 0.001f);
	_bool		 IsSameEdge(const _float3& A0, const _float3& A1, const _float3& B0, const _float3& B1);
	
	_bool	     TrySnapToExistingPoint(_fvector vPickedPos, _float3& OutSnappedPoint);
	
	void		 FixClockWiseXZ(_float3* pPoints);
	void		 RebuildNeighbors();

	void		 Clear_Cells();
	void		 Clear_PickedCell();
	void		 Remove_LastCell();
	void		 Undo_LastPoint();

	HRESULT		 Add_Cell(_float3 vPoints);
	HRESULT		 Block_SameInput(_float3 vInputPoint);
	HRESULT		 Save_NavMesh_ToFile(const char* pFilePath);

private:
	EDITOR_NAV_BUILD_DATA m_NavBuildData = {};

public:
	static CEditor_Navigation* Create();
	virtual void Free() override;
};

NS_END