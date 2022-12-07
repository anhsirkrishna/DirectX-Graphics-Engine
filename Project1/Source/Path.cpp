#include <vector>
#include <queue>
#include <iostream>
#include "Path.h"

#include "imgui/imgui.h"

static dx::XMMATRIX bezier_mat(
	-1, 3, -3, 1,
	3, -6, 3, 0,
	-3, 3, 0, 0,
	1, 0, 0, 0
);

static float lerp(float a, float b, float t) {
	return (t * a) + ((1 - t) * b);
}

/*
* Resets the path time
*/
void Path::Reset() {
	path_time = 0;
}

void Path::AddControlPoint(const dx::XMFLOAT3& new_point, unsigned int segment) {
	control_segments[segment].push_back(new_point);
}

void Path::PopControlPoint(unsigned int segment) {
	control_segments[segment].pop_back();
}

/*
* Gets a list of all the control points
* Returns: std::vector<dx::XMFLOAT3> List of all control points
*/
std::vector<dx::XMFLOAT3> Path::GetAllControlPoints() {
	std::vector<dx::XMFLOAT3> ret_list;
	for (auto& segment : control_segments) {
		for (auto& control_points : segment) {
			ret_list.push_back(control_points);
		}
	}
	return ret_list;
}

/*
* Replaces the last point in the segment,
* accounting for the appended extra points.
*/
void Path::ReplaceLastPoint(const dx::XMFLOAT3& new_point, unsigned int segment) {
	unsigned int segment_size = control_segments[segment].size();
	if (loop)
		control_segments[segment][segment_size - 1] = new_point;
	else {
		//Pop the extra appended point and the last point
		control_segments[segment].pop_back();
		control_segments[segment].pop_back();

		//Add the new last point
		dx::XMFLOAT3 pn_prev = control_segments[segment].back();
		control_segments[segment].push_back(new_point);

		dx::XMFLOAT3 pn = new_point;

		//Caclulate the new final point to append
		dx::XMFLOAT3 diff(pn.x - pn_prev.x, pn.y - pn_prev.y, pn.z - pn_prev.z);
		dx::XMFLOAT3 pn_next(pn.x + diff.x, pn.y + diff.y, pn.z + diff.z);

		//Append the new final point
		control_segments[segment].push_back(pn_next);
	}
}

void Path::AddControlSegment() {
	control_segments.push_back(control_points());
}

void Path::SetSubdivisionCount(int _subdivision_count) {
	subdivision_count = _subdivision_count;
}

