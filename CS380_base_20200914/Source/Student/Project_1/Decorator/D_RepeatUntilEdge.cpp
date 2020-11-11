#include <pch.h>
#include "D_RepeatUntilEdge.h"


D_RepeatUntilEdge::D_RepeatUntilEdge() : rangeRadius(50.0f)
{}

void D_RepeatUntilEdge::on_enter()
{
    BehaviorNode::on_enter();
}

// Perform child if agent is far enough away from center of the map
void D_RepeatUntilEdge::on_update(float dt)
{
    // get distance away
    const auto &currPos = agent->get_position();
    const Vec3 center = Vec3(50.0f, 0.0f, 50.0f);
    const float distance = Vec3::Distance(currPos, center);

    auto& bb = agent->get_blackboard();

    // if agent is inside given range, repeat
    if (distance <= rangeRadius)
    {
        BehaviorNode* child = children.front();

        child->tick(dt);

        // repeat
        child->set_status(NodeStatus::READY);
    }
    else
    {
        // stop repeating
        on_failure();
    }
}
