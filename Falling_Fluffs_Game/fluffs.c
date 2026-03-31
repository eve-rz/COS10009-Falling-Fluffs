// In summary, fluffs.c is the main driver, handling the game flow, logic, and rendering.

#include "object.h"
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// Global variable definitions
GameStates gameState;                           // Tracks the current state of the game (e.g., menu, playing, etc.)
int stage;                                      // Tracks the current game stage
int playerMode;                                 // Tracks the player mode (1 or 2 players)
bool startGame;                                 // Flag to determine if the game has started
float fallSpeed;                                // Speed at which objects fall
float spawnInterval;                            // Interval between object spawns
float spawnTimer, stageTimer, bombSpawnTimer;   // Timers for object spawning, stage duration, and bomb spawning
bool windActive, thunderActive;                 // Flags for environmental effects
float windTimer, thunderTimer;                  // Timers for wind and thunder effects
float screenShakeTimer;                         // Timer for screen shake effect
float windForce;                                // Strength of the wind effect
float musicVolume = 1.0f;                       // Volume for music
bool isMuted = false;                           // Flag for muting audio

// Texture definitions
// Preloaded textures for various game elements like background, characters, and objects
Texture2D backgroundTexture;
Texture2D calmBackground;
Texture2D nightBackground;
Texture2D volcanoBackground;
Texture2D optionsBackground;
Texture2D loadBackground;
Texture2D pauseBackground;
Texture2D transitionTexture;
Texture2D gameOverTexture;
Texture2D endTexture;
Texture2D pauseIconTexture;
Texture2D goodFoodTextures[5];
Texture2D badFoodTextures[3];
Texture2D koalaTexture;
Texture2D bearTexture;
Texture2D bombTexture;
Texture2D fallingStarTexture;
Texture2D fireTexture;
Texture2D heartTexture;
Texture2D windLeftTexture;
Texture2D windRightTexture;
Texture2D portalTexture;

// Sound/Music definitions
// Preloaded audio for game interactions and effects
Sound burningSound;
Sound flingSound;
Sound goodFoodAudio;
Sound badFoodAudio;
Sound bombAudio;
Sound thunderAudio;
Music startingScreenMusic;
Music backgroundMusic;

// Function to initializes the game environment (window, audio, global variables), manages the game loop by switching between states, and handles cleanup upon exit.
int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Falling Fluffs! (๑ᵔ⤙ᵔ๑)");
    InitAudioDevice();
    SetTargetFPS(60);

    // Initialize global variables
    gameState = GAME_STATE_MENU;                // Ensure we start at the menu
    stage = 1;
    fallSpeed = INITIAL_FALL_SPEED;
    spawnInterval = SPAWN_INTERVAL;
    startGame = false;
    playerMode = 1;                             // Default to 1 player
    
    Characters koala = {0};
    Characters bear = {0};
    Food foodArray[MAX_FOOD] = {0};
    Bomb bomb = {0};
    FallingStar starArray[3] = {0};

    // Initialize textures and game
    InitGame(&koala, &bear, foodArray, &bomb, starArray, playerMode);
    backgroundTexture = LoadTexture("assets/sky.png");                  // Load menu background first

    gameState = GAME_STATE_MENU;
    GameStates previousState = GAME_STATE_MENU;
    SetExitKey(0);                                                      // Disable default exit key (ESC)

    // Load game state at the start
    LoadGameState(&koala, &bear, &stage);

    while (!WindowShouldClose()) {          
        float deltaTime = GetFrameTime();                               // Get time elapsed since the last frame

        // Check for window close
        if (WindowShouldClose()) {
            printf("Window closing, saving game...\n");
            SaveGameState(&koala, &bear, stage);
            break;  // Exit the game loop
        }

        // Handle ESC key globally
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (gameState == GAME_STATE_PLAYING) {
                gameState = GAME_STATE_PAUSE;
            }
        }

        switch (gameState) {
            case GAME_STATE_MENU:
                BeginDrawing();
                ClearBackground(RAYWHITE);
                ShowMainMenu(&koala, &bear, foodArray, &bomb, starArray, playerMode);
                EndDrawing();
                if (previousState != GAME_STATE_MENU) {
                    InitGame(&koala, &bear, foodArray, &bomb, starArray, playerMode);
                    previousState = GAME_STATE_MENU;
                }
                break;

            case GAME_STATE_OPTIONS:
                ShowStartMenu(&koala, &bear, foodArray, &bomb, starArray);
                previousState = GAME_STATE_OPTIONS;
                break;

            case GAME_STATE_PLAYING:
                if (!startGame) {
                    InitGame(&koala, &bear, foodArray, &bomb, starArray, playerMode);
                    startGame = true;
                }
                UpdateGame(&koala, &bear, foodArray, &bomb, starArray, deltaTime);
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawGame(koala, bear, foodArray, bomb, starArray, stage);
                EndDrawing();
                previousState = GAME_STATE_PLAYING;
                break;

            case GAME_STATE_PAUSE:
                ShowPauseMenu(&koala, &bear);
                if (gameState == GAME_STATE_PLAYING) {
                    BeginDrawing();
                    ClearBackground(RAYWHITE);
                    EndDrawing();
                }
                break;

            case GAME_STATE_GAME_OVER:
                ShowGameOverScreen();
                if (gameState == GAME_STATE_MENU) {
                    // Reinitialize game when returning to menu
                    InitGame(&koala, &bear, foodArray, &bomb, starArray, playerMode);
                    PlayMusicStream(startingScreenMusic);
                }
                break;

            case GAME_STATE_END:
                ShowEndGameScreen();
                break;
        }
    }

    // Cleanup sequence
    printf("Starting cleanup...\n");
    CleanupAndExit();
    CloseWindow();  // Close window last
    printf("Cleanup complete\n");
    
    return 0;
}

