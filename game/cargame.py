import pygame, sys, random, pygame_menu, serial, threading
from pygame.locals import *
from time import sleep

pygame.init()
pygame.display.set_caption("Space Ship")
pygame.display.set_icon(pygame.image.load("icon.jpg"))

gameover = False

BLUE = (0, 0, 255)
GREEN = (0, 255, 0)
RED = (255, 0, 0)
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
HP = 10

SCREEN_SIZE = (400, 600)
screen = pygame.display.set_mode(SCREEN_SIZE, 0, 32)
screen.fill(BLACK)

FRAME = 50
fps = pygame.time.Clock()

pSize = (10, 10)
pCenter = (SCREEN_SIZE[0] // 2, SCREEN_SIZE[1] - 5)
pSpeed = 2
eSpeed = 1
bSpeed = 10
sSpeed = 2
no_bullet, bonus = True, True
score = 0

font = pygame.font.SysFont("Aliasblack", 30)
big_font = pygame.font.SysFont("Aliasblack", 60)

# sounds
laser_sound = pygame.mixer.Sound("laser-shoot.wav")
explosion = pygame.mixer.Sound("explosion.wav")
player_death = pygame.mixer.Sound("player-death.mp3")
lossing = pygame.mixer.Sound("lossing.mp3")
background_music = pygame.mixer.Sound("retro-music.mp3")
menu_music = pygame.mixer.Sound("menu-music.mp3")
pause = pygame.mixer.Sound("pause.mp3")
hit_HP = pygame.mixer.Sound("hit-HP.mp3")
falling = pygame.mixer.Sound("falling-castle.wav")
levelup = pygame.mixer.Sound("levelup.mp3")

button_effect = pygame_menu.sound.Sound()
button_effect.set_sound(pygame_menu.sound.SOUND_TYPE_WIDGET_SELECTION, "button-selection.mp3")
menu = pygame_menu.Menu('Menu', 400, 600, theme=pygame_menu.themes.THEME_DARK)
menu.set_sound(button_effect)

gameover_img = pygame.transform.scale(pygame.image.load("gameover.png"), (300, 200))

serial_port = 'COM5'
baud_rate = 115200
ser = serial.Serial(serial_port, baud_rate)
pitch = 0.0
button_str = ""

stop_event = threading.Event()

class Player(pygame.sprite.Sprite):
    def __init__(self, size, center, speed):
        super().__init__()
        self.rect = pygame.Rect((0, 0), size)
        self.rect.center = center
        self.head = pygame.Rect((0, 0), (4, 18))
        self.head.center = center
        self.speed = speed

    def move(self, pitch):        
        pressed_keys = pygame.key.get_pressed()
        # speed up player if K_s pressed or pitch angle > 40 degrees
        speed_p = self.speed + 1 if pressed_keys[K_s] or abs(pitch) > 40 else self.speed
        if self.rect.left > 5 and (pressed_keys[K_a] or pitch > 20):
            self.rect.move_ip(-speed_p, 0)
            self.head.move_ip(-speed_p, 0)
        if self.rect.right < SCREEN_SIZE[0] - 5 and (pressed_keys[K_d] or pitch < -20):
            self.rect.move_ip(speed_p, 0)
            self.head.move_ip(speed_p, 0)

    def draw(self, surface):
        pygame.draw.rect(surface, WHITE, self.rect)
        pygame.draw.rect(surface, WHITE, self.head)

class Enemy(pygame.sprite.Sprite):
    def __init__(self, speed):
        super().__init__()
        self.rect = pygame.Rect((0, 0), (10, 10))
        self.rect.center = (random.randrange(5, SCREEN_SIZE[0] - 5, 5), 5)
        self.speed = speed

    def move(self):
        global HP
        self.rect.move_ip(0, self.speed)
        if self.rect.bottom > SCREEN_SIZE[1]:
            hit_HP.play()
            self.kill()
            HP -= 1

    def draw(self, surface):
        pygame.draw.rect(surface, BLUE, self.rect)

class Bullet(pygame.sprite.Sprite):
    def __init__(self, speed, player, sound):
        super().__init__()
        self.rect = pygame.Rect((0, 0), (2, 10))
        self.rect.center = player.rect.center
        self.speed = speed
        sound.play()
    
    def move(self):
        global no_bullet, score
        self.rect.move_ip(0, -self.speed)
        if self.rect.bottom < 0:
            self.kill()
            no_bullet = True
        for enemy in enemies:
            if self.rect.colliderect(enemy):
                self.kill()
                enemy.kill()
                explosion.play()
                no_bullet = True
                score += 1
                break

    def draw(self, surface):
        pygame.draw.rect(surface, GREEN, self.rect)

class Star(pygame.sprite.Sprite):
    def __init__(self, speed, collor):
        super().__init__()
        self.rect = pygame.Rect((0, 0), (2, 2))
        self.rect.center = (random.randint(0, SCREEN_SIZE[0]),0)
        self.speed = speed
        self.collor = collor

    def move(self):
        self.rect.move_ip(0, self.speed)
        if self.rect.bottom > SCREEN_SIZE[1]:
            self.kill()

    def draw(self, surface):
        pygame.draw.rect(surface, self.collor, self.rect)

player = Player(pSize, pCenter, pSpeed)
enemies = pygame.sprite.Group()
all_sprite = pygame.sprite.Group()

def read_serial(stop_event):
    global pitch, button_str
    while not stop_event.is_set():
        line = ser.readline().decode().strip()
        pitch_str, button_str = line.split()
        pitch = float(pitch_str)
        # print(pitch)

def game_loop():
    global HP, bonus, no_bullet, score, gameover
    menu_music.stop()
    background_music.play(-1)
    gameover = False

    while not gameover:        
        for event in pygame.event.get():
            if event.type == QUIT:
                pygame.quit()
                sys.exit()
            if event.type == KEYDOWN and event.key == K_SPACE:
                gameover = True
                background_music.stop()
                pause.play()

        # shoot bullet with button
        if button_str == "0.00" and no_bullet:
            bullet = Bullet(bSpeed, player, laser_sound)
            all_sprite.add(bullet)
            no_bullet = False
        # add new enemy
        if random.random() > 0.99:
            enemy = Enemy(eSpeed)
            enemies.add(enemy)
            all_sprite.add(enemy)
        # add new star
        if random.random() > 0.9:
            all_sprite.add(Star(random.randint(1, 10), (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))))

        screen.fill(BLACK)
        score_font = font.render(str(score), True, WHITE)
        HP_font = font.render(str(HP), True, RED)
        screen.blit(score_font, (10, 10))
        screen.blit(HP_font, (SCREEN_SIZE[0] - 40, 10))
    
        # move all sprites
        for sprite in all_sprite:
            sprite.move()
            sprite.draw(screen)
        # move player
        player.move(pitch)
        player.draw(screen)

        # bonus
        if score % 10 == 0 and bonus and score != 0:
            HP += 1
            bonus = False
            levelup.play()
        if score % 10 != 0:
            bonus = True

        # collition checker
        if pygame.sprite.spritecollideany(player, enemies) or HP <= 0:
            gameover = True
            background_music.stop()
            falling.play() if HP <= 0 else player_death.play()
            sleep(3)
            for enemy in enemies:
                enemy.kill()
            screen.fill(BLACK)
            screen.blit(gameover_img, (50, 150))
            screen.blit(big_font.render("You Score:", True, WHITE), (60, 400))
            screen.blit(big_font.render(str(score), True, WHITE), (290, 400))
            lossing.play()
            pygame.display.update()
            sleep(4)
            menu_music.play(-1)
            score = 0
            HP = 10
            
        pygame.display.update()
        fps.tick(FRAME)

def main():
    global gameover, stop_event
    stop_event.clear()
    serial_thread = threading.Thread(target=read_serial, args=(stop_event,))
    serial_thread.start()
    game_loop()
    stop_event.set()
    serial_thread.join()

menu_music.play(-1)
menu.add.button("Play", main)
menu.add.button("Quite", pygame_menu.events.EXIT)
menu.mainloop(screen)