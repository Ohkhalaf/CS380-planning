#include <pch.h>
#include "L_CalcAlignmentVector.h"
#include "Agent/BehaviorAgent.h"

// set range that constitutes nearby
L_CalcAlignmentVector::L_CalcAlignmentVector() : range(15.0f)
{}

void L_CalcAlignmentVector::on_enter()
{
    // find the average direction of all nearby agents
    float workingYaw = 0.0f;
    float nearbyCount = 0;

    // get a list of all current agents
    const auto &allAgents = agents->get_all_agents();

    // and our agent's position
    const auto &currPos = agent->get_position();

    for (const auto & a : allAgents)
    {
        // make sure it's not our agent and that they are of the same type
        if (a != agent && agent->get_type() == a->get_type())
        {
            const auto &agentPos = a->get_position();
            const float distance = Vec3::Distance(currPos, agentPos);

            // only factor in nearby agents
            if (distance <= range)
            {
                workingYaw += agent->get_yaw();
                ++nearbyCount;
            }
        }
    }


    auto& bb = agent->get_blackboard();

    // no alignment vector exists if no neighbors are nearby
    if (nearbyCount > 0)
    {
        // calculate the average yaw
        workingYaw /= nearbyCount;

        // turn it into a vector
        Vec3 alignmentVector = Vec3(sinf(workingYaw), 0.0f, cosf(workingYaw));
        alignmentVector.Normalize();

        // submit calculation
        bb.set_value<Vec3>("AlignmentVector", alignmentVector);
    }
    else
    {
        // submit the zero vector
        bb.set_value<Vec3>("AlignmentVector", Vec3(0.0f, 0.0f, 0.0f));
    }

    BehaviorNode::on_leaf_enter();
}

void L_CalcAlignmentVector::on_update(float dt)
{
    on_success();
    display_leaf_text();
}

