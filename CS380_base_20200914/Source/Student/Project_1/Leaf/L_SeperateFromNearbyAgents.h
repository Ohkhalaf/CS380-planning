#pragma once
#include "BehaviorNode.h"

class L_SeperateFromNearbyAgents : public BaseNode<L_SeperateFromNearbyAgents>
{
protected:
    virtual void on_enter() override;
    virtual void on_update(float dt) override;

private:
    Vec3 targetPoint;
};