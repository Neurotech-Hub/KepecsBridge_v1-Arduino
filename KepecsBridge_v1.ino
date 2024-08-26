#include "Stepper.h"

#define MAX_LINEAR_STEPS 10000
#define LIMIT_PIN 2
#define MOVE_TREAD 1000
#define MOVE_LINEAR 10000
#define LINEAR_SPEED 500
#define TREAD_SPEED 50

#define LINEAR_EN_A A0
#define LINEAR_EN_B A1
#define TREAD_EN_A A2
#define TREAD_EN_B A3

// Define number of steps per revolution:
const int stepsPerRevolution = 200;

// Initialize the stepper library on pins 8 through 11:
Stepper linearStepper = Stepper(stepsPerRevolution, 8, 9, 10, 11);
Stepper treadStepper = Stepper(stepsPerRevolution, 4, 5, 6, 7);

bool initLinearMotor() {
  bool success = true;
  enableMotors();
  if (digitalRead(LIMIT_PIN) == LOW) {
    Serial.println("moving motor up...");
    linearStepper.step(-2000);  // clear the limit switch
    delay(2000);
  }
  int stepBy = 200;
  int stepCount = 0;
  while(digitalRead(LIMIT_PIN) == HIGH) {
    Serial.print("moving motor down - ");
    Serial.println(stepCount);
    linearStepper.step(stepBy);
    stepCount += stepBy;
    if (stepCount > MAX_LINEAR_STEPS + 500) {
      success = false;
      break;
    }
    killMotorIO();
  }
  disableMotors();
  killMotorIO();
  return success;
}

void setup() {
  pinMode(LINEAR_EN_A, OUTPUT);
  pinMode(LINEAR_EN_B, OUTPUT);
  pinMode(TREAD_EN_A, OUTPUT);
  pinMode(TREAD_EN_B, OUTPUT);

  pinMode(LIMIT_PIN, INPUT_PULLUP);

  // Set the motor speed (RPMs):
  linearStepper.setSpeed(LINEAR_SPEED);
  treadStepper.setSpeed(TREAD_SPEED);

  Serial.begin(9600);
  delay(3000);

  // if (!initLinearMotor()) {
  //   Serial.println("Could not initialize linear motor.");
  //   while (1) {
  //   }
  // }
  // use interrupt to turn off motors from now on
  attachInterrupt(digitalPinToInterrupt(LIMIT_PIN), limitSwitchISR, FALLING);

  Serial.println("Ready for commands.");
}

// !! need stepper tracker to make sure it never exceeds ~MAX_LINEAR_STEPS
void loop() {
  if (Serial.available() > 0) {
    enableMotors();
    int steps = Serial.parseInt();  // Read the number of steps from serial input
    if (abs(steps) == 1) {
      if (steps == 1) {
        Serial.println("Moving tread right.");
        treadStepper.step(MOVE_TREAD);
        killMotorIO();
        linearStepper.step(-MOVE_LINEAR);  // inverted
      } else {
        Serial.println("Moving tread left.");
        treadStepper.step(-MOVE_TREAD);
        killMotorIO();
        // safety condition
        if (digitalRead(LIMIT_PIN) == HIGH) {
            linearStepper.step(MOVE_LINEAR);  // inverted
        }
      }
    } else {
      Serial.print("Moving linear ");
      Serial.print(-steps);
      Serial.println(" steps.");
      linearStepper.step(-steps);
    }
    Serial.println("Motor stopped. Enter the number of steps again:");
  }
  killMotorIO();
}

void disableMotors() {
  digitalWrite(LINEAR_EN_A, LOW);
  digitalWrite(LINEAR_EN_B, LOW);
  digitalWrite(TREAD_EN_A, LOW);
  digitalWrite(TREAD_EN_B, LOW);
}

void enableMotors() {
  digitalWrite(LINEAR_EN_A, HIGH);
  digitalWrite(LINEAR_EN_B, HIGH);
  digitalWrite(TREAD_EN_A, HIGH);
  digitalWrite(TREAD_EN_B, HIGH);
}

void killMotorIO() {
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
}

void limitSwitchISR() {
  disableMotors();
  killMotorIO();
  Serial.println("Interrupt encountered.");
}
