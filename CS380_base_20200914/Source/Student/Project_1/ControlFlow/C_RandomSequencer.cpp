#include <pch.h>
#include "C_RandomSequencer.h"

C_RandomSequencer::C_RandomSequencer() : currentIndex(0)
{}

void C_RandomSequencer::on_enter()
{
    currentIndex = 0;
    shuffle(children, 3);
    BehaviorNode::on_enter();
}

void C_RandomSequencer::on_update(float dt)
{
    // if any child fails, the node fails
    // if all children succeed, the node succeeds
    BehaviorNode *currentNode = children[currentIndex];
    currentNode->tick(dt);

    if (currentNode->failed() == true)
    {
        on_failure();
    }
    else if (currentNode->succeeded() == true)
    {
        // move to the next node
        ++currentIndex;

        // if we hit the size, then all nodes suceeded
        if (currentIndex == children.size())
        {
            on_success();
        }
    }
}

void C_RandomSequencer::shuffle(std::vector<BehaviorNode*>& children, int times)
{
    if (times < 0) return;

    for (int t = 0; t < times; ++t)
    {
        // go through the list and swap elements at random
        for (int i = 0; i < children.size(); ++i)
        {
            int j = rand() % children.size();
            std::swap(children[i], children[j]);
        }
    }
}
