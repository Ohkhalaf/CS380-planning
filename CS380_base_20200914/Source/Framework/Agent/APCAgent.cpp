/******************************************************************************/
/*!
\file		APCAgent.cpp
\project	CS380/CS580 AI Framework
\author		Carlos Esteban Garcia-Perez
\summary	AStarAgent that can carry other agents

Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/

#include <pch.h>
#include "APCAgent.h"
#include <sstream>
#include "Projects/ProjectTwo.h"
#include "Projects/ProjectFour.h"

APCAgent::APCAgent(size_t id) : AStarAgent(id)
{
	set_movement_speed(movementSpeed);
}

void APCAgent::logic_tick()
{

}

std::vector<Agent*> APCAgent::get_nearby_soldiers()
{
	std::vector<Agent*> soldiers = agents->get_all_agents_by_type("Soldier Agent");

	for (int i = 0; i < soldiers.size(); ++i)
	{
		//soldiers[i]->get_position()
	}

	return soldiers;
}