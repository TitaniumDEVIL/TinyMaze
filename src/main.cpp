#include <raylib.h>
#include <vector>
#include <cstdint>
#include <queue>

using namespace std;

const int CELL_SIZE = 40;
const int MAZE_WIDTH = 20;
const int MAZE_HEIGHT = 20;

const int PADDING = 40;

const int SCREEN_WIDTH = PADDING * 2 + CELL_SIZE * MAZE_WIDTH;
const int SCREEN_HEIGHT = PADDING * 2 + CELL_SIZE * MAZE_HEIGHT;

constexpr uint8_t north = 1 << 0; // 1
constexpr uint8_t east = 1 << 1;  // 2
constexpr uint8_t south = 1 << 2; // 4
constexpr uint8_t west = 1 << 3;  // 8

const uint8_t dirs[4] = {north, east, south, west};
const int dr[4] = {-1, 0, 1, 0};
const int dc[4] = {0, 1, 0, -1};

struct Point
{
    int r, c;
};

uint8_t opposite(uint8_t dir)
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

Point Generate(vector<vector<uint8_t>> &maze, vector<vector<bool>> &visited, Point seed, int &remaining);
void Grid(const vector<vector<uint8_t>> &maze);
void MakePath(vector<vector<uint8_t>> &maze, int row, int col, uint8_t dir);
void DrawX(int row, int col, Color color);
vector<vector<int>> DistMap(const vector<vector<uint8_t>> &maze, Point start, Point end);
vector<Point> Solver(const vector<vector<uint8_t>> &maze, Point start, Point end, bool &solved);
void DrawPath(const vector<Point> &path);
void DrawHeatMap(const vector<vector<int>> &dist, float heatMapColor);
uint8_t GetRandomDirection();

