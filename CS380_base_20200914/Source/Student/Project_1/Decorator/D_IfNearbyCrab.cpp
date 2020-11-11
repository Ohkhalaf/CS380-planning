#include <pch.h>
#include "D_IfNearbyCrab.h"


D_IfNearbyCrab::D_IfNearbyCrab() : range(15.0f)
{}

void D_IfNearbyCrab::on_enter()
{
    /* check for a crab nearby */
    bool crabNearby = false;

    // get a list of all current agents
    const auto& allAgents = agents->get_all_agents();

    // and our agent's position
    const auto& currPos = agent->get_position();

    auto& bb = agent->get_blackboard();

    for (const auto& a : allAgents)
    {
        // targets are only crabs
        if (a != agent && a->get_type() == "Crab")
        {
            // get distances to all potential targets
            const auto& agentPos = a->get_position();
            const float distance = Vec3::Distance(currPos, agentPos);

            // find closest target
            if (distance < range)
            {
                crabNearby = true;
                agent->set_color(Vec3(5.0f, 0.0f, 0.0f));
                agent->set_movement_speed(35.0f);

                // get the normalize direction vector
                const Vec3 enemy_pos = a->get_position();
                Vec3 velocity = enemy_pos - currPos;
                velocity.Normalize();

                // turn to face them
                float workingYaw = atan2(velocity.x, velocity.z);
                agent->set_yaw(workingYaw);

                // go either to the right or left
                if (RNG::coin_toss())
                {
                    workingYaw -= PI / 2;
                }
                else
                {
                    workingYaw += PI / 2;
                }
                
                velocity = Vec3(sinf(workingYaw), 0.0f, cosf(workingYaw));
                velocity.Normalize();

                bb.set_value<Vec3>("Direction", velocity);
            }
        }
    }

    BehaviorNode::on_enter();

    if (!crabNearby)
    {
        bb.set_value<Vec3>("OtherCrabPosition", Vec3(0.0f, 0.0f, 0.0f));
        agent->set_color(Vec3(10.0f, 10.0f, 0.0f));
        agent->set_movement_speed(20.0f);
        on_failure();
    }
}

// Perform child if agent is far enough away from center of the map
void D_IfNearbyCrab::on_update(float dt)
{
    // target is found
    BehaviorNode* child = children.front();

    child->tick(dt);

    // assume same status as child
    set_status(child->get_status());
    set_result(child->get_result());

    if (child->failed())
    {
        auto& bb = agent->get_blackboard();

        bb.set_value<Vec3>("OtherCrabPosition", Vec3(0.0f, 0.0f, 0.0f));
        agent->set_color(Vec3(10.0f, 10.0f, 0.0f));
        agent->set_movement_speed(20.0f);
        on_failure();
    }
}
