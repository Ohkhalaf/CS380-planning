#pragma once
#include "BehaviorNode.h"

class L_CalcAlignmentVector : public BaseNode<L_CalcAlignmentVector>
{
public:
    L_CalcAlignmentVector();

protected:
    const float range;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;

private:
    
};