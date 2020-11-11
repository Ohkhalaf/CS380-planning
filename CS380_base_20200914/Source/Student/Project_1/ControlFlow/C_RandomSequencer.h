#pragma once
#include "BehaviorNode.h"

class C_RandomSequencer : public BaseNode<C_RandomSequencer>
{
public:
    C_RandomSequencer();
protected:
    size_t currentIndex;

    virtual void on_enter() override;
    virtual void on_update(float dt) override;
    
private:
    void shuffle(std::vector<BehaviorNode*>& children, int times);
};