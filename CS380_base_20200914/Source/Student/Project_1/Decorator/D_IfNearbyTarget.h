#pragma once
#include "BehaviorNode.h"

// Perform child if agent is far enough away from center of the map
class D_IfNearbyTarget : public BaseNode<D_IfNearbyTarget>
{
public:
    D_IfNearbyTarget();

protected:
    float range;  // distance of aggro circle in front
    float radius; // radius of aggro circle

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};