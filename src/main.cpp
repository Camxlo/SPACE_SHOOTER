// Librerías y definiciones iniciales
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Sprite.h"

// Pines para la conexión entre el Arduino Mega y el TFT LCD ILI9341
#define TFT_DC 7
#define TFT_CS 6
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 10
#define TFT_MISO 12

// Pines de los botones
#define botonRight 18
#define botonLeft 19
#define botonUp 21

// Pin del buzzer
#define buzzerPin 9

// Constantes de movimiento y dimensiones de la pantalla
const int XMAX = 240; // Ancho del display
const int YMAX = 320; // Alto del display
int x = 88;           // Posición inicial del jugador
int y = 256;

// Variables para el enemigo
int enemigoX = 88;
int enemigoY = 50;
bool enemigoMoviendoDerecha = true;
const int enemigoVelocidad = 2;

// Variables para el proyectil del jugador
int proyectilX = -1;
int proyectilY = -1;
bool proyectilActivo = false;
const int proyectilVelocidad = 60;

// Variables para el proyectil del enemigo
int enemigoProyectilX = -1;
int enemigoProyectilY = -1;
bool enemigoProyectilActivo = false;
const int enemigoProyectilVelocidad = 40;

// Variables para el sistema de puntuación y estado del juego
int puntuacion = 0;
int puntuacionMaxima = 0;
bool juegoActivo = true;

// Constantes para los sonidos
const int SOUND_SHOOT = 1000;
const int SOUND_HIT = 1500;
const int SOUND_GAME_OVER = 500;
const int SOUND_START = 2000;
const int DURATION_SHOOT = 100;
const int DURATION_HIT = 200;
const int DURATION_GAME_OVER = 500;
const int DURATION_START = 300;

Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
GFXcanvas1 canvas(64, 64);

// Declaración de funciones
void playSound(int frequency, int duration);
void pantallaInicio();
void pantallaGameOver();
void setPlayerPosition(int x, int y);
void animatePlayer(void);
void moverPlayer(int direccion);
void moverEnemigo(void);
void dispararProyectil(void);
void moverProyectil(void);
void dispararProyectilEnemigo(void);
void moverProyectilEnemigo(void);
void mostrarPuntuacion(void);
bool detectarColision(int obj1X, int obj1Y, int ancho1, int alto1, int obj2X, int obj2Y, int ancho2, int alto2);
void gameOver(void);

// Configuración inicial
void setup() {
    Serial.begin(9600);

    // Configuración de los pines de los botones
    pinMode(botonRight, INPUT);
    pinMode(botonLeft, INPUT);
    pinMode(botonUp, INPUT);

    // Configuración del buzzer
    pinMode(buzzerPin, OUTPUT);

    // Inicializar pantalla
    screen.begin();
    pantallaInicio();
    mostrarPuntuacion();

    // Establecer la posición inicial del jugador
    setPlayerPosition(x, y);
}

// Lazo principal
void loop() {
    if (juegoActivo) {
        // Mover y dibujar el enemigo
        moverEnemigo();

        // Mover y disparar el proyectil del enemigo
        moverProyectilEnemigo();

        // Leer los botones
        if (digitalRead(botonRight) == HIGH) {
            moverPlayer(1);
        }
        if (digitalRead(botonLeft) == HIGH) {
            moverPlayer(-1);
        }
        if (digitalRead(botonUp) == HIGH) {
            dispararProyectil();
        }

        // Mover el proyectil del jugador
        moverProyectil();

        // Dibujar el jugador
        animatePlayer();

        delay(16); // Suaviza el movimiento limitando la frecuencia de actualización
    }
}

// Función para reproducir sonidos
void playSound(int frequency, int duration) {
    tone(buzzerPin, frequency, duration);
    delay(duration);
    noTone(buzzerPin);
}

void pantallaInicio() {
    screen.fillScreen(ILI9341_BLACK); // Limpia la pantalla

    // Dibujar el título del juego
    screen.setTextColor(ILI9341_YELLOW); // Título en color amarillo
    screen.setTextSize(3); // Tamaño de letra grande
    screen.setCursor((XMAX - (13 * 18)) / 2, 60); // Ajustado para centrar completamente
    screen.print("SPACE SHOOTER");

    // Dibujar el mensaje de inicio
    screen.setTextColor(ILI9341_WHITE); // Mensaje en color blanco
    screen.setTextSize(2); // Tamaño de letra más grande
    screen.setCursor((XMAX - (19 * 12)) / 2, 150); // Centrar horizontalmente
    screen.print("Presione un boton");
    screen.setCursor((XMAX - (19 * 12)) / 2, 180);
    screen.print("para iniciar");

    // Reproducir sonido de inicio
    playSound(SOUND_START, DURATION_START);

    // Esperar hasta que se presione un botón
    while (digitalRead(botonRight) == LOW && digitalRead(botonLeft) == LOW && digitalRead(botonUp) == LOW) {
        delay(10);
    }
    screen.fillScreen(ILI9341_BLACK); // Limpia la pantalla al iniciar el juego
}

// Función para fijar la posición inicial del jugador
void setPlayerPosition(int x1, int y1) {
    x = x1;
    y = y1;
}

// Función para animar el jugador
void animatePlayer(void) {
    screen.fillRect(x, y, 64, 64, ILI9341_BLACK);
    canvas.fillScreen(0);
    canvas.drawBitmap(0, 0, Player[0], 64, 64, ILI9341_WHITE);
    screen.drawBitmap(x, y, canvas.getBuffer(), 64, 64, ILI9341_YELLOW);
}

