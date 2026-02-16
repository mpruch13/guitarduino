/*
 * This file contains the code for the main Arduino. It communicates with the two 
 * controller Arduinos and controls a 16x2 LCD display that outputs game information 
 * to the players. On startup, it waits for a ready signal from both controllers, 
 * and after receiving the ready signals, messages the controllers to start the game. 
 * While the game is in progress, it receives hit/miss data from each controller, 
 * then calculates and displays each player's score on the LCD. Once the total 
 * number of hits/misses received is equal to the predetermined round length, 
 * the game ends and the winning player is printed to the display.
 */

#include <LiquidCrystal.h>
#include <AltSoftSerial.h>

// Define game state values and the number of notes/button presses in a game
#define PREGAME 0
#define COUNTDOWN 1
#define IN_PROGRESS 2
#define GAME_OVER 6
#define ROUND_LEN 50

// Set up variables for the LCD display
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Variables for keeping track of overall game data.
int game_state;
char in_byte; // for reading in Serial input
char game_string[ROUND_LEN];
bool p1_ready = false;
bool p2_ready = false;

// Variables for in-progress data (scores, consecutive hits, current multiplier)
int p1_score = 0;
int p1_hits = 0;
int p1_multi = 1;
int p2_score = 0;
int p2_hits = 0;
int p2_multi = 1;
int p1_wins = 0;
int p2_wins = 0;

// Keep track of total hit/misses received from both players
int p1_counter = 0;
int p2_counter = 0;

// Variables for timings using millis()
unsigned long countdown_start;
unsigned long countdown_period = 3000;
unsigned long last_recieved; // for tracking

AltSoftSerial altSerial; // RX, TX (Pins 8, 9 usually)

// Generates a string of note sprites for 1 round.
void generate_game_string() {
  for (int i = 0; i < ROUND_LEN - 1; i++) {
    game_string[i] = random(1, 16);
  }
  game_string[ROUND_LEN - 1] = 0; // null terminate
}

// Sends a string of note sprites to the controllers
void send_string() {
  for (int i = 0; i < ROUND_LEN; i++) {
    Serial.write(game_string[i]);
    altSerial.write(game_string[i]);
  }
  Serial.write('$'); // Use '$' to indicate the end of sent data
  altSerial.write('$');
}

// Attempts to read from Serial and altSerial sources to see if each player is ready.
void check_ready() {
  // Check player 1 Ready
  if (Serial.available() > 0) {
    in_byte = Serial.read();
    if (in_byte == 'A') {
      p1_ready = true;
    }
  }
  // Check player 2 Ready
  if (altSerial.available() > 0) {
    in_byte = altSerial.read();
    if (in_byte == 'B') {
      p2_ready = true;
    }
  }
}

// Returns a score multiplier for the given player based on their number of consecutive hits.
int get_multiplier(int player) {
  int num_hits;
  if (player == 1)
    num_hits = p1_hits;
  else
    num_hits = p2_hits;

  // Return the appropriate multiplier value based on player streak
  if (num_hits > 20) return 4;
  else if (num_hits > 10) return 3;
  else if (num_hits > 5) return 2;
  else return 1;
}

// Updates the given players score. Each hit is worth 10 points times the current multiplier.
void update_score(int player) {
  if (player == 1) {
    p1_hits++;
    p1_score += 10 * get_multiplier(1);
  } else {
    p2_hits++;
    p2_score += 10 * get_multiplier(2);
  }
}

// player 1: Hit = 'A' Miss = 'X'
// player 2: Hit = 'B', Miss = 'Y'
void check_hit() {
  // Check hit/miss from player 1
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == 'A') {
      update_score(1);
      print_scores();
      p1_counter++;
    } else if (input == 'X') {
      p1_hits = 0;
      p1_counter++;
    }
  }
  // Check for hit/miss from player 2
  if (altSerial.available() > 0) {
    char input = altSerial.read();
    if (input == 'B') {
      update_score(2);
      print_scores();
      p2_counter++;
    } else if (input == 'Y') {
      p2_hits = 0;
      p2_counter++;
    }
  }
}

// Helper function that prints player scores to the lcd
void print_scores() {
  lcd.setCursor(0, 0);
  lcd.print("P1:");
  lcd.print(p1_score);
  lcd.print("     "); // Clear trailing digits
  
  lcd.setCursor(0, 1);
  lcd.print("P2:");
  lcd.print(p2_score);
  lcd.print("     ");
}

void setup() {
  Serial.begin(9600);
  altSerial.begin(9600);
  lcd.begin(16, 2);
  
  game_state = PREGAME;
  lcd.setCursor(0, 0);
  lcd.print("Guitarduino Hero");
  lcd.setCursor(0, 1);
  lcd.print("Ready?");
}

void loop() {
  if (game_state == PREGAME) {
    check_ready();
    if (p1_ready && p2_ready) {
      for (int i = 0; i < 3; i++) {
        generate_game_string();
        send_string();
      }
      Serial.write('@');
      altSerial.write('@');
      lcd.clear();
      game_state = COUNTDOWN;
      countdown_start = millis();
    }
  } 
  
  else if (game_state == COUNTDOWN) {
    unsigned long counter = millis() - countdown_start;
    lcd.setCursor(7, 0);
    if (counter < 3000) {
      lcd.print(3 - (counter / 1000));
    } else {
      lcd.setCursor(5, 0);
      lcd.print("START!");
      delay(1500);
      lcd.clear();
      print_scores();
      Serial.write('X');
      altSerial.write('X');
      game_state = IN_PROGRESS;
      p1_score = 0; p1_hits = 0;
      p2_score = 0; p2_hits = 0;
      p1_counter = 0; p2_counter = 0;
    }
  } 
  
  else if (game_state == IN_PROGRESS) {
    check_hit();
    if (p1_counter >= ROUND_LEN && p2_counter >= ROUND_LEN) {
      game_state = GAME_OVER; 
    }
  } 
  
  else if (game_state == GAME_OVER) {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Game Over!");
    lcd.setCursor(0, 1);
    
    // Updated logic to compare scores instead of wins
    if (p1_score > p2_score) {
      lcd.print("Winner: Player 1!");
    } else if (p2_score > p1_score) {
      lcd.print("Winner: Player 2!");
    } else {
      lcd.print("It's a Tie!");
    }
    game_state = 10; // Idle state
  }
}