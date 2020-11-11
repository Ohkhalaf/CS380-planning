#include <pch.h>
#include "D_IfNearbyTarget.h"


D_IfNearbyTarget::D_IfNearbyTarget() : range(12.0f), radius(10.0f)
{}

void D_IfNearbyTarget::on_enter()
{
    BehaviorNode::on_enter();
}

// Perform child if agent is far enough away from center of the map
void D_IfNearbyTarget::on_update(float dt)
{
    // check for a target nearby
    Vec3 workingVector = Vec3(0.0f, 0.0f, 0.0f);
    float minDistance = std::numeric_limits<float>::max();
    Agent* target = nullptr;

    // get a list of all current agents
    const auto& allAgents = agents->get_all_agents();

    // and our agent's position
    const auto& currPos = agent->get_position();

    // find center of aggro circle in front of shark
    Vec3 dir = Vec3(sinf(agent->get_yaw()), 0.0f, cosf(agent->get_yaw()));
    dir.Normalize();
    dir *= range;
    Vec3 center = currPos + dir;

    for (const auto& a : allAgents)
    {
        // targets are only fish
        if (a->get_type() == "Fish")
        {
            // get distances to all potential targets
            const auto& agentPos = a->get_position();
            const float distance = Vec3::Distance(center, agentPos);

            // find closest target
            if (distance < minDistance)
            {
                minDistance = distance;
                target = a;
            }
        }
    }

    auto& bb = agent->get_blackboard();

    // see if closest target is within the aggro circle
    if (minDistance <= radius)
    {
        bb.set_value<Agent*>("TargetPtr", target);
        bb.set_value<bool>("Hunting", true);

        // target is found
        BehaviorNode* child = children.front();

        child->tick(dt);

        // assume same status as child
        set_status(child->get_status());
        set_result(child->get_result());
    }
    else
    {
        // no target
        bb.set_value<bool>("Hunting", false);
        bb.set_value("TargetPtr", nullptr);
        agent->set_movement_speed(40.0f);
        agent->set_color(Vec3(0.0f, 1.65f, 0.0f));
        on_failure();
    }
}
