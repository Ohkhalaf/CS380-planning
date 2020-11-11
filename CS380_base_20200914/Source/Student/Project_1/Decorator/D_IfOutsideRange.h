#pragma once
#include "BehaviorNode.h"

// Perform child if agent is far enough away from center of the map
class D_IfOutsideRange : public BaseNode<D_IfOutsideRange>
{
public:
    D_IfOutsideRange();

protected:
    float rangeRadius;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};