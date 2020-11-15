/******************************************************************************/
/*!
\file		APCAgent.h
\project	CS380/CS580 AI Framework
\author		Carlos Esteban Garcia-Perez
\summary	AStarAgent that can carry other agents

Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/

#pragma once
#include "AStarAgent.h"
#include <list>
#include "misc/PathfindingDetails.hpp"

class SoldierAgent;

class APCAgent : public AStarAgent
{
public:
    APCAgent(size_t id);

    void logic_tick();

private:
    enum class State
    {
        IDLE,
        MOVING,
        BOARDING,
        DROPPING
    } state;

    static constexpr float movementSpeed = 2000.0f / 1.0f;

    std::vector<Agent*> get_nearby_soldiers();
    void pick_up_soldier(SoldierAgent* s);
};