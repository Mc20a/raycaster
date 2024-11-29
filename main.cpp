#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <string>
#include <thread>
#include <SDL.h>
#include <SDL_ttf.h>

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

// Define sprint multiplier
const float SPRINT_MULTIPLIER = 2.5f;

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

// Function to handle player movement
void handleMovement(const Uint8* state, double& playerX, double& playerY, double moveSpeed, double elapsedTimeInSeconds) {
    double currentSpeed = moveSpeed;

    // Check if shift key is pressed
    if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) {
        currentSpeed *= SPRINT_MULTIPLIER;
    }

    double moveX = 0.0;
    double moveY = 0.0;

    if (state[SDL_SCANCODE_W]) {
        moveX += cos(playerAngle) * currentSpeed * elapsedTimeInSeconds;
        moveY += sin(playerAngle) * currentSpeed * elapsedTimeInSeconds;
    }
    if (state[SDL_SCANCODE_S]) {
        moveX -= cos(playerAngle) * currentSpeed * elapsedTimeInSeconds;
        moveY -= sin(playerAngle) * currentSpeed * elapsedTimeInSeconds;
    }
    if (state[SDL_SCANCODE_D]) {
        moveX -= sin(playerAngle) * currentSpeed * elapsedTimeInSeconds;
        moveY += cos(playerAngle) * currentSpeed * elapsedTimeInSeconds;
    }
    if (state[SDL_SCANCODE_A]) {
        moveX += sin(playerAngle) * currentSpeed * elapsedTimeInSeconds;
        moveY -= cos(playerAngle) * currentSpeed * elapsedTimeInSeconds;
    }
    if (state[SDL_SCANCODE_LEFT]) {
        playerAngle -= 2.0 * elapsedTimeInSeconds;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        playerAngle += 2.0 * elapsedTimeInSeconds;
    }

    // Normalize the movement vector if moving diagonally
    double length = sqrt(moveX * moveX + moveY * moveY);
    if (length > 0) {
        moveX /= length;
        moveY /= length;
        moveX *= currentSpeed * elapsedTimeInSeconds;
        moveY *= currentSpeed * elapsedTimeInSeconds;
    }

    // Apply the movement
    playerX += moveX;
    playerY += moveY;
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "SDL_Init Error: " << SDL_GetError() << endl;
        return 1;
    }
    cout << "SDL initialized successfully." << endl;

    // Initialize SDL_ttf
    if (TTF_Init() != 0) {
        cerr << "TTF_Init Error: " << TTF_GetError() << endl;
        SDL_Quit();
        return 1;
    }
    cout << "SDL_ttf initialized successfully." << endl;

    // Create a window
    SDL_Window* window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    cout << "Window created successfully." << endl;

    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    cout << "Renderer created successfully." << endl;

    // Load a font
    TTF_Font* font = TTF_OpenFont("C:\\Users\\Bob\\projects\\raycaster\\fonts\\open-sans\\OpenSans-Bold.ttf", 24);
    if (font == nullptr) {
        cerr << "TTF_OpenFont Error: " << TTF_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    cout << "Font loaded successfully." << endl;

    // Render text
    SDL_Color color = {255, 255, 255, 255}; // White color
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Hello, World!", color);
    if (textSurface == nullptr) {
        cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << endl;
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    cout << "Text rendered successfully." << endl;

    // Create a texture from the surface
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    if (textTexture == nullptr) {
        cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << endl;
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Define the map
    wstring map;
    map += L"################";
    map += L"#........###...#";
    map += L"#...#....###...#";
    map += L"#...#..........#";
    map += L"#...#####..##..#";
    map += L"#......#....#..#";
    map += L"#......#....#..#";
    map += L"#......#....#..#";
    map += L"###....##..##..#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#.......#......#";
    map += L"#.......#......#";
    map += L"#....######....#";
    map += L"#.........#....#";
    map += L"################";

    if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
        cerr << "Player starts inside a wall!" << endl;
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

        handleMovement(state, playerX, playerY, speed, elapsedTimeInSeconds);

        // Collision detection
        if (map.c_str()[(int)playerY * mapWidth + (int)playerX] == '#') {
            playerX -= cos(playerAngle) * speed * elapsedTimeInSeconds;
            playerY -= sin(playerAngle) * speed * elapsedTimeInSeconds;
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the scene
        for (int x = 0; x < screenWidth; x++) {
            double fDistanceToWall = rayCast(x, map); // Cast a ray for each column
            int nCeiling = static_cast<int>((screenHeight / 2.0f) - screenHeight / fDistanceToWall); // Calculate ceiling position
            int nFloor = screenHeight - nCeiling; // Calculate floor position
            renderWallsAndFloor(renderer, fDistanceToWall, nCeiling, nFloor, x, false); // Render walls and floor
        }

        // Render the FPS
        double fps = getFps();
        string fpsText = "FPS: " + to_string(static_cast<int>(fps));
        SDL_Color textColor = {255, 255, 255, 255}; // White color

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, fpsText.c_str(), textColor);
        if (textSurface == nullptr) {
            cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << endl;
        } else {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture == nullptr) {
                cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << endl;
            } else {
                SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h}; // Position and size of the text
                SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }

        // Present the rendered frame
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    renderer = nullptr;
    window = nullptr;
    TTF_Quit(); // Quit SDL_ttf
    SDL_Quit(); // Quit SDL

    cout << "Program finished. Exiting in 3 seconds..." << endl;
    this_thread::sleep_for(chrono::seconds(3));

    return 0; // Return success
}