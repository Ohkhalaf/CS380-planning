#pragma once
#include "BehaviorNode.h"
#include "Misc/NiceTypes.h"

class L_CalcTetherVector : public BaseNode<L_CalcTetherVector>
{
protected:
    virtual void on_enter() override;
    virtual void on_update(float dt) override;

private:

};