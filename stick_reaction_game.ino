/*
  ===================================================================
   STICK REACTION GAME  —  Arduino Uno + 5 Servos + 2 Illuminated
   Push Buttons (Green = Start, Red = Restart) + Buzzer
  ===================================================================

  GAME FLOW:
   1. Player presses the GREEN button.
        -> Green LED turns ON
        -> Buzzer plays a bright "start" jingle
   2. The 5 servos fire ONE AT A TIME in RANDOM order (no repeats).
        -> Each servo rotates from 0 deg to ~90 deg and STAYS there
        -> Each turn "belongs" to that servo for 3 seconds before
           the next random servo fires
   3. Once all 5 servos have fired (all sticks are up):
        -> Green LED turns OFF
        -> Red LED turns ON  (this is the "press to restart" cue)
   4. Player presses the RED button.
        -> All 5 servos rotate back to 0 deg (initial position)
        -> Buzzer plays a "restart" jingle
        -> Red LED turns OFF
        -> System goes back to waiting for the GREEN button

  ===================================================================
  PIN MAP (change the #defines below if you wire it differently)
  ===================================================================
    Servo 1 signal   -> D3
    Servo 2 signal   -> D5
    Servo 3 signal   -> D6
    Servo 4 signal   -> D9
    Servo 5 signal   -> D10
    Green button     -> D2   (button to GND, uses INPUT_PULLUP)
    Green LED        -> D4   (skip this if using an illuminated
                               button that has its own built-in LED
                               wired straight to the button - see
                               wiring notes at the end of chat)
    Red button       -> D7   (button to GND, uses INPUT_PULLUP)
    Red LED          -> D8
    Buzzer (passive) -> D11
  ===================================================================
*/

#include <Servo.h>

// ---------------- PIN DEFINITIONS ----------------
const uint8_t SERVO_PINS[5] = {3, 5, 6, 9, 10};

const uint8_t GREEN_BTN_PIN = 2;
const uint8_t GREEN_LED_PIN = 4;
const uint8_t RED_BTN_PIN   = 7;
const uint8_t RED_LED_PIN   = 8;
const uint8_t BUZZER_PIN    = 11;

// ---------------- GAME SETTINGS ----------------
const int SERVO_HOME_ANGLE   = 0;    // resting position
const int SERVO_ACTIVE_ANGLE = 90;   // "triggered" position
const unsigned long TURN_DURATION_MS = 3000; // 3 sec per servo turn
const unsigned long DEBOUNCE_MS = 40;

// ---------------- OBJECTS / STATE ----------------
Servo servos[5];
uint8_t order[5] = {0, 1, 2, 3, 4}; // will be shuffled each round

// ---------------- NOTE FREQUENCIES ----------------
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_C6  1047
#define NOTE_G4  392
#define NOTE_E4  330
#define NOTE_C4  262

void setup() {
  // Servos: attach and send to home position
  for (uint8_t i = 0; i < 5; i++) {
    servos[i].attach(SERVO_PINS[i]);
    servos[i].write(SERVO_HOME_ANGLE);
  }

  pinMode(GREEN_BTN_PIN, INPUT_PULLUP);
  pinMode(RED_BTN_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  // Seed the random generator from the electrical noise on an
  // unconnected analog pin so the sequence is different every time
  randomSeed(analogRead(A0));

  Serial.begin(9600);
  Serial.println(F("Stick Reaction Game ready. Press GREEN to start."));
}

void loop() {
  // ---------- Wait for GREEN button to start the round ----------
  if (buttonPressed(GREEN_BTN_PIN)) {
    startGame();
    runServoSequence();
    waitForRestart();
  }
}

// ===================================================================
//  Helper: simple debounced "was the button just pressed" check
// ===================================================================
bool buttonPressed(uint8_t pin) {
  if (digitalRead(pin) == LOW) {      // active LOW (INPUT_PULLUP)
    delay(DEBOUNCE_MS);
    if (digitalRead(pin) == LOW) {
      // wait for release so one press = one action
      while (digitalRead(pin) == LOW) { delay(5); }
      return true;
    }
  }
  return false;
}

// ===================================================================
//  Step 1: Start the game — green LED on + start jingle
// ===================================================================
void startGame() {
  Serial.println(F("Game started!"));
  digitalWrite(GREEN_LED_PIN, HIGH);
  playStartSound();
  shuffleOrder();
}

// Fisher-Yates shuffle so all 5 servos fire once, in random order
void shuffleOrder() {
  for (uint8_t i = 0; i < 5; i++) order[i] = i;
  for (int i = 4; i > 0; i--) {
    int j = random(0, i + 1);
    uint8_t tmp = order[i];
    order[i] = order[j];
    order[j] = tmp;
  }
}

// ===================================================================
//  Step 2: Fire each servo, in the shuffled order, one every 3 sec
// ===================================================================
void runServoSequence() {
  for (uint8_t k = 0; k < 5; k++) {
    uint8_t servoIndex = order[k];
    Serial.print(F("Firing servo #"));
    Serial.println(servoIndex + 1);

    servos[servoIndex].write(SERVO_ACTIVE_ANGLE);
    delay(TURN_DURATION_MS);
  }

  // All 5 sticks are now up -> switch indicator to RED (restart cue)
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  Serial.println(F("All servos triggered. Press RED to restart."));
}

// ===================================================================
//  Step 3: Wait for RED button, reset servos, play restart jingle
// ===================================================================
void waitForRestart() {
  while (true) {
    if (buttonPressed(RED_BTN_PIN)) {
      Serial.println(F("Restarting..."));
      for (uint8_t i = 0; i < 5; i++) {
        servos[i].write(SERVO_HOME_ANGLE);
      }
      playRestartSound();
      digitalWrite(RED_LED_PIN, LOW);
      Serial.println(F("Ready. Press GREEN to start again."));
      break;
    }
  }
}

// ===================================================================
//  Sounds
// ===================================================================
void playStartSound() {
  // Bright rising arpeggio = "let's go!"
  int notes[]      = {NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6};
  int durations[]  = {120, 120, 120, 220};
  for (uint8_t i = 0; i < 4; i++) {
    tone(BUZZER_PIN, notes[i], durations[i]);
    delay(durations[i] + 30);
  }
  noTone(BUZZER_PIN);
}

void playRestartSound() {
  // Cool descending "power-down / reset" riff
  int notes[]      = {NOTE_A5, NOTE_G5, NOTE_E5, NOTE_C5, NOTE_G4};
  int durations[]  = {100, 100, 100, 100, 250};
  for (uint8_t i = 0; i < 5; i++) {
    tone(BUZZER_PIN, notes[i], durations[i]);
    delay(durations[i] + 30);
  }
  noTone(BUZZER_PIN);
}
