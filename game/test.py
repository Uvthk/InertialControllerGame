import pygame
import sys
import serial

serial_port = 'COM5'
baud_rate = 115200
ser = serial.Serial(serial_port, baud_rate)

# Initialize pygame
pygame.init()

# Screen dimensions
screen_width = 800
screen_height = 600

# Colors
black = (0, 0, 0)
white = (255, 255, 255)

# Create the screen
screen = pygame.display.set_mode((screen_width, screen_height))
pygame.display.set_caption('Move the Square with WASD')

# Square settings
square_size = 50
square_x = screen_width // 2 - square_size // 2
square_y = screen_height // 2 - square_size // 2
square_speed = 5

music = pygame.mixer.Sound("menu-music.mp3")
music.play(-1)

# Main game loop
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # Get the current state of all keys
    keys = pygame.key.get_pressed()

    line = ser.readline().decode().strip()
    pitch_str, button_str = line.split()
    pitch, button = float(pitch_str), float(button_str)
    print(f"{pitch}, {button}")

    # Move the square
    if keys[pygame.K_w]:
        square_y -= square_speed
    if keys[pygame.K_s]:
        square_y += square_speed
    if pitch>20.0:
        square_x -= square_speed
    if pitch<-20.0:
        square_x += square_speed

    # Ensure the square stays within the screen bounds
    if square_x < 0:
        square_x = 0
    if square_x > screen_width - square_size:
        square_x = screen_width - square_size
    if square_y < 0:
        square_y = 0
    if square_y > screen_height - square_size:
        square_y = screen_height - square_size

    # Fill the screen with black
    screen.fill(black)

    # Draw the square
    pygame.draw.rect(screen, white, (square_x, square_y, square_size, square_size))

    # Update the display
    pygame.display.flip()

    # Frame rate
    pygame.time.Clock().tick(30)

# Quit pygame
pygame.quit()
sys.exit()
