#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include "libs/SDL2/include/SDL.h"

using namespace std;

// Screen dimensions
int screenWidth = 800;
int screenHeight = 600;

// Map dimensions
int mapWidth = 16;
int mapHeight = 16;

// Player's position and viewing angle
double playerX = 8.0;
double playerY = 10.5;
double playerAngle = 0.0;

// Field of view and rendering depth
double fieldOfView = 0.75;
const double depth = 16.0;

// Player's movement speed
const double speed = 1.5;

// Global variables for FPS calculation
int frameCount = 0;
double fps = 0.0;
auto lastFpsTime = chrono::system_clock::now();

/**
 * Calculates and returns the current FPS.
 * 
 * @return The current FPS.
 */
double getFps() {
    frameCount++;
    auto currentTime = chrono::system_clock::now();
    chrono::duration<double> elapsedTime = currentTime - lastFpsTime;

    if (elapsedTime.count() >= 1.0) {
        fps = frameCount / elapsedTime.count();
        frameCount = 0;
        lastFpsTime = currentTime;
    }

    return fps;
}

/**
 * Renders walls and floor on the screen.
 * 
 * @param renderer SDL_Renderer object used for rendering
 * @param distanceToWall Distance from the player to the wall
 * @param ceiling Y-coordinate of the ceiling
 * @param floor Y-coordinate of the floor
 * @param x X-coordinate of the current column being rendered
 * @param isBoundary Flag indicating if the current wall is a boundary
 */
void renderWallsAndFloor(SDL_Renderer* renderer, double distanceToWall, int ceiling, int floor, int x, bool isBoundary) {
    // Calculate wall shade based on distance to wall
    int wallShade = static_cast<int>(255 * (1.0 - (distanceToWall / depth)));
    if (wallShade < 0) wallShade = 0; // Ensure wallShade is not negative

    if (isBoundary) wallShade = 0; // Boundary walls are black

    // Fog effect parameters
    double fogDistance = 8.0; // Distance at which fog starts
    double fogIntensity = (distanceToWall > fogDistance) ? (distanceToWall - fogDistance) / (depth - fogDistance) : 0.0;
    if (fogIntensity > 1.0) fogIntensity = 1.0; // Clamp fog intensity to 1.0

    // Ceiling color (static)
    SDL_SetRenderDrawColor(renderer, 0, 0, 64, 255); // Static ceiling color
    SDL_RenderDrawLine(renderer, x, 0, x, ceiling); // Draw ceiling line

    // Wall color with fog effect
    int foggedWallShade = static_cast<int>(wallShade * (1.0 - fogIntensity));
    SDL_SetRenderDrawColor(renderer, foggedWallShade, foggedWallShade, foggedWallShade, 255); // Wall color
    SDL_RenderDrawLine(renderer, x, ceiling, x, floor); // Draw wall line

    // Floor color (static)
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255); // Static floor color
    SDL_RenderDrawLine(renderer, x, floor, x, screenHeight); // Draw floor line
}

/**
 * Casts a ray from the player's position to determine the distance to the nearest wall.
 * 
 * @param x X-coordinate of the current column being rendered
 * @param map The map of the game world
 * @return Distance to the nearest wall
 */
