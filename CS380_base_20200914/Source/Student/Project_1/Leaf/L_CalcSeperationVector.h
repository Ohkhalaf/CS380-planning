#pragma once
#include "BehaviorNode.h"

class L_CalcSeperationVector : public BaseNode<L_CalcSeperationVector>
{
public:
    L_CalcSeperationVector();

protected:
    const float range;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;

private:
    
};