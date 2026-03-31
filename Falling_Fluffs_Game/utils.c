// In summary, utils.c provides supporting utility functions to manage effects, collisions, and save/load mechanics.

#include "object.h"
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#define STAGE_DURATION 5.0f  // Duration of each stage in seconds

//Function to handle damage and healing for the characters
void DamageCharacter(Characters *character, float damage) {
    character->score += damage;                                                         // Add damage value to the character score (+ = damage, - = healing)
    if (character->score < 0) character->score = 0;                                     // Prevent healing beyond max hearts
    printf("Health change: %.1f, Total score: %.1f\n", damage, character->score);       // Debug message for changes in health and score
    
    if (character->score >= MAX_HEARTS) {                                               // Check if character has run out of hearts
        if (playerMode == 2 || character->score >= MAX_HEARTS) {                        //if the character's score exceed the max value, the game is over 
            gameState = GAME_STATE_GAME_OVER;
        }
    }
}

    /* 
        The Character score increases by the damage value, so in easy terms: 
        Positive damage value = damage
        Negative damage valu = healing
    */

// Function to update the game state and handle gameplay logic
void UpdateGame(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray, float deltaTime) {
    PlayMusicStream(backgroundMusic);       // Play background music continuously
    static float screenFlashTimer = 0.0f;   //Thunder Flash Timer

    // Update spawn and stage timers
    spawnTimer += deltaTime;
    stageTimer += deltaTime;

    // Check if stage 30 has been completed
    if (stage == 30 && stageTimer >= STAGE_DURATION) {
        StopMusicStream(backgroundMusic);                // Stop the game music
        gameState = GAME_STATE_END;                      // Change game state to end
        ShowEndGameScreen();                             // Show the end game screen
        return;                                          // Exit immediately
    }

    // Check for stage transitions if less than stage 30
    if (stageTimer >= 10.0f && stage < 30) {
        if (stage + 1 == 10 || stage + 1 == 20) {  // Only show transitions for stages 10 and 20
            ShowStageTransition(stage + 1);        // Show transition to next stage
        }
        stage++;                                   // Move to the next stage
        stageTimer = 0.0f;                         // Reset the stage timer
    }
    
    // Adjust behavior based on stage
    if (stage < 10) {
        backgroundTexture = calmBackground;      // Calm Mode!
        fallSpeed = INITIAL_FALL_SPEED;          // Normal fall speed
        spawnInterval = SPAWN_INTERVAL;          // Standard spawn rate

    } else if (stage < 20) {
        backgroundTexture = nightBackground;               // Night Mode!
        fallSpeed = INITIAL_FALL_SPEED * 1.5f;             // Medium fall speed
        spawnInterval = SPAWN_INTERVAL * 0.5f;             // Medium spawn rate
        ApplyWindEffect(foodArray, bomb, deltaTime);       // Add wind effect
        ApplyThunderEffect(&screenFlashTimer, deltaTime);  // Add thunder effect

    } else if (stage <= 30) {
        backgroundTexture = volcanoBackground;
        fallSpeed = INITIAL_FALL_SPEED * 2.0f;              // Maximum fall speed
        spawnInterval = SPAWN_INTERVAL * 0.3f;              // Very fast spawn rate
        ApplyWindEffect(foodArray, bomb, deltaTime);        // Add wind effect
        ApplyThunderEffect(&screenFlashTimer, deltaTime);   // Add thunder effect
    }

    // Koala movement (WASD keys)
    if (koala->freezeTimer <= 0) {                                                                          // Only allow movement if not frozen
        if (IsKeyDown(KEY_A) && koala->position.x > 0) {                                            
            koala->position.x -= PLAYER_SPEED * koala->speedMultiplier;                                     // Move Left
        }
        if (IsKeyDown(KEY_D) && koala->position.x < WINDOW_WIDTH - koalaTexture.width * 0.25f) {
            koala->position.x += PLAYER_SPEED * koala->speedMultiplier;                                     // Move right
        }
    } else {
        koala->freezeTimer -= deltaTime;                                                                    // Decrease freeze timer
    }

    // Bear movement (Arrow keys)
    if (playerMode == 2) {                                                                                  // Check if in two-player mode
        if (bear->freezeTimer <= 0 && bear->texture.id != 0) {                                              // Check bear is not frozen
            if (IsKeyDown(KEY_LEFT) && bear->position.x > 0) {
                bear->position.x -= PLAYER_SPEED * bear->speedMultiplier;                                   // Move left
            }
            if (IsKeyDown(KEY_RIGHT) && bear->position.x < WINDOW_WIDTH - bearTexture.width * 0.25f) {
                bear->position.x += PLAYER_SPEED * bear->speedMultiplier;                                   // Move right
            }
        } else {
            bear->freezeTimer -= deltaTime;                                                                 // Decrease freeze timer
        }
    }

    // Continuous food spawning
    if (spawnTimer >= spawnInterval) {
        for (int i = 0; i < MAX_FOOD; i++) {                                // Look for an inactive food item in the foodArray
            if (!foodArray[i].active) {
                int newX = rand() % (WINDOW_WIDTH - SQUARE_SIZE);           // Generate a random X position within the window
                foodArray[i].position = (Vector2){newX, -SQUARE_SIZE};      // Set the food's position just above the visible screen
                foodArray[i].active = true;                                 // Activate the food item
                break;                                                      // Spawn only one food item per interval
            }
        }
        spawnTimer = 0.0f;                                                  // Reset the spawn timer after spawning
    }
    
    // Move each active food downward and reset if it reaches the bottom
    for (int i = 0; i < MAX_FOOD; i++) {
        if (foodArray[i].active) {
            foodArray[i].position.y += fallSpeed;                           // Move food down based on the fall speed
            if (foodArray[i].position.y >= WINDOW_HEIGHT) {                 // If the food moves below the screen, deactivate it
                foodArray[i].active = false;
            }
        }
    }

    bombSpawnTimer += deltaTime;                                           // Bomb spawning logic

    if (!bomb->active && bombSpawnTimer >= 5.0f) {
        bomb->position = (Vector2){rand() % (WINDOW_WIDTH - SQUARE_SIZE), -SQUARE_SIZE};    // Spawn a bomb at a random X position above the screen
        bomb->active = true;                                                                // Activate the bomb
        bombSpawnTimer = 0.0f;                                                              // Reset the bomb spawn timer
    }

    // Move the bomb downward and reset if it reaches the bottom
    if (bomb->active) {
        bomb->position.y += fallSpeed * 1.5f;                             // Bomb falls slightly faster than food
        if (bomb->position.y >= WINDOW_HEIGHT) {                          // If the bomb reaches the bottom, deactivate it
            bomb->active = false;
        }
    }

    // Handle bomb collisions with characters
    if (bomb->active) {
        // Koala collision with bomb
        if (CheckCollisionRecs(                                             // Check collision between bomb and koala
            (Rectangle){koala->position.x, koala->position.y, 40, 40},
            (Rectangle){bomb->position.x, bomb->position.y, 20, 20})) {
            
            PlaySound(bombAudio);                       // Play bomb sound
            bomb->active = false;                       // Deactivate the bomb
            koala->freezeTimer = 3.5f;                  // Freeze for 3.5 seconds
            printf("Koala frozen by bomb!\n");          // Debug message
        }

        // Check collision between bomb and bear (only in two-player mode)
        if (playerMode == 2 && bear->texture.id != 0) {
            if (CheckCollisionRecs(
                (Rectangle){bear->position.x, bear->position.y, 40, 40},
                (Rectangle){bomb->position.x, bomb->position.y, 20, 20})) {
                
                PlaySound(bombAudio);                   // Play bomb sound
                bomb->active = false;                   // Deactivate the bomb
                bear->freezeTimer = 3.5f;               // Freeze for 3.5 seconds
                printf("Bear frozen by bomb!\n");       // Debug message
            }
        }
    }

    // Spawn falling stars in stages 20-30
    if (stage >= 20 && stage <= 30) {
        static float starSpawnTimer = 0;
        starSpawnTimer += deltaTime;
        
        // Spawn a new set of falling stars every 1.5 seconds
        if (starSpawnTimer >= 1.5f) {
            SpawnFallingStars(starArray, 3);            // Activate 3 falling stars
            starSpawnTimer = 0;                         // Reset the spawn timer
        }
    }

    // Handle falling stars
    for (int i = 0; i < 3; i++) {
        if (starArray[i].active) {
            if (!starArray[i].isFire) {
                starArray[i].position.y += starArray[i].velocity * deltaTime * 60.0f;   // Update falling star position 

                // Check if the star reaches the platform and turns into fire
                if (starArray[i].position.y >= WINDOW_HEIGHT - 160) {
                    starArray[i].isFire = true;                                         // Star becomes fire
                    starArray[i].fireTimer = 5.0f;                                      // Fire lasts 5 seconds
                    starArray[i].position.y = WINDOW_HEIGHT - 160;                      // Fix star position
                }

                 // Define collision boxes for the star and characters 
                Rectangle starRect = {
                    starArray[i].position.x,
                    starArray[i].position.y,
                    starArray[i].texture.width * 0.2f,      // Star width (0.2 = 20% of texture)
                    starArray[i].texture.height * 0.2f      // Star height (0.2 = 20% of texture)
                };
                
                // Character collision boxes
                Rectangle koalaRect = {
                    koala->position.x,
                    koala->position.y,
                    SQUARE_SIZE,
                    SQUARE_SIZE
                    };

                Rectangle bearRect = {
                    bear->position.x, 
                    bear->position.y, 
                    SQUARE_SIZE, 
                    SQUARE_SIZE
                    };

                // Flinging Mechanism
                if (CheckCollisionRecs(koalaRect, starRect) && !koala->isFlinging) {
                    PlaySound(flingSound);                                               // Play fling sound
                    koala->isFlinging = true;                                           // Set koala to flinging state
                    koala->flingTimer = 1.0f;                                           // Fling lasts 1 second
                    koala->flingVelocity = (Vector2){                                  
                        (koala->position.x < starArray[i].position.x) ? -300 : 300,             
                        0
                    };
                    DamageCharacter(koala, 3.0f);                                       // Star hit = 3 hearts damage
                    starArray[i].active = false;                                        // Deactivate the star
                }

                if (bear->texture.id != 0 && CheckCollisionRecs(bearRect, starRect) && !bear->isFlinging) {
                    PlaySound(flingSound);                                              // Play fling sound
                    bear->isFlinging = true;                                            // Set bear to flinging state
                    bear->flingTimer = 1.0f;                                            // Fling lasts 1 second
                    bear->flingVelocity = (Vector2){
                        (bear->position.x < starArray[i].position.x) ? -300 : 300,
                        -500
                    };
                    DamageCharacter(bear, 3.0f);                                        // Star hit = 3 hearts damage
                    starArray[i].active = false;                                        // Deactivate the star
                }
            } else {
                // Update fire effect (when star turns into fire)
                starArray[i].fireTimer -= deltaTime;
                if (starArray[i].fireTimer <= 0) {
                    starArray[i].active = false;                                        // Deactivate fire after timer ends
                } else {
                    // Define fire collision box
                    Rectangle fireRect = {starArray[i].position.x,
                                          starArray[i].position.y, 
                                          fireTexture.width * 0.2f,
                                          fireTexture.height * 0.2f};
                    
                    // Check if koala touches fire
                    if (CheckCollisionRecs((Rectangle){koala->position.x, koala->position.y, SQUARE_SIZE, SQUARE_SIZE}, fireRect) 
                        && !koala->isBurning) {
                        PlaySound(burningSound);                                        // Play burning sound
                        koala->isBurning = true;                                        // Koala is burning
                        koala->burnTimer = 3.0f;                                        // Burn lasts 3 seconds
                        koala->speedMultiplier = 0.3f;                                  // Reduce Koala speed
                    }

                    if (bear->texture.id != 0 && 
                        CheckCollisionRecs((Rectangle){bear->position.x, bear->position.y, SQUARE_SIZE, SQUARE_SIZE}, fireRect) 
                        && !bear->isBurning) {
                        PlaySound(burningSound);                                        // Play burning sound
                        bear->isBurning = true;                                         // Bear is burning
                        bear->burnTimer = 3.0f;                                         // Burn lasts 3 seconds
                        bear->speedMultiplier = 0.3f;                                   // Reduce speed
                    }
                }
            }
        }
    }

    // Update flinging effect for Koala
    if (koala->isFlinging) {
        koala->position.x += koala->flingVelocity.x * deltaTime;        // Update Koala's position based on its fling velocity and deltaTime
        koala->position.y += koala->flingVelocity.y * deltaTime;
        koala->flingTimer -= deltaTime;                                 // Decrease the fling timer

        if (koala->flingTimer <= 0) {                                   // End fling effect when timer runs out
            koala->isFlinging = false;
            koala->position.y = WINDOW_HEIGHT - 160;                    // Reset Y position (Ground Level)
            koala->justLanded = true;                                   // Set landing flag
            koala->portalTimer = 2.5f;                                  // Activate portal effect for 2/5 seconds
        }

        // Keep Koala within screen bounds
        koala->position.x = MyClamp(koala->position.x, 0, WINDOW_WIDTH - koalaTexture.width * 0.25f);
    } else {
        koala->justLanded = false;                                      // Reset landing flag if not flinging
    }

    // Update portal timer
    if (koala->portalTimer > 0) {
        koala->portalTimer -= deltaTime;
    }

    // Update flinging effect for Bear
    if (bear->isFlinging) {
        bear->position.x += bear->flingVelocity.x * deltaTime;
        bear->position.y += bear->flingVelocity.y * deltaTime;
        bear->flingTimer -= deltaTime;

        if (bear->flingTimer <= 0) {
            bear->isFlinging = false;                                   // End the flinging effect
            bear->position.y = WINDOW_HEIGHT - 160;                     // Reset Y position to platform height
            bear->justLanded = true;                                    // Set landing flag
            bear->portalTimer = 2.5f;                                   // Activate portal effect for 2/5 seconds
        }

        // Keep character within screen bounds
        bear->position.x = MyClamp(bear->position.x, 0, WINDOW_WIDTH - bearTexture.width * 0.25f);
        } else {
            bear->justLanded = false;                                   // Reset landing flag if not flinging
        }

    // Update portal timer
    if (bear->portalTimer > 0) {
        bear->portalTimer -= deltaTime;
    }

    // Handle burn effect for stars that have turned into fire
    for (int i = 0; i < 3; i++) {
        if (starArray[i].isFire) {
            starArray[i].fireTimer -= deltaTime;                        // Decrease the fire's timer
            if (starArray[i].fireTimer <= 0) {
                starArray[i].isFire = false;                            // Deactivate fire when timer runs out
            }
        }
    }

    // Food collisions with larger collision boxes
    for (int i = 0; i < MAX_FOOD; i++) {
        if (foodArray[i].active) {
            // Define a slightly larger collision box for food
            Rectangle foodRect = {
                foodArray[i].position.x - 10,      // Wider left
                foodArray[i].position.y - 10,      // Higher top
                SQUARE_SIZE + 20,                  // Wider right
                SQUARE_SIZE + 20                   // Lower bottom
            };
            
            // Define slightly larger collision boxes for Koala and Bear
            Rectangle koalaRect = {
                koala->position.x - 5,
                koala->position.y - 5,
                SQUARE_SIZE + 10,
                SQUARE_SIZE + 10
            };
            
            Rectangle bearRect = {
                bear->position.x - 5,
                bear->position.y - 5,
                SQUARE_SIZE + 10,
                SQUARE_SIZE + 10
            };
            
            // Check collision between Koala and food
            if (CheckCollisionRecs(koalaRect, foodRect)) {
                if (foodArray[i].type == FOOD_TYPE_BAD) {
                    PlaySound(badFoodAudio);
                    DamageCharacter(koala, 0.5f);       // Bad food = 0.5 heart damage
                } else {
                    PlaySound(goodFoodAudio);
                    DamageCharacter(koala, -0.5f);      // Good food = recover 0.5 heart
                }
                foodArray[i].active = false;            // Deactivate the food after collision
            }

            // Bear collision with food
            if (playerMode == 2 && CheckCollisionRecs(bearRect, foodRect)) {
                if (foodArray[i].type == FOOD_TYPE_BAD) {
                    PlaySound(badFoodAudio);
                    DamageCharacter(bear, 0.5f);        // Bad food = 0.5 heart damage
                } else {
                    PlaySound(goodFoodAudio);
                    DamageCharacter(bear, -0.5f);       // Good food = recover 0.5 heart
                }
                foodArray[i].active = false;            // Deactivate the food after collision
            }
        }
    }

    // Bomb collisions with larger collision boxes
    if (bomb->active) {
        Rectangle bombRect = {                          // Define larger collision boxes for the bomb and characters
            bomb->position.x - 10,
            bomb->position.y - 10,
            SQUARE_SIZE + 20,
            SQUARE_SIZE + 20
        };
        
        Rectangle koalaRect = {
            koala->position.x - 5,
            koala->position.y - 5,
            SQUARE_SIZE + 10,
            SQUARE_SIZE + 10
        };
        
        Rectangle bearRect = {
            bear->position.x - 5,
            bear->position.y - 5,
            SQUARE_SIZE + 10,
            SQUARE_SIZE + 10
        };
        
        if (CheckCollisionRecs(koalaRect, bombRect)) {  // Check collision between bomb and Koala
            PlaySound(bombAudio);
            DamageCharacter(koala, 1.0f);               // Bomb causes 1 heart damage
            bomb->active = false;                       // Deactivate the bomb
        }

        if (playerMode == 2 && CheckCollisionRecs(bearRect, bombRect)) {        // Check collision between bomb and Bear
            PlaySound(bombAudio);       
            DamageCharacter(bear, 1.0f);                                        // Bomb causes 1 heart damage
            bomb->active = false;                                               // Deactivate the bomb
        }
    }

    // Falling star collisions with larger collision boxes
    for (int i = 0; i < 3; i++) {
        if (starArray[i].active) {
            if (!starArray[i].isFire) {
                // Star collision box
                Rectangle starRect = {
                    starArray[i].position.x, 
                    starArray[i].position.y, 
                    fallingStarTexture.width * 0.2f, 
                    fallingStarTexture.height * 0.2f
                    };
                
                // Check collision with Koala
                if (CheckCollisionRecs((Rectangle){koala->position.x, koala->position.y, SQUARE_SIZE, SQUARE_SIZE}, starRect) && !koala->isFlinging) {
                    PlaySound(flingSound);
                    koala->isFlinging = true;                                           // Activate flinging effect
                    koala->flingTimer = 3.0f;                                           // Fling lasts 3 seconds
                    koala->flingVelocity = (Vector2){
                        (koala->position.x < starArray[i].position.x) ? -300 : 300,
                        -200
                    };
                    DamageCharacter(koala, 3.0f);                                       // Star hit = 3 hearts damage
                    starArray[i].active = false;                                        // Deactivate star
                }

                // Check collision with Bear
                if (playerMode == 2 && CheckCollisionRecs((Rectangle){bear->position.x, bear->position.y, SQUARE_SIZE, SQUARE_SIZE}, starRect) && !bear->isFlinging) {
                    PlaySound(flingSound);
                    bear->isFlinging = true;                                            // Activate flinging effect
                    bear->flingTimer = 3.0f;                                            // Fling lasts 3 seconds
                    bear->flingVelocity = (Vector2){
                        (bear->position.x < starArray[i].position.x) ? -300 : 300,
                        -200
                    };
                    DamageCharacter(bear, 3.0f);                                        // Star hit = 3 hearts damage
                    starArray[i].active = false;                                        // Deactivate star
                }
            }
        }
    }

    // Update burning effect damage
    if (koala->isBurning) {
        koala->burnTimer -= deltaTime;
        koala->burnDamageTimer -= deltaTime;

        // Inflict damage every 0.5 seconds
        if (koala->burnDamageTimer <= 0.0f) {
            DamageCharacter(koala, 0.5f);       // Burn causes 0.5 heart damage
            koala->burnDamageTimer = 0.5f;      // Reset burn damage timer
        }

        // Stop burn effect after timer ends
        if (koala->burnTimer <= 0.0f) {
            koala->isBurning = false;
            koala->speedMultiplier = 1.0f;      // Reset speed
        }
    }

    // Update burning effect for Bear
    if (bear->isBurning) {
        bear->burnTimer -= deltaTime;
        bear->burnDamageTimer -= deltaTime;

        // Inflict damage every 0.5 seconds
        if (bear->burnDamageTimer <= 0.0f) {
            DamageCharacter(bear, 0.5f);        // Inflict half a heart of damage
            bear->burnDamageTimer = 0.5f;       // Reset damage timer
        }

        // Stop burn effect after timer ends
        if (bear->burnTimer <= 0.0f) {
            bear->isBurning = false;
            bear->speedMultiplier = 1.0f;       // Reset speed
        }
    }

    // Maximum number of hearts
    int maxHearts = 5;

    // Check if Koala or Bear has depleted all hearts
    if (koala->score >= maxHearts || bear->score >= maxHearts) {
        gameState = GAME_STATE_GAME_OVER;       // Transition to a game over state
        return;
    }

    // Update character effects (e.g., flinging, burning)
    UpdateCharacterEffects(koala, deltaTime);
    if (bear->texture.id != 0) {
        UpdateCharacterEffects(bear, deltaTime);
    }

    // Ensure background music continues playing
    UpdateMusicStream(backgroundMusic);

    // Handle pause menu
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        printf("Pause key pressed\n");          // Debug output
        gameState = GAME_STATE_PAUSE;           // Transition to pause state
        return;                                 // Exit UpdateGame when pausing
    }
}

