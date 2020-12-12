/******************************************************************************/
/*!
\file		ProjectFour.cpp
\project	CS380/CS580 AI Framework
\author		Dustin Holmes, Carlos Garcia-Perez, Oussama Khalaf
\summary	Specification for project four - planning

Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/

#include <pch.h>
#include "ProjectFour.h"

#include "Terrain/TerrainAnalysis.h"
#include "Agent/CameraAgent.h"

#include "UI/Elements/Buttons/UIButton.h"
#include "UI/Elements/Sliders/UISlider.h"
#include "UI/Elements/Text/UIValueTextField.h"
#include "UI/Elements/Buttons/UIDynamicButton.h"
#include "UI/Elements/Buttons/UIToggleButton.h"
#include "UI/Elements/Text/UIDynamicBannerTextField.h"
#include "UI/Elements/Buttons/UIConditionalButton.h"

#include "Misc/TimeTracker.h"

TimeTracker<std::chrono::microseconds> pathingTimer2;

std::wstring startPosText2;
std::wstring goalPosText2;

// Create all plans:
// for i : (from 0 to num of enemies)
//   for each enemy from 0 to i
//     for each agent
//     find closest agent to enemy through pathfinding
//     Plan += ElimateEnemy(enemy, closest_agent);
//       (update LevelState)

// For each plan
//   Plan += MovePresidentToGoal()
//     (update LevelState)

// Finally, chose the plan with the smallest total cost

const std::wstring &start_pos_text_getter2()
{
    return startPosText2;
}

const std::wstring &goal_pos_text_getter2()
{
    return goalPosText2;
}

void grid_pos_to_text2(const GridPos &pos, std::wstring &text)
{
    text = std::wstring(L"[" + std::to_wstring(pos.row) + L", " + std::to_wstring(pos.col) + L"]");
}

bool ProjectFour::initialize()
{
    std::cout << "Initializing Project Four..." << std::endl;

    // create all the systems that our project requires
    terrain = std::make_unique<Terrain>();
    agents = std::make_unique<AgentOrganizer>();
    ui = std::make_unique<UICoordinator>();
    pather = std::make_unique<AStarPather2>();

    set_analysis_frequency(15);

    // enable, show enemy vision layer
    terrain->enemyVisionLayer.set_enabled(true);

    old_layer = std::vector<float>(40 * 40, 0.0f);

    return terrain->initialize() &&
        agents->initialize() &&
        ui->initialize() &&
        pather->initialize() &&
        tester.initialize();
}

bool ProjectFour::finalize()
{
    agent = agents->create_apc_agent();
    agent->set_color(Vec3(134/256.0f, 97/256.0f, 193/256.0f));
    tester.set_agent(agent);

    /*
    enemy.push_back(agents->create_enemy_agent());
    enemy[0]->set_debug_coloring(false);
    enemy[0]->set_single_step(false);
    enemy[0]->set_color(Vec3(0.8f, 0.0f, 0.0f));
    enemy[0]->set_player(agent);
    */

    // initialize the position text
    grid_pos_to_text2(GridPos { -1, -1 }, startPosText2);
    grid_pos_to_text2(GridPos { -1, -1 }, goalPosText2);

    build_ui();

    terrain->goto_map(1);
    terrain->pathLayer.set_enabled(true);

    //tester.bootstrap();

    // set up a time tracker to listen for pathing messages
    Callback timerResetCB = std::bind(&decltype(pathingTimer2)::reset, &pathingTimer2);
    Messenger::listen_for_message(Messages::PATH_REQUEST_BEGIN, timerResetCB);

    Callback timerStartCB = std::bind(&decltype(pathingTimer2)::start, &pathingTimer2);
    Messenger::listen_for_message(Messages::PATH_REQUEST_TICK_START, timerStartCB);

    Callback timerStopCB = std::bind(&decltype(pathingTimer2)::stop, &pathingTimer2);
    Messenger::listen_for_message(Messages::PATH_REQUEST_TICK_FINISH, timerStopCB);

    Callback testBeginCB = std::bind(&ProjectFour::on_test_begin, this);
    Messenger::listen_for_message(Messages::PATH_TEST_BEGIN, testBeginCB);

    Callback testEndCB = std::bind(&ProjectFour::on_test_end, this);
    Messenger::listen_for_message(Messages::PATH_TEST_END, testEndCB);

    link_input();

    return true;
}

