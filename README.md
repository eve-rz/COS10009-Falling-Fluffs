# Falling Fluffs (๑ᵔ⤙ᵔ๑)

Falling Fluffs is a cozy, stress-relieving 2D collection game built in C using the Raylib library. Designed with a soft aesthetic and intuitive mechanics, the game offers a peaceful yet engaging experience where players help adorable characters stay healthy by catching falling treats while navigating whimsical environmental challenges.

## The Experience
The game is crafted to balance relaxation with a sense of progression. Players take on the role of a Koala (or a Bear in cooperative mode) in a world where the primary goal is simple: stay happy and healthy.

* **Adaptive Journey**: Across 30 stages, the pace naturally evolves from a calm stroll to a brisk challenge.
* **Whimsical Hazards**: Experience dynamic weather including gentle winds that drift objects sideways and sudden thunderstorm flashes that add a touch of drama to the atmosphere.
* **Magical Elements**: At the peak of the journey (Stage 30), falling stars appear. While beautiful, they require quick reflexes to avoid being flung across the screen or stepping into the lingering stardust fire they leave behind.

## Gameplay Features
* **Cooperative Play**: A dedicated two-player mode allows friends to work together, fostering coordination and shared success.
* **Progress Persistence**: A custom Save and Load system ensures your journey isn't lost, allowing you to resume your stage and score at any time.
* **Atmospheric Audio**: Features a curated selection of soothing background music and responsive sound effects that enhance the "fluffy" theme.

## Technical Foundation
This project serves as a showcase of modular software design and efficient resource management in C.
* **State-Driven Logic**: A robust state machine manages transitions between menus, gameplay, and stage intervals.
* **Precise Interactions**: Custom collision logic ensures responsive movement and accurate item collection.
* **Structured Data**: Utilizes custom structs for Characters, Items, and Environmental effects to maintain clean, scalable code.

## Controls
| Action | Koala (P1) | Bear (P2) |
| :--- | :--- | :--- |
| **Move** | A / D Keys | Arrow Keys |
| **Pause** | P Key | - |
| **Save & Quit** | X Key (Paused) | - |

## Repository Map
* `Falling_Fluffs_Game/`: Contains all source code (`.c`, `.h`) and game assets.
* `Project_Report/`: Detailed documentation regarding the development and technical specifications of the program.
