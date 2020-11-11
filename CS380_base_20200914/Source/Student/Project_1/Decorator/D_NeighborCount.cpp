#include <pch.h>
#include "D_NeighborCount.h"


D_NeighborCount::D_NeighborCount() : range(20.0f)
{}

void D_NeighborCount::on_enter()
{
    BehaviorNode::on_enter();
}

// Perform child if agent is far enough away from center of the map
void D_NeighborCount::on_update(float dt)
{
    // count all neighbors
    int neighborCount = 0;

    // get a list of all current agents
    const auto& allAgents = agents->get_all_agents();

    // and our agent's position
    const auto& currPos = agent->get_position();

    for (const auto& a : allAgents)
    {
        // don't count self
        if (a != agent)
        {
            // get distances
            const auto& agentPos = a->get_position();
            const float distance = Vec3::Distance(currPos, agentPos);

            // neighbor found
            if (distance < range)
            {
                ++neighborCount;
            }
        }
    }

    // submit count
    auto& bb = agent->get_blackboard();
    bb.set_value<int>("NeighborCount", neighborCount);

    BehaviorNode* child = children.front();

    child->tick(dt);

    // assume same status as child
    set_status(child->get_status());
    set_result(child->get_result());
}
