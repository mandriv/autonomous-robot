#include <Arduino.h>
#include <Servo.h>
#include "motor_controller.h"
#include "ultrasound_sensor_controller.h"
//Define pins
#define LEFT_TRACKING_SENSOR A0
#define MIDDLE_TRACKING_SENSOR A1
#define RIGHT_TRACKING_SENSOR 13
#define ULTRASOUND_TRIGGER A2
#define ULTRASOUND_ECHO A3
// States contants
const String LINE_TRACKING = "LINE_TRACKING";
const String OBSTACLE_AVOIDANCE = "OBSTACLE_AVOIDANCE";
const String RIGHT_TURN = "RIGHT_TURN";
const String END_RUN = "END_RUN";
// Servo angles constants
const unsigned short int SERVO_FULL_LEFT_ANGLE = 180;
const unsigned short int SERVO_LEFT_30_ANGLE = 150;
const unsigned short int SERVO_LEFT_60_ANGLE = 120;
const unsigned short int SERVO_FORWARD_ANGLE = 90;
const unsigned short int SERVO_RIGHT_60_ANGLE = 60;
const unsigned short int SERVO_RIGHT_30_ANGLE = 30;
const unsigned short int SERVO_FULL_RIGHT_ANGLE = 0;
// Distances contants
const unsigned int STOP_DISTANCE = 15;
const unsigned int DISTANCE_DIFFERENCE_FROM_INITIAL = 2;
const unsigned int MAX_STAY_DISTANCE_FROM_OBSTACLE = 40;
// Turn time constants
const unsigned long MAX_LOST_TIME = 1000;
const unsigned short int HALF_TURN_TIME_FULL_SPEED = 450;
const unsigned short int FORWARD_BEFORE_LEFT_TURN_TIME = 200;
const unsigned short int FORWARD_AFTER_LEFT_TURN_TIME = 150;
const unsigned short int SERVO_TURN_TIME = 300;
const unsigned short int INITIAL_DISTANCE_READ_TIME = 300;
const unsigned short int BACKWARD_AFTER_LINE_FOUND = 150;
const unsigned short int RIGHT_TURN_AFTER_LINE_FOUND = 200;
const unsigned short int TURN_TO_FIND_LINE_TIME = 787;
// Speed constants
const unsigned short int TURN_TO_FIND_LINE_SPEED = 180;
// Initialize variables
MotorController motorController(3, 4, 1, 2);
String state = LINE_TRACKING;
Servo distanceSensorNeck;
unsigned long timerStart = 0;
boolean isLost = false;
unsigned int initialDistance;
boolean isTurning = false;
boolean isTurningToFindLine = false;
int lastLeft = 0;
int lastCenter = 0;
int lastRight = 0;

void moveTrack(void) {
  distanceSensorNeck.write(SERVO_FORWARD_ANGLE);
  unsigned int distance = readPing(ULTRASOUND_TRIGGER, ULTRASOUND_ECHO);
  if (distance < STOP_DISTANCE) {
    motorController.stop();
    state = RIGHT_TURN;
    return;
  }

  int left = digitalRead(LEFT_TRACKING_SENSOR);
  int center = digitalRead(MIDDLE_TRACKING_SENSOR);
  int right = digitalRead(RIGHT_TRACKING_SENSOR);
  if (left == 1 || center == 1 || right == 1) {
    isLost = false;
    lastLeft = left;
    lastCenter = center;
    lastRight = right;
  }

  if (center == 1) {
    motorController.moveForward();
    return;
  }
  if (left == 1 && center == 0) {
    motorController.turnLeft();
    return;
  }
  if (right == 1 && center == 0) {
    motorController.turnRight();
    return;
  }
  if (left == 0 && right == 0) {
    // got lost, start timer
    if (!isLost) {
      isLost = true;
      timerStart = millis();
    }
    if (isLost) {
      unsigned long currentTime = millis();
      unsigned long timeLost = currentTime - timerStart;
      // too long, stop
      if (timeLost > MAX_LOST_TIME) {
        state = END_RUN;
        isLost = false;
        return;
      }
      if (lastCenter == 1) {
        motorController.moveForward();
        return;
      }
      if (lastLeft == 1 && lastCenter == 0) {
        motorController.turnLeft();
        return;
      }
      if (lastRight == 1 && lastCenter == 0) {
        motorController.turnRight();
        return;
      }
    }
  }
}

