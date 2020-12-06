#pragma once
#include "Misc/PathfindingDetails.hpp"
#include "Terrain/Terrain.h"
 
// XY - bit values
#define PARENT 0b1000000000000000 // HAS PARENT   (//NEXT)
#define ROW    0b0011111100000000 // ROW NUMBER
#define COL    0b0000000011111100 // COL NUMBER
#define OPEN   0b0000000000000001 // ON OPEN LIST
#define CLOSED 0b0000000000000010 // ON CLOSED LIST

// Maximum number of nodes (40 * 40)
#define MAX_SIZE 1600

// Number of buckets TODO: play with the number that see what works
#define MAX_BUCK 10
// size the openlist must be in order to use bucket sort
#define BARRIER 60

// Maximum distance allowed between points on the path when smoothing after rubberbanding (in square lengths)
#define MAX_DIST 1.5f

class AStarPather2
{
public:
    /* 
        The class should be default constructible, so you may need to define a constructor.
        If needed, you can modify the framework where the class is constructed in the
        initialize functions of ProjectTwo and ProjectThree.
    */

    /* ************************************************** */
    // DO NOT MODIFY THESE SIGNATURES
    bool initialize();
    void shutdown();
    PathResult compute_path(PathRequest &request);
    //float compute_path_cost(PathRequest& request);
    void update_path(PathRequest& request);
    /* ************************************************** */

    /*
        You should create whatever functions, variables, or classes you need.
        It doesn't all need to be in this header and cpp, structure it whatever way
        makes sense to you.
    */
    float AStarPather2::get_enemy_weight()
    {
        return enemyWeight;
    }

    void AStarPather2::set_enemy_weight(const float& weight)
    {
        enemyWeight = weight;
        std::wstringstream stream;
        stream.precision(3);
        stream << weight;
        enemyWeightText = stream.str();
    }

    const std::wstring& AStarPather2::get_enemy_weight_text()
    {
        return enemyWeightText;
    }

    typedef GridPos XY;

    enum SquareState
    {
        SS_NEW,
        SS_OPEN,
        SS_RAISE,
        SS_LOWER,
        SS_CLOSED
    };


    // 12 bytes total
    struct Square
    {
        XY parentXY = { 0,0 }; // 2 bytes
        //unsigned short int next   = 0; // 2 bytes
        float mincost = 0.0f;            // 4 bytes
        float curcost = 0.0f;            // 4 bytes
        SquareState state = SS_NEW;
        bool isDirty = true;
    };

    Square& getSquare(const GridPos& pos)
    {
        return grid[pos.row][pos.col];
    }

    Square& getSquare(const int& row, const int& col)
    {
        return grid[row][col];
    }


private:
    
    float enemyWeight = 5.0f;


    const float ONE = 1.0f;                  // straight cost
    const float SQUAREROOT2 = static_cast<float>(sqrt(2));//1.4142135624f; // diagonal cost

    // Open List
    // std::vector<XY> openlist;
    XY openlist[MAX_SIZE];
    unsigned int ol_size;

    // Buckets
    XY buckets[MAX_BUCK][MAX_SIZE];
    unsigned int buck_count[MAX_BUCK];

    // Path Stack
    // TODO - const int MAX_PATH_LENGTH = 1600;
    std::vector<Vec3> pathstack;

    // reserve maximum space required
    std::vector<std::vector<Square>> grid =
        std::vector<std::vector<Square>>(terrain->maxMapHeight, std::vector<Square>(terrain->maxMapWidth));

    void printSquare(const Square& s);

    int getRow(const XY& xy)
    {
        return xy.row;
    }

    int getCol(const XY& xy)
    {
        return xy.col;
    }

    void unpackPos(const XY& xy, GridPos& pos)
    {
        pos.row = getRow(xy);
        pos.col = getCol(xy);
    }

    void compactPos(XY& xy, const GridPos& pos)
    {
        // TODO - Check if row or col are positive
        //      - Reset ROW and COL values in xy to zeros
        xy.row = pos.row;
        xy.col = pos.col;
    }

    void compactPos(XY& xy, int row, int col)
    {
        // TODO - Check if row or col are positive
        //      - Reset ROW and COL values in xy to zeros
        xy.row = row;
        xy.col = col;
    }

    // checks to see if grid position is not out of bounds, not a wall, and not the way back
    // DOES NO CHECKING RESTRICTED DIAGONAL MOVEMENT NEAR WALLS
    bool validNeighbor(const int& row, const int& col, const GridPos& prev);

    // check neighboring squares and add them to the open list if valid
    void evaluateChildren(const Square& parent, const GridPos& pos, const GridPos& prev, const GridPos& goal, const PathRequest& request);

    // compute cost, update cost if needed, and add to the openlist
    void computeCost(const Square& parent, GridPos parent_pos, const int& row, const int& col, const float& dist, const GridPos& goal, const PathRequest& request);

    // clear the grid data and openlist
    void clearData();

    // insert sort addition
    void insertSortList(const Square& node, XY child_xy);

    // bucket and then insertion sort
    void bucketSort();

    /* HEURISTIC FORMULAS */
    float CalcHeuristicCost(const int& x1, const int& y1, const int& x2, const int& y2, const PathRequest& request);
    // Euclidean Heuristic 
    float H_Euclidean(const int& x1, const int& y1, const int& x2, const int& y2);
    // Octile Heuristic
    float H_Octile(const int& x1, const int& y1, const int& x2, const int& y2);
    // Manhattan Heuristic
    float H_Manhattan(const int& x1, const int& y1, const int& x2, const int& y2);
    // Chebyshev Heuristic
    float H_Chebyshev(const int& x1, const int& y1, const int& x2, const int& y2);

    /* POST PATHFINDING FUNCTIONS */
    // builds the path to the goal
    void buildPath(const Square& goal, PathRequest& request);
    // parse unnessecary middle nodes in the path
    void rubberBand(const Square& goal, PathRequest& request);
    // return false if there are no walls in the given rectangle, true otherwise
    bool wallsInRect(GridPos a, GridPos b);
    // prepare and already rubberbanded path for smoothing
    void rubberToSmooth();
    // smooth the path found with Catmull-Rom Spline
    void smoothPath();
    //to expand the area we're looking at
    void expand(GridPos, const PathRequest& request);

    // D*: Inserts all neighbors of a centertile as well as centertile
    void AddAllNeighbours(GridPos centertile, const PathRequest& request);

    // D*: Insert
    void Insert(GridPos pos, float new_cost, const PathRequest& request);

    protected:
        std::wstring enemyWeightText;
   
};