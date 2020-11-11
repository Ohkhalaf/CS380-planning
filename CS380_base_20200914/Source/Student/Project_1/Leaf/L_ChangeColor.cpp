#include <pch.h>
#include "L_ChangeColor.h"

void L_ChangeColor::on_enter()
{
	BehaviorNode::on_leaf_enter();
}

void L_ChangeColor::on_update(float dt)
{
	const float MAX_NEIGHBORS = 5.0f;

	const auto& bb = agent->get_blackboard();

	// get number of neighbors
	float count = static_cast<float>(bb.get_value<int>("NeighborCount"));

	// find factor
	if (count > MAX_NEIGHBORS) count = MAX_NEIGHBORS;
	float factor =  count / MAX_NEIGHBORS;

	// change color
	agent->set_color(Vec3(20.0f * factor, 0.0f, 20.0f * factor));

    display_leaf_text();
	on_success();
}
