#include <pch.h>
#include "L_MoveToCenter.h"
#include "Agent/BehaviorAgent.h"

void L_MoveToCenter::on_enter()
{
    // set animation, speed, etc

    targetPoint = Vec3(50.0f, 0.0f, 50.0f);
    //agent->look_at_point(targetPoint);

	BehaviorNode::on_leaf_enter();
}

void L_MoveToCenter::on_update(float dt)
{
    const auto result = agent->move_toward_point(targetPoint, dt);

    if (result == true)
    {
        on_success();
    }

    display_leaf_text();
}