// Function to display the main menu and handle navigation
void ShowMainMenu(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray, int playerMode) {
    // Reset game states when entering main menu
    startGame = false;                      // Ensure the game doesn't start yet
    gameState = GAME_STATE_MENU;            // Set the game state to menu

    // Play background music for the main menu, but only if it's not already playing
    if (!IsMusicStreamPlaying(startingScreenMusic)) {
        PlayMusicStream(startingScreenMusic);                // Start playing the main menu music
        SetMusicVolume(startingScreenMusic, 1.0f);           // Set music volume to maximum
    }

    backgroundTexture = LoadTexture("assets/sky.png");       // Load the background texture for the main menu

    // Define buttons for main menu with consistent positioning
    Button startButton = {
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 100, 200, 50},
        "Start",                                                        // Button text
        (Color){ 193, 207, 161, 255 }                                   // PALE_GREEN
    };

    Button tutorialButton = {
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 30, 200, 50},
        "Tutorial",                                                     // Button text
        (Color){ 231, 204, 204, 255 }                                   // LIGHT_PINK
    };

    Button exitButton = {
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 40, 200, 50},
        "Exit",                                                         // Button text
        (Color){ 237, 232, 220, 255 }                                   // SOFT_BEIGE
    };

    bool exitMenu = false;                                              // Flag to determine if the menu should close
    while (!exitMenu && !WindowShouldClose()) {                         // Loop to display the main menu until the user exits
        UpdateMusicStream(startingScreenMusic);                         // Keep music playing smoothly
        
        BeginDrawing();                                                 // Begin drawing the main menu screen
        ClearBackground(RAYWHITE);                                      // Clear the screen with a white background

        // Draw the background image
        if (backgroundTexture.id != 0) {
            DrawTexture(backgroundTexture, 0, 0, WHITE);                
        }

        // Draw title
        DrawText("Welcome to Falling Fluffs!", 
                 GetScreenWidth() / 2 - MeasureText("Welcome to Falling Fluffs!", 40) / 2, 
                 GetScreenHeight() / 2 - 200, 40, BLACK);

        // Draw the Start button
        DrawRectangleRec(startButton.bounds, startButton.color);
        DrawText(startButton.text, 
                startButton.bounds.x + startButton.bounds.width / 2 - MeasureText(startButton.text, 30) / 2, 
                startButton.bounds.y + 10, 30, BLACK);

        // Draw the Tutorial button
        DrawRectangleRec(tutorialButton.bounds, tutorialButton.color);
        DrawText(tutorialButton.text, 
                tutorialButton.bounds.x + tutorialButton.bounds.width / 2 - MeasureText(tutorialButton.text, 30) / 2, 
                tutorialButton.bounds.y + 10, 30, BLACK);

        // Draw the Exit button
        DrawRectangleRec(exitButton.bounds, exitButton.color);
        DrawText(exitButton.text, 
                exitButton.bounds.x + exitButton.bounds.width / 2 - MeasureText(exitButton.text, 30) / 2, 
                exitButton.bounds.y + 10, 30, BLACK);

        // Handle button clicks
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePosition = GetMousePosition();                                 // Get the mouse position

            if (CheckCollisionPointRec(mousePosition, startButton.bounds)) {            // Check if the Start button was clicked
                printf("Start button clicked!\n");
                EndDrawing();
                gameState = GAME_STATE_OPTIONS;                                         // Transition to options menu
                ShowStartMenu(koala, bear, foodArray, bomb, starArray);                 // Start game menu
                return;
            }
            if (CheckCollisionPointRec(mousePosition, tutorialButton.bounds)) {         // Check if the Tutorial button was clicked
                ShowTutorial();                                                         // Show tutorial screen
            }
            if (CheckCollisionPointRec(mousePosition, exitButton.bounds)) {             // Check if the Exit button was clicked
                printf("Exiting game...\n");
                SaveGameState(koala, bear, stage);                                      // Save the game state
                CleanupAndExit();                                                       // Perform cleanup tasks
                EndDrawing();
                exit(0);                                                                // Exit the application
            }
        }

        EndDrawing();                                                                   // End drawing for this frame
    }

    if (WindowShouldClose()) {                                                          // If the window is closed, perform cleanup and save state
        SaveGameState(koala, bear, stage);                                              // Save the game state
        CleanupAndExit();                                                               // Call Cleanup Function
        exit(0);                                                                        // Exit the application
    }
}

