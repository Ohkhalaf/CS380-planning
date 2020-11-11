#include <pch.h>
#include "D_SetupCrab.h"

D_SetupCrab::D_SetupCrab() : firstRun(true)
{}

void D_SetupCrab::on_enter()
{
    if (firstRun)
    {
        agent->set_position(RNG::world_position());
        agent->set_yaw(RNG::range<float>(0, 2 * PI));
        agent->set_scaling(1.2f);

        auto& bb = agent->get_blackboard();

        bb.set_value<float>("WanderOffset", 0.0f);
        bb.set_value<Vec3>("CohensionVector", Vec3(0.0f, 0.0f, 0.0f));
        bb.set_value<Vec3>("SeperationVector", Vec3(0.0f, 0.0f, 0.0f));
        bb.set_value<Vec3>("AlignmentVector", Vec3(0.0f, 0.0f, 0.0f));
        bb.set_value<Vec3>("TetherVector", Vec3(0.0f, 0.0f, 0.0f));
        bb.set_value<Vec3>("WanderVector", Vec3(0.0f, 0.0f, 0.0f));

        firstRun = false;
    }

    BehaviorNode::on_enter();
}

void D_SetupCrab::on_update(float dt)
{
    BehaviorNode* child = children.front();

    child->tick(dt);

    // assume same status as child
    set_status(child->get_status());
    set_result(child->get_result());
}