std::vector<dx::XMFLOAT3> Path::GeneratePath() {
	//If it's not a loop we need to add an extra point to the start and end of the path
	if (not loop) {
		AppendExtraPoints();
		//Generate the path using a different method if it's not looped
		return GenerateUnloopedPath();
	}
		

	std::vector<dx::XMFLOAT3> path_points;
	for (unsigned int j = 0; j < control_segments.size(); j++) {
		for (unsigned int i = 0; i < control_segments[j].size(); i++) {
			dx::XMVECTOR pi_prev;
			dx::XMVECTOR pi;
			dx::XMVECTOR pi_next;
			dx::XMVECTOR pi_next_next;

			GetPointsFromIndex(i, j, pi_prev, pi, pi_next, pi_next_next);

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
	}
	return path_points;
}

/*
* Generates a list of points that represents the path in world space.
* Constructs the path from the control_points consisting of a single
* segment with no loops.
* Returns: Vector<FLOAT3> - the list of points
*/
std::vector<dx::XMFLOAT3> Path::GenerateUnloopedPath() {
	std::vector<dx::XMFLOAT3> path_points;
	for (unsigned int i = 1; i < control_segments[0].size() - 2; i++) {
		dx::XMVECTOR pi_prev;
		dx::XMVECTOR pi;
		dx::XMVECTOR pi_next;
		dx::XMVECTOR pi_next_next;

		GetPointsFromIndexNoLoop(i, pi_prev, pi, pi_next, pi_next_next);

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

/*
* Generates the forward difference table to evalute the distance for the
* space curve using the adaptive approach.
* If use_adaptive = true then expand the table using adapative approach
* Returns: void
*/
void Path::GenerateForwardDiffTable(bool use_adaptive) {
	const float delta_u = 0.01f;
	unsigned int n = 1 / delta_u;
	for (unsigned int j = 0; j < control_segments.size(); j++) {
		//Create a forward difference table for each segment
		fwrd_diff_tables.push_back(forward_diff_table());
		fwrd_diff_tables.back().insert(std::make_pair(0.0f, 0.0f));

		float Si;
		float Ui = 0.0f;
		float Ui_prev = Ui;
		float approx_distance;
		for (unsigned int i = 1; i < n; i++) {
			Ui = Ui_prev + delta_u;
			//approx_distance = ||P(Ui) - P(Ui-1)||
			approx_distance = dx::XMVectorGetX(dx::XMVector4Length(
				dx::XMVectorSubtract(GetSegmentPosition(Ui, j), GetSegmentPosition(Ui_prev,  j))));
			Si = GetSegmentDistance(Ui_prev, j) + approx_distance;
			fwrd_diff_tables.back().insert(std::make_pair(Ui, Si));
			Ui_prev = Ui;
		}
		//Insert one more for Ui = 1
		Ui = 1.0f;
		//approx_distance = ||P(Ui) - P(Ui-1)||
		approx_distance = dx::XMVectorGetX(dx::XMVector4Length(
			dx::XMVectorSubtract(GetSegmentPosition(Ui, j), GetSegmentPosition(Ui_prev, j))));
		Si = GetSegmentDistance(Ui_prev, j) + approx_distance;
		fwrd_diff_tables.back().insert(std::make_pair(Ui, Si));

	}

	if (use_adaptive)
		AdaptiveExpand();
}

void Path::GenerateDefaultVelocityFunction() {
	velocity_function.clear();

	const float t1 = 0.3;
	const float t2 = 0.6;
	const float Vc = constant_velocity;

	//Accelerate to Vc until t1
	velocity_function.insert(
		{ t1, Vc }
	);
	//Maintain constant Vc until t2
	velocity_function.insert(
		{ t2, Vc }
	);
	//Deccelerate to 0 until 1
	velocity_function.insert(
		{ 1, 0 }
	);

	max_distance = GetDistanceFromTime(loop_time);
}

void Path::Update(float dt) {
	path_time += dt;
}

void Path::Scale(float s_x, float s_y, float s_z) {
	for (unsigned int j = 0; j < control_segments.size(); j++) {
		for (unsigned int i = 0; i < control_segments[j].size(); i++) {
			control_segments[j][i].x *= s_x;
			control_segments[j][i].y *= s_y;
			control_segments[j][i].z *= s_z;
		}
	}
}

/*
* Imgui controls for adding/removing control points and controlling their positions.
*/
void Path::ShowPathControls() {
	//if (ImGui::Begin("Path controls")) {
	//	/*if (ImGui::Button("Add Control Point")) {
	//		AddControlPoint(dx::XMFLOAT3());
	//	}
	//	for (unsigned int indx = 0; indx < control_points.size(); indx++) {
	//		ImGui::Text("Control Point P%d", indx);
	//		ImGui::SliderFloat("Point X", &control_points[indx].x, -100.0f, 100.0f);
	//		ImGui::SliderFloat("Point Y", &control_points[indx].y, -100.0f, 100.0f);
	//		ImGui::SliderFloat("Point Z", &control_points[indx].z, -100.0f, 100.0f);
	//	}*/
	//}
	//ImGui::End();
}

dx::XMVECTOR Path::GetSegmentPosition(float u, unsigned int segment_indx) {
	float point_indx_f = 0.0f;
	int point_indx = 0;
	float final_u = 0.0f;
	if (u == 1) {
		//If it's not a loop then discount the first and last points 
		// as they are not part of the path
		if (not loop)
			point_indx = control_segments[segment_indx].size() - 3;
		else
			point_indx = control_segments[segment_indx].size() - 1;
		final_u = 1;
	}
	else if (u == 0) {
		//If it's not a loop then discount the first and last points 
		// as they are not part of the path
		if (not loop) 
			point_indx = 1;
		else 
			point_indx = 0;
		final_u = 0;
	}
	else {
		if (not loop) {
			//If it's not a loop then discount the first and last points 
			// as they are not part of the path
			point_indx_f = u * (control_segments[segment_indx].size() - 3);
			point_indx = (int)point_indx_f;
			final_u = point_indx_f - point_indx;
			point_indx += 1;
		}
		else {
			point_indx_f = u * control_segments[segment_indx].size();
			point_indx = (int)point_indx_f;
			final_u = point_indx_f - point_indx;
		}
	}

	dx::XMVECTOR pi_prev;
	dx::XMVECTOR pi;
	dx::XMVECTOR pi_next;
	dx::XMVECTOR pi_next_next;
	
	if (not loop)
		GetPointsFromIndexNoLoop(point_indx, pi_prev, pi, pi_next, pi_next_next);
	else
		GetPointsFromIndex(point_indx, segment_indx, pi_prev, pi, pi_next, pi_next_next);

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

	return BezierFunc(final_u, pi, a, b, pi_next);
}

dx::XMVECTOR Path::GetPosition(float u) {
	int segment_indx, point_indx;
	float point_u = 0.0f;
	if (u == 1) {
		segment_indx = control_segments.size() - 1;
		point_u = 1;
	}
	else if (u == 0) {
		segment_indx = 0;
		point_u = 0;
	}
	else {
		float segment_indx_f = u * control_segments.size();
		segment_indx = (int)segment_indx_f;
		point_u = segment_indx_f - segment_indx;
	}
	
	return GetSegmentPosition(point_u, segment_indx);
}

float Path::GetSegmentDistance(float u, unsigned int segment_indx) {
	//Get corresponding segment forward diff table and
	//access it with the closest Ui >= U
	//Access is O(logN)
	auto Ui_iter = fwrd_diff_tables[segment_indx].lower_bound(u);
	float Ui = (*Ui_iter).first;

	//Found an exact match in the forward difference table
	if (Ui == u) {
		//Return corresponding Si
		return (*Ui_iter).second;
	}
	//No exact match, interpolate the distance between Ui and Ui-1
	else {
		float Si = (*Ui_iter).second;
		//Get Ui-1
		float Ui_prev = (*(--Ui_iter)).first;
		float t_u = (u - Ui_prev) / (Ui - Ui_prev);
		//Lerp between Si and Si-1
		float Si_prev = (*Ui_iter).second;
		return lerp(Si_prev, Si, t_u);;
	}
}

float Path::GetDistance(float u) {
	int segment_indx;
	float segment_u;
	//Convert normalized u to segment table u
	if (u == 1) {
		segment_indx = control_segments.size() - 1;
		segment_u = 1;
	}
	else {
		float t_u = u * control_segments.size();
		segment_indx = (int)t_u;
		segment_u = t_u - segment_indx;
	}
	

	//Concatenate the previous segment table distances
	float concat_distance = 0.0f;
	for (unsigned int i = 0; i < segment_indx; i++) {
		concat_distance += GetSegmentDistance(1, i);
	}
	
	return (GetSegmentDistance(segment_u, segment_indx) + concat_distance);
}

/*
* Returns the corresponding U for the given distance.
* Uses Bisect root finding algorithm
* I.e inverse arc length function
* Returns: float - u
*/
float Path::GetU(float S) {
	const float threshold = 0.00001f;
	const float compare_threshold = 0.0001f;
	float Ua = 0.0f;
	float Ub = 1.0f;
	float Um, Sm, Um_prev = 0.0f;
	do {
		Um = (Ub + Ua) / 2;

		//Check to prevent infinite loops.
		if (abs(Um - Um_prev) < compare_threshold)
			break;

		Sm = GetDistance(Um);
		if (Sm < S)
			Ua = Um;
		else
			Ub = Um;
		Um_prev = Um;
	} while (abs(S - Sm) > threshold);
	return Um;
}

float Path::GetVelocity(float t) {
	float normalized_t = GetNormalizedT(t);

	//Get Ti and Ti-1 where Ti-1 <= t <= Ti
	auto velo_func_iter = velocity_function.lower_bound(normalized_t);
	float curr_velo = (*velo_func_iter).second;
	float curr_t = (*velo_func_iter).first;
	float prev_velo, prev_t;
	if (velo_func_iter == velocity_function.begin()) {
		prev_velo = 0;
		prev_t = 0;
	}
	else {
		--velo_func_iter;
		prev_velo = (*velo_func_iter).second;
		prev_t = (*velo_func_iter).first;
	}
	// Vi(t) = ((Vi - Vi-1)/(ti - ti-1))(t - ti-1) + Vi-1
	float velo_change = (((curr_velo - prev_velo) * (normalized_t - prev_t)) /
							(curr_t - prev_t));
	return  velo_change + prev_velo;
}

float Path::GetDistanceFromTime(float t) {
	//S(0) = 0
	if (t == 0)
		return 0;

	float normalized_t = GetNormalizedT(t);

	//Get Ti and Ti-1 where Ti-1 <= t <= Ti
	auto velo_func_iter = velocity_function.lower_bound(normalized_t);
	
	float curr_velo = (*velo_func_iter).second;
	float curr_t = (*velo_func_iter).first;
	float prev_velo, prev_t;
	if (velo_func_iter == velocity_function.begin()) {
		prev_velo = 0;
		prev_t = 0;
	}
	else {
		--velo_func_iter;
		prev_velo = (*velo_func_iter).second;
		prev_t = (*velo_func_iter).first;
	}

	float distance = ((curr_velo - prev_velo) / (curr_t - prev_t)) * 
					 (((normalized_t * normalized_t) / 2) - (normalized_t * prev_t)) + 
					 (prev_velo * normalized_t);
	
	/*float distance = (0.5f * (curr_velo - prev_velo) * (normalized_t - prev_t)) +
		(prev_velo * (curr_velo - prev_velo));*/

	float prev_distance = GetDistanceFromTime(prev_t*loop_time);

	return distance + prev_distance;
}

float Path::GetNormalizedDistanceFromTime(float t) {
	float distance = GetDistanceFromTime(t);
	return distance / max_distance;
}

float Path::GetSinDistanceFromTime(float t) {
	float normalized_t = GetNormalizedT(t);
	return (dx::XMScalarSin(dx::XM_PI * normalized_t - dx::XM_PIDIV2) + 1) / 2;
}

float Path::GetCurrentPathTime() {
	return path_time;
}

dx::XMVECTOR Path::GetCurrentPosition() {
	/*float curr_distance = GetDistanceFromTime(path_time);*/
	float norm_distance = GetSinDistanceFromTime(path_time);
	float curr_distance = norm_distance * GetDistance(1);
	float curr_u = GetU(curr_distance);
	return GetPosition(curr_u);
}

dx::XMVECTOR Path::GetLookPosition() {
	unsigned int average_count = 3;
	float norm_distance;
	dx::XMVECTOR pos = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	for (unsigned int i = 1; i <= average_count; i++) {
		norm_distance = GetSinDistanceFromTime(path_time + (i*constant_look_ahead_time));
		float curr_distance = norm_distance * GetDistance(1);
		float curr_u = GetU(curr_distance);
		pos = dx::XMVectorAdd(GetPosition(curr_u), pos);
	}
	return dx::XMVectorScale(pos, 1.0f / average_count);
}

float Path::GetCurrentVelocity() {
	return GetVelocity(path_time);
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
	
}

void Path::GetPointsFromIndex(
	unsigned int point_indx, unsigned int segment_indx,
	dx::XMVECTOR& pi_prev, dx::XMVECTOR& pi, 
	dx::XMVECTOR& pi_next, dx::XMVECTOR& pi_next_next) {

	//If the path is not a loop use a different function
	if (!loop)
		GetPointsFromIndexNoLoop(point_indx, pi_prev, pi, pi_next, pi_next_next);

	unsigned int next_segment_indx = segment_indx + 1;
	unsigned int prev_segment_indx = segment_indx - 1;

	if (next_segment_indx == control_segments.size())
		next_segment_indx = 0;
	
	if (prev_segment_indx == UINT_MAX)
		prev_segment_indx = control_segments.size() - 1;

	//If the point is the first point in the control point list,
	//get the last point from the previous segment
	if (point_indx == 0) 
		pi_prev = dx::XMLoadFloat3(&control_segments[prev_segment_indx].back());
	else
		pi_prev = dx::XMLoadFloat3(&control_segments[segment_indx][point_indx - 1]);

	pi = dx::XMLoadFloat3(&control_segments[segment_indx][point_indx]);

	//If the point is the last point in the control point index,
	//Get the next point and next_next point from the next segment 
	if (point_indx == control_segments[segment_indx].size() - 1) {
		pi_next = dx::XMLoadFloat3(&control_segments[next_segment_indx][0]);
		pi_next_next = dx::XMLoadFloat3(&control_segments[next_segment_indx][1]);
	}
	//If the point is the second last point in the control point index,
	//Get the next point from the current segment and,
	//the next_next point from the next segment
	else if (point_indx == control_segments[segment_indx].size() - 2) {
		pi_next = dx::XMLoadFloat3(&control_segments[segment_indx].back());
		pi_next_next = dx::XMLoadFloat3(&control_segments[next_segment_indx][0]);
	}
	else {
		pi_next = dx::XMLoadFloat3(&control_segments[segment_indx][point_indx + 1]);
		pi_next_next = dx::XMLoadFloat3(&control_segments[segment_indx][point_indx + 2]);
	}
}

void Path::GetPointsFromIndexNoLoop(unsigned int point_indx, 
	dx::XMVECTOR& pi_prev, dx::XMVECTOR& pi, 
	dx::XMVECTOR& pi_next, dx::XMVECTOR& pi_next_next) {
	pi_prev = dx::XMLoadFloat3(&control_segments[0][point_indx - 1]);
	pi = dx::XMLoadFloat3(&control_segments[0][point_indx]);
	pi_next = dx::XMLoadFloat3(&control_segments[0][point_indx] + 1);

	if (point_indx == control_segments[0].size() - 2) {
		//Calculate a new point for a last point 
		pi_next_next = dx::XMVectorAdd(pi_next, dx::XMVectorSubtract(pi_next, pi));
	}
	else
		pi_next_next = dx::XMLoadFloat3(&control_segments[0][point_indx] + 2);
}

void Path::AdaptiveExpand() {
	const float len_diff_threshold = 1.0f;
	const float U_diff_threshold = 0.01f;
	unsigned int segment_indx = 0;
	for (auto& fwrd_diff_table : fwrd_diff_tables) {
		std::queue<std::pair<float, float>> segment_list;
		segment_list.push(std::make_pair(0.0f, 1.0f));
		float Um, Ua, Ub;
		float A, B, C, D;
		while (!segment_list.empty()) {
			Ua = segment_list.front().first;
			Ub = segment_list.front().second;
			segment_list.pop();
			//Find middle U
			Um = ((Ua + Ub) / 2);
			//Calculate A, B, C and D
			//A = |P(Ua) - P(Um)|
			A = dx::XMVectorGetX(dx::XMVector4Length(
				dx::XMVectorSubtract(GetSegmentPosition(Ua, segment_indx), 
									 GetSegmentPosition(Um, segment_indx))));

			//B = |P(Um) - P(Ub)|
			B = dx::XMVectorGetX(dx::XMVector4Length(
				dx::XMVectorSubtract(GetSegmentPosition(Um, segment_indx), 
									 GetSegmentPosition(Ub, segment_indx))));

			//C = |P(Ua) - P(Ub)|
			C = dx::XMVectorGetX(dx::XMVector4Length(
				dx::XMVectorSubtract(GetSegmentPosition(Ua, segment_indx),
									 GetSegmentPosition(Ub, segment_indx))));

			//D = |A + B - C|
			D = abs(A + B - C);

			if (D > len_diff_threshold ||
				abs(Ub - Ua) > U_diff_threshold) {
				segment_list.push(std::make_pair(Ua, Um));
				segment_list.push(std::make_pair(Um, Ub));
			}
			else {
				fwrd_diff_table.insert(
					std::make_pair(Um, A + GetSegmentDistance(Ua, segment_indx)));
				fwrd_diff_table.insert(
					std::make_pair(Ub, B + GetSegmentDistance(Um, segment_indx)));
			}
		}
		segment_indx++;
	}
}

/*
* Append extra points to the start and end of the path if there's no loop
* for the bezier function
*/
void Path::AppendExtraPoints() {
	
	dx::XMFLOAT3 p0 = control_segments[0][0];
	dx::XMFLOAT3 p1 = control_segments[0][1];

	//Calculate the position for the point to be appended at the start
	dx::XMFLOAT3 diff(p0.x - p1.x, p0.y - p1.y, p0.z - p1.z);
	dx::XMFLOAT3 p0_prev(p0.x + diff.x, p0.y + diff.y, p0.z + diff.z);

	//Get the last two points in the path to calculate the new last point
	unsigned int point_list_size = control_segments[0].size();
	dx::XMFLOAT3 pn = control_segments[0][point_list_size - 1];
	dx::XMFLOAT3 pn_prev = control_segments[0][point_list_size - 2];

	//Add an extra slot at the end
	control_segments[0].push_back(dx::XMFLOAT3());

	//Shift the control points to make room for the new starting point
	for (unsigned int i = point_list_size; i > 0; --i) {
		control_segments[0][i] = control_segments[0][i - 1];
	}

	//Place the control point at the start
	control_segments[0][0] = p0_prev;

	//Caclulate the new final point to append
	diff = dx::XMFLOAT3(pn.x - pn_prev.x, pn.y - pn_prev.y, pn.z - pn_prev.z);
	dx::XMFLOAT3 pn_next(pn.x + diff.x, pn.y + diff.y, pn.z + diff.z);

	//Append the new final point
	control_segments[0].push_back(pn_next);
}

float Path::GetNormalizedT(float t) {
	float normalized_t;
	if (t <= loop_time) {
		normalized_t = t / loop_time;
	}
	else {
		float temp = t / loop_time;
		int temp_i = temp;
		normalized_t = (temp - temp_i);
	}
	return normalized_t;
}

Path::Path() : subdivision_count(50), loop(true) {
}

Path::Path(bool _loop) : subdivision_count(50), loop(_loop) {
}

Path::Path(int _subdivision_count) : subdivision_count(_subdivision_count) {
}
