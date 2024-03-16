#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

int latchPin = 12;
int clockPin = 13;
int dataPin = 11;
int blankPin = 14;

int SHDNPin = 15;

//Mode
int SW1_R = 2; //Divergence
int SW1_M = 3; //Date
int SW1_L = 4; //Time

//Settings
int SW2_R = 5; //Special
int SW2_M = 6; //Setting
int SW2_L = 7; //Normal

//buttons
int BTN_L = 8; //Next
int BTN_M = 9; //Up
int BTN_R = 10;//Down

/*
  SCLPin = 19
  SDAPin = 18
*/


bool tubeDigitData[8][12] = {0};
//first value is the tube, 0 is MSB;
//second value is the value, 0 is 0, 10 is Ldot, 11 is Rdot
//bool 1 is light up

bool FinalStringData[96] = {0};
//Final data to be shifted out

bool PrevFinalStringData[96] = {0};
//Previous final data to be shifted out

int SettingBlinkTimeConst = 500; //time const for ON or OFF state when blinking in milliseconds
unsigned long SettingBlinkTime = 0; //time pass for ON or OFF state when blinking
//bool SettingBlink = 0; //ON or OFF state when blinking
int SettingPair = 1; //The pair of digits being set with 1(hr/date), 2(min/month), 3(sec/year)
//Setting mode for Time and Date
int prev_sec = 60;
//to determine if the time has changed

int PrevStateSW1 = 1; //1:date 2:time 3:divergence
int PrevStateSW2 = 1; //1:Normal 3:setting 3:special
bool PrevStateBTN_L = 0; //Next
bool PrevStateBTN_M = 0; //Up
bool PrevStateBTN_R = 0; //Down
//previous state of the inputs

bool LowHighBTN_L = 0; //Next
bool LowHighBTN_M = 0; //Up
bool LowHighBTN_R = 0; //Down
//current state of the inputs

unsigned long debounceTimeBTN_L = 0;
unsigned long debounceTimeBTN_M = 0;
unsigned long debounceTimeBTN_R = 0;
int debounceTime = 50;
//debouncing
int numbertest=0;

//Divergence number
long DivergenceNum[12] = {10130426, 571024, 571015, 523299, 456903, 409420, 337187, 409431, 456914, 523307, 571046, 10130205};
long SteinsGateNum = 10048596;
int CurrentDiv = 0;

void setup() {
  //Serial.println(mainData, BIN);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(blankPin, OUTPUT);

  pinMode(SHDNPin, OUTPUT);

  pinMode(SW1_R, INPUT);
  pinMode(SW1_M, INPUT);
  pinMode(SW1_L, INPUT);
  pinMode(SW2_R, INPUT);
  pinMode(SW2_M, INPUT);
  pinMode(SW2_L, INPUT);
  pinMode(BTN_L, INPUT);
  pinMode(BTN_M, INPUT);
  pinMode(BTN_R, INPUT);

  digitalWrite(blankPin, LOW);
  digitalWrite(SHDNPin, HIGH);

  Serial.begin(9600);
  while (! Serial); // Wait untilSerial is ready
  Serial.println("Test");

  tdd_reset();
  tdd_updateShiftRegister();
  digitalWrite(blankPin, HIGH);
  digitalWrite(SHDNPin, LOW);


}

