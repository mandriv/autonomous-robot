#include "motor_controller.h"
// Define states
#define LINE_TRACKING 0
//Define the tracking pin
#define leftSensor    A0
#define middleSensor  A1
#define rightSensor   13
// Moves
const String MOVE_FORWARD = "FORWARD";
const String MOVE_LEFT = "LEFT";
const String MOVE_RIGHT = "RIGHT";

MotorController motorController(3, 4, 1, 2);
int state = LINE_TRACKING;
String lastMove = MOVE_FORWARD;

// SETUP FUNCTION
void setup() {
  pinMode(leftSensor, INPUT_PULLUP);
  pinMode(middleSensor, INPUT_PULLUP);
  pinMode(rightSensor, INPUT_PULLUP);
  Serial.begin(115200);
}
// LOOP FUNCTION
void loop() {
  if (state == LINE_TRACKING) {
      moveTrack();
  }
}

void moveTrack(void)
{
  int left = digitalRead(leftSensor);
  int center = digitalRead(middleSensor);
  int right = digitalRead(rightSensor);

  if (center == 1) {
    lastMove = MOVE_FORWARD;
    motorController.moveForward();
    return;
  }
  if (left == 1 && center == 0) {
    lastMove = MOVE_LEFT;
    motorController.turnLeft();
    return;
  }
  if (right == 1 && center == 0) {
    lastMove = MOVE_RIGHT;
    motorController.turnRight();
    return;
  }
  if (left == 0 && right == 0) {
    if (lastMove == MOVE_FORWARD) {
      motorController.stop();
      return;
    }
    if (lastMove == MOVE_LEFT) {
      motorController.turnLeft();
      return;
    }
    motorController.turnRight();
  }
}
