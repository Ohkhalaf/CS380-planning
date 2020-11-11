#include <pch.h>
#include "Terrain/MapMath.h"
#include "Agent/AStarAgent.h"
#include "Terrain/MapLayer.h"
#include "Terrain/TerrainAnalysis.h"
#include "Projects/ProjectThree.h"

#include <iostream>

bool ProjectThree::implemented_fog_of_war() const // extra credit
{
    return false;
}

float distance_to_closest_wall(int row, int col)
{
    /*
        Check the euclidean distance from the given cell to every other wall cell,
        with cells outside the map bounds treated as walls, and return the smallest
        distance.  Make use of the is_valid_grid_position and is_wall member
        functions in the global terrain to determine if a cell is within map bounds
        and a wall, respectively.
    */

    float min_dist = std::numeric_limits<float>::max();

    // for each wall or corner of the map
    for (int r = -1; r <= terrain->get_map_height(); ++r)
    {
        for (int c = -1; c <= terrain->get_map_width(); ++c)
        {
            if (!terrain->is_valid_grid_position(r, c) || terrain->is_wall(r, c))
            {
                // find the distance to given square
                float dx = static_cast<float>(r - row);
                float dy = static_cast<float>(c - col);
                float distance = static_cast<float>(sqrt((dx * dx) + (dy * dy)));
                // if it's the closest wall, rewrite the distance
                if (min_dist > distance)
                {
                    min_dist = distance;
                }
            }
        }
    }
    
    return min_dist;
}

bool is_clear_path(int row0, int col0, int row1, int col1)
{
    /*
        Two cells (row0, col0) and (row1, col1) are visible to each other if a line
        between their centerpoints doesn't intersect the four boundary lines of every
        wall cell.  You should puff out the four boundary lines by a very tiny amount
        so that a diagonal line passing by the corner will intersect it.  Make use of the
        line_intersect helper function for the intersection test and the is_wall member
        function in the global terrain to determine if a cell is a wall or not.
    */

    // same square, skip
    if (row0 == row1 && col0 == col1) return true;

    // length of a square
    float size = terrain->get_world_position(1, 0).x - terrain->get_world_position(0, 0).x;

    // line of sight points
    Vec3 p0_3 = terrain->get_world_position(row0, col0);
    Vec3 p1_3 = terrain->get_world_position(row1, col1);
    Vec2 p0 = Vec2(p0_3.x, p0_3.z);
    Vec2 p1 = Vec2(p1_3.x, p1_3.z);


    // border of squares to check
    int r = (row0 < row1) ? row0 : row1;
    int rows = (row0 > row1) ? row0 : row1;
    int cols = (col0 > col1) ? col0 : col1;

    // for each space in the line's rectangle
    for (; r <= rows; ++r)
    {
        for (int c = (col0 < col1) ? col0 : col1; c <= cols; ++c)
        {
            // if there is a wall in that rectangle
            if (terrain->is_wall(r,c))
            {
                // get the position in world of the wall
                Vec3 wall_pos = terrain->get_world_position(r,c);

                // calculate the wall's corner points (plus a bit more)
                Vec2 bot_right = Vec2(wall_pos.x - (size * 0.52f), wall_pos.z + (size * 0.52f));
                Vec2 top_right = Vec2(wall_pos.x + (size * 0.52f), wall_pos.z + (size * 0.52f));
                Vec2 bot_left = Vec2(wall_pos.x - (size * 0.52f), wall_pos.z - (size * 0.52f));
                Vec2 top_left = Vec2(wall_pos.x + (size * 0.52f), wall_pos.z - (size * 0.52f));

                // check each side of the wall for an intersect
                if (line_intersect(p0, p1, top_right, top_left)  || 
                    line_intersect(p0, p1, top_left, bot_left) ||
                    line_intersect(p0, p1, bot_left, bot_right)  ||
                    line_intersect(p0, p1, bot_right, top_right))
                {
                    return false;
                }
            }
        }
    }

    // no intersect with a wall found
    return true;
}

