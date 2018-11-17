#include "motor_controller.h"
#include <Servo.h>
//Define pins
#define LEFT_TRACKING_SENSOR A0
#define MIDDLE_TRACKING_SENSOR A1
#define RIGHT_TRACKING_SENSOR 13
#define ULTRASOUND_TRIGGER A2
#define ULTRASOUND_ECHO A3
// States contants
const String LINE_TRACKING = "LINE_TRACKING";
const String TURNING_RIGHT_90_DEG = "TURNING_RIGHT_90_DEG";
const String TURNING_LEFT_90_DEG = "TURNING_LEFT_90_DEG";
const String MOVING_FORWARD_TIMED = "MOVING_FORWARD_TIMED";
const String MOVING_FORWARD_TIMED_2 = "MOVING_FORWARD_TIMED_2";
const String OBSTACLE_AVOIDANCE_MOVE_ALONG = "OBSTACLE_AVOIDANCE_MOVE_ALONG";
// Moves contants
const String MOVE_FORWARD = "FORWARD";
const String MOVE_LEFT = "LEFT";
const String MOVE_RIGHT = "RIGHT";
const String MOVE_STOP = "STOP";
// Servo angles constants
const int SERVO_FORWARD_ANGLE = 90;
const int SERVO_LEFT_ANGLE = 180;
const int SERVO_RIGHT_ANGLE = 0;
// Time to turn 90 degrees in ms
const unsigned long TURN_90_TIME = 610;
const unsigned long MOVING_FORWARD_TIME = 300;
// Initialize variables
MotorController motorController(3, 4, 1, 2);
String state = LINE_TRACKING;
String lastMove = MOVE_FORWARD;
Servo distanceSensorNeck;
unsigned long manouverStartTime;

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
  // int distanceCm = readPing();
  // Serial.println(distanceCm);
  if (state == LINE_TRACKING) {
    int distanceCm = readPing();
    if (distanceCm > 15) {
      // safe to roll
      moveTrack();
    } else {
      // start turning
      motorController.stop();
      manouverStartTime = millis();
      distanceSensorNeck.write(SERVO_LEFT_ANGLE);
      state = TURNING_RIGHT_90_DEG;
    }
    return;
  }
  if (state == TURNING_RIGHT_90_DEG) {
    unsigned long currentTime = millis();
    if ((currentTime - manouverStartTime) < TURN_90_TIME) {
      motorController.turnRight();
    }
    else {
      // we turned 90 degrees, time to move along
      motorController.stop();
      state = OBSTACLE_AVOIDANCE_MOVE_ALONG;
    }
    return;
  }
  if (state == OBSTACLE_AVOIDANCE_MOVE_ALONG) {
    int center = digitalRead(MIDDLE_TRACKING_SENSOR);
    if (center == 1) {
      motorController.stop();
      state = LINE_TRACKING;
      return;
    }
    int distanceCm = readPing();
    if (distanceCm < 15) {
      motorController.turnRight();
    } else if (distanceCm < 30) {
      motorController.moveForward(100);
    } else {
      // turn 90 left
      motorController.stop();
      manouverStartTime = millis();
      state = MOVING_FORWARD_TIMED;
    }
    return;
  }
  if (state == MOVING_FORWARD_TIMED) {
    int center = digitalRead(MIDDLE_TRACKING_SENSOR);
    if (center == 1) {
      lastMove = MOVE_RIGHT;
      motorController.turnRight();
      state = LINE_TRACKING;
      return;
    }
    int distanceCm = readPing();
    if (distanceCm < 30) {
      motorController.stop();
      state = OBSTACLE_AVOIDANCE_MOVE_ALONG;
      return;
    }
    unsigned long currentTime = millis();
    if ((currentTime - manouverStartTime) < MOVING_FORWARD_TIME) {
      motorController.moveForward();
    }
    else {
      // we moved forward a bit to make room for turn
      motorController.stop();
      manouverStartTime = millis();
      state = TURNING_LEFT_90_DEG;
    }
    return;
  }
  if (state == TURNING_LEFT_90_DEG) {
    int distanceCm = readPing();
    if (distanceCm < 30) {
      motorController.stop();
      state = OBSTACLE_AVOIDANCE_MOVE_ALONG;
      return;
    }
    unsigned long currentTime = millis();
    if ((currentTime - manouverStartTime) < TURN_90_TIME) {
      motorController.turnLeft();
    }
    else {
      // we turned 90 degrees, time to move along
      motorController.stop();
      manouverStartTime = millis();
      state = MOVING_FORWARD_TIMED_2;
    }
    return;
  }
  if (state == MOVING_FORWARD_TIMED_2) {
    int center = digitalRead(MIDDLE_TRACKING_SENSOR);
    if (center == 1) {
      lastMove = MOVE_RIGHT;
      motorController.turnRight();
      state = LINE_TRACKING;
      return;
    }
    int distanceCm = readPing();
    if (distanceCm < 30) {
      motorController.stop();
      state = OBSTACLE_AVOIDANCE_MOVE_ALONG;
      return;
    }
    unsigned long currentTime = millis();
    if ((currentTime - manouverStartTime) < MOVING_FORWARD_TIME) {
      motorController.moveForward(100);
    }
    else {
      // we moved forward a bit to make room for turn
      motorController.stop();
      state = OBSTACLE_AVOIDANCE_MOVE_ALONG;
    }
    return;
  }
}

int readPing() {
  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(ULTRASOUND_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASOUND_TRIGGER, HIGH);
  delayMicroseconds(5);
  digitalWrite(ULTRASOUND_TRIGGER, LOW);
  long duration = pulseIn(ULTRASOUND_ECHO, HIGH);
  // convert the time into a distance
  long cm = microsecondsToCentimeters(duration);
  return cm;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

void moveTrack(void) {
  int left = digitalRead(LEFT_TRACKING_SENSOR);
  int center = digitalRead(MIDDLE_TRACKING_SENSOR);
  int right = digitalRead(RIGHT_TRACKING_SENSOR);
  distanceSensorNeck.write(SERVO_FORWARD_ANGLE);

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
      lastMove = MOVE_STOP;
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
