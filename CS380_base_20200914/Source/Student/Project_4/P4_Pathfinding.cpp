#include <pch.h>
#include "Projects/ProjectFour.h"
#include "P4_Pathfinding.h"
#include "Terrain/TerrainAnalysis.h"

#define ENEMY_VISION_WEIGHT 5.0f

#define RAISED_COST 5000.0f

#pragma region Extra Credit
bool ProjectFour::implemented_floyd_warshall()
{
    return false;
}

bool ProjectFour::implemented_goal_bounding()
{
    return false;
}

bool ProjectFour::implemented_jps_plus()
{
    return false;
}
#pragma endregion

void AStarPather2::printSquare(const Square& s)
{
    std::cout << "Printing Square...\n";

    // Parent's Row and Col
    if (s.parentXY.row != -1 && s.parentXY.col != -1)
    {
        std::cout << "Has parent\n";
    }
    else
    {
        std::cout << "Has no parent\n";
    }
    std::cout << "  Row: " << s.parentXY.row << std::endl;
    std::cout << "  Col: " << s.parentXY.col << std::endl;

    // Lists
    if (s.state == SS_OPEN || s.state == SS_RAISE || s.state == SS_LOWER)
    {
        std::cout << "On the Open List\n";
    }
    else if (s.state == SS_CLOSED)
    {
        std::cout << "On the Closed List\n";
    }
    else
    {
        std::cout << "Not on any list\n";
    }

    // COSTS
    std::cout << "  Minimum cost: " << s.mincost << std::endl;
    std::cout << "  Current cost: " << s.curcost << std::endl;
}

// builds the path to the goal
void AStarPather2::buildPath(const Square& goal, PathRequest& request)
{
    pathstack.clear();

    // add the goal world coordinates
    pathstack.push_back(terrain->get_world_position(terrain->get_grid_position(request.start)));

    // RUBBERBANDING
    // no rubberbanding, build path as is
    XY parent = goal.parentXY;

    // while the next square has a parent
    while (parent.row != -1)
    {
        // add square to the path
        pathstack.push_back(terrain->get_world_position(getRow(parent), getCol(parent)));
        if (pathstack.size() > 100)
        {
            std::cout << "bruh" << std::endl;
        }

        // get next square
        parent = getSquare(parent).parentXY;
    }

    std::reverse(pathstack.begin(), pathstack.end());
    // pop off the complete path
    while (!pathstack.empty())
    {
        request.path.push_back(pathstack.back());
        pathstack.pop_back();
    }
}

// Skip unneccesary middle nodes in the path
void AStarPather2::rubberBand(const Square& goal, PathRequest& request)
{
  //// Take at most three squares into account
  //XY first, mid, last;
  //first = 0;
  //compactPos(first, terrain->get_grid_position(request.goal));
  //
  //// if there is another square besides the goal
  //if (goal.parentXY & PARENT)
  //{ 
  //    mid = goal.parentXY;
  //}
  //else 
  //{ 
  //    return; // done here, no need to skip nodes
  //}
  //
  //// if there is another square after that
  //// start the loop
  //while (mid & PARENT)
  //{
  //    last = getSquare(getRow(mid), getCol(mid)).parentXY;
  //
  //    // check the rectangle of squares on the grid for any walls
  //    GridPos a, b;
  //    unpackPos(first, a);
  //    unpackPos(last, b);
  //    if (wallsInRect(a, b))
  //    {
  //        // this node cannot be skipped
  //        // add to path
  //        pathstack.push_back(terrain->get_world_position(getRow(mid), getCol(mid)));
  //
  //        // set up next run
  //        first = mid;
  //    }
  //
  //    // go to next node
  //    mid = last;
  //}
  //
  //// add the last node (should be the starting player position)
  //pathstack.push_back(terrain->get_world_position(getRow(mid), getCol(mid)));
}

// return false if there are no walls in the given rectangle, true otherwise
bool AStarPather2::wallsInRect(GridPos a, GridPos b)
{
    // Find the dimensions of the rectangle
    int rows = abs(a.row - b.row); // height of the rectangle
    int cols = abs(a.col - b.col); // width of the rectangle

    // Find the bottom left most square
    GridPos pos;
    pos.row = (a.row < b.row) ? a.row : b.row; // min row to start on
    pos.col = (a.col < b.col) ? a.col : b.col; // min col to start on

    for (int r = 0; r <= rows; ++r, ++pos.row)
    {
        for (int c = 0; c <= cols; ++c, ++pos.col)
        {
            if (terrain->is_wall(pos))
            {
                return true; // wall found
            }
        }
        // reset column
        pos.col -= (cols + 1);
    }

    // no walls found
    return false;
}

