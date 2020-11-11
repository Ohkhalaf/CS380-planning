#include <pch.h>
#include "L_Move.h"

void L_Move::on_enter()
{
	BehaviorNode::on_leaf_enter();
}

void L_Move::on_update(float dt)
{
	// get the normalize direction vector
	const float yaw = agent->get_yaw();
	Vec3 velocity = Vec3(std::sinf(yaw), 0.0f, std::cosf(yaw));
	velocity.Normalize();

	// add speed
	velocity *= agent->get_movement_speed() * dt;

	// calculate next position and set it
	agent->set_position(agent->get_position() + velocity);

    display_leaf_text();
	on_success();
}