void ProjectFour::shutdown()
{
    std::cout << "Shutting Down Project four..." << std::endl;
    pather->shutdown();
    pather.reset();

    ui->shutdown();
    ui.reset();

    agents->shutdown();
    agents.reset();

    terrain->shutdown();
    terrain.reset();
}

void ProjectFour::clearEnemyVision()
{
    // clear off positive values
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            if (terrain->enemyVisionLayer.get_value(row, col) > 0.0f)
            {
                terrain->enemyVisionLayer.set_value(row, col, 0.0f);
            }
        }
    }
}

void ProjectFour::draw_meshes()
{
    terrain->draw();
    agents->draw();
}

void ProjectFour::draw_sprites()
{
    ui->draw_sprites();
}

void ProjectFour::draw_text()
{
    ui->draw_text();
}

void ProjectFour::draw_debug()
{
    terrain->draw_debug();
    agents->draw_debug();
    ui->draw_debug();
}

void ProjectFour::update()
{
    // have the ui coordinator determine its state
    ui->update();

    // have the input system update its current state and send out notifications
    InputHandler::update();

    propagate_solo_occupancy(terrain->seekLayer,0.3, 0.2);
    clearEnemyVision();
    for (auto i : enemy)
    {
        if (i->get_active())
        {
            i->logic_tick();
        }
    }

    CalcEnemyVisionLayer(terrain->enemyVisionLayer);

    agents->update(deltaTime);

}

