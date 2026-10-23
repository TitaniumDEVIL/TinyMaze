#ifndef ALGOS_H
#define ALGOS_H

#include <raylib.h>
#include <vector>
#include <cstdint>
#include <stack>

using namespace std;

const int CELL_SIZE = 40;
const int MAZE_WIDTH = 20;
const int MAZE_HEIGHT = 20;

const int PADDING = 40;

const int PANEL_WIDTH = 10;

const int SCREEN_WIDTH = PADDING * 2 + CELL_SIZE * MAZE_WIDTH + 36 * PANEL_WIDTH;
const int SCREEN_HEIGHT = PADDING * 2 + CELL_SIZE * MAZE_HEIGHT;

constexpr uint8_t north = 1 << 0;
constexpr uint8_t east = 1 << 1;
constexpr uint8_t south = 1 << 2;
constexpr uint8_t west = 1 << 3;

const uint8_t dirs[4] = {north, east, south, west};
const int dr[4] = {-1, 0, 1, 0};
const int dc[4] = {0, 1, 0, -1};

struct Point
{
    int r, c;
};

inline uint8_t opposite(uint8_t dir)
{
    switch (dir)
    {
    case north:
        return south;
    case east:
        return west;
    case south:
        return north;
    case west:
        return east;
    default:
        return 0;
    }
}

inline void MakePath(vector<vector<uint8_t>> &maze, int row, int col, uint8_t dir)
{
    if (row == 0 && dir == north)
        return;
    if (col == MAZE_WIDTH - 1 && dir == east)
        return;
    if (row == MAZE_HEIGHT - 1 && dir == south)
        return;
    if (col == 0 && dir == west)
        return;

    maze[row][col] |= dir;
    switch (dir)
    {
    case north:
        maze[row - 1][col] |= opposite(dir);
        break;
    case east:
        maze[row][col + 1] |= opposite(dir);
        break;
    case south:
        maze[row + 1][col] |= opposite(dir);
        break;
    case west:
        maze[row][col - 1] |= opposite(dir);
        break;
    }
}

inline Point AldousBroder(vector<vector<uint8_t>> &maze, vector<vector<bool>> &visited, Point seed, int &remaining)
{
    int d, nr, nc;
    do
    {
        d = GetRandomValue(0, 3);
        nr = seed.r + dr[d];
        nc = seed.c + dc[d];
    } while (nr < 0 || nr >= MAZE_HEIGHT || nc < 0 || nc >= MAZE_WIDTH);

    if (!visited[nr][nc])
    {
        MakePath(maze, seed.r, seed.c, dirs[d]);
        visited[nr][nc] = true;
        remaining--;
    }
    return {nr, nc};
}

inline Point BinaryTree(vector<vector<uint8_t>> &maze, vector<vector<bool>> &visited, Point seed, int &remaining)
{
    visited[seed.r][seed.c] = true;

    vector<uint8_t> validDirs;
    if (seed.r > 0)
        validDirs.push_back(north);
    if (seed.c > 0)
        validDirs.push_back(west);

    if (!validDirs.empty())
    {
        uint8_t dir = validDirs[GetRandomValue(0, validDirs.size() - 1)];
        MakePath(maze, seed.r, seed.c, dir);
    }

    remaining--;
    seed.c++;
    if (seed.c >= MAZE_WIDTH)
    {
        seed.c = 0;
        seed.r++;
    }
    return seed;
}

inline Point HuntAndKill(vector<vector<uint8_t>> &maze, vector<vector<bool>> &visited, Point seed, int &remaining)
{
    vector<int> unvisited;
    for (int d = 0; d < 4; d++)
    {
        int nr = seed.r + dr[d];
        int nc = seed.c + dc[d];
        if (nr >= 0 && nr < MAZE_HEIGHT && nc >= 0 && nc < MAZE_WIDTH && !visited[nr][nc])
            unvisited.push_back(d);
    }

    if (!unvisited.empty())
    {
        int d = unvisited[GetRandomValue(0, unvisited.size() - 1)];
        int nr = seed.r + dr[d];
        int nc = seed.c + dc[d];
        MakePath(maze, seed.r, seed.c, dirs[d]);
        visited[nr][nc] = true;
        remaining--;
        return {nr, nc};
    }

    for (int r = 0; r < MAZE_HEIGHT; r++)
    {
        for (int c = 0; c < MAZE_WIDTH; c++)
        {
            if (!visited[r][c])
            {
                vector<int> visitedNeighbors;
                for (int d = 0; d < 4; d++)
                {
                    int nr = r + dr[d];
                    int nc = c + dc[d];
                    if (nr >= 0 && nr < MAZE_HEIGHT && nc >= 0 && nc < MAZE_WIDTH && visited[nr][nc])
                        visitedNeighbors.push_back(d);
                }
                if (!visitedNeighbors.empty())
                {
                    int d = visitedNeighbors[GetRandomValue(0, visitedNeighbors.size() - 1)];
                    MakePath(maze, r, c, dirs[d]);
                    visited[r][c] = true;
                    remaining--;
                    return {r, c};
                }
            }
        }
    }
    return seed;
}

