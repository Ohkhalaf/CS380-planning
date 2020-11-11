#pragma once
#include "BehaviorNode.h"

class D_UpdateYaw : public BaseNode<D_UpdateYaw>
{
public:

protected:

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};