void ProjectFour::build_ui()
{
    // create the first button in the top right,
    // 90 is half button width + 16 (padding from side of screen) + 10 (padding so sliders aren't directly against the edge)
    // 32 is half button height + 16 (padding from top of screen)
    Callback nextMapCB = std::bind(&Terrain::goto_next_map, terrain.get());
    auto mapButton = ui->create_button(UIAnchor::TOP_RIGHT, -90, 32, nextMapCB, L"Next Map");

    // just below the map button, add a button that transitions through pathfinding algorithms
    Callback methodCB = std::bind(&AStarAgent::next_method_type, agent);
    TextGetter methodText = std::bind(&AStarAgent::get_method_text, agent);
    auto methodButton = ui->create_dynamic_button(UIAnchor::BOTTOM, mapButton,
        10, methodCB, methodText);

    // then a button for the heuristic to use
    Callback heuristicCB = std::bind(&AStarAgent::next_heuristic_type, agent);
    TextGetter heuristicText = std::bind(&AStarAgent::get_heuristic_type_text, agent);
    auto heuristicButton = ui->create_dynamic_button(UIAnchor::BOTTOM, methodButton,
        10, heuristicCB, heuristicText);

   // then a slider to control the weight
   Getter<float> weightGet = std::bind(&AStarAgent::get_heuristic_weight, agent);
   Setter<float> weightSet = std::bind(&AStarAgent::set_heuristic_weight, agent, std::placeholders::_1);
   TextGetter weightText = std::bind(&AStarAgent::get_heuristic_weight_text, agent);
   auto weightSlider = ui->create_slider<float>(UIAnchor::BOTTOM, heuristicButton,
       10, 0.0f, 2.0f, weightGet, weightSet, weightText, L"Weight:");

    // then a button for smoothing
    Callback smoothingCB = std::bind(&AStarAgent::toggle_smoothing, agent);
    Getter<bool> smoothingGet = std::bind(&AStarAgent::get_smoothing, agent);
    auto smoothingButton = ui->create_toggle_button(UIAnchor::BOTTOM, weightSlider,
        10, smoothingCB, L"Smoothing", smoothingGet);

    // then a button for rubberbanding
    Callback rubberCB = std::bind(&AStarAgent::toggle_rubberbanding, agent);
    Getter<bool> rubberGet = std::bind(&AStarAgent::get_rubberbanding, agent);
    auto rubberButton = ui->create_toggle_button(UIAnchor::BOTTOM, smoothingButton,
        10, rubberCB, L"Rubberbanding", rubberGet);

    // then a button for single step
    Callback singleCB = std::bind(&AStarAgent::toggle_single_step, agent);
    Getter<bool> singleGet = std::bind(&AStarAgent::get_single_step, agent);
    auto singleButton = ui->create_toggle_button(UIAnchor::BOTTOM, rubberButton,
        10, singleCB, L"Single Step", singleGet);

    // then a button for toggling enemy spawn
    Callback spawnCB = std::bind(&ProjectFour::toggleSpawnEnemy, this);
    Getter<bool> spawnGet = std::bind(&ProjectFour::getSpawnEnemy, this);
    auto enemyButton = ui->create_toggle_button(UIAnchor::BOTTOM, singleButton,
        10, spawnCB, L"Spawn Enemy", spawnGet);

    //a slider to control the enemyweight
    Getter<float> enemyRotationGet = std::bind(&ProjectFour::get_enemy_rotation, this);
    Setter<float> enemyRotationSet = std::bind(&ProjectFour::set_enemy_rotation, this, std::placeholders::_1);
    TextGetter enemyRotationText = std::bind(&ProjectFour::get_enemy_rotation_text, this);
    auto enemyRotationSlider = ui->create_slider<float>(UIAnchor::BOTTOM, enemyButton,
        20, 0.0f, 360.0f, enemyRotationGet, enemyRotationSet, enemyRotationText, L"Rotation:");

    //a slider to control the enemyweight
    Getter<float> enemyWeightGet = std::bind(&ProjectFour::get_enemy_weight, this);
    Setter<float> enemyWeightSet = std::bind(&ProjectFour::set_enemy_weight, this, std::placeholders::_1);
    TextGetter enemyWeightText = std::bind(&ProjectFour::get_enemy_weight_text, this);
    auto enemyWeightSlider = ui->create_slider<float>(UIAnchor::BOTTOM, enemyRotationSlider,
        10, 0.0f, 20.0f, enemyWeightGet, enemyWeightSet, enemyWeightText, L"Enemy Wght:");

    Callback EnemyDeleteCB = std::bind(&ProjectFour::delete_enemies, this);
    TextGetter EnemyDeleteText = std::bind(&ProjectFour::get_delete_enemy_text, this);
    auto EnemyDeleteButton = ui->create_dynamic_button(UIAnchor::BOTTOM, enemyWeightSlider, 10,
        EnemyDeleteCB, EnemyDeleteText);

    // then a button for debug coloring
    Callback debugCB = std::bind(&AStarAgent::toggle_debug_coloring, agent);
    Getter<bool> debugGet = std::bind(&AStarAgent::get_debug_coloring, agent);
    auto debugButton = ui->create_toggle_button(UIAnchor::BOTTOM, EnemyDeleteButton,
        10, debugCB, L"Debug Coloring", debugGet);

    // then a button for movement
    Callback movementCB = std::bind(&AStarAgent::next_movement_type, agent);
    TextGetter movementGet = std::bind(&AStarAgent::get_movement_type_text, agent);
    auto movementButton = ui->create_dynamic_button(UIAnchor::BOTTOM, debugButton,
        10, movementCB, movementGet);


    // add some text on the left side for displaying fps
    TextGetter fpsGetter = std::bind(&Engine::get_fps_text, engine.get());
    auto fpsText = ui->create_value_text_field(UIAnchor::TOP_LEFT, 90, 32, L"FPS:", fpsGetter);

    // and the pathing time
    TextGetter timeGetter = std::bind(&decltype(pathingTimer2)::get_text, &pathingTimer2);
    auto pathingTimeText = ui->create_value_text_field(UIAnchor::BOTTOM, fpsText,
        10, L"Time:", timeGetter);

    // and the current path start position
    TextGetter startGetter = start_pos_text_getter2;
    auto startText = ui->create_value_text_field(UIAnchor::BOTTOM, pathingTimeText,
        10, L"Start:", startGetter);

    // and the current path goal position
    TextGetter goalGetter = goal_pos_text_getter2;
    auto goalText = ui->create_value_text_field(UIAnchor::BOTTOM, startText,
        10, L"Goal:", goalGetter);

    Callback testCB = std::bind(&PathTester::execute_current_test, &tester);
    TextGetter testText = std::bind(&PathTester::get_button_text, &tester);
    auto testButton = ui->create_dynamic_button(UIAnchor::BOTTOM, goalText, 10,
        testCB, testText);

    Callback nextTestCB = std::bind(&PathTester::goto_next_test, &tester);
    auto nextTestButton = ui->create_button(UIAnchor::BOTTOM, testButton, 10,
        nextTestCB, L"Next Test");

    Callback allTestsCB = std::bind(&PathTester::execute_all_tests, &tester);
    auto allTestsButton = ui->create_button(UIAnchor::BOTTOM, nextTestButton, 10,
        allTestsCB, L"Run All Tests");

    Callback failureCB = std::bind(&PathTester::goto_next_failed, &tester);
    Getter<bool> failureCond = std::bind(&PathTester::has_multiple_failed_tests, &tester);
    auto failureButton = ui->create_conditional_button(UIAnchor::BOTTOM, allTestsButton,
        10, failureCB, L"Next Failed Test", failureCond);

    Callback speedCB = std::bind(&PathTester::execute_speed_test, &tester);
    auto speedTestButton = ui->create_button(UIAnchor::BOTTOM, failureButton, 10,
        speedCB, L"Run Speed Test");



    // add a text field at the top for the project
    auto projectBanner = ui->create_banner_text_field(UIAnchor::TOP, 0, 32,
        UIAnchor::CENTER, L"Student Project");

    // add a banner to display overall testing results
    TextGetter testingText = std::bind(&PathTester::get_status_text, &tester);
    auto testingBanner = ui->create_dynamic_banner_text_field(UIAnchor::BOTTOM,
        0, -16, UIAnchor::CENTER, testingText);

    // and a banner above that to display test specific failure messages
    TextGetter failMSGText = std::bind(&PathTester::get_failed_text, &tester);
    auto failedBanner = ui->create_dynamic_banner_text_field(UIAnchor::TOP,
        testingBanner, 10, UIAnchor::CENTER, failMSGText);
}