void analyze_openness(MapLayer<float> &layer)
{
    /*
        Mark every cell in the given layer with the value 1 / (d * d),
        where d is the distance to the closest wall or edge.  Make use of the
        distance_to_closest_wall helper function.  Walls should not be marked.
    */

    // for each valid empty square on the map
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            if (terrain->is_valid_grid_position(row, col) && !terrain->is_wall(row, col))
            {
                // TODO: Check for zero distance?
                // find the distance to the closest wall
                float dist = distance_to_closest_wall(row, col);
                layer.set_value(row, col, 1.0f / (dist * dist));
            }
        }
    }
}

void analyze_visibility(MapLayer<float> &layer)
{
    /*
        Mark every cell in the given layer with the number of cells that
        are visible to it, divided by 160 (a magic number that looks good).  Make sure
        to cap the value at 1.0 as well.

        Two cells are visible to each other if a line between their centerpoints doesn't
        intersect the four boundary lines of every wall cell.  Make use of the is_clear_path
        helper function.
    */

    float magic_number = 160.0f;

    // for each valid empty square on the map
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            if (terrain->is_valid_grid_position(row, col) && !terrain->is_wall(row, col))
            {
                // count how many squares this square has a clear path to
                int count = 0;
                for (int r = 0; r < terrain->get_map_height(); ++r)
                {
                    for (int c = 0; c < terrain->get_map_width(); ++c)
                    {
                        if (terrain->is_valid_grid_position(r, c) && !terrain->is_wall(r, c))
                        {
                            if (is_clear_path(row, col, r, c)) { ++count; }
                        }
                    }
                } // done with inner double for loop

                layer.set_value(row, col, (count > magic_number) ? 1.0f : count / magic_number);
            }
        }
    } // done with outer double for loop
}

void analyze_visible_to_cell(MapLayer<float> &layer, int row, int col)
{
    /*
        For every cell in the given layer mark it with 1.0
        if it is visible to the given cell, 0.5 if it isn't visible but is next to a visible cell,
        or 0.0 otherwise.

        Two cells are visible to each other if a line between their centerpoints doesn't
        intersect the four boundary lines of every wall cell.  Make use of the is_clear_path
        helper function.
    */

    // for each valid empty square on the map
    for (int r = 0; r < terrain->get_map_height(); ++r)
    {
        for (int c = 0; c < terrain->get_map_width(); ++c)
        {
            if (terrain->is_valid_grid_position(r, c) && !terrain->is_wall(r, c))
            {
                // if there is a clear path to this square
                if (is_clear_path(row, col, r, c))
                {
                    // set that square value as visible
                    layer.set_value(r, c, 1.0f);
                }
                else
                {
                    layer.set_value(r, c, 0.0f);
                }
            }
        }
    }

    // for each valid empty square on the map that isn't visible
    for (int r = 0; r < terrain->get_map_height(); ++r)
    {
        for (int c = 0; c < terrain->get_map_width(); ++c)
        {
            if (terrain->is_valid_grid_position(r, c) && !terrain->is_wall(r, c) && layer.get_value(r, c) != 1.0f)
            {
                bool neighborIsVisible = false;
                /*
                // check neigbors
                for (int i = r - 1; i <= r + 1; ++i)
                {
                    for (int j = c - 1; j <= c + 1; ++j)
                    {
                        if (terrain->is_valid_grid_position(i, j) && !terrain->is_wall(i, j) && layer.get_value(i, j) == 1.0f)
                        {
                            if (i == r || j == c)
                            {
                                // not a diagonal
                                neighborIsVisible = true;
                            }
                            else
                            {
                                // properly check diagonals
                            }
                        }
                    }
                } // end the double for loop for neighbors
                */

                /* Check Neighbors */

                // up, down, left, right
                if (terrain->is_valid_grid_position(r + 1, c) && !terrain->is_wall(r + 1, c) && layer.get_value(r + 1, c) == 1.0f)
                {
                    neighborIsVisible = true; // up
                }
                else if (terrain->is_valid_grid_position(r - 1, c) && !terrain->is_wall(r - 1, c) && layer.get_value(r - 1, c) == 1.0f)
                {
                    neighborIsVisible = true; // down
                }
                else if (terrain->is_valid_grid_position(r, c - 1) && !terrain->is_wall(r, c - 1) && layer.get_value(r, c - 1) == 1.0f)
                {
                    neighborIsVisible = true; // left
                }
                else if (terrain->is_valid_grid_position(r, c + 1) && !terrain->is_wall(r, c + 1) && layer.get_value(r, c + 1) == 1.0f)
                {
                    neighborIsVisible = true; // right
                }

                // diagonals
                if (terrain->is_valid_grid_position(r + 1, c) && terrain->is_valid_grid_position(r, c + 1) && !terrain->is_wall(r + 1, c) && !terrain->is_wall(r, c + 1))
                {
                    if (terrain->is_valid_grid_position(r + 1, c + 1) && !terrain->is_wall(r + 1, c + 1) && layer.get_value(r + 1, c + 1) == 1.0f)
                    {
                        neighborIsVisible = true; // up right
                    }
                }
                if (terrain->is_valid_grid_position(r + 1, c) && terrain->is_valid_grid_position(r, c - 1) && !terrain->is_wall(r + 1, c) && !terrain->is_wall(r, c - 1))
                {
                    if (terrain->is_valid_grid_position(r + 1, c - 1) && !terrain->is_wall(r + 1, c - 1) && layer.get_value(r + 1, c - 1) == 1.0f)
                    {
                        neighborIsVisible = true; // up left
                    }
                }
                if (terrain->is_valid_grid_position(r - 1, c) && terrain->is_valid_grid_position(r, c + 1) && !terrain->is_wall(r - 1, c) && !terrain->is_wall(r, c + 1))
                {
                    if (terrain->is_valid_grid_position(r - 1, c + 1) && !terrain->is_wall(r - 1, c + 1) && layer.get_value(r - 1, c + 1) == 1.0f)
                    {
                        neighborIsVisible = true; // down right
                    }
                }
                if (terrain->is_valid_grid_position(r - 1, c) && terrain->is_valid_grid_position(r, c - 1) && !terrain->is_wall(r - 1, c) && !terrain->is_wall(r, c - 1))
                {
                    if (terrain->is_valid_grid_position(r - 1, c - 1) && !terrain->is_wall(r - 1, c - 1) && layer.get_value(r - 1, c - 1) == 1.0f)
                    {
                        neighborIsVisible = true; // down left
                    }
                }

                // if there was a visible neigbor, set this square's value
                if (neighborIsVisible)
                {
                    layer.set_value(r, c, 0.5f);
                }
            }
        }
    } // end the double for loop for each square on the map
}

