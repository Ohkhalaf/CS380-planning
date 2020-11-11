#include <pch.h>
#include "D_IfOutsideRange.h"


D_IfOutsideRange::D_IfOutsideRange() : rangeRadius(50.0f)
{}

void D_IfOutsideRange::on_enter()
{
    BehaviorNode::on_enter();
}

// Perform child if agent is far enough away from center of the map
void D_IfOutsideRange::on_update(float dt)
{
    // get distance away
    const auto &currPos = agent->get_position();
    const Vec3 center = Vec3(50.0f, 0.0f, 50.0f);
    const float distance = Vec3::Distance(currPos, center);

    auto& bb = agent->get_blackboard();

    // if agent is out of given range, perform child
    if (distance > rangeRadius)
    {
        BehaviorNode *child = children.front();

        child->tick(dt);

        // assume same status as child
        set_status(child->get_status());
        set_result(child->get_result());

        // this ruins the flexibility of the decorator more
        bb.set_value<bool>("UseTether", true);
    }
    else
    {
        // this ruins the flexibility of the decorator more
        bb.set_value<bool>("UseTether", false);

        // don't update child, go back
        on_success();
    }
}
