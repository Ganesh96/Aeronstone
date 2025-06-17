# Aeronstone

Welcome to **Aeronstone**! This is a fast-paced, two-player dueling game that runs directly in the Windows command prompt. Built with modern C++, it uses classic ASCII graphics combined with a flicker-free rendering engine to create a smooth, real-time arcade experience.

## Features

* **Real-time, Two-Player Action:** Challenge a friend in a head-to-head duel on the same keyboard.
* **Flicker-Free Rendering:** A double-buffered rendering engine ensures smooth animations with no visual artifacts.
* **Color & Sound:** The command-line interface is brought to life with distinct colors for each player and simple sound effects for key actions.
* **Scoring System:** Be the first player to score 5 points to win the match.
* **Modern C++ Design:** The game is built with an object-oriented structure, making it clean, easy to read, and simple to expand with new features.

## How to Play

The goal is to hit your opponent with a projectile before they hit you. The first player to reach 5 points wins.

## Controls

| Action    | Player 1 (Left) | Player 2 (Right) |
| --------- | --------------- | ---------------- |
| Move Up   | W               | O                |
| Move Down | S               | L                |
| Shoot     | D               | K                |

> Press `ESC` at any time to quit the game.

## How to Compile and Run

This project is designed for Windows and is compiled using the G++ compiler from an MSYS2 environment.

### Prerequisites

You need to have MSYS2 with the MinGW-w64 UCRT64 toolchain installed. If you haven't set this up yet, follow these steps:

1. **Install MSYS2:** Download and run the installer from [msys2.org](https://www.msys2.org).
2. **Update MSYS2:** Open the MSYS2 MSYS terminal from the Start Menu and run:

   ```bash
   pacman -Syu
   ```

   You may need to close and re-open the terminal to run it a second time to complete all updates.
3. **Install Compiler:** Open the MSYS2 UCRT64 terminal from the Start Menu and run:

   ```bash
   pacman -S mingw-w64-ucrt-x86_64-toolchain
   ```

### Compilation Steps

1. **Open the Correct Terminal:** Open the MSYS2 UCRT64 terminal from your Start Menu.

2. **Navigate to Project Folder:** Use the `cd` command to navigate to the directory where you saved `aeronstone_v2.cpp`.

   ```bash
   # Example: If your project is on your desktop in a folder named 'Aeronstone'
   cd /c/Users/YourUsername/Desktop/Aeronstone
   ```

3. **Compile the Code:** Run the following `g++` command. This will create an executable file named `aeronstone.exe`.

   ```bash
   g++ -o aeronstone.exe aeronstone_v2.cpp -static-libgcc -static-libstdc++
   ```

4. **Run the Game:** Launch the game from the same terminal:

   ```bash
   ./aeronstone.exe
   ```

## Future Ideas

Aeronstone is built to be easily expandable. Here are some features that could be added next:

* **Power-ups:** Speed boosts, shields, or multi-shot abilities that appear randomly.
* **Obstacles:** Static or moving blocks in the center of the screen to make duels more challenging.
* **High Score Persistence:** Save the top scores to a file so they aren't lost when the game closes.