void analyze_agent_vision(MapLayer<float> &layer, const Agent *agent)
{
    /*
        For every cell in the given layer that is visible to the given agent,
        mark it as 1.0, otherwise don't change the cell's current value.

        You must consider the direction the agent is facing.  All of the agent data is
        in three dimensions, but to simplify you should operate in two dimensions, the XZ plane.

        Take the dot product between the view vector and the vector from the agent to the cell,
        both normalized, and compare the cosines directly instead of taking the arccosine to
        avoid introducing floating-point inaccuracy (larger cosine means smaller angle).

        Give the agent a field of view slighter larger than 180 degrees.

        Two cells are visible to each other if a line between their centerpoints doesn't
        intersect the four boundary lines of every wall cell.  Make use of the is_clear_path
        helper function.
    */

    // get the position of the agent and the direction the agent is viewing
    GridPos grid_pos = terrain->get_grid_position(agent->get_position());
    Vec2 agent_pos = Vec2(terrain->get_world_position(grid_pos).x, terrain->get_world_position(grid_pos).z);
    Vec2 view_dir = Vec2(agent->get_forward_vector().x, agent->get_forward_vector().z);

    // normalize the viewing vector
    view_dir.Normalize();

    // for every square on the map
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            // first check if the agent can see the square
            if (is_clear_path(grid_pos.row, grid_pos.col, row, col))
            {
                // get the vector from the square to the player
                Vec2 square_pos = Vec2(terrain->get_world_position(row, col).x, terrain->get_world_position(row, col).z);
                Vec2 square_dir = square_pos - agent_pos;
                square_dir.Normalize();

                // dot product the vectors
                float cosine = view_dir.Dot(square_dir);

                // give an angle slightly larger than 180 degrees
                // Based on TA Robert's suggestion, I'm using 185 degrees here: cos(185/2) = -0.0436
                if (cosine >= -0.0436)
                {
                    layer.set_value(row, col, 1.0f);
                }
            }
        }
    }

}

