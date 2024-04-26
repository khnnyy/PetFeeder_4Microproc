#include <DS3231.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 2, 3, 4, 5 };
byte colPins[COLS] = { 6, 7, 8, 9 };

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

DS3231 rtc(A4, A5);
Servo servo_test;
LiquidCrystal_I2C lcd(0x27, 16, 2);

boolean feed = false;
boolean feedingTimeSet = false;  
char key;
int feedingTime[4][2] = { { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 } };  
const char* mealNames[4] = { "Breakfast", "Lunch", "Dinner", "Snack" };
int feedingAmount[4] = { 1, 1, 1, 1 };  
int currentFeedingIndex = 0;
int currentAmountIndex = 0;

void setup() {
  servo_test.attach(12);
  rtc.begin();
  lcd.begin();
  lcd.backlight();
  servo_test.write(0);
  Serial.begin(9600);
}

void loop() {
  char key = kpd.getKey();
  if (key == 'A') {
    Serial.println(key);
    setFeedingTime();
  } else if (key == 'B') {
    Serial.println(key);
    viewFeeding();
  } else if (key == 'D') {
    Serial.println(key);
    manualfeed();
  } else {
    nextFeed();
    checkFeedingTime();
  }
}

void setFeedingTime() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Meal:");
  lcd.setCursor(0, 1);
  lcd.print(mealNames[currentFeedingIndex]);

  while (true) {
    char key = kpd.getKey();
    if (key != NO_KEY) {
      if (key == '#') {
        currentFeedingIndex = (currentFeedingIndex + 1) % 4;  
        lcd.setCursor(0, 1);
        lcd.print("                ");  
        lcd.setCursor(0, 1);
        lcd.print(mealNames[currentFeedingIndex]);  
      } else if (key == '*') {
        currentFeedingIndex = (currentFeedingIndex + 3) % 4; 
        lcd.setCursor(0, 1);
        lcd.print("                ");  
        lcd.setCursor(0, 1);
        lcd.print(mealNames[currentFeedingIndex]);  
      } else if (key == 'A') {
        setFeedingAmount(currentFeedingIndex);  
        break;
      }
    }
    delay(200); 
  }
}

void setFeedingTimeForMeal(int mealIndex) {
  while (true) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(mealNames[mealIndex]);
    lcd.print(": ^._.^ ");
    lcd.setCursor(0, 1);
    lcd.print("HH:MM");

    int hour = 0;
    int minute = 0;
    int j = 0;

    while (true) {
      key = kpd.getKey();
  
      if (key != NO_KEY) {
        lcd.setCursor(j, 1);
          if (key != 'B' && key != 'C' && key != 'D' && key != '*') {
        if (key != ':' && j == 2) {
          lcd.print(':');
          j++;
        }
        lcd.print(key);
        if (key != ':' && j < 5) {
          if (j % 3 == 0) {
            feedingTime[mealIndex][j / 3] = (key - '0') * 10;
          } else if (j % 3 == 1) {
            feedingTime[mealIndex][j / 3] += (key - '0');
          }
          j++;
        }if (j == 5) { 
        lcd.setCursor(4, 1); 
        lcd.noCursor(); 
      }
      }
      }
      if (key == 'A') {
        if (feedingTime[mealIndex][0] >= 0 && feedingTime[mealIndex][0] < 24 && feedingTime[mealIndex][1] >= 0 && feedingTime[mealIndex][1] < 60) {
          Serial.print("Feeding time set for ");
          Serial.print(mealNames[mealIndex]);
          Serial.print(" at ");
          Serial.print(feedingTime[mealIndex][0]);
          Serial.print(":");
          Serial.println(feedingTime[mealIndex][1]);
          feedingTimeSet = true; 
          lcd.clear();
          return;   
        } else {
          lcd.setCursor(0, 1);
          lcd.print("Invalid Input!");
          delay(2500); 
          break;      
        } 
      }
      else if (key == '#'){
          break;
        }
    }
  }
}


void setFeedingAmount(int amountIndex) {
  currentAmountIndex = amountIndex;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(mealNames[amountIndex]);
  lcd.print(": ^._.^ ");
  lcd.setCursor(0, 1);
  lcd.print("Portions:");
  lcd.print(feedingAmount[amountIndex]);

  while (true) {
    key = kpd.getKey();
    if (key != NO_KEY) {
      if (key == '#') {
        feedingAmount[amountIndex]++;  
        if (feedingAmount[amountIndex] > 3) {
          feedingAmount[amountIndex] = 1;  
        }
        lcd.setCursor(9, 1);
        lcd.print(" ");  
        lcd.setCursor(9, 1);
        lcd.print(feedingAmount[amountIndex]);
        delay(200);                            
      } else if (key == '*') {
        feedingAmount[amountIndex]--; 
        if (feedingAmount[amountIndex] < 1) {
          feedingAmount[amountIndex] = 3;  
        }
        lcd.setCursor(9, 1);
        lcd.print(" ");  
        lcd.setCursor(9, 1);
        lcd.print(feedingAmount[amountIndex]);   
        delay(200);                            
      } else if (key == 'A') {
        setFeedingTimeForMeal(amountIndex);
        return; 
      }
    }
  }
}

