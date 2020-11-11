#pragma once
#include "BehaviorNode.h"

class D_SetupShark : public BaseNode<D_SetupShark>
{
public:
    D_SetupShark();

protected:
    bool firstRun;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
};