// prepare and already rubberbanded path for smoothing
void AStarPather2::rubberToSmooth()
{
    // max distance in world coordinates
    GridPos a; a.row = 0; a.col = 0;
    GridPos b; b.row = 1; b.col = 0;
    Vec3 dist = terrain->get_world_position(b) - terrain->get_world_position(a);
    float max_dist = MAX_DIST * dist.x;

    // mark any change to the path
    bool modified;

    // while there was a change made during the last loop
    do
    {
        modified = false;

        // traverse the path
        std::vector<Vec3>::iterator it = pathstack.begin();
        for (; it != pathstack.end(); ++it)
        {
            // as long as we are not on the last stop
            if (it + 1 != pathstack.end())
            {
                // compare distance
                float dz = it->z - (it + 1)->z;
                float dx = it->x - (it + 1)->x;
                if (sqrt((dz * dz) + (dx * dx)) > max_dist)
                {
                    // find the mid point
                    Vec3 dir = (*(it + 1)) - (*it);
                    dir *= 0.5;
                    //add the mid point
                    pathstack.insert((it + 1), (*it) + dir);
                    // path has been changed, check again from the top
                    modified = true;
                    break;
                }
            }
        }
    } while (modified);
}

// smooth the path found with Catmull-Rom Spline
void AStarPather2::smoothPath()
{
    // do nothing if path is empty
    if (pathstack.empty()) return;

    // new smoothed path to create
    std::vector<Vec3> smoothedstack;
    // For every edge, we will be adding three new points
    smoothedstack.reserve(pathstack.size() + (pathstack.size() - 1)* 3);

    // to simplify loop, add a copy of the starting and ending points to the list
    pathstack.insert(pathstack.begin(), pathstack.front());
    pathstack.push_back(pathstack.back());

    // add the first point
    smoothedstack.push_back(pathstack.front());

    // traverse the list, while there are still three points ahead of the first
    std::vector<Vec3>::const_iterator it = pathstack.begin();
    for (; it + 3 != pathstack.end(); ++it)
    {
        // add all new spline points
        smoothedstack.push_back(Vec3::CatmullRom(*it, *(it + 1), *(it + 2), *(it + 3), 0.25));
        smoothedstack.push_back(Vec3::CatmullRom(*it, *(it + 1), *(it + 2), *(it + 3), 0.50));
        smoothedstack.push_back(Vec3::CatmullRom(*it, *(it + 1), *(it + 2), *(it + 3), 0.75));
        smoothedstack.push_back(*(it + 2)); // add next point on path
    }

    // make new smoothed path the actual path
    pathstack = smoothedstack;
}

// clear the grid data and openlist
void AStarPather2::clearData()
{
    for (int r = 0; r < terrain->get_map_height(); ++r)
    {
        for (int c = 0; c < terrain->get_map_width(); ++c)
        {
            grid[r][c].parentXY.col = -1;
            grid[r][c].parentXY.row = -1;
            grid[r][c].mincost = 0.0f;
            grid[r][c].curcost = 0.0f;
            grid[r][c].state = SS_NEW;
        }
    }
    // OPENLIST CHANGES: openlist.clear();
    ol_size = 0;
}

bool AStarPather2::initialize()
{
    // handle any one-time setup requirements you have

    /*
        If you want to do any map-preprocessing, you'll need to listen
        for the map change message.  It'll look something like this:

        Callback cb = std::bind(&AStarPather2::your_function_name, this);
        Messenger::listen_for_message(Messages::MAP_CHANGE, cb);

        There are other alternatives to using std::bind, so feel free to mix it up.
        Callback is just a typedef for std::function<void(void)>, so any std::invoke'able
        object that std::function can wrap will suffice.
    */

    // Find the size of each node
    /*
    std::cout << "Size of short int: " << sizeof(short int)
        << " Size of float: " << sizeof(float)
        << " Size of Square: " << sizeof(Square) << std::endl;
    */

    return true; // return false if any errors actually occur, to stop engine initialization
}

void AStarPather2::shutdown()
{
    /*
        Free any dynamically allocated memory or any other general house-
        keeping you need to do during shutdown.
    */
    clearData();
}

