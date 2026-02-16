/*
 * Guitarduino Hero - Player Controller Node
 * Optimized for non-blocking execution and bitwise input handling.
 * * Note: If uploading to Player 2, change 'A' to 'B' and 'X' to 'Y' 
 * in the Serial.write() commands.
 */

#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>
#include "notes.h" // must be in project folder

// --- Hardware Setup ---
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW 
#define MAX_DEVICES 4
#define CS_PIN 3
#define TRIGGER_PIN 2
#define ECHO_PIN 4
#define buzz_pin 10
#define ledPin 5

// Button Pins
const int b1 = 6, b2 = 7, b3 = 8, b4 = 9;

// --- Game Settings ---
#define ROUND_LEN 50 
#define ROUNDS 3
MD_Parola display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// --- State Management ---
// Fixed state management to be non-blocking
enum State { IDLE, WAITING_FOR_START, PLAYING };
State currentState = IDLE;

int curr_round = 0;
char allRounds[ROUNDS][ROUND_LEN];
int roundIndex = 0;
int SPEED = 150;

// Timing Variables
unsigned long roundStart = 0;
unsigned long lastSensorCheck = 0;
const int sensorInterval = 100; // Poll ultrasonic sensor every 100ms
int roundIter = 750;            // The "hit window" for a note

// Scoring flag
bool noteHitInRange = false;

// --- 1. Bitwise Input Logic ---
// Reads physical buttons and returns a 4-bit integer (0-15)
// modified from original version to replace many if/else if statements
int getButtonBits() {
  int bits = 0;
  if (digitalRead(b1) == HIGH) bits |= (1 << 0); // Bit 0
  if (digitalRead(b2) == HIGH) bits |= (1 << 1); // Bit 1
  if (digitalRead(b3) == HIGH) bits |= (1 << 2); // Bit 2
  if (digitalRead(b4) == HIGH) bits |= (1 << 3); // Bit 3
  return bits;
}

// Edge-detection for button presses
bool checkMatch(int targetPattern) {
  static int lastPattern = 0;
  int currentPattern = getButtonBits();
  
  if (currentPattern == targetPattern && lastPattern != targetPattern) {
    lastPattern = currentPattern;
    return true;
  }
  lastPattern = currentPattern;
  return false;
}

// --- 2. Sensor Logic ---
float getDistance() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 20000); // 20ms timeout
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

// --- 3. Gameplay Functions ---
void startNewRound() {
  roundIndex = 0;
  roundStart = millis();
  display.displayReset();
  display.displayText(allRounds[curr_round], PA_LEFT, SPEED, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  currentState = PLAYING;
}

void processGameplay() {
  if (roundIndex < ROUND_LEN) {
    // Check if player presses the correct combo for the CURRENT note
    int target = (int)allRounds[curr_round][roundIndex];
    if (checkMatch(target)) {
      noteHitInRange = true;
    }

    // When the timing window for this note expires
    if (millis() - roundStart >= roundIter) {
      if (noteHitInRange) {
        Serial.write('A'); // CHANGE TO 'B' FOR PLAYER 2
        tone(buzz_pin, 1473, 25);
      } else {
        Serial.write('X'); // CHANGE TO 'Y' FOR PLAYER 2
        tone(buzz_pin, 400, 50); // Low error tone
      }
      
      // Reset for next note
      noteHitInRange = false;
      roundStart = millis();
      roundIndex++;
    }
  }
}

// --- 4. Main Arduino Loops ---
void setup() {
  pinMode(b1, INPUT); pinMode(b2, INPUT); pinMode(b3, INPUT); pinMode(b4, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  pinMode(buzz_pin, OUTPUT); pinMode(ledPin, OUTPUT);

  display.begin();
  Serial.begin(9600);
  display.setIntensity(2);
  display.setFont(notes);
  
  randomSeed(analogRead(A0));
  for (int i = 0; i < ROUNDS; i++) {
    for (int j = 0; j < ROUND_LEN - 1; j++) allRounds[i][j] = random(1, 16);
    allRounds[i][ROUND_LEN - 1] = 0;
  }
}

void loop() {
  unsigned long now = millis();

  switch (currentState) {
    case IDLE:
      if (now - lastSensorCheck > sensorInterval) {
        if (getDistance() < 10) { // Distance trigger
          Serial.write('A'); // Send Ready Signal (CHANGE TO 'B' FOR P2)
          digitalWrite(ledPin, HIGH);
          currentState = WAITING_FOR_START;
        }
        lastSensorCheck = now;
      }
      break;

    case WAITING_FOR_START:
      if (Serial.available() > 0) {
        if (Serial.read() == 'X') { // 'X' is the Start command from Main
          startNewRound();
        }
      }
      break;

    case PLAYING:
      if (display.displayAnimate()) {
        // Round Finished
        Serial.write('$');
        curr_round++;
        digitalWrite(ledPin, LOW);
        currentState = (curr_round >= ROUNDS) ? IDLE : WAITING_FOR_START;
      } else {
        processGameplay();
      }
      break;
  }
}