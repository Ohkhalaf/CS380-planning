#pragma once
#include "BehaviorNode.h"

class L_CalcCohensionVector : public BaseNode<L_CalcCohensionVector>
{
public:
    L_CalcCohensionVector();

protected:
    const float range;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;

private:
    
};