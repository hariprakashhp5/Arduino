#include<LiquidCrystal.h>

const int potPin = A0;          // Potentiometer pin.
const int backlightPin = 10;    // LCD backlight pin.
const int holdBtn = 12;         // CountDown Time set pin.
const int trgrBtn = 8;          // Start count pin.
const int buzzerPin = 9;        // Buzzer power pin
int holdBtnValue = 0;           
int trgrBtnValue = 0;
int potValue = 0;
int countValue = 0;
const int rs=7, en=6, d4=5, d5=4, d6=3, d7=2;
unsigned long Watch, _micro, time = micros();
unsigned int Clock = 0, R_clock;
boolean Reset = false, Stop = false, Paused = false;
volatile boolean timeFlag = false;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  pinMode(holdBtn, INPUT);
  pinMode(trgrBtn, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(backlightPin, OUTPUT);
  analogWrite(backlightPin,255);
  lcd.begin(16,2);
  getCount(0);
  SetTimer(countValue);
  StartTimer();
  Serial.begin(9600);
}

void loop() {
  lcd.setCursor(0,0);
  holdBtnValue=digitalRead(holdBtn);
  trgrBtnValue=digitalRead(trgrBtn);
  
  if(holdBtnValue == HIGH){
    lcd.clear();
    getCount(1);
    SetTimer(countValue);
  }else{
    if(trgrBtnValue == HIGH){
      CountDownTimer();
      if (TimeHasChanged())
            printTime();

      if (Clock==0)
          startBeeping();
           
    }else{
      stopBeeping();
      ResetTimer();
      setCursorAndPrint(8,1, "00:00:00");      
    }
  }
}

void startBeeping(){
  digitalWrite(buzzerPin,LOW);
  delay(300);
  digitalWrite(buzzerPin,HIGH);
  delay(300);
}

void stopBeeping(){
  digitalWrite(buzzerPin,LOW);
}

void setCursorAndPrint(int col, int row, String string){
//  lcd.clear();
  lcd.setCursor(col, row);
  lcd.print(string);
  delay(200);
}

void printTime(){
    String c_time= (String(ShowHours())+":"+String(ShowMinutes())+":"+String(ShowSeconds()));
    lcd.setCursor(8,1);
    lcd.print(c_time);
}

void getCount(int from){
    if(from!=0){
      lcd.print("Set Value: ");
    }
    potValue = analogRead(potPin);
    countValue = map(potValue, 0, 1023, 0, 300);
    lcd.setCursor(0,1);
    lcd.print(countValue);
    delay(300);
    lcd.clear();
    printName();
}

void printName(){
  lcd.setCursor(0,0);
  lcd.print("TIMER:"); 
}

boolean CountDownTimer()
{
  static unsigned long duration = 1000000; // 1 second
  timeFlag = false;

  if (!Stop && !Paused) // if not Stopped or Paused, run timer
  {
    // check the time difference and see if 1 second has elapsed
    if ((_micro = micros()) - time > duration ) 
    {
      Clock--;
      timeFlag = true;

      if (Clock == 0){ // check to see if the clock is 0
        Stop = true; // If so, stop the timer
        // ResetTimer();
      }

     // check to see if micros() has rolled over, if not,
     // then increment "time" by duration
      _micro < time ? time = _micro : time += duration; 
    }
  }
  return !Stop; // return the state of the timer
}

void ResetTimer()
{
  SetTimer(R_clock);
  Stop = false;
  StartTimer();
}

void StartTimer()
{
  Watch = micros(); // get the initial microseconds at the start of the timer
  time = micros(); // hwd added so timer will reset if stopped and then started
  Stop = false;
  Paused = false;
}

void SetTimer(unsigned int seconds)
{
 // StartTimer(seconds / 3600, (seconds / 3600) / 60, seconds % 60);
 Clock = seconds;
 R_clock = Clock;
 Stop = false;
}

char* ShowHours()
{
  return format(Clock / 3600);
}

char* ShowMinutes()
{
  return format((Clock / 60) % 60);
}

char* ShowSeconds()
{
  return format(Clock % 60);
}

char* format(int value){
  char buffer[4];
  sprintf(buffer, "%02d", value);
  return buffer;
}

boolean TimeHasChanged()
{
  return timeFlag;
}
