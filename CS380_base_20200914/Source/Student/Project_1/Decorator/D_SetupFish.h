#pragma once
#include "BehaviorNode.h"

class D_SetupFish : public BaseNode<D_SetupFish>
{
public:
    D_SetupFish();

protected:
    bool firstRun;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};