// Movimiento del jugador
void moverPlayer(int direccion) {
    int delta = 20; // Velocidad del jugador
    screen.fillRect(x, y, 64, 64, ILI9341_BLACK);

    x += direccion * delta;
    if (x < 0) x = 0;
    if (x > XMAX - 64) x = XMAX - 64;

    setPlayerPosition(x, y);
}

// Movimiento del enemigo
void moverEnemigo() {
    screen.fillRect(enemigoX, enemigoY, 64, 64, ILI9341_BLACK);

    if (enemigoMoviendoDerecha) {
        enemigoX += enemigoVelocidad;
        if (enemigoX >= XMAX - 64) enemigoMoviendoDerecha = false;
    } else {
        enemigoX -= enemigoVelocidad;
        if (enemigoX <= 0) enemigoMoviendoDerecha = true;
    }

    screen.drawBitmap(enemigoX, enemigoY, Enemigo[0], 64, 64, ILI9341_RED);
}

// Disparar proyectil del jugador
void dispararProyectil() {
    if (!proyectilActivo) {
        proyectilX = x + 30;
        proyectilY = y;
        proyectilActivo = true;
        playSound(SOUND_SHOOT, DURATION_SHOOT);
    }
}

// Mover proyectil del jugador
void moverProyectil() {
    if (proyectilActivo) {
        screen.fillRect(proyectilX, proyectilY, 4, 10, ILI9341_BLACK);
        proyectilY -= proyectilVelocidad;

        if (detectarColision(proyectilX, proyectilY, 4, 10, enemigoX, enemigoY, 64, 64)) {
            screen.fillRect(proyectilX, proyectilY, 4, 10, ILI9341_BLACK);
            screen.fillRect(enemigoX, enemigoY, 64, 64, ILI9341_BLACK);

            proyectilActivo = false;
            enemigoY = 50;
            enemigoX = random(0, XMAX - 64);

            puntuacion += 50;
            mostrarPuntuacion();
            playSound(SOUND_HIT, DURATION_HIT);
        } else if (proyectilY >= 0) {
            screen.fillRect(proyectilX, proyectilY, 4, 10, ILI9341_WHITE);
        } else {
            screen.fillRect(proyectilX, proyectilY, 4, 10, ILI9341_BLACK);
            proyectilActivo = false;
        }
    }
}

// Disparar proyectil del enemigo
void dispararProyectilEnemigo() {
    if (!enemigoProyectilActivo) {
        enemigoProyectilX = enemigoX + 30;
        enemigoProyectilY = enemigoY + 64;
        enemigoProyectilActivo = true;
    }
}

// Mover proyectil del enemigo
void moverProyectilEnemigo() {
    if (enemigoProyectilActivo) {
        screen.fillRect(enemigoProyectilX, enemigoProyectilY, 4, 10, ILI9341_BLACK);
        enemigoProyectilY += enemigoProyectilVelocidad;

        if (detectarColision(enemigoProyectilX, enemigoProyectilY, 4, 10, x, y, 64, 64)) {
            screen.fillRect(x, y, 64, 64, ILI9341_BLACK);
            gameOver();
        }

        if (enemigoProyectilY <= YMAX) {
            screen.fillRect(enemigoProyectilX, enemigoProyectilY, 4, 10, ILI9341_RED);
        } else {
            enemigoProyectilActivo = false;
        }
    } else {
        dispararProyectilEnemigo();
    }
}

// Mostrar puntuación
void mostrarPuntuacion() {
    screen.fillRect(0, 0, XMAX, 20, ILI9341_BLACK);
    screen.setCursor(5, 5);
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(2);
    screen.print("Puntos: ");
    screen.print(puntuacion);
    screen.print(" Max: ");
    screen.print(puntuacionMaxima);
}

// Detectar colisión
bool detectarColision(int obj1X, int obj1Y, int ancho1, int alto1, int obj2X, int obj2Y, int ancho2, int alto2) {
    return obj1X < obj2X + ancho2 &&
           obj1X + ancho1 > obj2X &&
           obj1Y < obj2Y + alto2 &&
           obj1Y + alto1 > obj2Y;
}

// Game over
void gameOver() {
    juegoActivo = false;
    screen.fillScreen(ILI9341_BLACK);

    // Dibujar el título "Game Over"
    screen.setTextColor(ILI9341_RED);
    screen.setTextSize(3);
    screen.setCursor((XMAX - (9 * 18)) / 2, 60); // Centrar horizontalmente
    screen.print("Game Over");

    // Dibujar el puntaje actual
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(2);
    screen.setCursor((XMAX - (10 * 12)) / 2, 140); // Centrar horizontalmente
    screen.print("Puntos: ");
    screen.print(puntuacion);

   // Dibujar la puntuación máxima
    screen.setCursor((XMAX - (10 * 12)) / 2, 170);
    screen.print("Max: ");
    screen.print(puntuacionMaxima);

   // Mensaje para reiniciar
    screen.setCursor((XMAX - (19 * 12)) / 2, 210); // Centrar horizontalmente
    screen.print("Presione un boton");
    screen.setCursor((XMAX - (19 * 12)) / 2, 230);
    screen.print("para reiniciar");

    if (puntuacion > puntuacionMaxima) {
        puntuacionMaxima = puntuacion;
    }

    playSound(SOUND_GAME_OVER, DURATION_GAME_OVER);

    while (digitalRead(botonRight) == LOW && digitalRead(botonLeft) == LOW && digitalRead(botonUp) == LOW) {
        delay(10);
    }

    puntuacion = 0;
    juegoActivo = true;
    enemigoProyectilActivo = false;
    proyectilActivo = false;
    screen.fillScreen(ILI9341_BLACK);
    mostrarPuntuacion();
    setPlayerPosition(88, 256);
}
