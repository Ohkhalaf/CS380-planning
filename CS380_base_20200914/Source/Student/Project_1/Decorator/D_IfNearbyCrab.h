#pragma once
#include "BehaviorNode.h"

// Perform child if agent is far enough away from center of the map
class D_IfNearbyCrab : public BaseNode<D_IfNearbyCrab>
{
public:
    D_IfNearbyCrab();

protected:
    float range;  // constitues nearby

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};