// NOTE: ALSO ADDS CENTER SQUARE
void AStarPather2::AddAllNeighbours(GridPos centertile, const PathRequest& request)
{
    for (int col = -1; col <= 1; col++)
    {
        for (int row = -1; row <= 1; row++)
        {
            GridPos neighbour = centertile;
            neighbour.col += col;
            neighbour.row += row;
            if (!terrain->is_valid_grid_position(neighbour.row, neighbour.col))
            {
                continue;
            }
            if (terrain->is_wall(neighbour.row, neighbour.col))
            {
                continue;
            }

            /*
            // add neigbouring node to openlist
            Square& neighboursquare = grid[neighbour.row][neighbour.col];
            XY neighbourXY;
            compactPos(neighbourXY, neighbour);
            insertSortList(neighboursquare, neighbourXY);
            neighboursquare.state = SS_OPEN;
            if (request.settings.debugColoring)
            {
                if (row || col)
                {
                    terrain->set_color(neighbour, Colors::Cyan);
                }
            }
            */
            //Insert(neighbour, getSquare(neighbour).curcost, request);
            if (getSquare(neighbour).state != SS_OPEN)
            {
                insertSortList(getSquare(neighbour), neighbour);
                getSquare(neighbour).state = SS_OPEN;
            }
        }
    }
}

void AStarPather2::expand(GridPos centertile, const PathRequest& request)
{
    Square& centerSquare = grid[centertile.row][centertile.col];
    for (int col = -1; col <= 1; col++)
    {
        for (int row = -1; row <= 1; row++)
        {
            if (!(col || row)) //if this is the center tile
            {
                continue;
            }
            GridPos neighbour = centertile;
            neighbour.col += col;
            neighbour.row += row;
            if (!terrain->is_valid_grid_position(neighbour.row, neighbour.col))
            {
                continue;
            }
            if (terrain->is_wall(neighbour.row, neighbour.col))
            {
                continue;
            }
            // calc cost
            GridPos parentPos;
            Square& neighboursquare = grid[neighbour.row][neighbour.col];
            float distancetoNeighbour = (col && row) ? 1.4f : 1.0f; // if this is diagonal set distance to 1.4 else 1 
            unpackPos(neighboursquare.parentXY, parentPos);
            
            // calc total cost to reach this node from the center square, enemy weight included
            float total_cost = centerSquare.curcost + distancetoNeighbour + terrain->enemyVisionLayer.get_value(neighbour);

            if (
                (neighboursquare.state == SS_NEW) || //if it hasn't been on the open list
                (parentPos == centertile && (neighboursquare.curcost != total_cost)) || //or the cost isn't correct in the node
                (parentPos != centertile && (neighboursquare.curcost > total_cost)) // or we found a cheaper parent
                )
            {
                compactPos(neighboursquare.parentXY, centertile); // assign new parent
                // add to openlist
                Insert(neighbour, total_cost, request);
            }
        }
    }
}

bool AStarPather2::update_expand(GridPos centertile, const PathRequest& request)
{
    bool replaced = false;
    Square& centerSquare = grid[centertile.row][centertile.col];
    for (int col = -1; col <= 1; col++)
    {
        for (int row = -1; row <= 1; row++)
        {
            if (!(col || row)) //if this is the center tile
            {
                continue;
            }
            GridPos neighbour = centertile;
            neighbour.col += col;
            neighbour.row += row;
            if (!terrain->is_valid_grid_position(neighbour.row, neighbour.col))
            {
                continue;
            }
            if (terrain->is_wall(neighbour.row, neighbour.col))
            {
                continue;
            }
            // calc cost
            GridPos parentPos;
            Square& neighboursquare = grid[neighbour.row][neighbour.col];
            float distancetoNeighbour = (col && row) ? 1.4f : 1.0f; // if this is diagonal set distance to 1.4 else 1 
            unpackPos(neighboursquare.parentXY, parentPos);

            // calc total cost to reach this node from the center square, enemy weight included
            float total_cost = centerSquare.curcost + distancetoNeighbour + terrain->enemyVisionLayer.get_value(neighbour);

            if (
                (neighboursquare.state == SS_NEW) || //if it hasn't been on the open list
                (parentPos == centertile && (neighboursquare.curcost != total_cost)) //or the cost isn't correct in the node
                )
            {
                compactPos(neighboursquare.parentXY, centertile); // assign new parent
                // add to openlist
                Insert(neighbour, total_cost, request);
            }
            else if (parentPos != centertile && (neighboursquare.curcost > total_cost)) // or we found a cheaper parent
            {
                // centertile is in lower state
                Insert(centertile, centerSquare.curcost, request);
                replaced = true;

            }
            else if (parentPos != centertile && neighboursquare.state == SS_CLOSED && neighboursquare.curcost > centerSquare.mincost)
            {
                float cost_to_neighbour = neighboursquare.curcost + distancetoNeighbour + terrain->enemyVisionLayer.get_value(centertile);
                if (centerSquare.curcost > cost_to_neighbour)
                {
                    // neighbor is in lower state
                    Insert(neighbour, neighboursquare.curcost, request);
                }
            }
        }
    }  
    return replaced;
}

