#include <UCMotor.h>
#include <Servo.h>
#include <Wire.h>

#define TRIG_PIN A2
#define ECHO_PIN A3

//Define the turn distance
#define TURN_DIST 30

//Define the tracking pin
#define leftSensor    A0
#define middleSensor  A1
#define rightSensor   13
//Define the frequence of the beep
#define beepFrequence 50

//Define the turn time of the car
#define  turnTime    500
//You can change the TrackFactorUp and TrackFactorDown to adjust the value of tracking
//If the car deviate too far from the trail line,you can increase the value of TrackFactorUp and reduce the value of TrackFactorDown
//If the car is shaking very badly, even turn around,you'd better reduce the  value of the TrackFactorUp and increase the value of rackFactorDown
#define  TrackFactorUp     0.5
#define  TrackFactorDown   0.7

int buzzerPin = 2; //Beep pin
int MAX_SPEED_LEFT ;
int MAX_SPEED_RIGHT;
int SZ_SPEEDPRO = 0;
int SZ_SPEEDTHR = 150;

byte serialIn = 0;
byte commandAvailable = false;
String strReceived = "";

//The center Angle of two steering engines.
byte servoXCenterPoint = 88;
byte servoYCenterPoint = 70;

//The maximum Angle of two steering gear
byte servoXmax = 170;
byte servoYmax = 130;
//The minimum Angle of two steering gear
byte servoXmini = 10;
byte servoYmini = 10;

//The accuracy of servo
byte servoStep = 4;

//The current Angle of the two steering engines is used for retransmission
byte servoXPoint = 0;
byte servoYPoint = 0;

byte leftspeed = 0;
byte rightspeed = 0;

String motorSet = "";
int speedSet = 0;
int detecteVal = 0;
bool detected_flag = false;
bool timeFlag = true;
bool beepFlag = false;
bool trackFlag = false;
bool avoidFlag = false;
bool trackStopFlag = false;
bool avoidStopFlag = false;
long currentTime = 0;

UC_DCMotor leftMotor1(3, MOTOR34_64KHZ);
UC_DCMotor rightMotor1(4, MOTOR34_64KHZ);
UC_DCMotor leftMotor2(1, MOTOR34_64KHZ);
UC_DCMotor rightMotor2(2, MOTOR34_64KHZ);

Servo servoX;
Servo servoY;

String lastTurn = "";
int lostTime = 0;

// SETUP FUNCTION
void setup() {
  pinMode(leftSensor, INPUT_PULLUP);
  pinMode(middleSensor, INPUT_PULLUP);
  pinMode(rightSensor, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ECHO_PIN, INPUT); //Set the connection pin output mode Echo pin
  pinMode(TRIG_PIN, OUTPUT);//Set the connection pin output mode trog pin
  servoX.attach(9);
  servoY.attach(10);
  Serial.begin(115200);
  MAX_SPEED_LEFT = SZ_SPEEDTHR;
  MAX_SPEED_RIGHT = SZ_SPEEDTHR;
}
// LOOP FUNCTION
void loop() {
  trackFlag = true;
  trackStopFlag = false;
  moveTrack();
}


void trackturnLeft(void)
{
  static int MAX_SPEED_LEFT_A, MAX_SPEED_RIGHT_A;
  MAX_SPEED_RIGHT_A = MAX_SPEED_RIGHT > (255 - (int)(MAX_SPEED_RIGHT * TrackFactorUp)) ? 255 : MAX_SPEED_RIGHT + (int)(MAX_SPEED_RIGHT * TrackFactorUp);
  MAX_SPEED_LEFT_A  = (MAX_SPEED_LEFT - (int)(MAX_SPEED_LEFT * TrackFactorDown)) < 0 ? 0 : MAX_SPEED_LEFT - (int)(MAX_SPEED_LEFT * TrackFactorDown);

  motorSet = "LEFT";
  leftMotor1.run(BACKWARD);
  rightMotor1.run(FORWARD);
  leftMotor2.run(BACKWARD);
  rightMotor2.run(FORWARD);
  leftMotor1.setSpeed(MAX_SPEED_LEFT_A);
  rightMotor1.setSpeed(MAX_SPEED_RIGHT_A);
  leftMotor2.setSpeed(MAX_SPEED_LEFT_A);
  rightMotor2.setSpeed(MAX_SPEED_RIGHT_A);
}

void trackturnRight(void)
{
  static int MAX_SPEED_LEFT_A, MAX_SPEED_RIGHT_A;
  MAX_SPEED_LEFT_A = MAX_SPEED_LEFT > (255 - (int)(MAX_SPEED_LEFT * TrackFactorUp)) ? 255 : MAX_SPEED_LEFT + (int)(MAX_SPEED_LEFT * TrackFactorUp);
  MAX_SPEED_RIGHT_A  = (MAX_SPEED_RIGHT - (int)(MAX_SPEED_RIGHT * TrackFactorDown)) < 0 ? 0 : MAX_SPEED_RIGHT - (int)(MAX_SPEED_RIGHT * TrackFactorDown);

  motorSet = "RIGHT";
  leftMotor1.run(FORWARD);
  rightMotor1.run(BACKWARD);
  leftMotor2.run(FORWARD);
  rightMotor2.run(BACKWARD);
  leftMotor1.setSpeed(MAX_SPEED_LEFT_A);
  rightMotor1.setSpeed(MAX_SPEED_RIGHT_A);
  leftMotor2.setSpeed(MAX_SPEED_LEFT_A);
  rightMotor2.setSpeed(MAX_SPEED_RIGHT_A);
}

void moveStop(void) {
  leftMotor1.run(5); rightMotor1.run(5);
  leftMotor2.run(5); rightMotor2.run(5);
}

void moveForward(void) {
  motorSet = "FORWARD";
  leftMotor1.run(FORWARD);
  rightMotor1.run(FORWARD);
  leftMotor2.run(FORWARD);
  rightMotor2.run(FORWARD);
  leftMotor1.setSpeed((int)MAX_SPEED_LEFT * 0.5);
  rightMotor1.setSpeed((int)MAX_SPEED_RIGHT * 0.5);
  leftMotor2.setSpeed((int)MAX_SPEED_LEFT * 0.5);
  rightMotor2.setSpeed((int)MAX_SPEED_RIGHT * 0.5);
}

void moveTrack(void)
{
  int num1 = 0;
  int num2 = 0;
  int num3 = 0;
  while (1) {
    num1 = digitalRead(leftSensor);
    num2 = digitalRead(middleSensor);
    num3 = digitalRead(rightSensor);
    if (num2 == 1) {
      moveForward();
      lastTurn = "";
      lostTime = 0;
    } else if ( (num1 == 0) && num3 == 1) { //go to right
      trackturnRight();
      lastTurn = "right";
      lostTime = 0;
    } else if ((num1 == 1) && (num3 == 0)) { // go to left
      trackturnLeft();
      lastTurn = "left";
      lostTime = 0;
    } else {
      if (lostTime < 10000) {
        if (lastTurn == "left") {
          trackturnLeft();
        } else if (lastTurn == "right") {
          trackturnRight();
        }
        lostTime++;
      }
      else {
        moveStop();
      }
    }
  }
}
