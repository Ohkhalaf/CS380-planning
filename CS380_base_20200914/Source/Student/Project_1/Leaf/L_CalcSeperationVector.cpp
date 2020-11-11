#include <pch.h>
#include "L_CalcSeperationVector.h"
#include "Agent/BehaviorAgent.h"

// set range that constitutes nearby
L_CalcSeperationVector::L_CalcSeperationVector() : range(15.0f)
{}

void L_CalcSeperationVector::on_enter()
{
    // find a position away from all other nearby agents
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

                // really close, increase it's factor
                if (distance < range / 2)
                {
                    workingVector.x += (agentPos.x - currPos.x);
                    workingVector.z += (agentPos.z - currPos.z);
                }
                ++nearbyCount;
            }
        }
    }


    auto& bb = agent->get_blackboard();

    // no seperation vector exists if no neighbors are nearby
    if (nearbyCount > 0)
    {
        // calculate seperation vector
        workingVector.x /= nearbyCount;
        workingVector.z /= nearbyCount;
        workingVector *= -1;
        workingVector.Normalize();

        // submit calculation
        bb.set_value<Vec3>("SeperationVector", workingVector);
    }
    else
    {
        // submit the zero vector
        bb.set_value<Vec3>("SeperationVector", Vec3(0.0f, 0.0f, 0.0f));
    }

    BehaviorNode::on_leaf_enter();
}

void L_CalcSeperationVector::on_update(float dt)
{
    on_success();
    display_leaf_text();
}