// Function to display the start menu and handle game initialization
void ShowStartMenu(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray) {

    UpdateMusicStream(startingScreenMusic);          // Continue playing the starting screen music

    Button newGameButton = {                                            // Define the "New Game" button
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 100, 200, 50},
        "New Game",                                                     // Text displayed on the button
        (Color){ 193, 207, 161, 255 }                                   // Pale green color
    };

    Button loadGameButton = {                                           // Define the "Load Game" button
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 30, 200, 50},
        "Load Game",                                                    // Text displayed on the button
        (Color){ 231, 204, 204, 255 }                                   // Light pink color
    };

    Button backButton = {                                               // Define the "Back" button
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 40, 200, 50},      // Text displayed on the button
        "Back",
        (Color){ 237, 232, 220, 255 }                                   // Soft beige color
    };

    bool exitMenu = false;                                              // Flag to indicate whether to exit the menu
    while (!exitMenu && !WindowShouldClose()) {                         // Main loop to display the start menu
        UpdateMusicStream(startingScreenMusic);                         // Keep the background music playing
        BeginDrawing();
        ClearBackground(RAYWHITE);                                      // Clear the screen with a white background
        
        // Draw the load screen background
        if (loadBackground.id != 0) {
            DrawTexture(loadBackground, 0, 0, WHITE);
        }

        // Draw buttons with their respective colors and positions
        DrawRectangleRec(newGameButton.bounds, newGameButton.color);
        DrawText(newGameButton.text, 
                newGameButton.bounds.x + newGameButton.bounds.width / 2 - MeasureText(newGameButton.text, 30) / 2, 
                newGameButton.bounds.y + 10, 30, BLACK);

        DrawRectangleRec(loadGameButton.bounds, loadGameButton.color);
        DrawText(loadGameButton.text, 
                loadGameButton.bounds.x + loadGameButton.bounds.width / 2 - MeasureText(loadGameButton.text, 30) / 2, 
                loadGameButton.bounds.y + 10, 30, BLACK);

        DrawRectangleRec(backButton.bounds, backButton.color);
        DrawText(backButton.text, 
                backButton.bounds.x + backButton.bounds.width / 2 - MeasureText(backButton.text, 30) / 2, 
                backButton.bounds.y + 10, 30, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {                                  // Handle mouse input for buttons
            Vector2 mousePosition = GetMousePosition();

            if (CheckCollisionPointRec(mousePosition, newGameButton.bounds)) {
                printf("New Game button clicked!\n");
                EndDrawing();                                                           // End current drawing before transition
                ShowPlayerModeSelection(koala, bear, foodArray, bomb, starArray);
                return;                                                                 // Exit the menu
            }
            if (CheckCollisionPointRec(mousePosition, loadGameButton.bounds)) {
                printf("Loading saved game...\n");
                LoadGameState(koala, bear, &stage);                                     // Load the saved state
                gameState = GAME_STATE_PLAYING;                                         // Set game state to playing
                startGame = true;                                                       // Start the game
                exitMenu = true;                                                        // Exit the menu loop
                EndDrawing();
                return;                                                                 // Exit the menu
            }
            if (CheckCollisionPointRec(mousePosition, backButton.bounds)) {
                gameState = GAME_STATE_MENU;                                            // Return to the main menu
                exitMenu = true;                                                        // Exit the menu loop
            }
        }

        EndDrawing();                                                                   // End the current frame
    }
}

