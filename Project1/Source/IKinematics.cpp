#include "IKinematics.h"
#include "imgui/imgui.h"

IKController::Joint* IKController::GetJoint(int manipulator_index, int bone_index) {
    for (auto&& joint : manipulators[manipulator_index]) {
        if (joint.bone_index == bone_index)
            return &joint;
    }
    return nullptr;
}

IKController::IKController() : animation_time(0.0f), animation_speed(1.0f),
    p_skeleton(nullptr), p_base_animation(nullptr), target_position() {
}

IKController::~IKController() {

}

void IKController::Update(float dt, dx::XMVECTOR _model_pos, dx::XMMATRIX _model_rot) {
    animation_time += dt * animation_speed;
    base_model_position = _model_pos;
    base_model_rotation = _model_rot;
    ShowIKControls();
}

void IKController::Process(float dt) {
    ProcessAnimation();
    ProcessManipulators(dt);
}

void IKController::SetSkel(Skeleton* skel) {
    p_skeleton = skel;
    bone_matrix_buffer.resize(p_skeleton->hierarchy.size());
}

void IKController::SetBaseAnimation(Animation* p_anim) {
    p_base_animation = p_anim;
    p_skeleton->ProcessBaseAnimationGraph(bone_matrix_buffer, *p_base_animation);
}

void IKController::AddEndEffector(Bone* _ee) {
    end_effectors.push_back(_ee);
    AddManipulatorForEE(_ee);
}

void IKController::AddEndEffector(int bone_index) {
    AddEndEffector(p_skeleton->hierarchy[bone_index]);
}

void IKController::AddManipulatorForEE(Bone* _end_effector) {
    manipulator new_manipulator;
    dx::XMVECTOR origin = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    std::vector<int> parent_indeces;
    
    //Get the chain of links from the EE to the root
    Bone* curr_bone = p_skeleton->hierarchy[_end_effector->bone_indx];
    while (curr_bone->parent_indx != -1) {
        parent_indeces.push_back(curr_bone->bone_indx);
        curr_bone = p_skeleton->hierarchy[curr_bone->parent_indx];
    }

    //Get the root transform
    dx::XMMATRIX parent_transform = p_base_animation->GetBaseTransform(0).toMatrix();
    for (auto iter = parent_indeces.rbegin(); iter != parent_indeces.rend(); ++iter) {
        Joint curr_joint;
        //Get the current bone
        curr_bone = p_skeleton->hierarchy[*iter];

        //Get the local transform for the bone
        const VQS& curr_vqs = p_base_animation->GetBaseTransform(curr_bone->bone_indx);
        dx::XMMATRIX local_transform = curr_vqs.toMatrix();
        dx::XMMATRIX modelTransform = local_transform * parent_transform;

        //Get the position by applying the transform
        curr_joint.position = dx::XMVector3Transform(origin,
            modelTransform *
            dx::XMMatrixMultiply(base_model_rotation,
                dx::XMMatrixTranslationFromVector(base_model_position)));
        //Get the rotation axis
        dx::XMQuaternionToAxisAngle(
            &curr_joint.rot_axis, &curr_joint.rot_angle, curr_vqs.GetQ().toVector());

        //The parent_transform for the next iteration is the current model_transform
        parent_transform = modelTransform;

        //Add the joint to the manipulator
        curr_joint.bone_index = curr_bone->bone_indx;
        new_manipulator.push_back(curr_joint);
    }

    manipulators.push_back(new_manipulator);
}

void IKController::ProcessManipulators(float dt) {
    if (stop_animation)
        return;

    for (unsigned int i = 0; i < manipulators.size(); ++i) {
        dx::XMVECTOR Pc = EvalulateManipulator(i);
        dx::XMVECTOR diff_vector = dx::XMVectorSubtract(target_position, Pc);
        float distance_check = dx::XMVectorGetX(dx::XMVector3Length(diff_vector));
        if (distance_check < distance_threshold)
            continue;

        if (jacobian) {
            dx::XMVECTOR dP = diff_vector;

            arma::mat pseudo_J = GetPesudoJacobian(i);
            arma::mat dP_mat(3, 1);
            dP_mat(0) = dx::XMVectorGetX(dP);
            dP_mat(1) = dx::XMVectorGetY(dP);
            dP_mat(2) = dx::XMVectorGetZ(dP);
            arma::mat dQ = pseudo_J * dP_mat;

            for (unsigned int j = 0; j < manipulators[i].size() - 1; ++j) {
                manipulators[i][j].rot_angle += (dQ(j, 0) * dt);
            }
        }
        else {
            ProcessCCD(i);
        }
    }
    current_frame--;
}