// D*: Insert
void AStarPather2::Insert(GridPos pos, float new_cost, const PathRequest& request)
{
    Square& node = getSquare(pos);

    // figure what to set min cost to
    switch (node.state)
    {
    case SS_NEW:
        node.mincost = new_cost;
        if (request.settings.debugColoring) terrain->set_color(pos, Colors::SeaGreen);
        break;
    case SS_CLOSED:
        node.mincost = new_cost > node.curcost ? node.curcost : new_cost;
        if (request.settings.debugColoring) terrain->set_color(pos, Colors::Cyan);
        break;
    case SS_OPEN:
        node.mincost = new_cost > node.mincost ? node.mincost : new_cost;
        break;
    default:
        if (request.settings.debugColoring) terrain->set_color(pos, Colors::HotPink);
        break;
    }
    // set the new current cost
    node.curcost = new_cost;

    // add node to openlist if not already in
    insertSortList(node, pos);
    node.state = SS_OPEN;

    // does this belong here?
    //node.isDirty = false;
}

PathResult AStarPather2::compute_path(PathRequest &request)
{
    /*
        This is where you handle pathing requests, each request has several fields:

        start/goal - start and goal world positions
        path - where you will build the path upon completion, path should be
            start to goal, not goal to start
        heuristic - which heuristic calculation to use
        weight - the heuristic weight to be applied
        newRequest - whether this is the first request for this path, should generally
            be true, unless single step is on

        smoothing - whether to apply smoothing to the path
        rubberBanding - whether to apply rubber banding
        singleStep - whether to perform only a single A* step
        debugColoring - whether to color the grid based on the A* state:
            closed list nodes - yellow
            open list nodes - blue

            use terrain->set_color(row, col, Colors::YourColor);
            also it can be helpful to temporarily use other colors for specific states
            when you are testing your algorithms

        method - which algorithm to use: A*, Floyd-Warshall, JPS+, or goal bounding,
            will be A* generally, unless you implement extra credit features

        The return values are:
            PROCESSING - a path hasn't been found yet, should only be returned in
                single step mode until a path is found
            COMPLETE - a path to the goal was found and has been built in request.path
            IMPOSSIBLE - a path from start to goal does not exist, do not add start position to path
    */

    // WRITE YOUR CODE HERE

    // get starting and ending position
    GridPos goal = terrain->get_grid_position(request.start);
    GridPos start = terrain->get_grid_position(request.goal);

    if (request.newRequest)
    {   
        // color positions
        if (request.settings.debugColoring)
        {
            terrain->set_color(start, Colors::Yellow);
            terrain->set_color(goal, Colors::Yellow);
        }

        clearData();

        Insert(start, 0.0f, request);
    }


    GridPos pos;  // position on grid of cheapest node
    GridPos prev; // position of the cheapest node's parent

    // while the open list is not empty
    while (ol_size != 0) // OPENLIST CHANGES:!openlist.empty())
    {
        // pop the cheapest node off
        unpackPos(openlist[ol_size - 1], pos);
        Square& cheapest = getSquare(pos);

        unpackPos(cheapest.parentXY, prev);
        --ol_size;

        // place on closed list
        cheapest.state = SS_CLOSED;
        cheapest.isDirty = false;
        if (request.settings.debugColoring)
        {
            terrain->set_color(pos, Colors::Orange);
        }

        // check if this is the goal
        if (pos == goal)
        {
            // GOAL FOUND
            expand(pos, request);
            cheapest.state = SS_CLOSED;
            if (request.settings.debugColoring)
            {
                terrain->set_color(goal, Colors::Red);
            }
            buildPath(cheapest, request);
            //clearData();
            return PathResult::COMPLETE;
        }

        /* Get all children */
        expand(pos, request);

        // check for interrupt
        if (request.settings.singleStep)
        {
            return PathResult::PROCESSING;
        }
    }

    // open list was empty, no path found
    clearData();
    return PathResult::IMPOSSIBLE;
}