void avoidObstacle(void) {
  // check if line found
  int left = digitalRead(LEFT_TRACKING_SENSOR);
  int center = digitalRead(MIDDLE_TRACKING_SENSOR);
  int right = digitalRead(RIGHT_TRACKING_SENSOR);
  if (left == 1 || center == 1 || right == 1) {
    lastLeft = left;
    lastCenter = center;
    lastRight = right;
    if (!isTurningToFindLine) {
      motorController.stop();
      distanceSensorNeck.write(SERVO_FORWARD_ANGLE);
      delay(SERVO_TURN_TIME);
      isTurningToFindLine = true;
      timerStart = millis();
      return;
    }
  }
  if (isTurningToFindLine) {
    unsigned long currentTime = millis();
    unsigned long timeTurning = currentTime - timerStart;
    if (timeTurning < TURN_TO_FIND_LINE_TIME) {
      motorController.turnRight(TURN_TO_FIND_LINE_SPEED);
      return;
    }
    isTurningToFindLine = false;
    isTurning = false;
    state = LINE_TRACKING;
    return;
  }

  // keep ultrasound sensor in the full left position
  distanceSensorNeck.write(SERVO_FULL_LEFT_ANGLE);
  // move based on distance from obstacle
  unsigned int distance = readPing(ULTRASOUND_TRIGGER, ULTRASOUND_ECHO);
  // obstacle not found - move forward, make a turn and move forward again
  if (!isTurning && distance > MAX_STAY_DISTANCE_FROM_OBSTACLE) {
    isTurning = true;
    timerStart = millis();
  }
  if (isTurning) {
    unsigned long currentTime = millis();
    unsigned long timeTurning = currentTime - timerStart;
    unsigned short int leftTurnTime =
      FORWARD_BEFORE_LEFT_TURN_TIME +
      HALF_TURN_TIME_FULL_SPEED;
    unsigned short int afterTurnForwardTime =
      FORWARD_BEFORE_LEFT_TURN_TIME +
      HALF_TURN_TIME_FULL_SPEED +
      FORWARD_BEFORE_LEFT_TURN_TIME;

    if (timeTurning < FORWARD_BEFORE_LEFT_TURN_TIME) {
      motorController.moveForward();
      return;
    } else if (timeTurning < leftTurnTime) {
      motorController.turnLeft();
      return;
    } else if (timeTurning < afterTurnForwardTime) {
      motorController.moveForward();
      return;
    } else {
      isTurning = false;
      return;
    }
  }

  // obstacle found, keep initialDistance
  if (distance < (initialDistance - DISTANCE_DIFFERENCE_FROM_INITIAL)) {
    motorController.turnRightArc();
    return;
  } else if (distance > (initialDistance + DISTANCE_DIFFERENCE_FROM_INITIAL)) {
    motorController.turnLeftArc();
    return;
  } else {
    motorController.moveForward();
    return;
  }
}

void setup() {
  pinMode(LEFT_TRACKING_SENSOR, INPUT_PULLUP);
  pinMode(MIDDLE_TRACKING_SENSOR, INPUT_PULLUP);
  pinMode(RIGHT_TRACKING_SENSOR, INPUT_PULLUP);
  pinMode(ULTRASOUND_TRIGGER, OUTPUT);
  pinMode(ULTRASOUND_ECHO, INPUT);
  distanceSensorNeck.attach(10);
  Serial.begin(115200);
}

void loop() {
  if (state == LINE_TRACKING) {
    moveTrack();
    return;
  }
  if (state == RIGHT_TURN) {
    motorController.turnRight();
    delay(HALF_TURN_TIME_FULL_SPEED);
    motorController.stop();
    distanceSensorNeck.write(SERVO_FULL_LEFT_ANGLE);
    delay(SERVO_TURN_TIME);
    initialDistance = readPing(ULTRASOUND_TRIGGER, ULTRASOUND_ECHO);
    delay(INITIAL_DISTANCE_READ_TIME);
    state = OBSTACLE_AVOIDANCE;
    return;
  }
  if (state == OBSTACLE_AVOIDANCE) {
    avoidObstacle();
    return;
  }
  if (state == END_RUN) {
    motorController.stop();
  }
}
