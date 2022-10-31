#include <vector>
#include "Path.h"

#include "imgui/imgui.h"

static dx::XMMATRIX bezier_mat(
	-1, 3, -3, 1,
	3, -6, 3, 0,
	-3, 3, 0, 0,
	1, 0, 0, 0
);

void Path::AddControlPoint(const dx::XMFLOAT3& new_point) {
	control_points.push_back(new_point);
}

void Path::SetSubdivisionCount(int _subdivision_count) {
	subdivision_count = _subdivision_count;
}

std::vector<dx::XMFLOAT3> Path::GeneratePath() {
	std::vector<dx::XMFLOAT3> path_points;
	for (unsigned int i = 0; i < control_points.size(); i++) {
		dx::XMVECTOR pi_prev;
		dx::XMVECTOR pi;
		dx::XMVECTOR pi_next;
		dx::XMVECTOR pi_next_next;

		GetPointsFromIndex(i, pi_prev, pi, pi_next, pi_next_next);
		
		// itr = ((Pi+1 - Pi-1) / k);
		dx::XMVECTOR itr_a = dx::XMVectorScale(
			dx::XMVectorSubtract(pi_next, pi_prev), 1.0f / k);

		// a = Pi + ((Pi+1 - Pi-1) / k);
		dx::XMVECTOR a = dx::XMVectorAdd(pi, itr_a);

		// itr = ((Pi+2 - Pi) / k);
		dx::XMVECTOR itr_b = dx::XMVectorScale(
			dx::XMVectorSubtract(pi_next_next, pi), 1.0f / k);

		// b = Pi+1 - ((Pi+2 - Pi) / k);
		dx::XMVECTOR b = dx::XMVectorSubtract(pi_next, itr_b);

		for (int j = 0; j < subdivision_count; j++) {
			float u = (float)j / subdivision_count;
			dx::XMVECTOR new_point = BezierFunc(u, pi, a, b, pi_next);
			path_points.push_back(
				dx::XMFLOAT3(
					dx::XMVectorGetX(new_point),
					dx::XMVectorGetY(new_point),
					dx::XMVectorGetZ(new_point)));
		}
	}
	return path_points;
}

void Path::Update(float dt) {
	ShowPathControls();
}

void Path::ShowPathControls() {
	if (ImGui::Begin("Path controls")) {
		if (ImGui::Button("Add Control Point")) {
			AddControlPoint(dx::XMFLOAT3());
		}
		for (unsigned int indx = 0; indx < control_points.size(); indx++) {
			ImGui::Text("Control Point P%d", indx);
			ImGui::SliderFloat("Point X", &control_points[indx].x, -100.0f, 100.0f);
			ImGui::SliderFloat("Point Y", &control_points[indx].y, -100.0f, 100.0f);
			ImGui::SliderFloat("Point Z", &control_points[indx].z, -100.0f, 100.0f);
		}
	}
	ImGui::End();
}

dx::XMVECTOR Path::GetPosition(float u) {
	float point_indx_f = u * control_points.size();
	int point_indx = (int)point_indx_f;
	float full_u = point_indx_f - point_indx;
	dx::XMVECTOR pi_prev;
	dx::XMVECTOR pi;
	dx::XMVECTOR pi_next;
	dx::XMVECTOR pi_next_next;

	GetPointsFromIndex(point_indx, pi_prev, pi, pi_next, pi_next_next);

	// itr = ((Pi+1 - Pi-1) / k);
	dx::XMVECTOR itr_a = dx::XMVectorScale(
		dx::XMVectorSubtract(pi_next, pi_prev), 1.0f / k);

	// a = Pi + ((Pi+1 - Pi-1) / k);
	dx::XMVECTOR a = dx::XMVectorAdd(pi, itr_a);

	// itr = ((Pi+2 - Pi) / k);
	dx::XMVECTOR itr_b = dx::XMVectorScale(
		dx::XMVectorSubtract(pi_next_next, pi), 1.0f / k);

	// b = Pi+1 - ((Pi+2 - Pi) / k);
	dx::XMVECTOR b = dx::XMVectorSubtract(pi_next, itr_b);

	return BezierFunc(full_u, pi, a, b, pi_next);
}

dx::XMVECTOR Path::BezierFunc(float u, 
	const dx::XMVECTOR& p0, const dx::XMVECTOR& p1, 
	const dx::XMVECTOR& p2, const dx::XMVECTOR& p3) {
	dx::XMVECTOR u_v = dx::XMVectorSet(u * u * u, u * u, u, 1);

	// X value for the point ----
	dx::XMVECTOR x_points = 
		dx::XMVectorSet(
			dx::XMVectorGetX(p0),
			dx::XMVectorGetX(p1),
			dx::XMVectorGetX(p2),
			dx::XMVectorGetX(p3));

	dx::XMVECTOR x_res = dx::XMVector4Transform(x_points, bezier_mat);

	x_res = dx::XMVector4Dot(u_v, x_res);
	// -----------

	// Y value for the point ----
	dx::XMVECTOR y_points = 
		dx::XMVectorSet(
			dx::XMVectorGetY(p0),
			dx::XMVectorGetY(p1),
			dx::XMVectorGetY(p2),
			dx::XMVectorGetY(p3));

	dx::XMVECTOR y_res = dx::XMVector4Transform(y_points, bezier_mat);

	y_res = dx::XMVector4Dot(u_v, y_res);
	// -----------

	// Z value for the point ----
	dx::XMVECTOR z_points = 
		dx::XMVectorSet(
			dx::XMVectorGetZ(p0),
			dx::XMVectorGetZ(p1),
			dx::XMVectorGetZ(p2),
			dx::XMVectorGetZ(p3));

	dx::XMVECTOR z_res = dx::XMVector4Transform(z_points, bezier_mat);

	z_res = dx::XMVector4Dot(u_v, z_res);
	// -----------

	return dx::XMVectorSet(
		dx::XMVectorGetX(x_res),
		dx::XMVectorGetX(y_res),
		dx::XMVectorGetX(z_res),
		1.0f);
}

void Path::AddDefaultControlPoints() {
	control_points.push_back(dx::XMFLOAT3(-20.0f, 0.0f, 0.0f));
	control_points.push_back(dx::XMFLOAT3(20.0f, 0.0f, 0.0f));
}

void Path::GetPointsFromIndex(unsigned int point_indx,
	dx::XMVECTOR& pi_prev, dx::XMVECTOR& pi, 
	dx::XMVECTOR& pi_next, dx::XMVECTOR& pi_next_next) {
	if (point_indx >= control_points.size())
		point_indx = 0;

	if (point_indx == 0)
		pi_prev = dx::XMLoadFloat3(&control_points[control_points.size() - 1]);
	else
		pi_prev = dx::XMLoadFloat3(&control_points[point_indx - 1]);

	pi = dx::XMLoadFloat3(&control_points[point_indx]);

	if (point_indx >= control_points.size() - 2)
		pi_next_next = dx::XMLoadFloat3(&control_points[0]);
	else
		pi_next_next = dx::XMLoadFloat3(&control_points[point_indx + 2]);

	if (point_indx >= control_points.size() - 1) {
		pi_next = dx::XMLoadFloat3(&control_points[0]);
		pi_next_next = dx::XMLoadFloat3(&control_points[1]);
	}
	else
		pi_next = dx::XMLoadFloat3(&control_points[point_indx + 1]);
}

Path::Path() : subdivision_count(50) {
}

Path::Path(int _subdivision_count) : subdivision_count(_subdivision_count) {
}
