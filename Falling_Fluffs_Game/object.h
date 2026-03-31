// In summary, object.h defines the shared structures, enums, and global variables used across the program for modularity and clarity.

#ifndef OBJECT_H
#define OBJECT_H

#include <raylib.h>

// Define different game states for managing game flow
typedef enum {
    GAME_STATE_MENU,          // Main menu
    GAME_STATE_OPTIONS,       // Options menu
    GAME_STATE_PLAYING,       // Gameplay
    GAME_STATE_PAUSE,         // Pause menu
    GAME_STATE_GAME_OVER,     // Game over screen
    GAME_STATE_TRANSITION,    // Transition between stages
    GAME_STATE_END            // End game screen
} GameStates;

// Constants
#define CUSTOM_WINDOW_CLOSE 999   // Custom flag for closing the window
#define WINDOW_HEIGHT 820         // Height of the game window
#define WINDOW_WIDTH 800          // Width of the game window
#define SQUARE_SIZE 20            // Size of falling objects
#define PLAYER_SPEED 4.0f         // Speed of the player character
#define INITIAL_FALL_SPEED 1.0f   // Initial speed of falling objects
#define MAX_FOOD 8                // Maximum number of food objects
#define SPAWN_INTERVAL 2.0f       // Time interval between spawns
#define MAX_HEARTS 5              // Maximum number of hearts/health points

// Global Variables for textures, sounds, and game states
extern Texture2D backgroundTexture;       // Background texture
extern Texture2D calmBackground;          // Background for calm stages
extern Texture2D nightBackground;         // Background for night stages
extern Texture2D volcanoBackground;       // Background for volcano stages
extern Texture2D loadBackground;          // Background for load menu
extern Texture2D optionsBackground;       // Background for options menu
extern Texture2D pauseBackground;         // Background for pause menu
extern Texture2D transitionTexture;       // Transition screen texture
extern Texture2D gameOverTexture;         // Game over screen texture
extern Texture2D endTexture;              // End game screen texture
extern Texture2D pauseIconTexture;        // Pause button texture
extern Texture2D goodFoodTextures[5];     // Textures for good food items
extern Texture2D badFoodTextures[3];      // Textures for bad food items
extern Texture2D koalaTexture;            // Texture for Koala character
extern Texture2D bearTexture;             // Texture for Bear character
extern Texture2D bombTexture;             // Texture for bombs
extern Texture2D fallingStarTexture;      // Texture for falling stars
extern Texture2D fireTexture;             // Texture for fire effect
extern Texture2D heartTexture;            // Texture for health hearts
extern Texture2D windLeftTexture;         // Texture for left wind indicator
extern Texture2D windRightTexture;        // Texture for right wind indicator
extern Texture2D portalTexture;           // Texture for portal effect

// Sound and music effects
extern Sound burningSound;                // Sound effect for burning
extern Sound flingSound;                  // Sound effect for flinging
extern Sound goodFoodAudio;               // Sound effect for collecting good food
extern Sound badFoodAudio;                // Sound effect for collecting bad food
extern Sound bombAudio;                   // Sound effect for bomb collision
extern Sound thunderAudio;                // Sound effect for thunder
extern Music startingScreenMusic;         // Music for the starting screen
extern Music backgroundMusic;             // Music for the main gameplay

// Global Variables for game logic and state management
extern GameStates gameState;              // Current state of the game
extern int stage;                         // Current stage of the game
extern int playerMode;                    // Player mode (1 or 2 players)
extern bool startGame;                    // Flag to indicate if the game has started
extern float fallSpeed;                   // Speed of falling objects
extern float spawnInterval;               // Interval between spawns
extern float spawnTimer;                  // Timer for spawn events
extern float stageTimer;                  // Timer for stage progression
extern float bombSpawnTimer;              // Timer for bomb spawns
extern bool windActive;                   // Flag for wind effect activation
extern bool thunderActive;                // Flag for thunder effect activation
extern float windTimer;                   // Timer for wind effect
extern float thunderTimer;                // Timer for thunder effect
extern float screenShakeTimer;            // Timer for screen shake effect
extern float windForce;                   // Strength of the wind
extern float musicVolume;                 // Volume for music
extern bool isMuted;                      // Flag for muting audio

