#include <pch.h>
#include "L_SetSideways.h"

void L_SetSideways::on_enter()
{
	BehaviorNode::on_leaf_enter();
}

void L_SetSideways::on_update(float dt)
{
	// get the normalize direction vector
	float workingYaw = agent->get_yaw();
	
	workingYaw -= PI / 2;

	agent->set_yaw(workingYaw);

    display_leaf_text();
	on_success();
}