void AStarPather2::update_path(PathRequest& request)
{
    // need a copy of the built path in order to check for weight changes
    std::list<Vec3> &path = request.path;

    GridPos loc;
    Square* node;
    bool changeFound = false;

    // walk through the path
    std::list<Vec3>::const_iterator ahead = path.begin();
    std::list<Vec3>::const_iterator it = ahead++;
    for (; ahead != path.end(); ++ahead)
    {
        loc = terrain->get_grid_position(*ahead);
        node = &getSquare(loc);
        // if a square on the path has changed weight
        if (node->isDirty)
        {
            // insert dirty node and all it's neighbors
            AddAllNeighbours(loc, request);

            // NOTE: Do we only do this if cost is now higher?
            // what if it's actually lower than before?
            //node->curcost += RAISED_COST;

            // changing color to signify raised
            if (request.settings.debugColoring)
            {
                terrain->set_color(loc, Colors::IndianRed);
            }
            node->isDirty = false;
            changeFound = true;
            break;
        }
        it = ahead;
    }

    // no changes to path
    if (!changeFound) return;

    Square& pathbreaker = getSquare(terrain->get_grid_position(*it));

    // path must be recalculated
    GridPos pos;  // position on grid of cheapest node

    // while the open list is not empty
    while (ol_size != 0) // OPENLIST CHANGES:!openlist.empty())
    {
        // pop the cheapest node off
        unpackPos(openlist[ol_size - 1], pos);
        Square& cheapest = getSquare(pos);
        cheapest.state = SS_CLOSED;
        --ol_size;

        // place on closed list
        //cheapest.state = SS_CLOSED;
        if (request.settings.debugColoring)
        {
            terrain->set_color(pos, Colors::Blue);
        }

        // if the next tile on the openlist is unessecary for the new path
        if (pathbreaker.curcost < cheapest.mincost)
        {
            // PATH RESTRUCTURED
            // build new path from original starting tile
            Square& start = getSquare(terrain->get_grid_position(path.front()));
            request.path.clear();
            buildPath(start, request);
            return;
        }

        // if this node did not change value
        if (cheapest.mincost == cheapest.curcost)
        {
            expand(pos, request);
        }
        else if (cheapest.mincost < cheapest.curcost) // current cost is greater
        {
            // get all neighbors
            for (int col = -1; col <= 1; col++)
            {
                for (int row = -1; row <= 1; row++)
                {
                    if (!(col || row)) //if this is the center tile
                    {
                        continue;
                    }
                    GridPos neighbour = pos;
                    neighbour.col += col;
                    neighbour.row += row;
                    if (!terrain->is_valid_grid_position(neighbour.row, neighbour.col))
                    {
                        continue;
                    }
                    if (terrain->is_wall(neighbour.row, neighbour.col))
                    {
                        continue;
                    }

                    // neighbor is ready and valid
                    Square& neighSquare = getSquare(neighbour);
                    float distancetoNeighbour = (col && row) ? 1.4f : 1.0f; // if this is diagonal set distance to 1.4 else 1 
                    float total_cost = neighSquare.curcost + distancetoNeighbour + terrain->enemyVisionLayer.get_value(pos);

                    // if this neighbour is a better parent now                                     // only evaluated nodes count
                    if (neighSquare.curcost <= cheapest.mincost && cheapest.curcost > total_cost && total_cost < RAISED_COST)
                    {
                        cheapest.parentXY = neighbour; // this is the new parent now
                        cheapest.curcost = total_cost; // assign the new & less expensive cost
                    }
                }
            }// end of get all neighbors

            if (!update_expand(pos, request))
            {
                //cheapest.state = SS_CLOSED;
            }
        } // end of if (cheapest.mincost < cheapest.curcost)
    } // end of while (ol_size != 0)

    // PATH RESTRUCTURED
    // build new path from original starting tile
    Square& start = getSquare(terrain->get_grid_position(path.front()));
    request.path.clear();
    buildPath(start, request);
    std::cout << "\nRAN OUT OF LIST!\n";
}

