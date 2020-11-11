#include <pch.h>
#include "L_RunAway.h"

void L_RunAway::on_enter()
{
	BehaviorNode::on_leaf_enter();
}

void L_RunAway::on_update(float dt)
{
	const auto& bb = agent->get_blackboard();

	Vec3 velocity = bb.get_value<Vec3>("Direction");

	// add speed
	velocity *= agent->get_movement_speed() * dt;

	// calculate next position and set it
	agent->set_position(agent->get_position() + velocity);

    display_leaf_text();
	on_success();
}