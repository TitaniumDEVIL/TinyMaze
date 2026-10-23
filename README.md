

```
# TinyMaze

A maze generator and solver built in C++ using raylib. It renders a grid based maze on screen and lets you watch it being carved out step by step, then solve it with a shortest path search.

The project supports six generation algorithms, each picked live from the keyboard. These are Aldous Broder, Binary Tree, Hunt and Kill, Recursive Backtracker, Prim's, and Sidewinder. Once a maze is finished, you can run a BFS based solver that animates the path from the bottom left cell to the top right one, or turn on a heat map that colors each cell by its distance from the start.

The book Mazes for Programmers by Jamis Buck is what got me started on this. Most of the algorithms here, and the way walls are stored as bits on each cell, come straight from that book. This repo is my own C++ and raylib take on those ideas, written so I could actually see them run on screen instead of just reading about them.

Built with C++ and raylib.

To run it, install raylib on your system, compile the files in the src folder, and launch the resulting binary. Inside the window, press 1 to 6 to pick an algorithm, S to solve, H to show the heat map, C while holding H to switch to a multi color variant, R to reset, and the up and down arrow keys to change the speed.

This is a learning project, not a finished tool.
```