void loop() {
  bool CurrentstateBTN_L = digitalRead(BTN_L);
  bool CurrentstateBTN_M = digitalRead(BTN_M);
  bool CurrentstateBTN_R = digitalRead(BTN_R);

  if ((PrevStateBTN_L != CurrentstateBTN_L) && (millis() - debounceTimeBTN_L > debounceTime)) {
    LowHighBTN_L = 1;
    numbertest++;
  }
  else {
    LowHighBTN_L = 0;
  }
  if(CurrentstateBTN_L){
    debounceTimeBTN_L = millis();
  }

  if ((PrevStateBTN_M != CurrentstateBTN_M) && (millis() - debounceTimeBTN_M > debounceTime)) {
    LowHighBTN_M = 1;
    numbertest++;
  }
  else {
    LowHighBTN_M = 0;
  }
  if(CurrentstateBTN_M){
    debounceTimeBTN_M = millis();
  }

  if ((PrevStateBTN_R != CurrentstateBTN_R) && (millis() - debounceTimeBTN_R > debounceTime)) {
    LowHighBTN_R = 1;
    numbertest++;
  }
  else {
    LowHighBTN_R = 0;
  }
  if(CurrentstateBTN_R){
    debounceTimeBTN_R = millis();
  }

  PrevStateBTN_L = CurrentstateBTN_L;
  PrevStateBTN_M = CurrentstateBTN_M;
  PrevStateBTN_R = CurrentstateBTN_R;


  if (digitalRead(SW1_L)) {
    printDATE();
    Serial.print("Date ");
  }
  else if (digitalRead(SW1_M)) {
    printTIME();
    Serial.print("Time ");
  }
  else if (digitalRead(SW1_R)) {
    printDIVERGENCE();
    Serial.print("DIVERGENCE ");
  }
  else {
    tdd_reset();
    tubeDigitData[7][1] = 1;
  }

  tdd_updateShiftRegister();
  tdd_reset();
  Serial.println();
  Serial.print(numbertest); //test
  Serial.print("  ");       //test
  Serial.print("BTN_L:");
  Serial.print(LowHighBTN_L);
  Serial.print(" BTN_M:");
  Serial.print(LowHighBTN_M);
  Serial.print(" BTN_R:");
  Serial.print(LowHighBTN_R);
  Serial.println();
  Serial.println();

  /*print0To9DotDotRolling();
    tdd_reset();
    tdd_updateShiftRegister();
    delay(1000);*/

}


//----------------------------------
//------------FUNCTIONS-------------
//----------------------------------


//prints the current 24hr time in HH MM SS
void printTIME()
{
  tmElements_t tm;
  if (RTC.read(tm)) {
    tubeDigitData[0][tm.Hour / 10] = 1;
    tubeDigitData[1][tm.Hour % 10] = 1;

    tubeDigitData[3][tm.Minute / 10] = 1;
    tubeDigitData[4][tm.Minute % 10] = 1;

    tubeDigitData[6][tm.Second / 10] = 1;
    tubeDigitData[7][tm.Second % 10] = 1;

    if (tm.Second % 2) {
      tubeDigitData[2][10] = 1;
      tubeDigitData[5][10] = 1;
    }
    else {
      tubeDigitData[2][11] = 1;
      tubeDigitData[5][11] = 1;
    }

  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
  }

  if (digitalRead(SW2_M)) {
    if (PrevStateSW1 != 2) {
      SettingBlinkTime = 1;
      PrevStateSW1 = 2;
      SettingPair = 1;
    }

    if (prev_sec != tm.Second) {
      SettingBlinkTime = millis();
    }

    if (((millis() - SettingBlinkTime) / SettingBlinkTimeConst) % 2 == 0) {
      if (SettingPair == 1) {
        tubeDigitData[0][tm.Hour / 10] = 0;
        tubeDigitData[1][tm.Hour % 10] = 0;
      }
      else if (SettingPair == 2) {
        tubeDigitData[3][tm.Minute / 10] = 0;
        tubeDigitData[4][tm.Minute % 10] = 0;
      }
      else {
        tubeDigitData[6][tm.Second / 10] = 0;
        tubeDigitData[7][tm.Second % 10] = 0;
      }
    }
  }
  prev_sec = tm.Second;
}