inline Point RecursiveBacktracker(vector<vector<uint8_t>> &maze, vector<vector<bool>> &visited, Point seed, int &remaining)
{
    static stack<Point> st;
    if (remaining == MAZE_WIDTH * MAZE_HEIGHT - 1)
    {
        while (!st.empty())
            st.pop();
        st.push(seed);
    }

    if (st.empty())
    {
        remaining = 0;
        return seed;
    }

    Point cur = st.top();
    vector<int> unvisited;
    for (int d = 0; d < 4; d++)
    {
        int nr = cur.r + dr[d];
        int nc = cur.c + dc[d];
        if (nr >= 0 && nr < MAZE_HEIGHT && nc >= 0 && nc < MAZE_WIDTH && !visited[nr][nc])
            unvisited.push_back(d);
    }

    if (!unvisited.empty())
    {
        int d = unvisited[GetRandomValue(0, unvisited.size() - 1)];
        int nr = cur.r + dr[d];
        int nc = cur.c + dc[d];
        MakePath(maze, cur.r, cur.c, dirs[d]);
        visited[nr][nc] = true;
        remaining--;
        st.push({nr, nc});
        return {nr, nc};
    }
    else
    {
        st.pop();
        return st.empty() ? cur : st.top();
    }
}

inline Point Prims(vector<vector<uint8_t>> &maze, vector<vector<bool>> &visited, Point seed, int &remaining)
{
    vector<pair<Point, int>> frontier;
    for (int r = 0; r < MAZE_HEIGHT; r++)
    {
        for (int c = 0; c < MAZE_WIDTH; c++)
        {
            if (visited[r][c])
            {
                for (int d = 0; d < 4; d++)
                {
                    int nr = r + dr[d];
                    int nc = c + dc[d];
                    if (nr >= 0 && nr < MAZE_HEIGHT && nc >= 0 && nc < MAZE_WIDTH && !visited[nr][nc])
                    {
                        frontier.push_back({{r, c}, d});
                    }
                }
            }
        }
    }

    if (!frontier.empty())
    {
        int idx = GetRandomValue(0, frontier.size() - 1);
        Point p = frontier[idx].first;
        int d = frontier[idx].second;
        int nr = p.r + dr[d];
        int nc = p.c + dc[d];
        MakePath(maze, p.r, p.c, dirs[d]);
        visited[nr][nc] = true;
        remaining--;
        return {nr, nc};
    }

    remaining = 0;
    return seed;
}

inline Point Sidewinder(vector<vector<uint8_t>> &maze, vector<vector<bool>> &visited, Point seed, int &remaining)
{
    static vector<Point> run;
    if (seed.r == 0 && seed.c == 0)
    {
        run.clear();
    }

    visited[seed.r][seed.c] = true;
    run.push_back(seed);

    bool at_eastern_boundary = (seed.c == MAZE_WIDTH - 1);
    bool at_northern_boundary = (seed.r == 0);
    bool should_close_out = at_eastern_boundary || (!at_northern_boundary && GetRandomValue(0, 1) == 0);

    if (should_close_out)
    {
        if (!at_northern_boundary)
        {
            Point member = run[GetRandomValue(0, run.size() - 1)];
            MakePath(maze, member.r, member.c, north);
        }
        run.clear();
    }
    else
    {
        MakePath(maze, seed.r, seed.c, east);
    }

    remaining--;
    seed.c++;
    if (seed.c >= MAZE_WIDTH)
    {
        seed.c = 0;
        seed.r++;
    }
    return seed;
}

#endif