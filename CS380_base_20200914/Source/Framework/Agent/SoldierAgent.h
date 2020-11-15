/******************************************************************************/
/*!
\file		SoldierAgent.h
\project	CS380/CS580 AI Framework
\author		Carlos Esteban Garcia-Perez
\summary	AStarAgent that can storm a battlefield

Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/

#pragma once
#include "AStarAgent.h"
#include <list>
#include "misc/PathfindingDetails.hpp"

class SoldierAgent : public AStarAgent
{
public:
    SoldierAgent(size_t id);

protected:

};