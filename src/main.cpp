#include <raylib.h>
#include <vector>
#include <cstdint>
#include <queue>
#include "algos.h"

using namespace std;

void Grid(const vector<vector<uint8_t>> &maze);
void DrawX(int row, int col, Color color);
vector<vector<int>> DistMap(const vector<vector<uint8_t>> &maze, Point start, Point end);
vector<Point> Solver(const vector<vector<uint8_t>> &maze, Point start, Point end, bool &solved);
void DrawPath(const vector<Point> &path);
void DrawHeatMap(const vector<vector<int>> &dist, float heatMapColor);
uint8_t GetRandomDirection();

int main()
{
    int FPS = 120;
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
    double delta = 5;
    vector<Point> animatedPath = {};

    Point seed = {GetRandomValue(0, MAZE_HEIGHT - 1), GetRandomValue(0, MAZE_WIDTH - 1)};
    queue<Point> positions;
    positions.push(seed);

    vector<vector<bool>> visited(MAZE_HEIGHT, vector<bool>(MAZE_WIDTH, false));
    visited[seed.r][seed.c] = true;
    int remaining = MAZE_WIDTH * MAZE_HEIGHT - 1;

    float color;
    int currentAlgo = 1;

    auto Reset = [&]() {
        completed = false;
        solved = false;
        maze.assign(MAZE_HEIGHT, vector<uint8_t>(MAZE_WIDTH, 0));
        path.clear();
        animatedPath.clear();
        i = 0;

        visited.assign(MAZE_HEIGHT, vector<bool>(MAZE_WIDTH, false));
        if (currentAlgo == 2 || currentAlgo == 6)
        {
            seed = {0, 0};
            remaining = MAZE_WIDTH * MAZE_HEIGHT;
        }
        else
        {
            seed = {GetRandomValue(0, MAZE_HEIGHT - 1), GetRandomValue(0, MAZE_WIDTH - 1)};
            visited[seed.r][seed.c] = true;
            remaining = MAZE_WIDTH * MAZE_HEIGHT - 1;
        }
    };

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        if(IsKeyPressed(KEY_UP)) {
            FPS += 30;
            SetTargetFPS(FPS);
        }
        if(IsKeyPressed(KEY_DOWN)) {
            FPS -= 30;
            FPS = max(FPS, 30);
            SetTargetFPS(FPS);
        }

        if (IsKeyPressed(KEY_R)) Reset();
        if (IsKeyPressed(KEY_ONE))   { currentAlgo = 1; Reset(); }
        if (IsKeyPressed(KEY_TWO))   { currentAlgo = 2; Reset(); }
        if (IsKeyPressed(KEY_THREE)) { currentAlgo = 3; Reset(); }
        if (IsKeyPressed(KEY_FOUR))  { currentAlgo = 4; Reset(); }
        if (IsKeyPressed(KEY_FIVE))  { currentAlgo = 5; Reset(); }
        if (IsKeyPressed(KEY_SIX))   { currentAlgo = 6; Reset(); }

        if (!completed)
        {
            DrawRectangle(PADDING + CELL_SIZE * seed.c, PADDING + CELL_SIZE * seed.r, CELL_SIZE, CELL_SIZE, GREEN);
            switch (currentAlgo)
            {
            case 1:
                seed = AldousBroder(maze, visited, seed, remaining);
                break;
            case 2:
                seed = BinaryTree(maze, visited, seed, remaining);
                break;
            case 3:
                seed = HuntAndKill(maze, visited, seed, remaining);
                break;
            case 4:
                seed = RecursiveBacktracker(maze, visited, seed, remaining);
                break;
            case 5:
                seed = Prims(maze, visited, seed, remaining);
                break;
            case 6:
                seed = Sidewinder(maze, visited, seed, remaining);
                break;
            }
            if (remaining <= 0)
                completed = true;
        }
        else if (!solved)
        {
            path = Solver(maze, start, end, solved);
        }

        DrawText("ALGORITHMS :-", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING, 30, RAYWHITE);
        DrawText("1 - AldousBroder", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 1, 20, currentAlgo == 1 ? GREEN : GRAY);
        DrawText("2 - BinaryTree", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 2, 20, currentAlgo == 2 ? GREEN : GRAY);
        DrawText("3 - HuntAndKill", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 3, 20, currentAlgo == 3 ? GREEN : GRAY);
        DrawText("4 - Backtracker", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 4, 20, currentAlgo == 4 ? GREEN : GRAY);
        DrawText("5 - Prims", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 5, 20, currentAlgo == 5 ? GREEN : GRAY);
        DrawText("6 - Sidewinder", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 6, 20, currentAlgo == 6 ? GREEN : GRAY);
        DrawText("1-6 - Algorithms", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 8, 20, Fade(GRAY, 0.8));
        DrawText("H or H+C - HeatMap", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 9, 20,  Fade(GRAY, 0.8));
        DrawText("S - Solve", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 10, 20, Fade(GRAY, 0.8));
        DrawText("R - Reset", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 11, 20, Fade(GRAY, 0.8));
        DrawText("UP/DOWN - Speed", PADDING + MAZE_WIDTH * CELL_SIZE + CELL_SIZE, PADDING + CELL_SIZE * 12, 20, Fade(GRAY, 0.8));

        if (IsKeyDown(KEY_S))
        {
            DrawPath(animatedPath);
            if (i < path.size() && (GetTime() - time) >= delta)
            {
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
        DrawLine(PADDING, PADDING + CELL_SIZE * MAZE_HEIGHT, PADDING, PADDING + CELL_SIZE * (MAZE_HEIGHT - 1), BLACK);
        DrawLine(PADDING + CELL_SIZE * MAZE_WIDTH, PADDING, PADDING + CELL_SIZE * (MAZE_WIDTH - 1), PADDING, BLACK);
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
    return 0;
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