void IKController::ProcessAnimation() {
    dx::XMVECTOR origin = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    for (unsigned int boneIndex = 0; boneIndex < p_skeleton->hierarchy.size(); ++boneIndex)
    {
        Bone* bone = p_skeleton->hierarchy[boneIndex];
        const VQS& animation_transform = p_base_animation->GetBaseTransform(boneIndex);
        Joint* joint_p = GetJoint(0, boneIndex);
        dx::XMMATRIX local_transform = animation_transform.toMatrix();
        if (joint_p) {
            Quaternion modified_q(dx::XMQuaternionRotationAxis(joint_p->rot_axis, joint_p->rot_angle));
            VQS modified_transform(animation_transform.GetV(), modified_q, animation_transform.GetS());
            local_transform = modified_transform.toMatrix();
        }

        dx::XMMATRIX parent_transform = bone->parent_indx != -1 ? bone_matrix_buffer[bone->parent_indx] : dx::XMMatrixIdentity();
        dx::XMMATRIX modelTransform = local_transform * parent_transform;
        bone_matrix_buffer[boneIndex] = modelTransform;

        //Update the manipulator joint positions
        if (joint_p) {
            joint_p->position = dx::XMVector3Transform(origin,
                modelTransform *
                dx::XMMatrixMultiply(base_model_rotation,
                dx::XMMatrixTranslationFromVector(base_model_position)));
            joint_p->curr_rot_axis = dx::XMVector3Normalize(
                dx::XMVector3Transform(joint_p->rot_axis,
                    base_model_rotation));/*
                dx::XMMatrixMultiply(base_model_rotation,
                    dx::XMMatrixTranslationFromVector(base_model_position))));*/
        }
    }
}

void IKController::ShowIKControls() {
    int bone_index;
    if (ImGui::Begin("IK Controller")) 
    {
        ImGui::Checkbox("Stop animation", &stop_animation);

        ImGui::Text("Manipulator joint angles");
        for(unsigned int i = 0; i < manipulators.size(); ++i)
        {
            for(unsigned int j = 0; j < manipulators[0].size(); ++j)
            {
                std::string str = "Bone angle indx " + std::to_string(manipulators[i][j].bone_index);
                ImGui::SliderAngle(str.c_str() , &manipulators[i][j].rot_angle);

                ImGui::Text("Joint bone indx : %d", manipulators[i][j].bone_index);
                ImGui::Text("Joint rot_axis : X - %f | Y - %f | Z - %f", 
                    dx::XMVectorGetX(manipulators[i][j].curr_rot_axis),
                    dx::XMVectorGetY(manipulators[i][j].curr_rot_axis),
                    dx::XMVectorGetZ(manipulators[i][j].curr_rot_axis));
                ImGui::Text("Joint position : X - %f | Y - %f | Z - %f",
                    dx::XMVectorGetX(manipulators[i][j].position),
                    dx::XMVectorGetY(manipulators[i][j].position),
                    dx::XMVectorGetZ(manipulators[i][j].position));
            }
        }

        ImGui::SliderInt("Max Frame count", &frame_count, 10, 6000);
        ImGui::Text("Current Frame count %d", current_frame);
        if (ImGui::Button("Reset CF")) {
            current_frame = frame_count;
        }

        ImGui::SliderFloat("Distance Threshold", &distance_threshold, 0.0f, 300.0f);
        dx::XMVECTOR Pc = EvalulateManipulator(0);
        float distance = dx::XMVectorGetX(
            dx::XMVector3Length(dx::XMVectorSubtract(target_position, Pc)));
        ImGui::Text("Current distance : %f", distance);
        
        ImGui::Text("EE Position : X-%f Y-%f Z-%f",
            dx::XMVectorGetX(Pc), dx::XMVectorGetY(Pc), dx::XMVectorGetZ(Pc));
        ImGui::Text("target Position : X-%f Y-%f Z-%f", 
            dx::XMVectorGetX(target_position), 
            dx::XMVectorGetY(target_position), 
            dx::XMVectorGetZ(target_position));
    }
    ImGui::End();
}

dx::XMVECTOR IKController::EvalulateManipulator(unsigned int manipulator_indx) {
    dx::XMMATRIX modelTransform = bone_matrix_buffer[end_effectors[manipulator_indx]->bone_indx];
    dx::XMVECTOR origin = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    //return manipulators[manipulator_indx].back().position;
    return dx::XMVector3Transform(origin, 
        modelTransform * 
        dx::XMMatrixMultiply(base_model_rotation,
            dx::XMMatrixTranslationFromVector(base_model_position)));
}

