#pragma once
#include "BehaviorNode.h"

class D_SetupCrab : public BaseNode<D_SetupCrab>
{
public:
    D_SetupCrab();

protected:
    bool firstRun;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};