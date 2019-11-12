#include <TinyOzOLED.h>
#include <TinyWireM.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <Bounce2.h>
#include <EEPROM.h>

#define BUTTON_PIN_1 3
#define BUTTON_PIN_2 A2

#define DL_ADDR 0x00

#define CHANNEL_UP 0x986730CF
#define CHANNEL_DOWN 0x98678877

IRsend irsend;
int selectedOption = -1;
int delayTime;

unsigned long interval = 1000;
unsigned long previousMillis = 0;

bool scanUp = false;
bool scanDown = false;
bool delayMenu = false;

long lastDebounceTime = 0;
long debounceDelay = 100;

Bounce btnEnter = Bounce(BUTTON_PIN_1, 5); 

void setup() {
  OSCCAL = 0x4E;

  pinMode(BUTTON_PIN_1,INPUT);
  pinMode(BUTTON_PIN_2,INPUT);

  delayTime = EEPROM.read(DL_ADDR);
  delay(10);

  if (delayTime > 60) {
    OzOled.printNumber(delayTime, 13, 7);
    saveDelayTime(1);
  }

  OzOled.init();
  OzOled.clearDisplay();
  OzOled.setNormalDisplay();

  OzOled.sendCommand(0xA1);
  OzOled.sendCommand(0xC8);

  OzOled.printString("CAMBIACANALES", 2, 1);
  OzOled.printString("Scan arriba", 3, 3);
  OzOled.printString("Scan abajo", 3, 4);
  OzOled.printString("Detener scan", 3, 5);
  OzOled.printString("Delay", 3, 7);
  OzOled.printString("s", 15, 7);

  showDelayTime();
  changeOption(true);
}

void loop() {
  unsigned long currentMillis = millis();
  btnEnter.update();
  int btnRead = analogRead(BUTTON_PIN_2);

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (btnRead < 600 && btnRead > 100) {
      if (delayMenu)
        changeDelayValue(true);
      else
        changeOption(true);    
    }
    else if (btnRead > 1000) {
      if (delayMenu)
        changeDelayValue(false);
      else
        changeOption(false);    
    }
    lastDebounceTime = millis();
  }

  if (btnEnter.fell()) {
    optionSelect();
  }
  if (scanUp || scanDown) {
    if ((unsigned long)(currentMillis - previousMillis) >= interval * delayTime) {
      if (scanUp) irsend.sendNEC(CHANNEL_UP,32);
      if (scanDown) irsend.sendNEC(CHANNEL_DOWN,32);
      previousMillis = millis();
    }
  }
}

void changeOption(bool optUp) {
  int options[4] = {3, 4, 5, 7};

  OzOled.printString(" ", 1, options[selectedOption]);

  if (optUp) {
    selectedOption++;
  }
  else {
    selectedOption--;
  }
  if (selectedOption == 4)
    selectedOption = 0;
  if (selectedOption == -1)
    selectedOption = 3;

  OzOled.printString("x", 1, options[selectedOption]);
}

void changeDelayValue(bool addSeconds) {
  if (addSeconds) {
    if (delayTime < 60) delayTime++;
  }
  else {
    if (delayTime > 0) delayTime--;
  }
  showDelayTime();
}

void optionSelect() {
  switch (selectedOption) {
    case 0:
      scanUp = true;
      scanDown = false;
    break;
    case 1:
      scanUp = false;
      scanDown = true;
    break;
    case 2:
      scanUp = false;
      scanDown = false;
    break;
    case 3:
      if (delayMenu) saveDelayTime(delayTime);
      delayMenu = !delayMenu;
    break;
  }
}

void saveDelayTime(int seconds) {
  EEPROM.write(DL_ADDR, seconds);
  delay(10); 
}

void showDelayTime() {
  OzOled.setCursorXY(13, 7);
  if (delayTime < 10) {
    OzOled.printString(" ");  
  }
  OzOled.printNumber(delayTime);
}