// Function to display the player mode selection screen
void ShowPlayerModeSelection(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray) {
    
    // Continue playing the starting screen music
    UpdateMusicStream(startingScreenMusic);

    Button onePlayerButton = {                                              // Define the "One Player" button
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 100, 200, 50},
        "One Player",                                                       // Text displayed on the button
        (Color){ 193, 207, 161, 255 }                                       // Pale Green - Hex C1CFA1
    };

    Button twoPlayerButton = {
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 30, 200, 50},
        "Two Players",                                                      // Text displayed on the button
        (Color){ 231, 204, 204, 255 }                                       // Light pink - Hex E7CCCC
    };

    Button backButton = {
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 40, 200, 50},
        "Back",                                                             // Text displayed on the button
        (Color){ 237, 232, 220, 255 }                                       // Soft Beige - Hex EDE8DC
    };

    bool exitMenu = false;                                                  // Flag to indicate whether to exit the menu
    while (!exitMenu && !WindowShouldClose()) {                             // Main loop for the player mode selection menu
        UpdateMusicStream(startingScreenMusic);                             // Keep the background music playing
        BeginDrawing();
        ClearBackground(RAYWHITE);                                          // Clear the screen with a white background
        
        if (optionsBackground.id != 0) {                                    // Draw the options screen background
            DrawTexture(optionsBackground, 0, 0, WHITE);
        }

        // Draw buttons
        DrawRectangleRec(onePlayerButton.bounds, onePlayerButton.color);
        DrawText(onePlayerButton.text, 
                onePlayerButton.bounds.x + onePlayerButton.bounds.width / 2 - MeasureText(onePlayerButton.text, 30) / 2, 
                onePlayerButton.bounds.y + 10, 30, BLACK);

        DrawRectangleRec(twoPlayerButton.bounds, twoPlayerButton.color);
        DrawText(twoPlayerButton.text, 
                twoPlayerButton.bounds.x + twoPlayerButton.bounds.width / 2 - MeasureText(twoPlayerButton.text, 30) / 2, 
                twoPlayerButton.bounds.y + 10, 30, BLACK);

        DrawRectangleRec(backButton.bounds, backButton.color);
        DrawText(backButton.text, 
                backButton.bounds.x + backButton.bounds.width / 2 - MeasureText(backButton.text, 30) / 2, 
                backButton.bounds.y + 10, 30, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {                                   // Handle mouse input for buttons
            Vector2 mousePosition = GetMousePosition();

            if (CheckCollisionPointRec(mousePosition, onePlayerButton.bounds)) {
                playerMode = 1;                                                         // Set player mode to single-player
                InitGame(koala, bear, foodArray, bomb, starArray, playerMode);          // Initialize the game
                gameState = GAME_STATE_PLAYING;                                         // Set game state to playing
                startGame = true;                                                       // Start the game
                exitMenu = true;                                                        // Exit the menu loop
            }
            if (CheckCollisionPointRec(mousePosition, twoPlayerButton.bounds)) {
                playerMode = 2;                                                         // Set player mode to two-player
                InitGame(koala, bear, foodArray, bomb, starArray, playerMode);          // Initialize the game
                gameState = GAME_STATE_PLAYING;                                         // Set game state to playing
                startGame = true;                                                       // Start the game
                exitMenu = true;                                                        // Exit the menu loop
            }
            if (CheckCollisionPointRec(mousePosition, backButton.bounds)) {
                gameState = GAME_STATE_MENU;                                             // Return to the main menu
                exitMenu = true;                                                         // Exit the menu loop
            }
        }

        EndDrawing();                                                                    // End the current frame
    }
}

