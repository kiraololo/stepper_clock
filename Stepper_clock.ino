#include <Stepper.h>
#include <microDS3231.h>

// Пин подающий сигнал 
// на пин step драйвера
// (делает заданный шаг на двигателе)
#define STEP_PIN 2
// Пин подающий сигнал 
// на пин dir драйвера
// (отвечает за направление вращения двигателя)
#define DIR_PIN 3
// Пин, к которому подключен светодиод на плате
// Будем мигать им в такт секундам для визуального контроля
#define BOARD_LED 13
// Пины контроля микрошага
#define MS1 7
#define MS2 8
#define MS3 9

// Угол поворота вала за один шаг
// для самых распространенных двигателей- это 
// значение составляет 1.8 градуса
#define step_angle 1.8
// Коэффициент поправки времени тика (в милисекундах)
// зависящий от механических свойств мотора, подбирается имперически
#define mechanical_time_coeff 5
// Инвертировать направление движения мотора
#define reverse false

// Считаем скорость мотора в шагах в секунду
float motorSpeed = 200 / 60;
// Предидущая секунда
word prevSecond = 0;
// Флаг первой итерации программы
boolean isFirstTime = true;
// Флаг включения индикатора
boolean ledOn = false;
// Время последнего включения индикатора
unsigned long ledOnTime;

// Инициализируем мотор (по умолчанию 200 шагов)
Stepper stepper(200, STEP_PIN, DIR_PIN);
MicroDS3231 rtc;

void setup() {
  Serial.begin(9600);
  // Инициализируем светодиод на плате
  pinMode(BOARD_LED, OUTPUT);

  word microStep = 1;
  if(digitalRead(MS1) == 1) {
    microStep *= 4;
  }
  if(digitalRead(MS2) == 1) {
    microStep *= 4;
  }
  if(digitalRead(MS3) == 1) {
    microStep *= 4;
  }

  // Устанавливаем скорость
  // Считаем кол-во шагов с учетом микрошага и угла, которое мотор 
  // может сделать за 1 оборот
  float motorStepsCount = (360 / step_angle) * microStep;
  motorSpeed = motorStepsCount / 60;
  stepper = Stepper(motorStepsCount, STEP_PIN, DIR_PIN);

  stepper.setSpeed(motorSpeed);
  // проверка наличия модуля на линии i2c
  if (!rtc.begin()) {
    Serial.println("DS3231 not found");
    digitalWrite(BOARD_LED, HIGH);
    ledOn = true;
    for(;;);
  }
}

void turnOnLed() {
  digitalWrite(BOARD_LED, HIGH);
  ledOn = true;
  ledOnTime = millis();
}

void turnOffLed() {
  if(ledOn && millis() - ledOnTime > 100) {
    ledOn = false;
    digitalWrite(BOARD_LED, LOW);
  }
}

// Метод движения мотора
void doStep() {
  DateTime now = rtc.getTime();
  if(isFirstTime) {
    prevSecond = now.second;
    isFirstTime = false;
  }
  if(now.second - prevSecond != 0) {
    prevSecond = now.second;
    if(reverse == true) {
      stepper.step(-motorSpeed);
    }
    else {
      stepper.step(motorSpeed);
    }
    turnOnLed();
  }
}


void loop() {
  doStep();
  turnOffLed();
}