void propagate_solo_occupancy(MapLayer<float> &layer, float decay, float growth)
{
    /*
        For every cell in the given layer:

            1) Get the value of each neighbor and apply decay factor
            2) Keep the highest value from step 1
            3) Linearly interpolate from the cell's current value to the value from step 2
               with the growing factor as a coefficient.  Make use of the lerp helper function.
            4) Store the value from step 3 in a temporary layer.
               A float[40][40] will suffice, no need to dynamically allocate or make a new MapLayer.

        After every cell has been processed into the temporary layer, write the temporary layer into
        the given layer;
    */

    float copy[40][40];

    // for every cell in the given layer
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            // skip if wall
            if (terrain->is_wall(row, col))
            {
                copy[row][col] = 0.0f;
                continue;
            }

            // find the largest influencial neighbor after decay
            float largest = 0.0f;
            for (int r = row - 1; r <= row + 1; ++r)
            {
                for (int c = col - 1; c <= col + 1; ++c)
                {
                    // has to be a valid empty position and not itself
                    if (terrain->is_valid_grid_position(r, c) && !terrain->is_wall(r,c) && is_clear_path(row, col, r, c) && !(r == row && c == col))
                    {
                        // calculate influence after decay
                        float influence = layer.get_value(r, c);
                        if (r == row || c == col)
                        {
                            influence *= static_cast<float>(exp(-1 * decay));
                        }
                        else
                        {
                            influence *= static_cast<float>(exp(-sqrt(2) * decay));
                        }
                        // update largest influence
                        if (largest < influence)
                        {
                            largest = influence;
                        }
                    }
                }
            }// done checking neighbors

            // lerp to largest influence and set in the layer copy
            copy[row][col] = lerp(layer.get_value(row, col), largest, growth);
        }
    } // done with every tile

    // pass over the final values back into layer
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            layer.set_value(row, col, copy[row][col]);
        }
    }
}

void propagate_dual_occupancy(MapLayer<float> &layer, float decay, float growth)
{
    /*
        Similar to the solo version, but the values range from -1.0 to 1.0, instead of 0.0 to 1.0

        For every cell in the given layer:

        1) Get the value of each neighbor and apply decay factor
        2) Keep the highest ABSOLUTE value from step 1
        3) Linearly interpolate from the cell's current value to the value from step 2
           with the growing factor as a coefficient.  Make use of the lerp helper function.
        4) Store the value from step 3 in a temporary layer.
           A float[40][40] will suffice, no need to dynamically allocate or make a new MapLayer.

        After every cell has been processed into the temporary layer, write the temporary layer into
        the given layer;
    */

    // WRITE YOUR CODE HERE
}

void normalize_solo_occupancy(MapLayer<float> &layer)
{
    /*
        Determine the maximum value in the given layer, and then divide the value
        for every cell in the layer by that amount.  This will keep the values in the
        range of [0, 1].  Negative values should be left unmodified.
    */

    // find the largest value in the layer
    float largest = 0.0f;
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            if (largest < layer.get_value(row, col))
            {
                largest = layer.get_value(row, col);
            }
        }
    }

    // if the largest value is zero, skip
    if (largest == 0.0f) return;

    // if the largest value is one, no division needed
    if (largest >= 1.0f) return;

    // divide by the largest value
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            layer.set_value(row, col, layer.get_value(row, col) / largest);
        }
    }
}

void normalize_dual_occupancy(MapLayer<float> &layer)
{
    /*
        Similar to the solo version, but you need to track greatest positive value AND 
        the least (furthest from 0) negative value.

        For every cell in the given layer, if the value is currently positive divide it by the
        greatest positive value, or if the value is negative divide it by -1.0 * the least negative value
        (so that it remains a negative number).  This will keep the values in the range of [-1, 1].
    */

    // WRITE YOUR CODE HERE
}

