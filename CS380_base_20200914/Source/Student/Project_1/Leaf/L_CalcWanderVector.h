#pragma once
#include "BehaviorNode.h"

class L_CalcWanderVector : public BaseNode<L_CalcWanderVector>
{
public:
    L_CalcWanderVector();

protected:
    const float range; // random range to turn agent's yaw

    virtual void on_enter() override;
    virtual void on_update(float dt) override;

private:
    
};