// Function to display the tutorial screens
void ShowTutorial(void) {
    static int currentPage = 0;                             // Track the current tutorial page
    const int totalPages = 3;                               // Total number of tutorial pages
    Texture2D tutorialPages[3];                             // Array of textures for tutorial pages
    
    // Load tutorial page textures
    tutorialPages[0] = LoadTexture("assets/tutorial1.png");  // Good food page
    tutorialPages[1] = LoadTexture("assets/tutorial2.png");  // Bad food page
    tutorialPages[2] = LoadTexture("assets/tutorial3.png");  // Special effects page

    // Buttons
    Button nextButton = {
        {WINDOW_WIDTH - 130, WINDOW_HEIGHT / 2 - 120, 115, 35},  
        "Next",
        (Color){ 193, 207, 161, 255 }
    };

    Button prevButton = {
        {WINDOW_WIDTH - 130, WINDOW_HEIGHT / 2 - 20, 115, 35},   
        "Previous",
        (Color){ 231, 204, 204, 255 }
    };

    Button backButton = {
        {WINDOW_WIDTH - 130, WINDOW_HEIGHT / 2 + 80, 115, 35},   
        "Back",
        (Color){ 237, 232, 220, 255 }
    };

    while (!WindowShouldClose()) {
        UpdateMusicStream(startingScreenMusic);             // Keep background music playing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw current tutorial page
        DrawTexture(tutorialPages[currentPage], 0, 0, WHITE);

        // Draw navigation buttons
        if (currentPage > 0) {
            DrawRectangleRec(prevButton.bounds, prevButton.color);
            DrawText(prevButton.text, 
                    prevButton.bounds.x + prevButton.bounds.width / 2 - MeasureText(prevButton.text, 20) / 2,  // Previous Button (Go to previous tutorial page)
                    prevButton.bounds.y + 10, 20, BLACK);
        }

        if (currentPage < totalPages - 1) {
            DrawRectangleRec(nextButton.bounds, nextButton.color);
            DrawText(nextButton.text, 
                    nextButton.bounds.x + nextButton.bounds.width / 2 - MeasureText(nextButton.text, 20) / 2,   // Next Button (Go to Next page)
                    nextButton.bounds.y + 10, 20, BLACK);
        }

        DrawRectangleRec(backButton.bounds, backButton.color);
        DrawText(backButton.text, 
                backButton.bounds.x + backButton.bounds.width / 2 - MeasureText(backButton.text, 20) / 2,       // Back Button (Go to main menu)
                backButton.bounds.y + 10, 20, BLACK);

        // Handle button clicks
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePosition = GetMousePosition();

            if (currentPage > 0 && CheckCollisionPointRec(mousePosition, prevButton.bounds)) {                  // Go to the previous tutorial page
                currentPage--;
            }
            if (currentPage < totalPages - 1 && CheckCollisionPointRec(mousePosition, nextButton.bounds)) {
                currentPage++;
            }
            if (CheckCollisionPointRec(mousePosition, backButton.bounds)) {
                for (int i = 0; i < totalPages; i++) {
                    UnloadTexture(tutorialPages[i]);                                                            // Unload tutorial textures before returning
                }
                return;                                                                                         // Exit the tutorial
            }
        }

        EndDrawing();                                                                                           // End the current frame
    }

    for (int i = 0; i < totalPages; i++) {
        UnloadTexture(tutorialPages[i]);                                                                        // Unload tutorial textures
    }
}