// checks to see if grid position is not out of bounds, not a wall, and not the way back
// DOES NO CHECKING RESTRICTED DIAGONAL MOVEMENT NEAR WALLS
bool AStarPather2::validNeighbor(const int& row, const int& col, const GridPos& prev)
{
    return terrain->is_valid_grid_position(row, col) && !terrain->is_wall(row, col) && !(prev.row == row && prev.col == col);
}

// check neighboring squares and add them to the open list if valid
void AStarPather2::evaluateChildren(const Square& parent, const GridPos& pos, const GridPos& prev, const GridPos& goal, const PathRequest& request)
{
    // Children must be valid positions, not walls and not the parent

    /* STRAIGHTS */

    // North
    if (validNeighbor(pos.row, pos.col + 1, prev))
    {
        computeCost(parent, pos, pos.row, pos.col + 1, ONE, goal, request);
    }
    // South
    if (validNeighbor(pos.row, pos.col - 1, prev))
    {
        computeCost(parent, pos, pos.row, pos.col - 1, ONE, goal, request);
    }
    // East
    if (validNeighbor(pos.row + 1, pos.col, prev))
    {
        computeCost(parent, pos, pos.row + 1, pos.col, ONE, goal, request);
    }
    // West
    if (validNeighbor(pos.row - 1, pos.col, prev))
    {
        computeCost(parent, pos, pos.row - 1, pos.col, ONE, goal, request);
    }

    /* DIAGONALS */

    // North East
    if (validNeighbor(pos.row + 1, pos.col + 1, prev))
    {
        // check for walls
        if (!terrain->is_wall(pos.row + 1, pos.col) && !terrain->is_wall(pos.row, pos.col + 1))
        {
            computeCost(parent, pos, pos.row + 1, pos.col + 1, SQUAREROOT2, goal, request);
        }
    }
    // South East
    if (validNeighbor(pos.row + 1, pos.col - 1, prev))
    {
        // check for walls
        if (!terrain->is_wall(pos.row + 1, pos.col) && !terrain->is_wall(pos.row, pos.col - 1))
        {
            computeCost(parent, pos, pos.row + 1, pos.col - 1, SQUAREROOT2, goal, request);
        }
    }
    // South West
    if (validNeighbor(pos.row - 1, pos.col - 1, prev))
    {
        // check for walls
        if (!terrain->is_wall(pos.row - 1, pos.col) && !terrain->is_wall(pos.row, pos.col - 1))
        {
            computeCost(parent, pos, pos.row - 1, pos.col - 1, SQUAREROOT2, goal, request);
        }
    }
    // North West
    if (validNeighbor(pos.row - 1, pos.col + 1, prev))
    {
        // check for walls
        if (!terrain->is_wall(pos.row - 1, pos.col) && !terrain->is_wall(pos.row, pos.col + 1))
        {
            computeCost(parent, pos, pos.row - 1, pos.col + 1, SQUAREROOT2, goal, request);
        }
    }
}

// compute cost, update cost if needed, and add to the openlist
void AStarPather2::computeCost(const Square& parent, GridPos parent_pos, const int& row, const int& col, const float& dist, const GridPos& goal, const PathRequest& request)
{
    // get reference to Square
    Square& child = getSquare(row, col);
    XY child_xy;
    child_xy.col = -1;
    child_xy.row = -1;
    compactPos(child_xy, row, col);
    GridPos child_pos;
    child_pos.row = row;
    child_pos.col = col;

    /* Compute Cost */

    // if we are looking at a node that was already on the open/closed list
    if (!child.state == SS_NEW)
    {
        // Given Cost plus enemy vision layer
        float given = parent.mincost + dist;
        given += terrain->enemyVisionLayer.get_value(child_pos);

        // Heuristic Cost
        // TODO - should be its own function
        float total = given + CalcHeuristicCost(row, col, goal.row, goal.col, request);

        if (child.curcost > total)
        {
            ///* CHANGES: NEW SORT
            //TODO - temp
            if (child.state == SS_OPEN)
            {
                for (unsigned int i = 0; i < ol_size; ++i)
                {
                    // TODO - is something wrong here?
                    if (openlist[i] == child_xy)
                    {
                        while (i != ol_size)
                        {
                            openlist[i] = openlist[i + 1];
                            ++i;
                        }
                        --ol_size;
                        break;
                    }
                }
            }
            //*/

            // update node
            child.parentXY = parent_pos;
            child.state = SS_OPEN;
            if (request.settings.debugColoring)
            {
                terrain->set_color(child_pos, Colors::Cyan);
            }
            child.mincost = given;
            child.curcost = total;

            // TODO - temp
            // CHANGES: NEW SORT 
            insertSortList(child, child_xy);
        }
    }
    else // looking at a new node
    {
        // Set Parent
        child.parentXY = parent_pos;

        // Given Cost and weight from enemy vision layer
        child.mincost = parent.mincost + dist;
        child.mincost += terrain->enemyVisionLayer.get_value(child_pos);

        // Heuristic Cost
        // TODO - should be its own function
        child.curcost = child.mincost + CalcHeuristicCost(row, col, goal.row, goal.col, request);

        // place on open list
        child.state = SS_OPEN;
        if (request.settings.debugColoring)
        {
            terrain->set_color(child_pos, Colors::Cyan);
        }

        // CHANGES: NEW SORT 
        insertSortList(child, child_xy);
        //openlist[ol_size] = child_xy;
        //++ol_size;
    }
}