// Function to load textures, sounds, and initialize gameplay variables
void InitGame(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray, int playerMode) {
    backgroundTexture = LoadTexture("assets/sky.png");
    calmBackground = LoadTexture("assets/calm.png");
    nightBackground = LoadTexture("assets/night.png");
    volcanoBackground = LoadTexture("assets/volcano.png");
    optionsBackground = LoadTexture("assets/options.png");
    pauseBackground = LoadTexture("assets/pause.png");
    transitionTexture = LoadTexture("assets/transition.png");
    gameOverTexture = LoadTexture("assets/over.png");
    loadBackground = LoadTexture("assets/load.png");
    optionsBackground = LoadTexture("assets/options.png");
    endTexture = LoadTexture("resources/end.png");
    koalaTexture = LoadTexture("assets/koala.png");
    bearTexture = LoadTexture("assets/bear.png");

    goodFoodTextures[0] = LoadTexture("assets/blueberry.png");
    goodFoodTextures[1] = LoadTexture("assets/carrot.png");
    goodFoodTextures[2] = LoadTexture("assets/eggplant.png");
    goodFoodTextures[3] = LoadTexture("assets/tomato.png");
    goodFoodTextures[4] = LoadTexture("assets/cherry.png");
    badFoodTextures[0] = LoadTexture("assets/chili.png");
    badFoodTextures[1] = LoadTexture("assets/garlic.png");
    badFoodTextures[2] = LoadTexture("assets/mushroom.png");
    bombTexture = LoadTexture("assets/bomb.png");
    fallingStarTexture = LoadTexture("assets/shooting_star.png");
    fireTexture = LoadTexture("assets/fire.png");
    heartTexture = LoadTexture("assets/heart5.1.png");
    windLeftTexture = LoadTexture("assets/wind_left.png");
    windRightTexture = LoadTexture("assets/wind_right.png");
    portalTexture = LoadTexture("assets/portal_1.png");

    goodFoodAudio = LoadSound("assets/good_food_audio.mp3");
    badFoodAudio = LoadSound("assets/bad_food_audio.mp3");
    bombAudio = LoadSound("assets/bomb_audio.mp3");
    thunderAudio = LoadSound("assets/thunder_audio.mp3");
    startingScreenMusic = LoadMusicStream("assets/starting_screen.mp3");
    backgroundMusic = LoadMusicStream("assets/background.mp3");
    burningSound = LoadSound("assets/burning.mp3");
    flingSound = LoadSound("assets/fling.mp3");

    // Initialize game state variables
    gameState = GAME_STATE_PLAYING;
    stage = 1;
    fallSpeed = INITIAL_FALL_SPEED;
    spawnInterval = SPAWN_INTERVAL;
    spawnTimer = 0.0f;
    stageTimer = 0.0f;
    bombSpawnTimer = 0.0f;
    windActive = false;
    thunderActive = false;
    windTimer = 0.0f;
    thunderTimer = 0.0f;
    screenShakeTimer = 0.0f;
    windForce = 0.0f;

    // Initialize koala character
    koala->position = (Vector2){WINDOW_WIDTH / 4, WINDOW_HEIGHT - 160};
    koala->speedMultiplier = 1.0f;
    koala->score = 0.0f;
    koala->isBurning = false;
    koala->isFlinging = false;
    koala->texture = koalaTexture;

    // Initialize bear character only if in two-player mode
    if (playerMode == 2) {
        printf("Initializing Bear character\n");
        bear->position = (Vector2){(3 * WINDOW_WIDTH) / 4, WINDOW_HEIGHT - 160};
        bear->speedMultiplier = 1.0f;
        bear->score = 0.0f;
        bear->isBurning = false;
        bear->isFlinging = false;
        bear->texture = bearTexture;
    } else {
        bear->texture.id = 0; // Set to an invalid texture ID to indicate no bear
        printf("Bear character not initialized (one player mode)\n");
    }

    // Initialize food array
    for (int i = 0; i < MAX_FOOD; i++) {
        foodArray[i].active = true; // Set food to active
        foodArray[i].position = (Vector2){rand() % WINDOW_WIDTH, -SQUARE_SIZE}; // Random position above the window

        if (i < 5) {
            foodArray[i].type = FOOD_TYPE_GOOD; // Good food
            foodArray[i].texture = goodFoodTextures[i]; // Assign good food texture
        } else {
            foodArray[i].type = FOOD_TYPE_BAD; // Bad food
            foodArray[i].texture = badFoodTextures[i - 5]; // Assign bad food texture
        }
    }

    // Initialize bomb
    bomb->active = false;

    // Initialize falling stars
    for (int i = 0; i < 3; i++) {
        starArray[i].active = false;
    }
}

