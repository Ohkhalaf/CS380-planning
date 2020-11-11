#include <pch.h>
#include "D_SetupShark.h"

D_SetupShark::D_SetupShark() : firstRun(true)
{}

void D_SetupShark::on_enter()
{
    if (firstRun)
    {
        auto& bb = agent->get_blackboard();
        bb.set_value<float>("WanderOffset", 0.0f);
        bb.set_value<Vec3>("CohensionVector", Vec3(0.0f, 0.0f, 0.0f));
        bb.set_value<Vec3>("SeperationVector", Vec3(0.0f, 0.0f, 0.0f));
        bb.set_value<Vec3>("AlignmentVector", Vec3(0.0f, 0.0f, 0.0f));
        bb.set_value<Vec3>("TetherVector", Vec3(0.0f, 0.0f, 0.0f));
        bb.set_value<Vec3>("WanderVector", Vec3(0.0f, 0.0f, 0.0f));

        bb.set_value<bool>("Hunting", false);

        agent->set_position(RNG::world_position());
        agent->set_yaw(RNG::range<float>(0, 2 * PI));
        agent->set_scaling(2.0f);

        

        firstRun = false;
    }

    BehaviorNode::on_enter();
}

void D_SetupShark::on_update(float dt)
{
    BehaviorNode* child = children.front();

    child->tick(dt);

    // assume same status as child
    set_status(child->get_status());
    set_result(child->get_result());
}
