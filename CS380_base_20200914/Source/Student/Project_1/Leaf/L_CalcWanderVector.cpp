#include <pch.h>
#include "L_CalcWanderVector.h"
#include "Agent/BehaviorAgent.h"

// set range that constitutes nearby
L_CalcWanderVector::L_CalcWanderVector() : range(PI/6.0f) //range(PI/5.0f)
{}

// chose a random direction in front of the agent
void L_CalcWanderVector::on_enter()
{
    auto& bb = agent->get_blackboard();

    const float FOURTH_PI = PI / 4.0f;

    float workingYaw = bb.get_value<float>("WanderOffset");

    if (workingYaw - range < (-1 * FOURTH_PI))
    {
        // under range
        workingYaw = RNG::range<float>(-1 * FOURTH_PI, (-1 * FOURTH_PI) + (2 * range));
    }
    else if (workingYaw + range > FOURTH_PI)
    {
        // over range
        workingYaw = RNG::range<float>(FOURTH_PI - (2 * range), FOURTH_PI);
    }
    else
    {
        //default
        workingYaw = RNG::range<float>(workingYaw - range, workingYaw + range);
    }

    bb.set_value<float>("WanderOffset", workingYaw);

    // turn yaw into a direction vector
    workingYaw += agent->get_yaw();
    Vec3 wanderVector = Vec3(sinf(workingYaw), 0.0f, cosf(workingYaw));
    wanderVector.Normalize();

    // submit vector
    bb.set_value<Vec3>("WanderVector", wanderVector);

    BehaviorNode::on_leaf_enter();
}

void L_CalcWanderVector::on_update(float dt)
{
    on_success();
    display_leaf_text();
}