int main()
{
    int FPS = 600;
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "MAZE");
    SetTargetFPS(FPS);

    vector<vector<uint8_t>> maze(MAZE_HEIGHT, vector<uint8_t>(MAZE_WIDTH, 0));

    bool completed = false;

    bool solved = false;
    vector<Point> path = {};

    Point start = {MAZE_HEIGHT - 1, 0};
    Point end = {0, MAZE_WIDTH - 1};
    int i = 0;
    double time = GetTime();
    double delta = 2 * FPS/120;
    vector<Point> animatedPath = {};

    Point seed = {GetRandomValue(0, MAZE_HEIGHT - 1), GetRandomValue(0, MAZE_WIDTH - 1)};
    queue<Point> positions;
    positions.push(seed);

    vector<vector<bool>> visited(MAZE_HEIGHT, vector<bool>(MAZE_WIDTH, false));
    visited[seed.r][seed.c] = true;
    int remaining = MAZE_WIDTH * MAZE_HEIGHT - 1;

    float color;

    while (!WindowShouldClose())
    {
        BeginDrawing();

        if(IsKeyPressed(KEY_UP)) {
            FPS += 120;
            delta = 2 * FPS/120;
            SetTargetFPS(FPS);
        }
        if(IsKeyPressed(KEY_DOWN)) {
            FPS -= 120;
            FPS = max(FPS, 120);
            delta = 2 * FPS/120;
            SetTargetFPS(FPS);
        }

        if (IsKeyPressed(KEY_R))
        {
            completed = false;
            solved = false;
            maze.assign(MAZE_HEIGHT, vector<uint8_t>(MAZE_WIDTH, 0));
            path.clear();
            i = 0;

            visited.assign(MAZE_HEIGHT, vector<bool>(MAZE_WIDTH, false));
            seed = {GetRandomValue(0, MAZE_HEIGHT - 1), GetRandomValue(0, MAZE_WIDTH - 1)};
            visited[seed.r][seed.c] = true;
            remaining = MAZE_WIDTH * MAZE_HEIGHT - 1;
        }

        if (!completed)
        {
            DrawRectangle(PADDING + CELL_SIZE*seed.c, PADDING + CELL_SIZE*seed.r, CELL_SIZE, CELL_SIZE, GREEN);
            seed = Generate(maze, visited, seed, remaining);
            if (remaining == 0)
                completed = true;
        }
        else if (!solved)
        {
            path = Solver(maze, start, end, solved);
        }

        
        ClearBackground(BLACK);

        if (IsKeyDown(KEY_S))
        {
            DrawPath(animatedPath);
            if (i < path.size() && (GetTime() - time) >= delta)
            {
                //DrawRectangle(PADDING + path[i].c * CELL_SIZE, PADDING + path[i].r * CELL_SIZE, CELL_SIZE, CELL_SIZE, RED);
                animatedPath.push_back(path[i]);
                i++;
            }
        }
        if (IsKeyPressed(KEY_H)) {
            color = GetRandomValue(1, 6) * 60.0f;
        }
        if (IsKeyDown(KEY_H))
        {
            DrawHeatMap(DistMap(maze, start, end), color);
        }
        if (i >= path.size())
        {
            i = 0;
        }
        Grid(maze);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void Grid(const vector<vector<uint8_t>> &maze)
{
    for (int row = 0; row < MAZE_HEIGHT; row++)
    {
        for (int col = 0; col < MAZE_WIDTH; col++)
        {
            int x = PADDING + col * CELL_SIZE;
            int y = PADDING + row * CELL_SIZE;

            if (!(maze[row][col] & north))
                DrawLine(x, y, x + CELL_SIZE, y, WHITE);
            if (!(maze[row][col] & east))
                DrawLine(x + CELL_SIZE, y, x + CELL_SIZE, y + CELL_SIZE, WHITE);
            if (!(maze[row][col] & south))
                DrawLine(x, y + CELL_SIZE, x + CELL_SIZE, y + CELL_SIZE, WHITE);
            if (!(maze[row][col] & west))
                DrawLine(x, y, x, y + CELL_SIZE, WHITE);
        }
    }
}

void MakePath(vector<vector<uint8_t>> &maze, int row, int col, uint8_t dir)
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

Point Generate(vector<vector<uint8_t>> &maze, vector<vector<bool>> &visited, Point seed, int &remaining)
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

uint8_t GetRandomDirection()
{
    int val = GetRandomValue(0, 3);
    if (val == 0)
        return north;
    if (val == 1)
        return east;
    if (val == 2)
        return south;
    if (val == 3)
        return west;
}

vector<Point> Solver(const vector<vector<uint8_t>> &maze, Point start, Point end, bool &solved)
{
    vector<vector<int>> dist(MAZE_HEIGHT, vector<int>(MAZE_WIDTH));
    dist = DistMap(maze, start, end);

    solved = true;

    if (dist[end.r][end.c] == -1)
        return {};

    vector<Point> path;
    Point cur = end;
    path.push_back(cur);

    while (cur.r != start.r || cur.c != start.c)
    {
        int r = cur.r;
        int c = cur.c;
        int target = dist[r][c] - 1;

        if ((maze[r][c] & north) && dist[r - 1][c] == target)
        {
            cur = {r - 1, c};
        }
        else if ((maze[r][c] & east) && dist[r][c + 1] == target)
        {
            cur = {r, c + 1};
        }
        else if ((maze[r][c] & south) && dist[r + 1][c] == target)
        {
            cur = {r + 1, c};
        }
        else if ((maze[r][c] & west) && dist[r][c - 1] == target)
        {
            cur = {r, c - 1};
        }
        else
        {
            break;
        }
        path.push_back(cur);
    }
    return path;
}

void DrawPath(const vector<Point> &path)
{
    for (size_t i = 0; i < path.size(); i++)
    {
        DrawRectangle(PADDING + path[i].c * CELL_SIZE, PADDING + path[i].r * CELL_SIZE, CELL_SIZE, CELL_SIZE, ORANGE);
    }
}

vector<vector<int>> DistMap(const vector<vector<uint8_t>> &maze, Point start, Point end)
{
    vector<vector<int>> dist(MAZE_HEIGHT, vector<int>(MAZE_WIDTH, -1));
    queue<Point> q;

    dist[start.r][start.c] = 0;
    q.push(start);

    while (!q.empty())
    {
        Point cur = q.front();
        q.pop();

        int r = cur.r;
        int c = cur.c;
        int d = dist[r][c];

        if ((maze[r][c] & north) && dist[r - 1][c] == -1)
        {
            dist[r - 1][c] = d + 1;
            q.push({r - 1, c});
        }
        if ((maze[r][c] & east) && dist[r][c + 1] == -1)
        {
            dist[r][c + 1] = d + 1;
            q.push({r, c + 1});
        }
        if ((maze[r][c] & south) && dist[r + 1][c] == -1)
        {
            dist[r + 1][c] = d + 1;
            q.push({r + 1, c});
        }
        if ((maze[r][c] & west) && dist[r][c - 1] == -1)
        {
            dist[r][c - 1] = d + 1;
            q.push({r, c - 1});
        }
    }
    return dist;
}

void DrawHeatMap(const vector<vector<int>> &dist, float heatMapColor)
{
    int maxD = 1;
    for (int i = 0; i < MAZE_HEIGHT; i++)
    {
        for (int j = 0; j < MAZE_WIDTH; j++)
        {
            maxD = max(maxD, dist[i][j]);
        }
    }
    for (int i = 0; i < MAZE_HEIGHT; i++)
    {
        for (int j = 0; j < MAZE_WIDTH; j++)
        {
            if (dist[i][j] < 0)
                continue;
            float t = (float)dist[i][j] / maxD;
            float hue = (1-t) * 240.0f;
            Color col;
            if (IsKeyDown(KEY_C))
                col = ColorFromHSV(hue, 0.8f, 0.85f);
            else
                col = ColorFromHSV(heatMapColor, 0.9f, 0.1 + 0.9f * (1 - t));
            DrawRectangle(PADDING + j * CELL_SIZE, PADDING + i * CELL_SIZE, CELL_SIZE, CELL_SIZE, col);
        }
    }
}