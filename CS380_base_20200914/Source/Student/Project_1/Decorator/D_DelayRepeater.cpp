#include <pch.h>
#include "D_DelayRepeater.h"

D_DelayRepeater::D_DelayRepeater() : delay(0.0f)
{}

void D_DelayRepeater::on_enter()
{
    BehaviorNode::on_enter();
}

// delays the on_enter of the child, keeps sequencer going
void D_DelayRepeater::on_update(float dt)
{
    BehaviorNode* child = children.front();

    delay -= dt;

    if (delay < 0.0f)
    {
        child->tick(dt);

        // assume same status as child
        set_status(child->get_status());
        set_result(child->get_result());

        delay = RNG::range(0.05f, 0.15f);
    }

    on_success();
}
