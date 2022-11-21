#pragma once
#include <DirectXMath.h>
#include <map>

namespace dx = DirectX;

class Path
{
private:
	static const int k = 8;
	int subdivision_count;
	
	bool loop;
	/*
	* A list of control segments.
	* Each control segment is a list of control points.
	* We need separate control segments for the distance function.
	* Since the forward difference table cannot handle loopbacks.
	*/
	typedef std::vector<dx::XMFLOAT3> control_points;
	std::vector<control_points> control_segments;
	
	/*
	* A list of forward difference tables.
	* Each forward difference table is used to evaluate the distance function
	* for a particular segment.
	* Final distance is calculated by concatenating the distances.
	*/
	typedef std::map<float, float> forward_diff_table;
	std::vector<forward_diff_table> fwrd_diff_tables;
	
	//The constant velocity for movement 
	std::map<float, float> velocity_function;
	
	float path_time = 0.0f;

	//max distance returned by the distance function;
	float max_distance;

	/*
	* Evaluates the polynomial using a bezier matrix and 4 points
	* Returns: VECTOR  - the point evaluated at u
	*/
	static dx::XMVECTOR BezierFunc(float u, 
		const dx::XMVECTOR& p0, const dx::XMVECTOR& p1, 
		const dx::XMVECTOR& p2, const dx::XMVECTOR& p3);

	void AddDefaultControlPoints();
	
	/*
	* Helper function to determine which 4 control points to use 
	* to calculate the a and b points for Bezier evaluation.
	*/
	void GetPointsFromIndex(
		unsigned int point_indx, unsigned int segment_indx,
		dx::XMVECTOR& pi_prev, dx::XMVECTOR& pi,
		dx::XMVECTOR& pi_next, dx::XMVECTOR& pi_next_next);

	/*
	* Helper function to determine which 4 control points to use 
	* to calculate the a and b points for Bezier evaluation.
	* Used when the path consists of a single segment with no loop
	*/
	void GetPointsFromIndexNoLoop(
		unsigned int point_indx, 
		dx::XMVECTOR& pi_prev, dx::XMVECTOR& pi,
		dx::XMVECTOR& pi_next, dx::XMVECTOR& pi_next_next);

	/*
	* Expand the forward difference table using the adaptive approach
	*/
	void AdaptiveExpand();

	/*
	* Append extra points to the start and end of the path if there's no loop
	* for the bezier function
	*/
	void AppendExtraPoints();

public:
	float constant_velocity = 670.0f;
	float constant_look_ahead_time = 0.2f;
	//The time taken for 1 complete loop. Used to normalize t
	float loop_time = 30.0f;

	/*
	* Converts a T value to the noramlized 0-1 range based on loop_time;
	*/
	float GetNormalizedT(float t);
	Path();
	Path(bool _loop);
	Path(int _subdivision_count);
	
	//Adds a control point to the list of control points
	void AddControlPoint(const dx::XMFLOAT3& new_point, unsigned int segment);

	//Adds an empty control segment to the list of control segments
	void AddControlSegment();

	//Specifies the number of subdivisions of line segments between 
	//two control points. Increase the count for a smoother appearing path.
	void SetSubdivisionCount(int _subdivision_count);
	/*
	* Generates a list of points that represents the path in world space.
	* Constructs the path from the control_points
	* Returns: Vector<FLOAT3> - the list of points
	*/
	std::vector<dx::XMFLOAT3> GeneratePath();

	/*
	* Generates a list of points that represents the path in world space.
	* Constructs the path from the control_points consisting of a single 
	* segment with no loops.
	* Returns: Vector<FLOAT3> - the list of points
	*/
	std::vector<dx::XMFLOAT3> GenerateUnloopedPath();

	/*
	* Generates the forward difference table to evalute the distance for the 
	* space curve using the adaptive approach.
	* If use_adaptive = true then expand the table using adapative approach
	* Returns: void
	*/
	void GenerateForwardDiffTable(bool use_adaptive=true);

	/*
	* Generates the default velocity function 
	* Returns: void
	*/
	void GenerateDefaultVelocityFunction();

	void Update(float dt);

	/*
	* Scale all the control points in the path by the specified values
	* Returns: void
	*/
	void Scale(float x, float y, float z);

	/*
	* Imgui controls for adding/removing control points and controlling their positions.
	*/
	void ShowPathControls();

	/*
	* Returns the position in world space of the path for the 
	* corresponding u within a segment
	* Returns: VECTOR - position in world space
	*/
	dx::XMVECTOR GetSegmentPosition(float u, unsigned int segment_indx);
	
	/*
	* Returns the position in world space of the path for the corresponding u
	* P(u) function
	* Returns: VECTOR - position in world space
	*/
	dx::XMVECTOR GetPosition(float u);

	/*
	* Returns the distance on the path for the corresponding u within a segment
	* G(u) function
	* Returns: float - distance on the path
	*/
	float GetSegmentDistance(float u, unsigned int segment_indx);

	/*
	* Returns the distance on the path for the corresponding u
	* G(u) function
	* Returns: float - distance on the path
	*/
	float GetDistance(float u);

	/*
	* Returns the corresponding U for the given distance. 
	* I.e inverse arc length function
	* Uses Bisect root finding algorithm
	* Returns: float - u 
	*/
	float GetU(float S);

	/*
	* Returns the velocity for the corresponding time elapsed.
	* i.e the Velocity function V(t)
	* Uses parabolic ease-in/out approach
	* Returns: float - velocity
	*/
	float GetVelocity(float t);

	/*
	* Returns the distance travelled after the given time t
	* i.e Distance-time function S(t)
	* Uses parabolic ease-in/out approach
	* Returns: float - distance
	*/
	float GetDistanceFromTime(float t);

	/*
	* Returns the normalized distance travelled after the given time t
	* i.e Distance-time function S(t)
	* Uses parabolic ease-in/out approach
	* Returns: float - distance
	*/
	float GetNormalizedDistanceFromTime(float t);

	/*
	* Return the distance travelled after the given time t
	* Uses sin interpolation
	* Returns: float - normalized distance
	*/
	float GetSinDistanceFromTime(float t);

	//Gets the current path time
	float GetCurrentPathTime();

	/*
	* Returns the position in world space of the path for the current animation time
	* P(u) function
	* Returns: VECTOR - position in world space
	*/
	dx::XMVECTOR GetCurrentPosition();

	/*
	* Returns the position in world space of Center of interest
	* Returns: VECTOR - position in world space
	*/
	dx::XMVECTOR GetLookPosition();

	/*
	* Returns the velocity function evaluated at the current time.
	*/
	float GetCurrentVelocity();
};
 
