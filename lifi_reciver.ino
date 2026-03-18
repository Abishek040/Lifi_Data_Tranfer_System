#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize the LCD display (address 0x27 is common, but yours might be different)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin Assignments
const int ldrModulePin = 8;    // LDR Module's Digital Output (DOUT) connected to Digital Pin 8
const int resetButtonPin = 2;  // Reset button is connected to Digital Pin 2

// --- MORSE CODE TIMING FOR LDR MODULE (Still VERY SLOW!) ---
//
// IMPORTANT: You MUST calibrate these three values for your specific setup!
// These timings are for a "Unit Duration" of 400ms in the transmitter app.
// If you change the app's speed, you MUST adjust these values proportionally.
const int dotTime = 600;       // Max duration of a dot in milliseconds
const int dashTime = 2000;     // Max duration of a dash & min duration for a word space
const int letterGap = 800;     // Min duration of a space between letters

// Variables to store timing and morse code data
String morseCode = "";
String message = "";
unsigned long pulseStartTime = 0;
unsigned long gapStartTime = 0;
bool lightOn = false;

// Morse Code to Alphabet lookup array
String morseLetters[] = {
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
  "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"
};
String morseCodes[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
  "..-", "...-", ".--", "-..-", "-.--", "--..", ".----", "..---", "...--",
  "....-", ".....", "-....", "--...", "---..", "----.", "-----"
};

void setup() {
  Serial.begin(9600); // Initialize Serial Monitor at 9600 baud
  pinMode(ldrModulePin, INPUT);
  pinMode(resetButtonPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("LDR Module Rx");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // --- Check for a Reset Button Press ---
  if (digitalRead(resetButtonPin) == LOW) {
    morseCode = "";
    message = "";
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Message Cleared!");
    Serial.println("\n--- Message Cleared ---"); // <-- NEW: Also print to Serial
    delay(1000);
    lcd.clear();
    delay(100);
  }

  int ldrModuleValue = digitalRead(ldrModulePin);

  // --- Light Pulse Detection (LOGIC for LDR Module: LOW means light is ON) ---
  if (ldrModuleValue == LOW) {
    if (!lightOn) {
      lightOn = true;
      pulseStartTime = millis();
      gapStartTime = 0;
      // Serial.print("<ON>"); // Optional: for very detailed debugging of light state
    }
  } else {
    if (lightOn) {
      lightOn = false;
      unsigned long pulseDuration = millis() - pulseStartTime;
      if (pulseDuration < dotTime) {
        morseCode += ".";
        Serial.print("."); // <-- NEW: Print the dot to Serial Monitor
      } else {
        morseCode += "-";
        Serial.print("-"); // <-- NEW: Print the dash to Serial Monitor
      }
      gapStartTime = millis();
      // Serial.print("<OFF>"); // Optional: for very detailed debugging of light state
    }
  }

  // --- Gap (Space) Detection & Decoding ---
  if (!lightOn && gapStartTime > 0) {
    unsigned long gapDuration = millis() - gapStartTime;
    
    // Check for a new letter (medium gap)
    if (gapDuration > letterGap && gapDuration < dashTime && morseCode.length() > 0) {
      Serial.print(" "); // <-- NEW: Print a space for gap between elements in a letter to Serial
      decodeMorse();
      Serial.print(" "); // <-- NEW: Print a space after a decoded letter
      Serial.print(message.charAt(message.length()-1)); // <-- NEW: Print the decoded char
      Serial.print(" "); // <-- NEW: Print a space
    }
    
    // Check for a new word (long gap)
    if (gapDuration > dashTime && morseCode.length() > 0) {
      Serial.print("   "); // <-- NEW: Print a larger space for word gap to Serial
      decodeMorse();
      addCharacter(' ');
      Serial.print(" "); // <-- NEW: Print a space after a decoded letter
      Serial.print(message.charAt(message.length()-2)); // <-- NEW: Print the decoded char (previous char)
      Serial.print(" "); // <-- NEW: Print a space
      Serial.print(message.charAt(message.length()-1)); // <-- NEW: Print the space
      Serial.print(" "); // <-- NEW: Print a space
    }
  }

  // --- Display Handling ---
  lcd.setCursor(0, 0);
  lcd.print("Morse: " + morseCode + " ");
  lcd.setCursor(0, 1);
  lcd.print("Msg: " + message + " ");
}

void decodeMorse() {
  for (int i = 0; i < 36; i++) {
    if (morseCodes[i] == morseCode) {
      addCharacter(morseLetters[i][0]);
      break;
    }
  }
  morseCode = "";
}

void addCharacter(char c) {
  message += c;
  if (message.length() > 16) {
    message = message.substring(message.length() - 16);
  }
}