void enemy_field_of_view(MapLayer<float> &layer, float fovAngle, float closeDistance, float occupancyValue, AStarAgent *enemy)
{
    /*
        First, clear out the old values in the map layer by setting any negative value to 0.
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

    // clear off negative values
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            if (layer.get_value(row, col) < 0.0f)
            {
                layer.set_value(row, col, 0.0f);
            }
        }
    }

    // get the position of the agent and the direction the agent is viewing
    GridPos grid_pos = terrain->get_grid_position(enemy->get_position());
    Vec2 enemy_pos = Vec2(terrain->get_world_position(grid_pos).x, terrain->get_world_position(grid_pos).z);
    Vec2 view_dir = Vec2(enemy->get_forward_vector().x, enemy->get_forward_vector().z);

    // normalize the viewing vector
    view_dir.Normalize();

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
                if (cosine >= cos((fovAngle * PI)/360.0f))
                {
                    layer.set_value(row, col, occupancyValue);
                    continue;
                }

                // create radius (testing the example solution, it only makes sense to use row and col)
                float dx = static_cast<float>(row - grid_pos.row);//square_pos.x - enemy_pos.x;
                float dy = static_cast<float>(col - grid_pos.col);//square_pos.y - enemy_pos.y;
                float distance = sqrt((dx*dx) + (dy*dy));
                if (distance < closeDistance)
                {
                    layer.set_value(row, col, occupancyValue);
                }
            }
        }
    }
}

bool enemy_find_player(MapLayer<float> &layer, AStarAgent *enemy, Agent *player)
{
    /*
        Check if the player's current tile has a negative value, ie in the fov cone
        or within a detection radius.
    */

    const auto &playerWorldPos = player->get_position();

    const auto playerGridPos = terrain->get_grid_position(playerWorldPos);

    // verify a valid position was returned
    if (terrain->is_valid_grid_position(playerGridPos) == true)
    {
        if (layer.get_value(playerGridPos) < 0.0f)
        {
            return true;
        }
    }

    // player isn't in the detection radius or fov cone, OR somehow off the map
    return false;
}

bool enemy_seek_player(MapLayer<float> &layer, AStarAgent *enemy)
{
    /*
        Attempt to find a cell with the highest nonzero value (normalization may
        not produce exactly 1.0 due to floating point error), and then set it as
        the new target, using enemy->path_to.

        If there are multiple cells with the same highest value, then pick the
        cell closest to the enemy.

        Return whether a target cell was found.
    */
    // get row and coll the enemy is in
    GridPos enemy_pos = terrain->get_grid_position(enemy->get_position());

    // variables to save closest highest value cell
    float highest = 0.0f;
    int h_row = -1;
    int h_col = -1;

    // go through all empty cells in the layer
    for (int row = 0; row < terrain->get_map_height(); ++row)
    {
        for (int col = 0; col < terrain->get_map_width(); ++col)
        {
            // skip if wall
            if (terrain->is_wall(row, col)) continue;

            // get the cell's value
            float value = layer.get_value(row, col);

            // if it's the highest value so far
            if (highest < value)
            {
                // mark as the highest value cell
                highest = value;
                h_row = row;
                h_col = col;
            }
            else if (highest == value && !(row == enemy_pos.row && col == enemy_pos.col)) // value is equal to the highest value cell
            {
                // if we don't have a highest value cell yet, this is it
                if (h_row == -1)
                {
                    // mark as the highest value cell
                    highest = value;
                    h_row = row;
                    h_col = col;
                }
                else
                {
                    // get distance to highest value cell
                    float hdx = static_cast<float>(h_row - enemy_pos.row);
                    float hdy = static_cast<float>(h_col - enemy_pos.col);
                    float h_dist = sqrt((hdx * hdx) + (hdy * hdy));

                    // get distance to contender
                    float dx = static_cast<float>(row - enemy_pos.row);
                    float dy = static_cast<float>(col - enemy_pos.col);
                    float dist = sqrt((dx * dx) + (dy * dy));

                    // if contender is closer, replace highest value cell
                    if (dist < h_dist)
                    {
                        // mark as the highest value cell
                        highest = value;
                        h_row = row;
                        h_col = col;
                    }
                }
            } // end  if (highest < value)
        }
    } // end of double for loop

    // return false if no cell was found
    if (h_row == -1) return false;

    // WHY DON'T WE RETURN FALSE WHEN THE HIGHEST VALUE IS ZERO? THE RANDOM SEEKING BEHAVIOR LOOKS BETTER THAT WAY.
    // return false if no cell has a value higher than zero
    //if (highest == 0.0f) return false;

    enemy->path_to(terrain->get_world_position(h_row, h_col));
    return true;
}
