#pragma once
#include "BehaviorNode.h"

class L_ChangeColor : public BaseNode<L_ChangeColor>
{
public:

protected:

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};