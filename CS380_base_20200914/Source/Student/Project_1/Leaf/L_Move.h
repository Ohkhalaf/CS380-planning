#pragma once
#include "BehaviorNode.h"

class L_Move : public BaseNode<L_Move>
{
public:

protected:

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};