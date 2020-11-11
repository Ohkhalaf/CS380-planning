#include <pch.h>
#include "L_UpdateStatus.h"

L_UpdateStatus::L_UpdateStatus() : huntSpeed(55.0f),
                                   huntColor(Vec3(2.55f, 1.65f, 0.0f)),
                                   idleSpeed(40.0f),
                                   idleColor(Vec3(0.0f, 1.65f, 0.0f))
{}

void L_UpdateStatus::on_enter()
{
    const auto& bb = agent->get_blackboard();
    if (bb.get_value<bool>("Hunting"))
    {
        // Set Hunt Speed and Color
        agent->set_movement_speed(huntSpeed);
        agent->set_color(huntColor);
    }
    else
    {
        // Set Idle Speed and Color
        agent->set_movement_speed(idleSpeed);
        agent->set_color(idleColor);
    }

	BehaviorNode::on_leaf_enter();
}

void L_UpdateStatus::on_update(float dt)
{
    on_success();
    display_leaf_text();
}
