#include <pch.h>
#include "Projects/ProjectFour.h"
#include "P4_Pathfinding.h"
#include "Terrain/TerrainAnalysis.h"

#define ENEMY_VISION_WEIGHT 5.0f

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
    if (s.xylist & PARENT)
    {
        std::cout << "Has parent\n";
    }
    else
    {
        std::cout << "Has no parent\n";
    }
    std::cout << "  Row: " << ((s.xylist & ROW) >> 8) << std::endl;
    std::cout << "  Col: " << ((s.xylist & COL) >> 2) << std::endl;

    // Lists
    if (s.xylist & OPEN)
    {
        std::cout << "On the Open List\n";
    }
    else if (s.xylist & CLOSED)
    {
        std::cout << "On the Closed List\n";
    }
    else
    {
        std::cout << "Not on any list\n";
    }

    // COSTS
    std::cout << "  Heuestric cost: " << s.cost << std::endl;
    std::cout << "  Given cost: " << s.given << std::endl;
}

// builds the path to the goal
void AStarPather2::buildPath(const Square& goal, PathRequest& request)
{
    // allocate memory first off
    pathstack.reserve(static_cast<int>(floor(goal.given)));

    // add the goal world coordinates
    pathstack.push_back(terrain->get_world_position(terrain->get_grid_position(request.goal)));

    // RUBBERBANDING
    if (request.settings.rubberBanding)
    {
        // Only add needed nodes to define path
        rubberBand(goal, request);

        if (request.settings.smoothing)
        {
            // Add new points for smoothing
            rubberToSmooth();
        }
    }
    else
    {
        // no rubberbanding, build path as is
        XY parent = goal.xylist;

        // while the next square has a parent
        while (parent & PARENT)
        {
            // add square to the path
            pathstack.push_back(terrain->get_world_position(getRow(parent), getCol(parent)));

            // get next square
            parent = getSquare(getRow(parent), getCol(parent)).xylist;
        }
    }

    // SMOOTHING (Catmull-Rom Spline)
    if (request.settings.smoothing)
    {
        smoothPath();
    }

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
    // Take at most three squares into account
    XY first, mid, last;
    first = 0;
    compactPos(first, terrain->get_grid_position(request.goal));

    // if there is another square besides the goal
    if (goal.xylist & PARENT)
    { 
        mid = goal.xylist;
    }
    else 
    { 
        return; // done here, no need to skip nodes
    }

    // if there is another square after that
    // start the loop
    while (mid & PARENT)
    {
        last = getSquare(getRow(mid), getCol(mid)).xylist;

        // check the rectangle of squares on the grid for any walls
        GridPos a, b;
        unpackPos(first, a);
        unpackPos(last, b);
        if (wallsInRect(a, b))
        {
            // this node cannot be skipped
            // add to path
            pathstack.push_back(terrain->get_world_position(getRow(mid), getCol(mid)));

            // set up next run
            first = mid;
        }

        // go to next node
        mid = last;
    }

    // add the last node (should be the starting player position)
    pathstack.push_back(terrain->get_world_position(getRow(mid), getCol(mid)));
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
            grid[r][c].xylist = 0;
            grid[r][c].cost = 0.0f;
            grid[r][c].given = 0.0f;
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
    GridPos start = terrain->get_grid_position(request.start);
    GridPos goal = terrain->get_grid_position(request.goal);

    if (request.newRequest)
    {   
        // color positions
        if (request.settings.debugColoring)
        {
            terrain->set_color(start, Colors::Yellow);
            terrain->set_color(goal, Colors::ForestGreen);
        }

        clearData();

        // push starting node into the open list
        Square& begin = getSquare(start);
        begin.xylist = OPEN | ((start.row << 8) & ROW) | ((start.col << 2) & COL);
        // OPENLIST CHANGES: openlist.push_back(begin.xylist);
        openlist[ol_size] = begin.xylist;
        ++ol_size;
        prev_cheapest = 0;
    }


    GridPos pos;  // position on grid of cheapest node
    GridPos prev; // position of the cheapest node's parent

    // while the open list is not empty
    while (ol_size != 0) // OPENLIST CHANGES:!openlist.empty())
    {
        // pop the cheapest node off
        //bucketSort();
        unpackPos(openlist[ol_size - 1], pos); // OPENLIST CHANGES:openlist.back(), pos);
        Square& cheapest = getSquare(pos);
        prev_cheapest = cheapest.cost;
        unpackPos(cheapest.xylist, prev);
        --ol_size; // OPENLIST CHANGES:openlist.pop_back();

        // check if this is the goal
        if (pos == goal)
        {
            // GOAL FOUND
            if (request.settings.debugColoring)
            {
                terrain->set_color(goal, Colors::Red);
            }
            buildPath(cheapest, request);
            clearData();
            return PathResult::COMPLETE;
        }

        /* Get all children */
        evaluateChildren(cheapest, pos, prev, goal, request);

        // remove from open list
        cheapest.xylist ^= OPEN;
        // place parent on closed list
        cheapest.xylist |= CLOSED;
        if (request.settings.debugColoring)
        {
            terrain->set_color(pos, Colors::Orange);
        }

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
    XY child_xy = 0;
    compactPos(child_xy, row, col);
    GridPos child_pos;
    child_pos.row = row;
    child_pos.col = col;

    /* Compute Cost */

    // if we are looking at a node that was already on the open/closed list
    if ((child.xylist & OPEN) || (child.xylist & CLOSED))
    {
        // Given Cost plus enemy vision layer
        float given = parent.given + dist;
        given += ENEMY_VISION_WEIGHT * terrain->enemyVisionLayer.get_value(child_pos);

        // Heuristic Cost
        // TODO - should be its own function
        float total = given + CalcHeuristicCost(row, col, goal.row, goal.col, request);

        if (child.cost > total)
        {
            ///* CHANGES: NEW SORT
            //TODO - temp
            if (child.xylist & OPEN)
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
            child.xylist = 0;
            child.xylist |= PARENT;
            compactPos(child.xylist, parent_pos);
            child.xylist |= OPEN;
            if (request.settings.debugColoring)
            {
                terrain->set_color(child_pos, Colors::Cyan);
            }
            child.given = given;
            child.cost = total;

            // TODO - temp
            // CHANGES: NEW SORT 
            insertSortList(child, child_xy);
        }
    }
    else // looking at a new node
    {
        // Set Parent
        child.xylist |= PARENT;
        compactPos(child.xylist, parent_pos);

        // Given Cost and weight from enemy vision layer
        child.given = parent.given + dist;
        child.given += ENEMY_VISION_WEIGHT * terrain->enemyVisionLayer.get_value(child_pos);

        // Heuristic Cost
        // TODO - should be its own function
        child.cost = child.given + CalcHeuristicCost(row, col, goal.row, goal.col, request);

        // place on open list
        child.xylist |= OPEN;
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
    if (ol_size == 0)
    {
        openlist[ol_size] = child_xy;
        ++ol_size;
        return;
    }

    if (getSquare(openlist[ol_size - 1]).cost >= node.cost)
    {
        openlist[ol_size] = child_xy;
        ++ol_size;
        return;
    }

    for (int i = 0; i < static_cast<int>(ol_size); ++i)
    {
        if (getSquare(openlist[i]).cost < node.cost)
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

// bucket then use insertion sort
void AStarPather2::bucketSort()
{
    // array small enough to just use insertion sort
    if (ol_size < BARRIER)
    {
        // for each node
        for (int i = 1; i < static_cast<int>(ol_size); ++i)
        {
            // if a node is out of place, move it back until it is sorted
            for (int j = i; j > 0 && getSquare(openlist[j]).cost > getSquare(openlist[j - 1]).cost; --j)
            {
                std::swap(openlist[j], openlist[j-1]);
            }
        }
        return;
    }

    // sort into buckets first
    float prev_highest = getSquare(openlist[0]).cost;
    float range = (prev_highest - prev_cheapest) / (MAX_BUCK - 2);

    // clear values in buckets
    std::memset(buck_count, 0, sizeof(unsigned int) * MAX_BUCK);

    // for each node in the openlist
    for (unsigned int i = 0; i < ol_size; ++i)
    {
        // get cost
        float cost = getSquare(openlist[i]).cost;

        // find which bucket to place it in
        int b = MAX_BUCK - 1;
        if (cost > prev_cheapest && cost < prev_highest)
        {
            b = static_cast<int>((cost - prev_cheapest) / range) + 1;
        }
        else if (cost <= prev_cheapest)
        {
            b = 0;
        }

        // just place it in the bucket
        buckets[b][buck_count[b]] = openlist[i];
        ++buck_count[b];
    } // done with each node in the open list

    // use insertion sort inside each bucket then place it back into the openlist
    int i = ol_size - 1;
    for (int b = 0; b < MAX_BUCK; ++b)
    {
        // insertion sort the bucket
        for (int i = 1; i < static_cast<int>(buck_count[b]); ++i)
        {
            // if a node is out of place, move it back until it is sorted
            for (int j = i; j > 0 && getSquare(buckets[b][j]).cost > getSquare(buckets[b][j - 1]).cost; --j)
            {
                std::swap(buckets[b][j], buckets[b][j - 1]);
            }
        }

        // place back
        for (int j = static_cast<int>(buck_count[b]) - 1; j >= 0; --j) // for each item in bucket from cheapest to highest
        {
            // place in the openlist starting from the end and going towards the front
            openlist[i] = buckets[b][j];
            --i;
        }
    }

    /*
    // piece back together
    int i = ol_size - 1;
    for (int b = 0; b < MAX_BUCK; ++b) // for each bucket from smallest values to largest values
    {
        for (int j = static_cast<int>(buck_count[b]) - 1; j >= 0; --j) // for each item in bucket from cheapest to highest
        {
            // place in the openlist starting from the end and going towards the front
            openlist[i] = buckets[b][j];
            --i;
        }
    }
    */
}