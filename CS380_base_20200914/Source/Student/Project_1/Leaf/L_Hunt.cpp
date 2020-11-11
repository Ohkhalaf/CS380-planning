#include <pch.h>
#include "L_Hunt.h"

L_Hunt::L_Hunt() : attackRange(5.0f)
{}

void L_Hunt::on_enter()
{
	// Set Hunt Speed and Color
	agent->set_movement_speed(55.0f);
	agent->set_color(Vec3(2.55f, 1.65f, 0.0f));
	BehaviorNode::on_leaf_enter();
}

void L_Hunt::on_update(float dt)
{
	const Vec3& currPos = agent->get_position();

	auto& bb = agent->get_blackboard();

	/* find target's next supposed position */
	const Agent* target = bb.get_value<Agent*>("TargetPtr");

	// gather target data
	const Vec3 target_pos = target->get_position();
	const float target_dir = target->get_yaw();
	const float target_speed = target->get_movement_speed();

	// check to see if we are in attack range
	const float distance = Vec3::Distance(currPos, target_pos);
	if (distance <= attackRange)
	{
		// kill target
		// end hunt
		bb.set_value("Hunting", false);
		bb.set_value("TargetPtr", nullptr);
		agent->set_movement_speed(40.0f);
		agent->set_color(Vec3(0.0f, 1.65f, 0.0f));
		on_success();
	}

	// find target velocity
	Vec3 target_velocity = Vec3(sinf(target_dir), 0.0f, cosf(target_dir));
	target_velocity.Normalize();
	target_velocity *= target_speed;

	// get target's next position
	Vec3 target_point = target_pos + target_velocity;

	/* Move hunter towards that position */
	Vec3 velocity = Vec3(target_pos.x - currPos.x, 0.0f, target_pos.z - currPos.z);
	velocity.Normalize();
	agent->set_yaw(atan2(velocity.x, velocity.z));

	// add speed
	velocity *= agent->get_movement_speed() * dt;

	// calculate next position and set it
	agent->set_position(agent->get_position() + velocity);

    display_leaf_text();
}