// Character structure to define player attributes
typedef struct {
    Vector2 position;              // Position of the character
    int length;                    // Length for rendering or logic
    float speedMultiplier;         // Multiplier for speed effects
    float freezeTimer;             // Timer for freeze effects
    float slowTimer;               // Timer for slow effects
    float score;                   // Current score (or damage)
    bool isBurning;                // Flag for burning state
    float burnTimer;               // Timer for burning effect
    float burnDamageTimer;         // Timer for applying burn damage
    bool isFlinging;               // Flag for flinging state
    float flingTimer;              // Timer for flinging effect
    bool justLanded;               // Flag for landing detection
    float portalTimer;             // Timer for portal effect
    Vector2 flingVelocity;         // Velocity during flinging
    Texture2D texture;             // Texture for the character
} Characters;

// Enum for food types
typedef enum {
    FOOD_TYPE_GOOD,                // Good food (increases health or score)
    FOOD_TYPE_BAD                  // Bad food (reduces health or score)
} FoodType;

// Structure for food objects
typedef struct {
    bool isGood;                   // Whether the food is good or bad
    Vector2 position;              // Position of the food
    float value;                   // Value of the food item
    Texture2D texture;             // Texture for the food
    bool active;                   // Active state of the food
    int type;                      // Type of food (0: good or 1: bad)
} Food;

// Structure for bombs
typedef struct {
    Vector2 position;              // Position of the bomb
    Texture2D texture;             // Texture for the bomb
    bool active;                   // Active state of the bomb
} Bomb;

// Structure for falling stars
typedef struct {
    Vector2 position;              // Position of the falling star
    Texture2D texture;             // Texture for the star
    bool active;                   // Active state of the star
    float velocity;                // Falling velocity
    bool isFire;                   // Whether the star has turned into fire
    float fireTimer;               // Timer for fire state
} FallingStar;

// Structure for buttons
typedef struct {
    Rectangle bounds;              // Bounds of the button
    const char *text;              // Text displayed on the button
    Color color;                   // Color of the button
} Button;

// Function prototypes
void DamageCharacter(Characters *character, float damage);
void InitGame(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray, int playerMode);
void UpdateGame(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray, float deltaTime);
void DrawGame(Characters koala, Characters bear, Food foodArray[], Bomb bomb, FallingStar *starArray, int stage);
void ShowStageTransition(int stageNumber);
void ApplyWindEffect(Food foodArray[], Bomb *bomb, float deltaTime);
void DrawWindEffects(Food foodArray[], Bomb *bomb);
void ApplyThunderEffect(float *screenFlashTimer, float deltaTime);
void SpawnFallingStars(FallingStar *starArray, int maxStars);
void ShowMainMenu(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray, int playerMode);
void ShowStartMenu(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray);
void ShowPlayerModeSelection(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray);
void ShowTutorial(void);
int GetSelectedMode(void);
void ShowPauseMenu(Characters *koala, Characters *bear);
void UpdateCharacterEffects(Characters *character, float deltaTime);
void ShowGameOverScreen(void);
float MyClamp(float value, float min, float max);
void DrawPortalEffect(Vector2 position);
void SaveGameState(Characters *koala, Characters *bear, int stage);
void LoadGameState(Characters *koala, Characters *bear, int *stage);
void ShowLoadMenu(Characters *koala, Characters *bear, Food foodArray[], Bomb *bomb, FallingStar *starArray, int playerMode);
void CleanupAndExit(void); 
void ShowEndGameScreen(void);

#endif // OBJECT_H  
