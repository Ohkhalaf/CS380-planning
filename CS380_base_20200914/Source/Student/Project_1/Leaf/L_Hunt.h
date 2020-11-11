#pragma once
#include "BehaviorNode.h"

class L_Hunt : public BaseNode<L_Hunt>
{
public:
    L_Hunt();

protected:
    const float attackRange;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};