void viewFeeding() {
  int currentIndex = 0;
  char key = NO_KEY;

  lcd.clear();

  while (true) {
    lcd.setCursor(0, 0);
    lcd.print(mealNames[currentIndex]);
    lcd.print(": ^._.^ ");
    lcd.setCursor(0, 1);

    if (feedingTime[currentIndex][0] == -1 || feedingTime[currentIndex][1] == -1) {
      lcd.print("No feeding time.");
    } else {
      lcd.print("Portions:");
      lcd.print(feedingAmount[currentIndex]);
      lcd.print("|");
      lcd.print(feedingTime[currentIndex][0]);
      lcd.print(":");
      if (feedingTime[currentIndex][1] < 10) {
        lcd.print("0");
      }
      lcd.print(feedingTime[currentIndex][1]);
    }

    delay(200);  

    key = kpd.getKey();

    if (key == 'A') {
      lcd.clear();
      delay(100);  
      break;     
    } else if (key == '*') {
      currentIndex = (currentIndex + 3) % 4;  
      lcd.clear();                            
    } else if (key == '#') {
      currentIndex = (currentIndex + 1) % 4;  
      lcd.clear();                           
    }
  }
}

boolean feedingTimeTriggered[4] = { false, false, false, false }; 

void checkFeedingTime() {
  Time currentTime = rtc.getTime();

  if (!feed && feedingTimeSet) {
    for (int i = 0; i < 4; i++) {
      if (!feedingTimeTriggered[i] && currentTime.hour == feedingTime[i][0] && currentTime.min == feedingTime[i][1]) {
        feed = true;
        dispenseFood();
        delay(1000);                    
        feedingTimeTriggered[i] = true;  
        return;                       
      }
    }
  }
}

void nextFeed() {
  lcd.setCursor(0, 0);
  lcd.print("Time:  ");
  lcd.print(rtc.getTimeStr());

  Time currentTime = rtc.getTime();
  int nextMealIndex = -1;
  int nextMealHour = -1;
  int nextMealMinute = -1;

  for (int i = 0; i < 4; i++) {
    if (feedingTime[i][0] != -1 && feedingTime[i][1] != -1) {
      if (nextMealIndex == -1 || (feedingTime[i][0] < nextMealHour) || (feedingTime[i][0] == nextMealHour && feedingTime[i][1] < nextMealMinute)) {
        if ((feedingTime[i][0] > currentTime.hour) || (feedingTime[i][0] == currentTime.hour && feedingTime[i][1] > currentTime.min)) {
          nextMealIndex = i;
          nextMealHour = feedingTime[i][0];
          nextMealMinute = feedingTime[i][1];
        }
      }
    }
  }

  if (nextMealIndex != -1) {
    lcd.setCursor(0, 1);
    lcd.print(mealNames[nextMealIndex]);
    lcd.print(": ");
    lcd.print(nextMealHour);
    lcd.print(":");
    if (nextMealMinute < 10) {
      lcd.print("0");
    }
    lcd.print(nextMealMinute);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("No next meal.");
  }
}

void manualfeed() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Toggle D!=^._.^=");
  lcd.setCursor(0, 1);

  char key;
  bool isDispensing = false;  

  while (true) {
    key = kpd.getKey();
    if (key == 'B') {
      if (!isDispensing) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Toggle D!=^._.^=");
        lcd.setCursor(0, 1);
        lcd.print("Dispensing...");
        servo_test.write(45);
        delay(1);         
        isDispensing = true; 
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Toggle D!=^._.^=");
        lcd.setCursor(0, 1);
        lcd.print("Not Dispensing.");
        servo_test.write(0);
        delay(1);          
        isDispensing = false;  
      }
    } else if (key == 'A') {
      lcd.clear();
      break;
    }
  }
}


void dispenseFood() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dispensing...");

  int portions = feedingAmount[currentAmountIndex];
  int delayS = 0;

  switch (portions) {
    case 1:
      delayS = 500;
      break;
    case 2:
      delayS = 1000;
      break;
    case 3:
      delayS = 1500;
      break;
    default:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Invalid Portions");
      delay(2000);
      return;
  }

  Serial.print("Dispensing ");
  Serial.print(portions);
  Serial.print(" portions with delay of ");
  Serial.print(delayS);
  Serial.println(" milliseconds");

  servo_test.write(45);
  delay(delayS);
  servo_test.write(0);
  delay(1000);

  feed = false;
}

void displayDateTime() {
  lcd.setCursor(0, 0);
  lcd.print("Time:  ");
  lcd.print(rtc.getTimeStr());

  lcd.setCursor(0, 1);
  lcd.print("Date: ");
  lcd.print(rtc.getDateStr());

  delay(200);
}
