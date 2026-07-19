#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

#define buzzer 11
Servo s;
LiquidCrystal_I2C l(0x27, 16, 2); // LCD initialization

const byte r = 4;
const byte c = 4;

char k[r][c] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rp[r] = {9, 8, 7, 6};
byte cp[c] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(k), rp, cp, r, c);

String enteredPIN = "";  // Store the entered PIN
String changePinInput = "";  // Input for changing PIN
String correctPIN = "1234";  // Default correct PIN
bool locked = true;      // Initial state (locked)
bool keyPressed = false;  // Flag to track key press state
bool isChangingPin = false; // Flag to track if we are changing the PIN
bool isCurrentPinCorrect = false;  // Flag for current PIN verification during change

void setup() {
  l.begin(16, 2);    // Initialize the LCD
  l.backlight();     // Ensure backlight is on
  delay(1000);       // Give time for initialization
  l.clear();         // Clear the screen

  pinMode(buzzer, OUTPUT); // Initialize the buzzer pin

  s.attach(10);      // Attach the servo to pin 10
  s.write(0);        // Lock the door initially (servo at position 0)

  l.setCursor(0, 0); // Position cursor at the start
  l.print("Enter Pin: "); // Prompt user for input
}

void loop() {
  char ke = keypad.getKey();  // Check if any key is pressed

  // Only process the key if it's pressed and the key isn't being processed already
  if (ke && !keyPressed) {
    keyPressed = true;  // Set keyPressed flag to true (indicating the key is being processed)

    handlePinEntry(ke);  // Handle PIN entry
    handleActions(ke);  // Handle actions like submit, change PIN, and lock/unlock
    keyPressed = false;  // Reset keyPressed flag after handling the input
  }
}

// Function to handle PIN entry (add characters to entered PIN and display it)
void handlePinEntry(char ke) {
  if (ke != 'D' && ke != '*' && ke != '#') {  // Ignore 'D', '*' and '#' for now
    if (isChangingPin) {
      changePinInput += ke;  // Add to the change PIN input string
      l.setCursor(0, 1);  // Move the cursor to the second line
      l.print(changePinInput);  // Display the current entered PIN for change
    } else {
      enteredPIN += ke;  // Add to the entered PIN string
      l.setCursor(0, 1);  // Move the cursor to the second line
      l.print(enteredPIN);  // Display the current entered PIN
    }
  }
}

// Function to check if the entered PIN matches the correct PIN
bool checkPin(String pin) {
  return pin == correctPIN;
}

// Function to handle the PIN change process
void changePin() {
  if (isCurrentPinCorrect) {
    correctPIN = changePinInput;  // Set new PIN
    clearScreenAndDisplayMessage("PIN Changed!", 2000);
    resetEnteredPin();
    isChangingPin = false;
    l.clear();
    l.setCursor(0, 0);
    l.print("Enter Pin: ");
  } else {
    clearScreenAndDisplayMessage("Incorrect PIN", 2000);
    resetEnteredPin();
    l.setCursor(0, 0);
    l.print("Enter Current PIN: ");
    isCurrentPinCorrect = false;  // Reset current PIN check
  }
}

// Function to unlock or lock the door based on the entered PIN
void unlockOrLockDoor() {
  if (checkPin(enteredPIN)) {  // Check if entered PIN is correct
    clearScreenAndDisplayMessage("PIN Correct", 2000);

    if (locked) {
      s.write(100);  // Unlock the door
      locked = false;
      clearScreenAndDisplayMessage("Door Unlocked", 2000);
      digitalWrite(buzzer, HIGH);
      delay(300);
      digitalWrite(buzzer, LOW);
    } else {
      s.write(0);  // Lock the door
      locked = true;
      clearScreenAndDisplayMessage("Door Locked", 2000);
      digitalWrite(buzzer, HIGH);
      delay(300);
      digitalWrite(buzzer, LOW);
    }

    resetEnteredPin();
  } else {
     digitalWrite(buzzer, HIGH);
     delay(200);
     digitalWrite(buzzer, LOW);
     delay(200);
     digitalWrite(buzzer, HIGH);
     delay(200);
     digitalWrite(buzzer, LOW);
     delay(200);
     digitalWrite(buzzer, HIGH);
     delay(200);
     digitalWrite(buzzer, LOW);
     delay(200);
    clearScreenAndDisplayMessage("Incorrect Pin", 2000);
    resetEnteredPin();
    l.setCursor(0, 0);
    l.print("Enter Pin: ");
  }
}

// Function to reset the entered PIN
void resetEnteredPin() {
  enteredPIN = "";  // Reset the entered PIN
  changePinInput = "";  // Reset the change PIN input
}

// Function to display a message and clear the screen after a delay
void clearScreenAndDisplayMessage(String message, unsigned long delayTime) {
  l.clear();
  l.setCursor(0, 0);
  l.print(message);
  delay(delayTime);  // Wait for specified delay before clearing screen
  l.clear();
}

// Function to handle actions like submit (D), change PIN (B), and lock/unlock
void handleActions(char ke) {
  // Handle the 'D' key (done or submit)
  if (ke == 'D') {
    if (isChangingPin) {
      if (!isCurrentPinCorrect) {
        // Verify current PIN during change
        if (checkPin(changePinInput)) {
          isCurrentPinCorrect = true;  // Mark current PIN as correct
          resetEnteredPin();
          l.clear();
          l.setCursor(0, 0);
          l.print("Enter New PIN:");
        } else {
          clearScreenAndDisplayMessage("Incorrect PIN", 2000);
          resetEnteredPin();
          l.setCursor(0, 0);
          l.print("Enter Current PIN:");
        }
      } else {
        // If current PIN was correct, set the new PIN
        if (changePinInput.length() >= 4) { // Ensure the new PIN is at least 4 digits
          correctPIN = changePinInput;  // Set new PIN
          clearScreenAndDisplayMessage("PIN Changed!", 2000);
          resetEnteredPin();
          isChangingPin = false;  // Exit PIN change mode
          l.clear();
          l.setCursor(0, 0);
          l.print("Enter Pin:");
        } else {
          clearScreenAndDisplayMessage("PIN Too Short", 2000);
          resetEnteredPin();
          l.setCursor(0, 0);
          l.print("Enter New PIN:");
        }
      }
    } else {
      unlockOrLockDoor();  // Normal PIN verification to unlock/lock
    }
  }

  // Handle '*' key (clear input)
  if (ke == '*') {
    resetEnteredPin();  // Clear the entered PIN
    clearScreenAndDisplayMessage("Cleared", 1000);
    l.setCursor(0, 0);
    l.print("Enter Pin:");
  }

  // Handle the 'B' key (Change PIN)
  if (ke == 'B' && !isChangingPin) {
    if (!locked) {  // Only allow PIN change if door is unlocked
      isChangingPin = true;
      resetEnteredPin();
      l.clear();
      l.setCursor(0, 0);
      l.print("Enter Current PIN:");
    } else {
      clearScreenAndDisplayMessage("Unlock first to change PIN!", 2000);
    }
  }
}