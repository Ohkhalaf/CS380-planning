/******************************************************************************/
/*!
\file		ProjectTwo.h
\project	CS380/CS580 AI Framework
\author		Dustin Holmes, Carlos Garcia-Perez, Oussama Khalaf
\summary	Specification for project four - planning

Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/

#pragma once
#include "Project.h"
#include "../Student/Project_4/P4_Pathfinding.h"
#include "Agent/AStarAgent.h"
#include "Testing/PathingTester.h"

class ProjectFour final : public Project
{
public:
    virtual bool initialize() override final;
    virtual bool finalize() override final;
    virtual void shutdown() override final;

    void clearEnemyVision();

    virtual void draw_meshes() override final;
    virtual void draw_sprites() override final;
    virtual void draw_text() override final;
    virtual void draw_debug() override final;

    virtual void update() override final;

    static bool implemented_floyd_warshall();
    static bool implemented_goal_bounding();
    static bool implemented_jps_plus();

    bool getSpawnEnemy() { return spawnEnemy; }
    void toggleSpawnEnemy() { spawnEnemy = !spawnEnemy; }

    float get_enemy_weight()
    {
        return enemyWeight;
    }

    void set_enemy_weight(const float& weight)
    {
        enemyWeight = weight;
        std::wstringstream stream;
        stream.precision(2);
        stream << weight;
        enemyWeightText = stream.str();
    }

    const std::wstring& get_enemy_weight_text()
    {
        return enemyWeightText;
    }

    float get_enemy_rotation()
    {
        return enemyRotation;
    }

    void set_enemy_rotation(const float& rotation)
    {
        enemyRotation = rotation;
        std::wstringstream stream;
        stream.precision(3);
        stream << rotation;
        enemyRotationText = stream.str();
    }

    const std::wstring& get_enemy_rotation_text()
    {
        return enemyRotationText;
    }


private:
    AStarAgent *agent;
    std::vector<EnemyAgent*> enemy;
    std::wstring enemyWeightText;
    std::wstring enemyRotationText;
    float enemyWeight = 5.0f;
    float enemyRotation = 0;
    PathTester tester;
    bool testRunning;
    bool spawnEnemy;

    std::wstring analysisFrequencyText;

    unsigned analysisFrequency;
    unsigned frequencyMod;
    unsigned frequencyOffset;

    void build_ui();
    void link_input();

    void on_left_mouse_click();
	void set_analysis_frequency(const unsigned& val);
    unsigned get_analysis_frequency() { return analysisFrequency; }
	void on_f1();
    void on_f3();

    void on_test_begin();
    void on_test_end();

    void enemy_field_of_view(MapLayer<float>& layer, float fovAngle, float closeDistance, float occupancyValue, AStarAgent* enemy);
};