/******************************************************************************/
/*!
\file		EnemyAgent.h
\project	CS380/CS580 AI Framework
\author		Dustin Holmes
\summary	Agent capable of searching for another agent declaration

Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/

#pragma once
#include "AStarAgent.h"

class EnemyAgent final : public AStarAgent
{
public:
    EnemyAgent(size_t id);
    virtual void path_to(const Vec3 &point);

    bool logic_tick();

    float get_fov();
    void set_fov(const float &val);
    const std::wstring &get_fov_text();

    float get_radius();
    void set_radius(const float &val);
    const std::wstring &get_radius_text();

    void set_player(AStarAgent *player);

    void set_active(bool newActive) { active = newActive; }
    bool get_active() { return active; }

private:
    enum class State
    {
        IDLE,
        CHASE,
        SEEK,
        PATROL
    } state;

    float fov;
    float radius;
    std::wstring fovText;
    std::wstring radiusText;
    AStarAgent *player;
    GridPos playerPrevious;
    bool active = true;
    float timer;

    static constexpr float movementSpeed = 0.0f;
    static constexpr float repathFactor = 0.6f;
    static constexpr float reactTimeIdle = 0.4f;
    static constexpr float reactTimeSeek = 0.1f;

    void choose_random_goal();
    bool update_timer(float time);
};
