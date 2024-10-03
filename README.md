# InertialControllerGame

This repository contains a project for designing and developing a computer game controlled using a custom game controller equipped with inertial sensors (IMU). The project integrates hardware for the controller and software for the game to create an immersive experience.

## Features

- **Custom Game Controller**: Utilizes an inertial sensor (IMU) for controlling the game by detecting pitch and button inputs from the controller.
- **Space Ship Game**: A 2D space shooter built with Python's `pygame` library.
- **Serial Communication**: Receives inputs from the custom controller over serial communication to interact with the game.
- **Multithreading**: Uses threading to handle real-time input from the controller while running the game loop.

## Files

- **`server.ino`**: Arduino code for reading sensor data from the inertial measurement unit (IMU) and sending it over serial communication.
- **`client.ino`**: Code for managing button inputs and communicating with the server to control the game.
- **`cargame.py`**: Python script for the game. The game features a 2D space shooter where the player controls a spaceship using inputs from the IMU and buttons via serial communication.

## Requirements

- Python 3.x
- `pygame`
- `pygame_menu`
- Serial communication library (`pyserial`)

## How to Run

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/InertialControllerGame.git
   ```
2. Install the required dependencies:
   ```bash
   pip install pygame pygame-menu pyserial
   ```
3. Upload the Arduino code (`server.ino` and `client.ino`) to your custom game controller.
4. Connect the controller to your PC and set the correct serial port in `cargame.py` (default is `COM5`).
5. Run the game:
   ```bash
   python cargame.py
   ```

## Game Controls

- Move left/right: Tilt the controller (pitch) or use `A`/`D` keys.
- Pause: Press the button on the controller or the `SPACE` key.

## Credits

This project was developed as part of a game design and development project using a controller with inertial sensors.
