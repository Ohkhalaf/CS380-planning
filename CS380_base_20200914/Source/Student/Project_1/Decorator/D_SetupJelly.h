#pragma once
#include "BehaviorNode.h"

class D_SetupJelly : public BaseNode<D_SetupJelly>
{
public:
    D_SetupJelly();

protected:
    bool firstRun;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};