double rayCast(int x, const wstring& map) {
    double rayAngle = (playerAngle - fieldOfView / 2.0) + ((double)x / (double)screenWidth) * fieldOfView;

    double rayDirX = cos(rayAngle);
    double rayDirY = sin(rayAngle);

    int mapX = static_cast<int>(playerX);
    int mapY = static_cast<int>(playerY);

    double sideDistX;
    double sideDistY;

    double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX);
    double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);
    double perpWallDist;

    int stepX;
    int stepY;

    int hit = 0;
    int side;

    if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (playerX - mapX) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (mapX + 1.0 - playerX) * deltaDistX;
    }
    if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (playerY - mapY) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = (mapY + 1.0 - playerY) * deltaDistY;
    }

    while (hit == 0) {
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }
        if (map[mapY * mapWidth + mapX] == '#') hit = 1;
    }

    if (side == 0) perpWallDist = (mapX - playerX + (1 - stepX) / 2) / rayDirX;
    else           perpWallDist = (mapY - playerY + (1 - stepY) / 2) / rayDirY;

    return perpWallDist;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO); // Initialize SDL
    
    // Create SDL window
    SDL_Window* window = SDL_CreateWindow("Raycaster", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        screenWidth, screenHeight, 0);
    // Create SDL renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Define the map
    wstring map;
    map += L"################"; // 1
    map += L"#........###...#"; // 2
    map += L"#...#....###...#"; // 3
    map += L"#...#..........#"; // 4
    map += L"#...#####..##..#"; // 5
    map += L"#......#....#..#"; // 6
    map += L"#......#....#..#"; // 7
    map += L"#......#....#..#"; // 8
    map += L"###....##..##..#"; // 9
    map += L"#..............#"; // 10
    map += L"#..............#"; // 11
    map += L"#.......#......#"; // 12
    map += L"#.......#......#"; // 13
    map += L"#....######....#"; // 14
    map += L"#.........#....#"; // 15
    map += L"################"; // 16

    if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
        std::cerr << "Player starts inside a wall!" << std::endl;
        return 1;
    }

    auto tp1 = chrono::system_clock::now(); // Time point 1
    auto tp2 = chrono::system_clock::now(); // Time point 2

    SDL_Event ev; // SDL event
    bool running = true; // Running flag
    while (running) {

        tp2 = chrono::system_clock::now(); // Update time point 2
        chrono::duration<double> elapsedTime = tp2 - tp1; // Calculate elapsed time
        tp1 = tp2; // Update time point 1
        double elapsedTimeInSeconds = elapsedTime.count(); // Convert elapsed time to seconds

        // Handle SDL events
        while (SDL_PollEvent(&ev) != 0) {
            if (ev.type == SDL_QUIT || ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) {
                running = false; // Quit if the user closes the window or presses ESC
            }
        }

        // Get the current state of the keyboard
        const Uint8* state = SDL_GetKeyboardState(NULL);

        double cosPlayerAngle = cos(playerAngle);
        double sinPlayerAngle = sin(playerAngle);

        double moveX = 0.0;
        double moveY = 0.0;

        if (state[SDL_SCANCODE_W]) {
            moveX += cos(playerAngle) * speed * elapsedTimeInSeconds;
            moveY += sin(playerAngle) * speed * elapsedTimeInSeconds;
        }
        if (state[SDL_SCANCODE_S]) {
            moveX -= cos(playerAngle) * speed * elapsedTimeInSeconds;
            moveY -= sin(playerAngle) * speed * elapsedTimeInSeconds;
        }
        if (state[SDL_SCANCODE_D]) {
            moveX -= sin(playerAngle) * speed * elapsedTimeInSeconds;
            moveY += cos(playerAngle) * speed * elapsedTimeInSeconds;
        }
        if (state[SDL_SCANCODE_A]) {
            moveX += sin(playerAngle) * speed * elapsedTimeInSeconds;
            moveY -= cos(playerAngle) * speed * elapsedTimeInSeconds;
        }

        // Normalize the movement vector if moving diagonally
        double length = sqrt(moveX * moveX + moveY * moveY);
        if (length > 0) {
            moveX /= length;
            moveY /= length;
            moveX *= speed * elapsedTimeInSeconds;
            moveY *= speed * elapsedTimeInSeconds;
        }

        // Apply the movement
        playerX += moveX;
        playerY += moveY;

        // Collision detection
        if (map.c_str()[(int)playerY * mapWidth + (int)playerX] == '#') {
            playerX -= moveX;
            playerY -= moveY;
        }

        if (state[SDL_SCANCODE_LEFT]) {
            // Rotate left
            playerAngle -= (speed * 0.75f) * elapsedTimeInSeconds;
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            // Rotate right
            playerAngle += (speed * 0.75f) * elapsedTimeInSeconds;
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black
        SDL_RenderClear(renderer); // Clear the screen

        // Render the scene
        for (int x = 0; x < screenWidth; x++) {
            double fDistanceToWall = rayCast(x, map); // Cast a ray for each column
            int nCeiling = static_cast<int>((screenHeight / 2.0f) - screenHeight / fDistanceToWall); // Calculate ceiling position
            int nFloor = screenHeight - nCeiling; // Calculate floor position
            renderWallsAndFloor(renderer, fDistanceToWall, nCeiling, nFloor, x, false); // Render walls and floor
        }

        SDL_RenderPresent(renderer); // Present the rendered frame

        // Clear the console and print the FPS
        system("cls");
        cout << "FPS: " << getFps() << endl;
    }

    // Clean up SDL resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    renderer = nullptr;
    window = nullptr;
    SDL_Quit(); // Quit SDL
    return 0; // Return success
}