arma::mat IKController::GetPesudoJacobian(unsigned int manipulator_indx) {
    int joint_n = manipulators[manipulator_indx].size();
    joint_n -= 1;
    arma::mat J(3, joint_n);

    dx::XMVECTOR ee_position = EvalulateManipulator(manipulator_indx);
    for (unsigned int i = 0; i < joint_n; ++i) {
        dx::XMVECTOR curr_rot_axis = manipulators[manipulator_indx][i].curr_rot_axis;
        dx::XMVECTOR curr_position = manipulators[manipulator_indx][i].position;
        dx::XMVECTOR mat_val = dx::XMVector3Cross(curr_rot_axis,
            dx::XMVectorSubtract(ee_position, curr_position));

        J(0, i) = dx::XMVectorGetX(mat_val);
        J(1, i) = dx::XMVectorGetY(mat_val);
        J(2, i) = dx::XMVectorGetZ(mat_val);
    }

    arma::mat J_T = J.t();
    arma::mat J_T_p = J * J_T;
    return J_T * J_T_p.i();
}

void IKController::GenerateDefaultRightArmConstraints(unsigned int manipulator_indx) {
    //Spine
    manipulators[manipulator_indx][0].min_angle = dx::XMConvertToRadians(-100);
    manipulators[manipulator_indx][0].max_angle = dx::XMConvertToRadians(20);

    //Upper spine
    manipulators[manipulator_indx][1].min_angle = dx::XMConvertToRadians(-50);
    manipulators[manipulator_indx][1].max_angle = dx::XMConvertToRadians(20);

    //Upper arm
    manipulators[manipulator_indx][2].min_angle = dx::XMConvertToRadians(80);
    manipulators[manipulator_indx][2].max_angle = dx::XMConvertToRadians(170);

    //Lower arm
    manipulators[manipulator_indx][3].min_angle = dx::XMConvertToRadians(15);
    manipulators[manipulator_indx][3].max_angle = dx::XMConvertToRadians(150);

    //Hand
    manipulators[manipulator_indx][4].min_angle = dx::XMConvertToRadians(0);
    manipulators[manipulator_indx][4].max_angle = dx::XMConvertToRadians(150);
}

void IKController::SetManipulatorConstraits(unsigned int manipulator_indx) {
    for (auto& joint : manipulators[manipulator_indx]) {
        joint.rot_angle = joint.rot_angle < joint.min_angle ? joint.min_angle : joint.rot_angle;
        joint.rot_angle = joint.rot_angle > joint.max_angle ? joint.max_angle : joint.rot_angle;
    }
}

void IKController::ProcessCCD(unsigned int manipulator_indx) {
    dx::XMVECTOR Pc = EvalulateManipulator(manipulator_indx);
    for (int k = manipulators[manipulator_indx].size() - 2; k >= 0; --k) {
        dx::XMVECTOR Vck =  
            dx::XMVectorSubtract(Pc, manipulators[manipulator_indx][k].position);
        dx::XMVECTOR Vdk =
            dx::XMVectorSubtract(target_position, manipulators[manipulator_indx][k].position);
        float alpha_k = dx::XMScalarACos(
            dx::XMVectorGetX(dx::XMVector3Dot(Vck, Vdk)) /
            (dx::XMVectorGetX(dx::XMVector3Length(Vck)) * dx::XMVectorGetX(dx::XMVector3Length(Vdk)))
        );
        for (unsigned int j = k; j < manipulators[manipulator_indx].size(); ++j) {
            manipulators[manipulator_indx][j].rot_angle += alpha_k;
        }
        SetManipulatorConstraits(manipulator_indx);
    }
}

void IKController::Reset() {
    current_frame = 0;
    dx::XMVECTOR origin = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    for (unsigned int i = 0; i < manipulators.size(); ++i) {
        //Get the root transform
        dx::XMMATRIX parent_transform = p_base_animation->GetBaseTransform(0).toMatrix();
        for (unsigned int j = 0; j < manipulators[i].size(); ++j) {
            Joint& curr_joint = manipulators[i][j];
            const VQS& curr_vqs = p_base_animation->GetBaseTransform(curr_joint.bone_index);
            dx::XMMATRIX local_transform = curr_vqs.toMatrix();
            dx::XMMATRIX modelTransform = local_transform * parent_transform;

            //Get the position by applying the transform
            curr_joint.position = dx::XMVector3Transform(origin,
                modelTransform *
                dx::XMMatrixMultiply(base_model_rotation,
                    dx::XMMatrixTranslationFromVector(base_model_position)));
            //Get the rotation axis
            dx::XMQuaternionToAxisAngle(
                &curr_joint.rot_axis, &curr_joint.rot_angle, curr_vqs.GetQ().toVector());

            //The parent_transform for the next iteration is the current model_transform
            parent_transform = modelTransform;
        }
    }
}