void ProjectFour::link_input()
{
    Callback escapeCB = std::bind(&Engine::stop_engine, engine.get());
    InputHandler::notify_when_key_pressed(KBKeys::ESCAPE, escapeCB);

    Callback leftMouseCB = std::bind(&ProjectFour::on_left_mouse_click, this);
    InputHandler::notify_when_mouse_pressed(MouseButtons::LEFT, leftMouseCB);

    Callback f1CB = std::bind(&ProjectFour::on_f1, this);
    InputHandler::notify_when_key_pressed(KBKeys::F1, f1CB);

    Callback f3CB = std::bind(&ProjectFour::on_f3, this);
    InputHandler::notify_when_key_pressed(KBKeys::F3, f3CB);
}

void ProjectFour::on_left_mouse_click()
{
    const auto &mousePos = InputHandler::get_mouse_position();

    // convert the mouse position to a point on the terrain plane
    const auto worldPos = renderer->screen_to_world(mousePos.x, mousePos.y, terrain->get_terrain_plane());

    // verify a valid point was determine
    if (worldPos.second == true)
    {

        // convert the world position to a terraing grid position
        const auto gridPos = terrain->get_grid_position(worldPos.first);

        // verify that the grid position is valid and not a wall
        if (terrain->is_valid_grid_position(gridPos) && !terrain->is_wall(gridPos))
        {
            if (spawnEnemy)
            {
                enemy.push_back(agents->create_enemy_agent());
                enemy[enemy.size() - 1]->set_debug_coloring(false);
                enemy[enemy.size() - 1]->set_single_step(false);
                enemy[enemy.size() - 1]->set_color(Vec3(0.8f, 0.0f, 0.0f));
                enemy[enemy.size() - 1]->set_yaw(enemyRotation);
                enemy[enemy.size() - 1]->set_player(agent);
                enemy[enemy.size() - 1]->set_position(terrain->get_world_position(terrain->get_grid_position(worldPos.first)));
            }
            else
            {

                const auto agentPos = terrain->get_grid_position(agent->get_position());

                grid_pos_to_text2(gridPos, goalPosText2);
                grid_pos_to_text2(agentPos, startPosText2);

                // have the agent path to the position
                agent->path_to(worldPos.first);
            }
        }
    }
    
}

//sets the ticks per second
void ProjectFour::set_analysis_frequency(const unsigned& val)
{
    analysisFrequency = val;
    analysisFrequencyText = std::to_wstring(analysisFrequency);

    frequencyMod = 60 / analysisFrequency;
    frequencyOffset = frequencyMod / 4;
}

void ProjectFour::on_f1()
{
    engine->change_projects(Project::Type::ONE);
}

void ProjectFour::on_f3()
{
    engine->change_projects(Project::Type::THREE);
}

void ProjectFour::on_test_begin()
{
    testRunning = true;
}

void ProjectFour::on_test_end()
{
    testRunning = false;
}

