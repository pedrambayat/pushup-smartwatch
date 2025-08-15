#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int numSets = 0;
int numSetTotalReps = 0;
int numSetGoodReps = 0;
int numWorkTotalReps = 0;
int numWorkGoodReps = 0;
float prop = 0.0;

const int thresh_value = 4;
const int rom_value = 300; // wire it backwards
bool rom = true;
bool thresh_rep = false;
int reply;

// initializing
int setRead = 0;
int workoutRead = 0;
int emg_input = 0;
int rom_input = 0;

// EMG
const int points = 30;
int values[points];
int count = 0;
int switchState = 0;
int prevSwitchState = 0;

// pins
#define LED_PIN 7
#define SET_BUTTON 9
#define WORKOUT_BUTTON 8
const int FILTER_PIN = A0;
const int GONIOMETER_PIN = A1;

// REP SWITCH STATES
int repSwitchState = 0;
int prevRepSwitchState = 0;


bool repInProgress = false;
bool repCandidateGood = false;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SET_BUTTON, INPUT); // start a new set
  pinMode(WORKOUT_BUTTON, INPUT); // start a new workout

  
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Press to begin!");
  lcd.setCursor(0, 1);
  lcd.print("Do good reps!");
}

void loop() {
  setRead = digitalRead(SET_BUTTON);
  workoutRead = digitalRead(WORKOUT_BUTTON);
  emg_input = analogRead(FILTER_PIN);
  rom_input = analogRead(GONIOMETER_PIN);
  
  float sum = 0.0;
  float mean = 0.0;
  if (count < points - 1) {
    values[count] = emg_input;
  } else {
    for (int j = 0; j <= points - 2; j++) {
      values[j] = values[j + 1];
    }
    values[points - 1] = emg_input;
  }
  for (int i = 0; i < points; i++) {
    sum += values[i];
  }
  mean = sum / points; // rolling avg
  count += 1;

  if (mean < thresh_value) {
    switchState = 0;
    prevSwitchState = 0;
  }
  if (switchState == 0 && prevSwitchState == 0 && mean >= thresh_value) {
    switchState = 1;
    thresh_rep = true;
    prevSwitchState = 1;
  }
  if (switchState == 1 && prevSwitchState == 1 && mean <= thresh_value) {
    switchState = 0;
  }

  if (rom_input < rom_value) {
    rom = true;
  } else {
    rom = false;
  }
  
 
  // LOW = NOT PRESSED
  if (workoutRead == HIGH) { // check for new workout
    //prevWSS = 0;
    //prevSSS = 0;
    //setSwitchState = 1;
    //workoutSwitchState = 1;
    numSets = 0;
    numSetTotalReps = 0;
    numSetGoodReps = 0;
    numWorkTotalReps = 0;
    numWorkGoodReps = 0;
    prop = 0.0;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start first set!");
    //lcd.setCursor(0,1);
    //lcd.print("S:0,GR:0,P:.00");
  }


  if (setRead == HIGH) { // check for new set
    numSets += 1;
    numWorkGoodReps += numSetGoodReps;
    numWorkTotalReps += numSetTotalReps;
    prop = numWorkGoodReps / numWorkTotalReps;

    numSetGoodReps = 0;
    numSetTotalReps = 0;


    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("GR:0,TR:0");
    lcd.setCursor(0, 1);
    lcd.print("S:" + String(numSets) + " GR:" + String(numWorkGoodReps) + " TR:" + String(numWorkTotalReps));
//    Serial.println("NEW SET");
    delay(750); // so count sets one by one
  }
  
  if (numSets > 0) {
    
    if (!repInProgress && mean >= thresh_value) {
      repInProgress = true;
     
      if (rom_input <= rom_value) {
        repCandidateGood = true;
        digitalWrite(LED_PIN, HIGH); 
      } else {
        repCandidateGood = false;
        digitalWrite(LED_PIN, LOW);
      }
    }
   
    else if (repInProgress && mean < thresh_value) {
      repInProgress = false;
      numSetTotalReps++;  // Count every rep
      if (repCandidateGood) {
        numSetGoodReps++;  // Count as good rep if criteria met at start
      } else {
      }
      digitalWrite(LED_PIN, LOW);  // Turn off LED after rep
      lcd.setCursor(0, 0);
      lcd.print("GR:" + String(numSetGoodReps) + " TR:" + String(numSetTotalReps));
    }
  }
  //Serial.println(rom_input);
  Serial.println(mean);
  //Serial.println(emg_input);
}
