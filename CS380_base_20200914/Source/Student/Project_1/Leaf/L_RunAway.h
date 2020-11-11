#pragma once
#include "BehaviorNode.h"

class L_RunAway : public BaseNode<L_RunAway>
{
public:

protected:

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};