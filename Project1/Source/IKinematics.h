#pragma once
#include <armadillo>
#include "Quaternion.h"
#include "Animation.h"

class IKController 
{
public:
	struct Joint {
		dx::XMVECTOR position;
		dx::XMVECTOR rot_axis;
		dx::XMVECTOR curr_rot_axis;
		int bone_index;
		float rot_angle;
		float max_angle;
		float min_angle;
	};

	Joint* GetJoint(int manipulator_index, int bone_index);

	IKController();
	~IKController();
	//Update the Controller
	void Update(float dt, dx::XMVECTOR _model_pos, dx::XMMATRIX _model_rot);
	
	//Method to use for IK
	bool jacobian = true;

	dx::XMVECTOR base_model_position;
	dx::XMMATRIX base_model_rotation;

	//Animation control parameters
	bool stop_animation = false;
	float animation_time;
	float animation_speed;
	int frame_count = 120*10;
	unsigned int current_frame = frame_count;

	//Pointer to the skeleton
	Skeleton* p_skeleton;
	//Pointer to the animation to use as the base animation
	Animation* p_base_animation;
	//The buffer of matrices for the final bone transforms
	std::vector<dx::XMMATRIX> bone_matrix_buffer;

	//The end effectors to move to the target position
	std::vector<Bone*> end_effectors;
	//The manipulator for each end_effector;
	typedef std::vector<Joint> manipulator;
	std::vector<manipulator> manipulators;

	//the target position;
	dx::XMVECTOR target_position;
	//The threshold to check if the EE is close enough to the target_position
	float distance_threshold = 3.0f;

	//Process the animation
	void Process(float dt);

	//Sets the skeleton pointer and adjusts the size of the bone_matrix_buffer accordingly
	void SetSkel(Skeleton* skel);

	//Sets the base animation
	void SetBaseAnimation(Animation* p_anim);

	/*
	* Adds an end effector to the list of end effectors to move to the target position
	* Also generates the manipulator for the corresponding EE
	*/
	void AddEndEffector(Bone* _ee);

	/*
	* Adds an end effector using the bone index of the EE bone instead of the bone pointer
	*/
	void AddEndEffector(int bone_index);

	//Adds a manipulator for the specified end_effector
	void AddManipulatorForEE(Bone* _end_effector);

	//Calculate the angles of the joins within all the manipulators for this frame
	void ProcessManipulators(float dt);

	//Caclulate the bone transformation matrix for rendering the animation
	void ProcessAnimation();

	//IMgui window for the IK controls
	void ShowIKControls();

	/*
	* Evaluates all the angles along the manipulator to get the current 
	* end effector position
	* i.e f(q)
	* Returns: XMVector - EE position
	*/
	dx::XMVECTOR EvalulateManipulator(unsigned int manipulator_indx);

	/*
	* Calculate the PseudoJacobian Matrix for the corresponding manipulator
	* Returns: Matrix 
	*/
	arma::mat GetPesudoJacobian(unsigned int manipulator_indx);

	/*
	* Generates the constraints based on pre-defined values considered
	* for a human right arm.
	*/
	void GenerateDefaultRightArmConstraints(unsigned int manipulator_indx);

	/*
	* Limits the rotation angle of the joins in a manipulator based
	* on the min/max values
	*/
	void SetManipulatorConstraits(unsigned int manipulator_indx);

	/*
	* Process the manipulator with CCD instead of jacobian
	*/
	void ProcessCCD(unsigned int manipulator_indx);

	/*
	* Reset the joint angles to the base animation angles
	*/
	void Reset();
};
