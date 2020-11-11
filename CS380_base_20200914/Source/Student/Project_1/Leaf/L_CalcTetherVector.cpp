#include <pch.h>
#include "L_CalcTetherVector.h"
#include "Agent/BehaviorAgent.h"

void L_CalcTetherVector::on_enter()
{
    // calculate the direction towards the center
    const auto& currPos = agent->get_position();
    const Vec3 center = Vec3(50.0f, 0.0f, 50.0f);
    Vec3 workingVector = center - currPos;
    workingVector.Normalize();

    // make the distance away afect it more
    const float distance = Vec3::Distance(currPos, center);
    workingVector *= ((distance - 50.0f) / 50.0f);

    // submit calculation
    auto& bb = agent->get_blackboard();
    bb.set_value<Vec3>("TetherVector", workingVector);

	BehaviorNode::on_leaf_enter();
}

void L_CalcTetherVector::on_update(float dt)
{
    on_success();
    display_leaf_text();
}