void ProjectFour::enemy_field_of_view(MapLayer<float>& layer, float fovAngle, float closeDistance, float occupancyValue, AStarAgent* enemy)
{
    /*
        First, clear out the old values in the map layer by setting any positive value to 0.
        Then, for every cell in the layer that is within the field of view cone, from the
        enemy agent, mark it with the occupancy value.  Take the dot product between the view
        vector and the vector from the agent to the cell, both normalized, and compare the
        cosines directly instead of taking the arccosine to avoid introducing floating-point
        inaccuracy (larger cosine means smaller angle).

        If the tile is close enough to the enemy (less than closeDistance),
        you only check if it's visible to enemy.  Make use of the is_clear_path
        helper function.  Otherwise, you must consider the direction the enemy is facing too.
        This creates a radius around the enemy that the player can be detected within, as well
        as a fov cone.
    */


    // get the position of the agent and the direction the agent is viewing
    GridPos grid_pos = terrain->get_grid_position(enemy->get_position());
    Vec2 enemy_pos = Vec2(terrain->get_world_position(grid_pos).x, terrain->get_world_position(grid_pos).z);
    Vec2 view_dir = Vec2(enemy->get_forward_vector().x, enemy->get_forward_vector().z);

    // normalize the viewing vector
    view_dir.Normalize();

    layer.set_value(grid_pos.row, grid_pos.col, occupancyValue);

    // TEMP TODO
    /*
    // for every cell in the layer
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            // first check if the agent can see the square
            if (is_clear_path(grid_pos.row, grid_pos.col, row, col))
            {
                // get the vector from the square to the player
                Vec2 square_pos = Vec2(terrain->get_world_position(row, col).x, terrain->get_world_position(row, col).z);
                Vec2 square_dir = square_pos - enemy_pos;
                square_dir.Normalize();

                // dot product the vectors
                float cosine = view_dir.Dot(square_dir);

                // compare cosines to create viewing cone
                if (cosine >= cos((fovAngle * PI) / 360.0f))
                {
                    layer.set_value(row, col, occupancyValue);
                    continue;
                }

                // create radius (testing the example solution, it only makes sense to use row and col)
                float dx = static_cast<float>(row - grid_pos.row);//square_pos.x - enemy_pos.x;
                float dy = static_cast<float>(col - grid_pos.col);//square_pos.y - enemy_pos.y;
                float distance = sqrt((dx * dx) + (dy * dy));
                if (distance < closeDistance)
                {
                    layer.set_value(row, col, occupancyValue);
                }
            }
        }
    }
    */
}

// clears out the given map layer
void ProjectFour::ClearLayer(MapLayer<float>& layer)
{
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            layer.set_value(row, col, 0.0f);
        }
    }
}

void CompareLayers(const std::vector<float>& layer1, const MapLayer<float>& layer2)
{
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            if (layer1[row * terrain->get_map_height() + col] != layer2.get_value(row, col))
            {
                AStarPather2::Square& affected = pather->getSquare(row, col);
                affected.isDirty = true;

                // TODO: I think we need to do this?                          <---------------------------- I'M NOT SURE
                // MAYBE THIS IS WHERE WE SET RAISED VERSUS LOWERED STATE????
                //affected.curcost -= layer1.get_value(row, col);
                //affected.curcost += layer2.get_value(row, col);
                //affected.curcost += 5000.0f;
            }
        }
    }
}

// gets the vision of all given enemies onto a layer
void ProjectFour::CalcEnemyVisionLayer(MapLayer<float>& layer)
{
    for (int i = 0; i < static_cast<int>(enemy.size()); ++i)
    {
        if (enemy[i]->get_active())
        {
            enemy_field_of_view(layer, 120.0f, 1.5f, get_enemy_weight(), enemy[i]);
        }
    }

    // check for changes
    CompareLayers(old_layer, layer);

    
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            old_layer[row * terrain->get_map_height() + col] = layer.get_value(row, col);
        }
    }
    
}

// TASK
    // > removes given enemy from list
    // > places given soldier in enemy position
    // > calculates new layer to come from enemy removal
    // > adds pathfinding cost to total cost
void ProjectFour::EliminateEnemy(const EnemyAgent* e, const SoldierAgent* s, ProjectFour::MapState& state)
{
    // TODO: Call new pathfinding function that returns cost
    PathRequest request;
    request.newRequest = true;
    //pather->compute_path_cost()

    // find soldier
    for (int i = 0; i < static_cast<int>(state.soldiers.size()); ++i)
    {
        // place soldier in enemy's position
        if (state.soldiers[i] == s)
        {
            state.soldiers[i]->set_position(e->get_position());
        }
    }

    // find enemy
    for (int i = 0; i < static_cast<int>(enemy.size()); ++i)
    {
        // deactivate enemy
        if (enemy[i] == e)
        {
            enemy[i]->set_active(false);
            break;
        }
    }

    // calculate new enemy vision layer
    ClearLayer(state.layer);
    CalcEnemyVisionLayer(state.layer);
};