// TODO - this is temporary
// this was temporary, but nothing seems to be faster
void AStarPather2::insertSortList(const Square& node, XY child_xy)
{
    // is the node already on the list?
    if (node.state == SS_OPEN)
    {
        // remove from the list first
        for (int i = 0; i < static_cast<int>(ol_size); ++i)
        {
            if (openlist[i] == child_xy)
            {
                for (int j = i; j < static_cast<int>(ol_size) - 1; ++j)
                {
                    openlist[j] = openlist[j + 1];
                }
                --ol_size;
                break;
            }
        }
    }

    // empty? add to front
    if (ol_size == 0)
    {
        openlist[ol_size] = child_xy;
        ++ol_size;
        return;
    }

    // quick check to see if it belongs at the end
    if (getSquare(openlist[ol_size - 1]).mincost >= node.mincost)
    {
        openlist[ol_size] = child_xy;
        ++ol_size;
        return;
    }

    // search for proper place in sorted array
    for (int i = 0; i < static_cast<int>(ol_size); ++i)
    {
        if (getSquare(openlist[i]).mincost < node.mincost)
        {
            for (int j = static_cast<int>(ol_size) - 1; j >= i; --j)
            {
                openlist[j + 1] = openlist[j];
            }
            openlist[i] = child_xy;
            ++ol_size;
            return;
        }
    }
}

float AStarPather2::CalcHeuristicCost(const int& x1, const int& y1, const int& x2, const int& y2, const PathRequest& request)
{
    switch (request.settings.heuristic)
    {
        case Heuristic::EUCLIDEAN:
            return H_Euclidean(x1, y1, x2, y2) * request.settings.weight;
        case Heuristic::OCTILE:
            return H_Octile(x1, y1, x2, y2) * request.settings.weight;
        case Heuristic::MANHATTAN:
            return H_Manhattan(x1, y1, x2, y2) * request.settings.weight;
        case Heuristic::CHEBYSHEV:
            return H_Chebyshev(x1, y1, x2, y2) * request.settings.weight;
        default: return 0.0f;
    }
}

// Euclidean Distance Heuristic
float AStarPather2::H_Euclidean(const int& x1, const int& y1, const int& x2, const int& y2)
{
    int dx = x1 - x2;
    int dy = y1 - y2;
    return static_cast<float>(sqrt((dx*dx) + (dy*dy))); // TODO - square root slows this down
}

// Octile Distance Heuristic
float AStarPather2::H_Octile(const int& x1, const int& y1, const int& x2, const int& y2)
{
    int dx = abs(x1 - x2);
    int dy = abs(y1 - y2);
    if (dx < dy)
    {
        return (dx * SQUAREROOT2) + (dy - dx);
    }
    else
    {
        return (dy * SQUAREROOT2) + (dx - dy);
    }
}

// Manhattan Distance Heuristic
float AStarPather2::H_Manhattan(const int& x1, const int& y1, const int& x2, const int& y2)
{
    return static_cast<float>(abs(x1 - x2) + abs(y1 - y2));
}

// Chebyshev Distance Heuristic
float AStarPather2::H_Chebyshev(const int& x1, const int& y1, const int& x2, const int& y2)
{
    int dx = abs(x1 - x2);
    int dy = abs(y1 - y2);
    if (dx > dy)
    {
        return static_cast<float>(dx);
    }
    else
    {
        return static_cast<float>(dy);
    }
}