// Function to display the pause menu and handle pause-related interactions
void ShowPauseMenu(Characters *koala, Characters *bear) {
    bool exitPauseMenu = false;                                         // Flag to indicate if the pause menu should close
    
    while (!exitPauseMenu && gameState == GAME_STATE_PAUSE) {           // Pause Menu Loop
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        if (pauseBackground.id != 0) {
            DrawTexture(pauseBackground, 0, 0, WHITE);                  // Draw the pause background
        }
        
        const char* pausedText = "PAUSED";                              // Draw "PAUSED" text
        int textWidth = MeasureText(pausedText, 60);                
        DrawText(pausedText, (WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 3, 60, BLACK);

        DrawText("Volume Control:", (WINDOW_WIDTH - MeasureText("Volume Control:", 30)) / 2, WINDOW_HEIGHT / 2 - 40, 30, BLACK);        // Draw volume controls
        
        Rectangle sliderBounds = {WINDOW_WIDTH/4, WINDOW_HEIGHT/2, WINDOW_WIDTH/2, 20};                                                 // Volume slider background
        DrawRectangleRec(sliderBounds, LIGHTGRAY);
        
        DrawRectangle(sliderBounds.x, sliderBounds.y, 
                     (int)(sliderBounds.width * musicVolume), sliderBounds.height,                                                      // Volume slider fill
                     isMuted ? GRAY : DARKGREEN);

        Rectangle muteBtn = {WINDOW_WIDTH/4 - 60, WINDOW_HEIGHT/2 - 5, 50, 30};
        DrawRectangleRec(muteBtn, isMuted ? RED : DARKGRAY);                                                                            // Mute button
        DrawText(isMuted ? "MUTED" : "MUTE", muteBtn.x + 5, muteBtn.y + 5, 15, WHITE);


        char volumeText[32];
        sprintf(volumeText, "%d%%", (int)(musicVolume * 100));                                                                          // Volume percentage text
        DrawText(volumeText, WINDOW_WIDTH*3/4 + 10, WINDOW_HEIGHT/2, 20, BLACK);
        
        DrawText("Press P to resume", (WINDOW_WIDTH - MeasureText("Press P to resume", 30)) / 2, WINDOW_HEIGHT/2 + 80, 30, DARKGRAY);   // Draw menu options
        DrawText("Press X to save and return to main menu", 
                (WINDOW_WIDTH - MeasureText("Press X to save and return to main menu", 30)) / 2, 
                WINDOW_HEIGHT/2 + 120, 30, DARKGRAY);

        EndDrawing();
        
        Vector2 mousePos = GetMousePosition();                                                                                          // Handle volume control input
        
        if (CheckCollisionPointRec(mousePos, sliderBounds) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {                                   // Handle slider interaction
            musicVolume = (mousePos.x - sliderBounds.x) / sliderBounds.width;
            musicVolume = MyClamp(musicVolume, 0.0f, 1.0f);
            if (!isMuted) {
                SetMusicVolume(backgroundMusic, musicVolume);
                SetSoundVolume(burningSound, musicVolume);
                SetSoundVolume(flingSound, musicVolume);
                SetSoundVolume(goodFoodAudio, musicVolume);
                SetSoundVolume(badFoodAudio, musicVolume);
                SetSoundVolume(bombAudio, musicVolume);
                SetSoundVolume(thunderAudio, musicVolume);
            }
        }

        if (CheckCollisionPointRec(mousePos, muteBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {                                     // Handle mute button
            isMuted = !isMuted;
            float newVolume = isMuted ? 0.0f : musicVolume;
            SetMusicVolume(backgroundMusic, newVolume);
            SetSoundVolume(burningSound, newVolume);
            SetSoundVolume(flingSound, newVolume);
            SetSoundVolume(goodFoodAudio, newVolume);
            SetSoundVolume(badFoodAudio, newVolume);
            SetSoundVolume(bombAudio, newVolume);
            SetSoundVolume(thunderAudio, newVolume);
        }

        if (IsKeyPressed(KEY_P)) {                                                                                                      // Handle keyboard input for menu navigation
            printf("Resuming game\n");
            gameState = GAME_STATE_PLAYING;
            exitPauseMenu = true;
            EndDrawing();                                                                                                               // Make sure we end any current drawing
            return;                                                                                                                     // Exit immediately
        }
        
        if (IsKeyPressed(KEY_X)) {
            printf("Saving game and returning to main menu\n");
            SaveGameState(koala, bear, stage);
            gameState = GAME_STATE_MENU;
            startGame = false;                                                                                                          // Reset startGame flag
            exitPauseMenu = true;
            EndDrawing();                                                                                                               // Make sure we end any current drawing
            return;                                                                                                                     // Exit immediately
        }
    }
}

// Function to handle character burning and flinging effects
void UpdateCharacterEffects(Characters *character, float deltaTime) {
    
    if (character->isFlinging) {                                                // Handle flinging effect
        character->position.x += character->flingVelocity.x * deltaTime;        // Update character position based on fling velocity
        character->position.y += character->flingVelocity.y * deltaTime;
        
        character->flingVelocity.y += 9.8f * deltaTime;                         // Apply gravity effects

        if (character->position.y >= WINDOW_HEIGHT - 160) {                     // Check if the character has landed
            character->position.y = WINDOW_HEIGHT - 160;
            character->isFlinging = false;
            character->flingVelocity = (Vector2){0, 0};
            
            DrawPortalEffect(character->position);                              // Draw the portal effect at the character's position
        }
    }

    
    if (character->isBurning) {                                                 // Check if the character is burning
        character->burnTimer -= deltaTime;                                      // Decrease the burn duration timer
        character->burnDamageTimer -= deltaTime;                                // Decrease the damage interval timer

        if (character->burnDamageTimer <= 0.0f) {                               // Apply damage every 0.5 seconds
            character->score += 0.5f;                                           // Inflict half a heart of damage (positive score means damage)
            character->burnDamageTimer = 0.5f;                                  // Reset the damage timer for the next interval
        }

        if (character->burnTimer <= 0.0f) {                                     // Stop the burn effect after the burn timer ends
            character->isBurning = false;                                       // Turn off the burning state
            character->speedMultiplier = 1.0f;                                  // Reset character's speed to normal
        }
    }
}

// Function to apply wind effects to food and bombs
void ApplyWindEffect(Food foodArray[], Bomb *bomb, float deltaTime) {
    static float windDirectionTimer = 0.0f;
    static float windForce = 0.0f;
    static int affectedFoodIndices[MAX_FOOD] = {0};                   // Increased array size to handle higher stages
    
    windTimer += deltaTime;
    windDirectionTimer += deltaTime;
    
    // Determine number of affected items based on stage
    int numAffectedFood = 1;                                          // Affected food start with 1
    if (stage >= 15) {
        numAffectedFood = 2;                                          // Increase to 2 foods after stage 15
    }
    if (stage >= 25) {
        numAffectedFood = 3;                                          // Increase to 3 foods after stage 25
    }
    
    // Change wind direction periodically
    if (windDirectionTimer >= 1.0f) {
        windForce = (float)(rand() % 7 - 3);                          // Generate a random wind force between -3 and 3
        
        // Reset affected items
        for (int i = 0; i < MAX_FOOD; i++) {
            affectedFoodIndices[i] = -1;                              // Clear previously affected items
        }
        
        // Select random active food items
        int affectedCount = 0;                                        // Count of food items currently affected by wind
        while (affectedCount < numAffectedFood) {
            int randomIndex = rand() % MAX_FOOD;                      // Randomly choose a food item
            
            // Ensure the food item is not already selected
            bool alreadySelected = false;
            for (int i = 0; i < affectedCount; i++) {
                if (affectedFoodIndices[i] == randomIndex) {
                    alreadySelected = true;
                    break;
                }
            }
            
            if (!alreadySelected && foodArray[randomIndex].active) {    // If the food item is active and not already selected, mark it as affected
                affectedFoodIndices[affectedCount] = randomIndex;
                affectedCount++;
            }
        }
        
        windDirectionTimer = 0.0f;                                      // Reset the wind direction timer
    }

    // Activate or deactivate the wind effect every 8 seconds
    if (windTimer >= 8.0f) {
        windActive = !windActive;                                       // Toggle the wind's active state
        windTimer = 0.0f;                                               // Reset the wind activation timer
        printf("Wind %s\n", windActive ? "started" : "stopped");        // Print wind state for debugging

        // Trigger thunder effect when wind starts or stops
        if (windActive || !windActive) {                                
            thunderActive = true;                                       // Activate thunder effect                                   
            PlaySound(thunderAudio);                                    // Play thunder sound
            float screenFlashTimer = 0.2f;                              // Set flash duration
        }
    }

    if (windActive) {
        // Apply wind to selected food items
        for (int i = 0; i < numAffectedFood; i++) {
            if (affectedFoodIndices[i] != -1 && foodArray[affectedFoodIndices[i]].active) {
                Food *affectedFood = &foodArray[affectedFoodIndices[i]];                        // Get the affected food item
                float newX = affectedFood->position.x + windForce;                              // Update the food's X position with wind
                affectedFood->position.x = MyClamp(newX, 0, WINDOW_WIDTH - SQUARE_SIZE);        // Ensure the food stays within screen bounds
            }
        }
        
        // Bomb is also affected by wind, but less frequently in early stages
        if (bomb->active && (stage >= 20 || (rand() % 4 == 0))) {                               // Bombs are affected by wind 25% of the time in early stages, but are way more frequently affected after stage 20
            float newX = bomb->position.x + windForce;                                          // Adjust the bomb's horizontal position (x position) based on wind force
            bomb->position.x = MyClamp(newX, 0, WINDOW_WIDTH - SQUARE_SIZE);                    // Ensure the bomb stays within screen bounds
        }
    }
}

// Function to draw wind indicators on the screen
void DrawWindEffects(Food foodArray[], Bomb *bomb) {
    if (windActive) {
        // Scale of the wind texture; increase for larger size (e.g, 0.5f), decrease for smaller size (e.g, 0.2f)
        float windScale = 0.3f;

        // Vertical position of the wind indicators; increase to move down, decrease to move up
        float yPosition = 60.0f;

        // Position of the left wind indicator; adjust X to move left or right
        Vector2 leftWindPos = {
            40.0f,  // Distance from the left edge of the window
            yPosition
        };

        // Position of the right wind indicator; adjust X to move left or right
        Vector2 rightWindPos = {
            WINDOW_WIDTH - 130.0f,  // Distance from the right edge of the window
            yPosition
        };

        // Draw wind indicators based on the wind direction
        if (windForce > 0) {  // Wind blowing to the right
            DrawTextureEx(windLeftTexture, leftWindPos, 0.0f, windScale, WHITE);    // Draw left indicator
            DrawTextureEx(windRightTexture, rightWindPos, 0.0f, windScale, WHITE);  // Draw right indicator
        } else {  // Wind blowing to the left
            DrawTextureEx(windLeftTexture, leftWindPos, 0.0f, windScale, WHITE);    // Draw left indicator
            DrawTextureEx(windRightTexture, rightWindPos, 0.0f, windScale, WHITE);  // Draw right indicator
        }
    }
}

// Function to simulate screen flash during thunder
void ApplyThunderEffect(float *screenFlashTimer, float deltaTime) {
    thunderTimer += deltaTime;          
    if (thunderTimer > 10.0f) {          
        thunderActive = true;                   // Activate thunder effect
        PlaySound(thunderAudio);                // Play thunder sound
        *screenFlashTimer = 0.2f;               // Flash for 0.2 seconds
        thunderTimer = 0.0f;                    // Reset the thunder timer
    }

    // If thunder is active and the screen flash timer is running
    if (thunderActive && *screenFlashTimer > 0.0f) { 
        *screenFlashTimer -= deltaTime;         // Reduce the screen flash timer

        // Simulate the thunder flash by drawing a white rectangle over the entire screen
        BeginDrawing();
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, WHITE);
        EndDrawing();

        // Deactivate thunder effect when the flash timer ends
        if (*screenFlashTimer <= 0.0f) {
            thunderActive = false;
        }
    }
}

// Function to spawn falling stars on the screen
void SpawnFallingStars(FallingStar *starArray, int maxStars) {
    for (int i = 0; i < maxStars; i++) {                    // Loop through the star array to find an inactive star
        if (!starArray[i].active) {                         // Check if the star is inactive

            // Set the position of the new star randomly across the screen width
            // Start slightly above the visible screen area (-SQUARE_SIZE)  

            starArray[i].position = (Vector2){
                rand() % (WINDOW_WIDTH - SQUARE_SIZE),      // Random X position within screen width
                -SQUARE_SIZE                                // Y position just above the screen
            };
            // Activate the star and reset its properties

            starArray[i].active = true;                     // Mark the star as active
            starArray[i].isFire = false;                    // Ensure it's not in the fire state
            starArray[i].fireTimer = 0;                     // Reset the fire timer
            starArray[i].velocity = 3.0f;                   // Set a consistent downward velocity

            break;  // Exit the loop after spawning one star
        }
    }
}

// Function to display a transition screen between stages
void ShowStageTransition(int stageNumber) {
    int countdown = 3;                                  // Set the countdown timer to 3 seconds
    
    // Loop for the countdown until it reaches 0 or the window is closed
    while (countdown > 0 && !WindowShouldClose()) {
        BeginDrawing();                                 // Begin drawing the transition screen
        DrawTexture(transitionTexture, 0, 0, WHITE);    // Draw the transition background
        
        // Display the stage number, centered horizontally on the screen
        int stageTextWidth = MeasureText(TextFormat("Stage %d", stageNumber), 50);
        DrawText(TextFormat("Stage %d", stageNumber), 
            (WINDOW_WIDTH - stageTextWidth) / 2,        // Center horizontally
            300,                                        // Vertical position
            50,                                         // Font size
            BEIGE);                                     // Text color
        
        // Display the countdown timer, centered horizontally below the stage text
        int countdownTextWidth = MeasureText(TextFormat("Starting in: %d", countdown), 40);
        DrawText(TextFormat("Starting in: %d", countdown), 
            (WINDOW_WIDTH - countdownTextWidth) / 2,    // Center horizontally
            400,                                        // Vertical position
            40,                                         // Font size
            WHITE);                                     // Text color

        EndDrawing();  // Finish drawing this frame

        // Wait for approximately one second by looping 60 times (simulate a frame delay)
        for (int i = 0; i < 60; i++) {
            if (WindowShouldClose()) return;            // Exit immediately if the window is closed
            WaitTime(1.0 / 60.0);                       // Delay for one frame (1/60th of a second)
        }

        countdown--;                                    // Decrement the countdown timer by 1
    }
}

// Function to display the Game Over screen and handle player choices
void ShowGameOverScreen(void) {
    StopMusicStream(backgroundMusic);                               // Stop the current background music when entering the Game Over screen
    
    Button playAgainButton = {
        {WINDOW_WIDTH / 2 - 220, WINDOW_HEIGHT / 2 + 50, 200, 50},  // Positioned left of center
        "Play Again",                                               // Button text
        (Color){ 193, 207, 161, 255 }                               // Pale Green
    };

    Button exitButton = {
        {WINDOW_WIDTH / 2 + 20, WINDOW_HEIGHT / 2 + 50, 200, 50},   // Positioned to the right of center
        "Exit",                                                     // Button text
        (Color){ 231, 204, 204, 255 }                               // Soft Pink
    };
    
    while (gameState == GAME_STATE_GAME_OVER) {                     // Enter the Game Over screen loop
        BeginDrawing();                                             // Begin drawing the Game Over screen
        ClearBackground(RAYWHITE);                                  // Clear the screen with a white background

        DrawTexture(gameOverTexture, 0, 0, WHITE);                  // Draw the Game Over background image

        const char *gameOverText = "Game Over!";                    // Display Game Over! text in the center of the screen
        int textWidth = MeasureText(gameOverText, 50);              // Measure the width of the text for centering
        DrawText(gameOverText,
                (WINDOW_WIDTH - textWidth) / 2,                     // Center horizontally
                WINDOW_HEIGHT / 2 - 50,                             // Vertical position slightly above the buttons
                50,                                                 // Font size
                RED);                                               // Text color

        // Draw Play Again button
        DrawRectangleRec(playAgainButton.bounds, playAgainButton.color);        
        DrawText(playAgainButton.text, 
                playAgainButton.bounds.x + playAgainButton.bounds.width / 2 - MeasureText(playAgainButton.text, 30) / 2, 
                playAgainButton.bounds.y + 10, 30, BLACK);

        // Draw Exit button
        DrawRectangleRec(exitButton.bounds, exitButton.color);
        DrawText(exitButton.text, 
                exitButton.bounds.x + exitButton.bounds.width / 2 - MeasureText(exitButton.text, 30) / 2, 
                exitButton.bounds.y + 10, 30, BLACK);

        // Handle button clicks
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();                  // Get the current mouse position
            
            if (CheckCollisionPointRec(mousePos, playAgainButton.bounds)) {
                // Reset all game states
                gameState = GAME_STATE_MENU;                        // Go back to the main menu
                stage = 1;                                          // Reset stage to the first level
                fallSpeed = INITIAL_FALL_SPEED;                     // Reset fall speed
                spawnInterval = SPAWN_INTERVAL;                     // Reset spawn interval
                startGame = false;                                  // Ensure the game starts fresh
                spawnTimer = 0;                                     // Reset timers
                stageTimer = 0;
                bombSpawnTimer = 0;
                windActive = false;                                 // Reset environmental effects #1
                thunderActive = false;                              // Reset environmental effects #2
                windTimer = 0.0f;
                thunderTimer = 0.0f;
                screenShakeTimer = 0;
                
                // Start menu music
                PlayMusicStream(startingScreenMusic);
                break;                                              // Exit the Game Over loop
            }
            
            if (CheckCollisionPointRec(mousePos, exitButton.bounds)) {
                CloseWindow();                                      // Close the game window
                break;                                              // Exit the Game Over loop
            }
        }

        EndDrawing();                                               // End drawing for the current frame
    }
}

// Function to Clamps a float value between a minimum and maximum range
float MyClamp(float value, float min, float max) {
    if (value < min) return min;        // If the value is less than the minimum, return the minimum
    if (value > max) return max;        // If the value is greater than the maximum, return the maximum
    return value;                       // If the value is within range, return the value as-is
}

// Function to retrieves the currently selected player mode
int GetSelectedMode(void) {
    return playerMode;                  // Return the global playerMode variable (e.g., 1 for single-player, 2 for two-player)
}

// Function to draw a portal effect at the given position
void DrawPortalEffect(Vector2 position) {
    Rectangle portalSrcRect = {0, 0, portalTexture.width, portalTexture.height};
    Rectangle portalDestRect = {                             // Define the destination rectangle where the portal will be drawn
        position.x - (portalTexture.width * 0.5f) / 2,       // Center horizontally
        position.y - (portalTexture.height * 0.5f) / 2,      // Center vertically    
        portalTexture.width * 0.5f,                          // Scale width to 50% of the original
        portalTexture.height * 0.5f                          // Scale height to 50% of the original
    };

    // Draw the portal texture with scaling and no rotation
    DrawTexturePro(portalTexture, 
                   portalSrcRect,   // Source rectangle
                   portalDestRect,  // Destination rectangle
                   (Vector2){0, 0}, // No offset for rotation
                   0.0f,            // No rotation
                   WHITE);          // Use white tint (default)
}


// Function to save the game state
void SaveGameState(Characters *koala, Characters *bear, int stage) {
    FILE *file = fopen("save_game.txt", "w"); // Open a file in write mode to save the game state
    if (!file) { // Check if the file was created successfully
        printf("Error: Could not create save file\n");
        return; // Exit the function if the file could not be created
    }

    // Calculate remaining hearts for Koala and Bear
    float koalaHearts = MAX_HEARTS - koala->score; // Remaining hearts for Koala
    float bearHearts = (playerMode == 2) ? (MAX_HEARTS - bear->score) : 0.0f; // Remaining hearts for Bear (only in two-player mode)

    // Write the game state to the file
    // Format: playerMode, Koala hearts, Bear hearts, current stage
    fprintf(file, "%d %.1f %.1f %d", playerMode, koalaHearts, bearHearts, stage);
    fclose(file); // Close the file after writing
    printf("Game saved! Mode: %d, Koala Hearts: %.1f, Bear Hearts: %.1f, Stage: %d\n", playerMode, koalaHearts, bearHearts, stage); // Debug Process
}

// Function to load the game state from the save file
void LoadGameState(Characters *koala, Characters *bear, int *stage) {
    // Open the save file in read mode
    FILE *file = fopen("save_game.txt", "r");
    if (!file) { // Check if the file exists
        printf("No save file found. Creating a new one...\n");
        SaveGameState(koala, bear, 1); // Create a default save file if none exists
        return; // Exit the function if the file is missing
    }

    // Variables to store the loaded data
    int savedPlayerMode; // Saved player mode (1 or 2)
    float koalaHearts, bearHearts; // Saved health for Koala and Bear
    int savedStage; // Saved stage number

    // Attempt to read the saved data from the file
    if (fscanf(file, "%d %f %f %d", &savedPlayerMode, &koalaHearts, &bearHearts, &savedStage) == 4) {
        // Successfully read the file

        // Assign values to global and character variables
        playerMode = savedPlayerMode; // Set the player mode
        *stage = savedStage; // Set the current stage

        // Configure Koala's state
        koala->score = MAX_HEARTS - koalaHearts; // Convert hearts to score
        koala->position = (Vector2){WINDOW_WIDTH / 4, WINDOW_HEIGHT - 160}; // Reset position
        koala->speedMultiplier = 1.0f; // Reset speed multiplier
        koala->isBurning = false; // Reset burning state
        koala->isFlinging = false; // Reset flinging state
        koala->texture = koalaTexture; // Set Koala's texture

        // Configure Bear's state if in two-player mode
        if (savedPlayerMode == 2) {
            bear->score = MAX_HEARTS - bearHearts; // Convert hearts to score
            bear->position = (Vector2){(3 * WINDOW_WIDTH) / 4, WINDOW_HEIGHT - 160}; // Reset position
            bear->speedMultiplier = 1.0f; // Reset speed multiplier
            bear->isBurning = false; // Reset burning state
            bear->isFlinging = false; // Reset flinging state
            bear->texture = bearTexture; // Set Bear's texture
        } else {
            bear->texture.id = 0; // Indicate no Bear in single-player mode
        }

        // Debug: Print a success message with the loaded data
        printf("Game loaded successfully! Mode: %d, Koala Hearts: %.1f, Bear Hearts: %.1f, Stage: %d\n", savedPlayerMode, koalaHearts, bearHearts, savedStage);
    } else {
        // Failed to read the save file due to format mismatch or corruption
        printf("Error reading save file. Save file might be corrupted or invalid.\n");
    }

    // Close the save file
    fclose(file);
}

// Function to handle cleanup
void CleanupAndExit(void) {
    // First stop and unload all audio
    StopMusicStream(startingScreenMusic);
    StopMusicStream(backgroundMusic);
    
    // Unload sounds
    UnloadSound(burningSound);
    UnloadSound(flingSound);
    UnloadSound(goodFoodAudio);
    UnloadSound(badFoodAudio);
    UnloadSound(bombAudio);
    UnloadSound(thunderAudio);

    // Unload music streams
    UnloadMusicStream(startingScreenMusic);
    UnloadMusicStream(backgroundMusic);

    // Close audio device
    CloseAudioDevice();

    // Unload all textures
    if (IsWindowReady()) {  // Only unload textures if window is still valid
        UnloadTexture(backgroundTexture);
        UnloadTexture(calmBackground);
        UnloadTexture(nightBackground);
        UnloadTexture(volcanoBackground);
        UnloadTexture(optionsBackground);
        UnloadTexture(pauseBackground);
        UnloadTexture(transitionTexture);
        UnloadTexture(gameOverTexture);
        UnloadTexture(loadBackground);
        UnloadTexture(optionsBackground);
        UnloadTexture(endTexture);
        UnloadTexture(pauseIconTexture);
        UnloadTexture(heartTexture);
        UnloadTexture(windLeftTexture);
        UnloadTexture(windRightTexture);
        UnloadTexture(portalTexture);
        UnloadTexture(koalaTexture);
        UnloadTexture(bearTexture);
        UnloadTexture(bombTexture);
        UnloadTexture(fallingStarTexture);
        UnloadTexture(fireTexture);
    }
}


// Function to display the Load Menu
void ShowLoadMenu(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray, int playerMode) {
    bool exitMenu = false;                                                              // Flag to determine if the menu should close
    
    // Define the "New Game" button
    Button newGameButton = {
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50, 200, 50},                      // Position and size of the button
        "New Game",                                                                     // Text displayed on the button
        (Color){ 193, 207, 161, 255 }                                                   // Pale Green
    };

    Button loadGameButton = {
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 20, 200, 50},                      // Position and size of the button
        "Load Game",                                                                    // Text displayed on the button
        (Color){ 237, 232, 220, 255 }                                                   // Soft Beige
    };

    while (!exitMenu && !WindowShouldClose()) {                                         // Menu loop: Runs until the user chooses an option or closes the window
        BeginDrawing();                                                                 // Start drawing the frame
        ClearBackground(RAYWHITE);                                                      // Clear the screen with a white background

        if (backgroundTexture.id != 0) {                                                // Draw the background texture if available
            DrawTexture(backgroundTexture, 0, 0, WHITE);                                // Draw the background image
        }

        // Draw buttons
        DrawRectangleRec(newGameButton.bounds, newGameButton.color);
        DrawText(newGameButton.text, 
                newGameButton.bounds.x + newGameButton.bounds.width / 2 - MeasureText(newGameButton.text, 30) / 2, 
                newGameButton.bounds.y + 10, 30, BLACK);

        DrawRectangleRec(loadGameButton.bounds, loadGameButton.color);
        DrawText(loadGameButton.text, 
                loadGameButton.bounds.x + loadGameButton.bounds.width / 2 - MeasureText(loadGameButton.text, 30) / 2, 
                loadGameButton.bounds.y + 10, 30, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePosition = GetMousePosition();                                         // Get the current position of the mouse
            printf("Mouse clicked at: %f, %f\n", mousePosition.x, mousePosition.y);

            // Check if the "New Game" button was clicked
            if (CheckCollisionPointRec(mousePosition, newGameButton.bounds)) {          
                printf("New Game button clicked\n");        
                InitGame(koala, bear, foodArray, bomb, starArray, playerMode);                  // Initialize a new game
                gameState = GAME_STATE_PLAYING;                                                 // Set game state to playing
                startGame = true;                                                               // Start the game
                break;                                                                          // Exit the menu loop
            }

            // Check if the "Load Game" button was clicked
            if (CheckCollisionPointRec(mousePosition, loadGameButton.bounds)) {
                printf("Loading saved game...\n");
                LoadGameState(koala, bear, &stage);                                             // Load the saved state
                
                gameState = GAME_STATE_PLAYING;                                                 // Set game state to playing
                startGame = true;                                                               // Start the game
                exitMenu = true;                                                                // Exit the menu loop
                EndDrawing();                                                                   // Ensure drawing ends before returning
                return;                                                                         // Exit the function
            }
        }

        EndDrawing();                                                                           // End drawing for the current frame
    }       
}

