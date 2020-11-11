#pragma once
#include "BehaviorNode.h"

class L_UpdateStatus : public BaseNode<L_UpdateStatus>
{
public:
    L_UpdateStatus();

protected:
    const float huntSpeed;
    const Vec3 huntColor;
    const float idleSpeed;
    const Vec3 idleColor;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};