// Function that handles all rendering tasks
void DrawGame(Characters koala, Characters bear, Food foodArray[], Bomb bomb, FallingStar *starArray, int stage) {
    // Draw background
    DrawTexture(backgroundTexture, 0, 0, WHITE);
    
    // Draw wind effects (right after background, before everything else)
    if (stage >= 10) {  // Only draw wind effects after stage 10
        DrawWindEffects(foodArray, &bomb);
    }
    
    // Define colors
    Color koalaTextColor = (Color){85, 93, 98, 255}; // #555D62
    Color bearTextColor = (Color){104, 85, 80, 255}; // #685550

    // Maximum number of hearts
    int maxHearts = 5;

    // Declare stageText and stageTextWidth only once
    const char *stageText = TextFormat("Stage: %d", stage);
    int stageTextWidth = MeasureText(stageText, 20);

    int pauseButtonX = WINDOW_WIDTH - 50; // Position for the pause button
    int pauseButtonY = 10; // Align with stage text

    // Draw pause button
    Rectangle pauseButtonRect = {WINDOW_WIDTH - 50, 10, 40, 40};
    DrawRectangleRec(pauseButtonRect, (Color){208, 223, 218, 255});
    DrawText("||", pauseButtonRect.x + 16, pauseButtonRect.y + 8, 20, BLACK);
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePoint = GetMousePosition();
        if (CheckCollisionPointRec(mousePoint, pauseButtonRect)) {
            printf("Pause button clicked\n");
            gameState = GAME_STATE_PAUSE;
        }
    }

    // Draw the stage text
    DrawText(stageText, WINDOW_WIDTH - stageTextWidth - 65, 18, 20, BLACK); // Adjust position as needed

    float characterScaleFactor = 0.25f;
    float bombScaleFactor = 0.2f;

    // Draw Koala
    Rectangle koalaSrcRect = {0, 0, koalaTexture.width, koalaTexture.height};
    Rectangle koalaDestRect = {koala.position.x, koala.position.y, 
                             koalaTexture.width * characterScaleFactor, 
                             koalaTexture.height * characterScaleFactor};
    DrawTexturePro(koalaTexture, koalaSrcRect, koalaDestRect, (Vector2){0, 0}, 0.0f, WHITE);

    // Draw Bear only if in two-player mode
    if (playerMode == 2) {
        Rectangle bearSrcRect = {0, 0, bearTexture.width, bearTexture.height};
        Rectangle bearDestRect = {bear.position.x, bear.position.y, 
                                bearTexture.width * characterScaleFactor, 
                                bearTexture.height * characterScaleFactor};
        DrawTexturePro(bearTexture, bearSrcRect, bearDestRect, (Vector2){0, 0}, 0.0f, WHITE);
        
        // Draw Bear label
        DrawText("Bear:", 10, 48, 20, bearTextColor);

        // Draw hearts for Bear
        float bearRemaining = maxHearts - bear.score;
        int bearFullHearts = (int)bearRemaining;
        int bearHalfHearts = (bearRemaining - bearFullHearts) >= 0.5f ? 1 : 0;

        for (int i = 0; i < bearFullHearts; i++) {
            DrawTexture(heartTexture, 80 + i * (heartTexture.width + 5), 40, WHITE);
        }
        if (bearHalfHearts) {
            DrawTextureRec(heartTexture, (Rectangle){0, 0, heartTexture.width / 2, heartTexture.height}, 
                          (Vector2){80 + bearFullHearts * (heartTexture.width + 5), 40}, WHITE);
        }
    }

    // Draw food
    for (int i = 0; i < MAX_FOOD; i++) {
        if (foodArray[i].active) {
            Rectangle srcRect = {0, 0, foodArray[i].texture.width, foodArray[i].texture.height};
            Rectangle destRect = {foodArray[i].position.x, foodArray[i].position.y, 
                                foodArray[i].texture.width * 0.5f, 
                                foodArray[i].texture.height * 0.5f};

            // Use global textures based on food type
            if (foodArray[i].type == FOOD_TYPE_GOOD) {
                DrawTexturePro(goodFoodTextures[i], srcRect, destRect, (Vector2){0, 0}, 0.0f, WHITE);
            } else if (foodArray[i].type == FOOD_TYPE_BAD) {
                DrawTexturePro(badFoodTextures[i - 5], srcRect, destRect, (Vector2){0, 0}, 0.0f, WHITE);
            }
        }
    }

    // Draw bombs
    if (bomb.active) {
        Rectangle bombSrcRect = {0, 0, bombTexture.width, bombTexture.height};
        Rectangle bombDestRect = {bomb.position.x, bomb.position.y, 
                                bombTexture.width * bombScaleFactor, 
                                bombTexture.height * bombScaleFactor};
        DrawTexturePro(bombTexture, bombSrcRect, bombDestRect, (Vector2){0, 0}, 0.0f, WHITE);
    }

    // Draw falling stars and fire
    float starScaleFactor = 0.2f;
    for (int i = 0; i < 3; i++) {
        if (starArray[i].active) {
            if (!starArray[i].isFire) {
                // Star collision box
                Rectangle starSrcRect = {0, 0, fallingStarTexture.width, fallingStarTexture.height};
                Rectangle starDestRect = {
                    starArray[i].position.x, 
                    starArray[i].position.y, 
                    fallingStarTexture.width * starScaleFactor, 
                    fallingStarTexture.height * starScaleFactor
                };
                DrawTexturePro(fallingStarTexture, starSrcRect, starDestRect, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                // Draw fire
                Rectangle fireSrcRect = {0, 0, fireTexture.width, fireTexture.height};
                Rectangle fireDestRect = {
                    starArray[i].position.x, 
                    starArray[i].position.y, 
                    fireTexture.width * starScaleFactor, 
                    fireTexture.height * starScaleFactor
                };
                DrawTexturePro(fireTexture, fireSrcRect, fireDestRect, (Vector2){0, 0}, 0.0f, WHITE);
            }
        }
    }

    // Draw portal effect if character has just landed
    if (koala.portalTimer > 0) {
        Vector2 portalPosition = {koala.position.x + 60, koala.position.y + 80};  // Adjust position
        DrawPortalEffect(portalPosition);
    }
    if (playerMode == 2 && bear.portalTimer > 0) {
        Vector2 portalPosition = {bear.position.x + 60, bear.position.y + 80};  // Adjust position
        DrawPortalEffect(portalPosition);
    }

    // Draw burning effect on characters
    if (koala.isBurning) {
        DrawText("BURNING!", koala.position.x, koala.position.y - 20, 20, RED);
    }
    if (bear.texture.id != 0 && bear.isBurning) {
        DrawText("BURNING!", bear.position.x, bear.position.y - 20, 20, RED);
    }

    // Draw Koala label
    DrawText("Koala:", 10, 18, 20, koalaTextColor);

    // Draw hearts for Koala
    float koalaRemaining = maxHearts - koala.score;
    int koalaFullHearts = (int)koalaRemaining;
    int koalaHalfHearts = (koalaRemaining - koalaFullHearts) >= 0.5f ? 1 : 0;

    for (int i = 0; i < koalaFullHearts; i++) {
        DrawTexture(heartTexture, 80 + i * (heartTexture.width + 5), 10, WHITE);
    }
    if (koalaHalfHearts) {
        DrawTextureRec(heartTexture, (Rectangle){0, 0, heartTexture.width / 2, heartTexture.height}, 
                      (Vector2){80 + koalaFullHearts * (heartTexture.width + 5), 10}, WHITE);
    }
}

/*
# Compile the program
gcc fluffs.c utils.c -o fluffs.exe -I C:\raylib\raylib\include -L C:\raylib\raylib\lib -lraylib -lopengl32 -lgdi32 -lwinmm

# Run the program
.\fluffs.exe

*/
