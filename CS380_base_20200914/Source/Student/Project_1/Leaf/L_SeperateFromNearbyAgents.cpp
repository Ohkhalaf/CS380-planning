#include <pch.h>
#include "L_SeperateFromNearbyAgents.h"
#include "Agent/BehaviorAgent.h"

void L_SeperateFromNearbyAgents::on_enter()
{
    // set animation, speed, etc

    // set range that constitutes nearby
    const float range = 50.0f;

    // find a position away from all other agents
    Vec3 workingVector;
    float nearbyCount = 0;

    // get a list of all current agents
    const auto &allAgents = agents->get_all_agents();

    // and our agent's position
    const auto &currPos = agent->get_position();
    workingVector = Vec3(0,0,0);

    for (const auto & a : allAgents)
    {
        // make sure it's not our agent
        if (a != agent)
        {
            const auto &agentPos = a->get_position();
            const float distance = Vec3::Distance(currPos, agentPos);

            // only factor in nearby agents
            if (distance <= range)
            {
                workingVector.x += (agentPos.x - currPos.x);
                workingVector.z += (agentPos.z - currPos.z);
                ++nearbyCount;
            }
        }
    }

    // calculate seperation vector
    workingVector.x /= nearbyCount;
    workingVector.z /= nearbyCount;
    workingVector *= -1;

    // find target position
    targetPoint = currPos + workingVector;
    BehaviorNode::on_leaf_enter();
}

void L_SeperateFromNearbyAgents::on_update(float dt)
{
    const auto result = agent->move_toward_point(targetPoint, dt);

    if (result == true)
    {
        on_success();
    }

    display_leaf_text();
}