// Function to display the End Game screen after completing all stages
void ShowEndGameScreen(void) {
    StopMusicStream(backgroundMusic);                                                           // Stop background music
    endTexture = LoadTexture("assets/end.png");                                                 // Load the end game background texture
    bool exitEndScreen = false;                                                                 // Flag to determine when to exit the end game screen
    
    Button playAgainButton = {
        {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 100, 200, 50},                             // Position and size of the button
        "Play Again",                                                                           // Text displayed on the button
        (Color){ 166, 174, 191, 255 }                                                           // Ashy Blue - Hex A6AEBF
    };

    while (!exitEndScreen) {                                                                    // Loop to display the end game screen until the user exits
        BeginDrawing();                                                                         // Begin drawing the current frame
        ClearBackground(RAYWHITE);                                                              // Clear the screen with a white background

        // Draw the end game background
        if (endTexture.id != 0) {
            DrawTexture(endTexture, 0, 0, WHITE);                                               // Draw the loaded background texture 
        } else {
            DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, DARKGRAY);                         // Fallback background if texture is missing
        }

        // Draw the end game background
        DrawTexture(endTexture, 0, 0, WHITE);                                                   // Make sure to load this texture

        // Display "The End" text centered at the top of the screen
        const char *endText = "The End";                                                        // Loop to display the end game screen until the user exits
        int textWidth = MeasureText(endText, 60);                                               // Measure the width of the text for centering
        DrawText(endText, 
                (WINDOW_WIDTH - textWidth) / 2,                                                 // Center horizontally
                WINDOW_HEIGHT / 3,                                                              // Vertical position
                60,                                                                             // Font size
                WHITE);                                                                         // Text color

        // Draw congratulations text
        const char *congratsText = "Congratulations! You've completed all 30 stages!";
        textWidth = MeasureText(congratsText, 30);                                              // Measure the width of the text for centering
        DrawText(congratsText, 
                (WINDOW_WIDTH - textWidth) / 2,                                                 // Center horizontally
            WINDOW_HEIGHT / 2,                                                                  // Vertical position
                30,                                                                             // Font size
                WHITE);                                                                         // Text color

        // Draw Play Again button
        DrawRectangleRec(playAgainButton.bounds, playAgainButton.color);
        DrawText(playAgainButton.text, 
                playAgainButton.bounds.x + playAgainButton.bounds.width / 2 - MeasureText(playAgainButton.text, 30) / 2, 
                playAgainButton.bounds.y + 10, 30, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {                                          // Check for mouse input
            Vector2 mousePosition = GetMousePosition();                                         // Get the current position of the mouse

            if (CheckCollisionPointRec(mousePosition, playAgainButton.bounds)) {                // Check if the "Play Again" button was clicked
                gameState = GAME_STATE_MENU;                                                    // Set game state to the main menu
                stage = 1;                                                                      // Reset stage to 1
                startGame = false;                                                              // Ensure the game starts fresh
                exitEndScreen = true;                                                           // Exit the end game screen loop
                
                // Start playing the main menu music
                PlayMusicStream(startingScreenMusic);
            }
        }

        EndDrawing();                                                                           // End drawing for the current frame
    }
}