#pragma once
#include <DirectXMath.h>
namespace dx = DirectX;

class Path
{
private:
	static const int k = 8;
	int subdivision_count;
	std::vector<dx::XMFLOAT3> control_points;
	static dx::XMVECTOR BezierFunc(float u, 
		const dx::XMVECTOR& p0, const dx::XMVECTOR& p1, 
		const dx::XMVECTOR& p2, const dx::XMVECTOR& p3);
	void AddDefaultControlPoints();
	void GetPointsFromIndex(unsigned int point_indx,
		dx::XMVECTOR& pi_prev, dx::XMVECTOR& pi,
		dx::XMVECTOR& pi_next, dx::XMVECTOR& pi_next_next);

public:
	Path();
	Path(int _subdivision_count);
	
	void AddControlPoint(const dx::XMFLOAT3& new_point);
	void SetSubdivisionCount(int _subdivision_count);
	std::vector<dx::XMFLOAT3> GeneratePath();
	void Update(float dt);
	void ShowPathControls();
	dx::XMVECTOR GetPosition(float u);
};

