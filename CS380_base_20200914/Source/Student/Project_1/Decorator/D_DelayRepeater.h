#pragma once
#include "BehaviorNode.h"

class D_DelayRepeater : public BaseNode<D_DelayRepeater>
{
public:
    D_DelayRepeater();

protected:
    float delay;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};