//prints the current Date DDMMYYYY
void printDATE()
{
  tmElements_t tm;

  if (RTC.read(tm)) {
    tubeDigitData[0][tm.Day / 10] = 1;
    tubeDigitData[1][tm.Day % 10] = 1;

    tubeDigitData[3][tm.Month / 10] = 1;
    tubeDigitData[4][tm.Month % 10] = 1;

    //tubeDigitData[4][tmYearToCalendar(tm.Year)/1000] = 1;
    //tubeDigitData[5][tmYearToCalendar(tm.Year)/100%10] = 1;
    tubeDigitData[6][tmYearToCalendar(tm.Year) / 10 % 10] = 1;
    tubeDigitData[7][tmYearToCalendar(tm.Year) % 10] = 1;

    tubeDigitData[2][10] = 1;
    tubeDigitData[5][10] = 1;


  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
  }

  if (digitalRead(SW2_M)) {
    if (PrevStateSW1 != 1) {
      SettingBlinkTime = 1;
      PrevStateSW1 = 1;
      SettingPair = 1;
    }

    if (prev_sec != tm.Second) {
      SettingBlinkTime = millis();
    }

    if (((millis() - SettingBlinkTime) / SettingBlinkTimeConst) % 2 == 0) {
      if (SettingPair == 1) {
        tubeDigitData[0][tm.Day / 10] = 0;
        tubeDigitData[1][tm.Day % 10] = 0;
      }
      else if (SettingPair == 2) {
        tubeDigitData[3][tm.Month / 10] = 0;
        tubeDigitData[4][tm.Month % 10] = 0;
      }
      else {
        tubeDigitData[6][tm.Year / 10] = 0;
        tubeDigitData[7][tm.Year % 10] = 0;
      }
    }
  }
  prev_sec = tm.Second;
}


void printDIVERGENCE()
{
  long tempDivergenceNum = DivergenceNum[CurrentDiv];

  for (int i = 7; i >= 0; i--) {
    if (i == 1) {
      tubeDigitData[i][11] = 1;
    }
    else {
      tubeDigitData[i][tempDivergenceNum % 10] = 1;
    }
    tempDivergenceNum = tempDivergenceNum / 10;
  }


}


//----------------------------------
//----------BASE FUNCTION-----------
//----------------------------------


//This function is good
//uses tube digit data to update
void tdd_updateShiftRegister() {

  bool Temp0DataSwap; //swapping of 0 digit and Rdot

  for (int i = 0; i < 8; i++) {
    Temp0DataSwap = tubeDigitData[i][0];
    tubeDigitData[i][0] = tubeDigitData[i][10];
    tubeDigitData[i][10] = Temp0DataSwap;
  }

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 12; j++) {
      int count = 12 * i + j;
      // Serial.println(count);
      FinalStringData[count] = tubeDigitData[i][j];
    }
  }
  //Serial.println("Binary");
  for (int i = 0; i < 96; i++) {
    //Serial.print(FinalStringData[i]);
  }
  //Serial.println("");

  if (FinalStringData != PrevFinalStringData) {
    digitalWrite(latchPin, LOW);
    for (int i = 0; i < 12; i++) {
      byte temp8Bits = 0; //8 bits
      for (int j = 0; j < 8; j++) {
        bitWrite(temp8Bits, 7 - j, FinalStringData[i * 8 + j]);
      }
      shiftOut(dataPin, clockPin, MSBFIRST, temp8Bits);
      //Serial.print(temp8Bits, BIN);
      //Serial.print(" ");
    }
    digitalWrite(latchPin, HIGH);
    //Serial.println("");
  }
  memcpy(PrevFinalStringData, FinalStringData, 96);
}


//resets data to all 0
void tdd_reset() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 12; j++) {
      tubeDigitData[i][j] = 0;
    }
  }
}



//----------------------------------
//----------TEST FUNCTION-----------
//----------------------------------


// prints 0,1,...,9, Ldot, Rdot
// all tubes print the same
void print0To9DotDotRolling() {
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 8; j++) {
      tubeDigitData[j][i] = 1;
    }
    tdd_updateShiftRegister();

    delay(1000);
    tdd_reset();
  }
}
