#pragma once
#include "BehaviorNode.h"

class L_SetSideways : public BaseNode<L_SetSideways>
{
public:

protected:

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};