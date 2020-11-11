#include <pch.h>
#include "D_UpdateYaw.h"

void D_UpdateYaw::on_enter()
{
    /*
    * Bring together all component vectors and use them to 
    * find the direction of movement
    */
    const auto& bb = agent->get_blackboard();
    Vec3 workingVector = Vec3(0.0f, 0.0f, 0.0f);

    // FACTORS
    const float cohensionFactor  = 0.30f;
    const float seperationFactor = 0.25f;
    const float alignmentFactor  = 0.80f;
    const float tetherFactor     = 0.25f;
    const float wanderFactor     = 0.10f;

    // Cohension Vector
    Vec3 cohensionVector = bb.get_value<Vec3>("CohensionVector");
    workingVector += cohensionVector * cohensionFactor;

    // Seperation Vector
    Vec3 seperationVector = bb.get_value<Vec3>("SeperationVector");
    workingVector += seperationVector * seperationFactor;

    // Alignment Vector
    Vec3 alignmentVector = bb.get_value<Vec3>("AlignmentVector");
    workingVector += alignmentVector * alignmentFactor;

    // Tether Vector
    if (bb.get_value<bool>("UseTether"))
    {
        Vec3 tetherVector = bb.get_value<Vec3>("TetherVector");
        workingVector += tetherVector * tetherFactor;
    }

    // Wander Vector
    Vec3 wanderVector = bb.get_value<Vec3>("WanderVector");
    workingVector += wanderVector * wanderFactor;

    // set the new yaw
    if (workingVector != Vec3(0.0f, 0.0f, 0.0f))
        agent->set_yaw(atan2(workingVector.x, workingVector.z));

    BehaviorNode::on_enter();
}

void D_UpdateYaw::on_update(float dt)
{
    BehaviorNode* child = children.front();

    child->tick(dt);

    // assume same status as child
    set_status(child->get_status());
    set_result(child->get_result());
}
