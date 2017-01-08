// Created by Arduino User JohnChi (MPU-6050 Short Example Sketch, August 17, 2014)
// Modified by Tristan Toupin & Gabriel Charland
// Dec. 2016
// Public Domain
#include<Wire.h>

#define forward 5
#define backward 6
#define zero 0      //minimal speed 

#define redLed 9    // Set Led Pins
#define greenLed 10
#define blueLed 11
/*
 * ROBOT oriantation
 *       ^ -z
 *       |
 *       |
 *       |
 *       |
 *       |
 *-x <___|_________________> +x
 *      /|
 *     / |
 *    /  v +z
 *  |/_
 *  +y
 *  
 *  It will start at (0,0,0) facing +x.
 *  It will move along x.
 *  The floor is the x y plan.
 */

//constants (final)
const int MPU_addr=0x68;  // I2C address of the MPU-6050
const int samples = 50;   //samples taken when setup
const double GForce = 9.80665;
const double DIST = 10.0; //mm

//variable
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float aX, aY, aZ;
double rawFactor; 
double gForceSamples[samples];  //during setup, get 10 values of each acceleration
double motorSpeed;

void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);

  //setup motors
  pinMode(forward, OUTPUT);
  pinMode(backward, OUTPUT);
  delay(1000);

  Serial.println("Do NOT touch the sensor");
  Serial.println("We are taking some values...");
  delay(2000);

  //get sample from acceleration `samples` time to find Gforce and factor
  for (int i = 0; i < samples; i++){
    getSamples();
    //printComponents();
    double tempGFroce = sqrt(sq(aX) + sq(aY) + sq(aZ));
    gForceSamples[i] = tempGFroce;
    delay(10);
  }
  Serial.println("DONE.");

  setRawFactor();
  convertToMetric();

  Serial.println("Get Ready!");
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  Serial.println("GO! \n\n");
  Serial.println("--------------------------------");
}


void loop(){

  getSamples();
  convertToMetric();
  getSpeed();

  moveBot();
  delay(400);
}


void getSamples(){
    setWire();
    
    AcX = Wire.read()<<8|Wire.read();    
    AcY = Wire.read()<<8|Wire.read();
    AcZ = Wire.read()<<8|Wire.read();
    
    aX = (float) AcX / 1000;
    aY = (float) AcY / 1000;
    aZ = (float) AcZ / 1000; 
    
}

void setWire(){
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
}

void setRawFactor(){
  double sum = 0;
  double average;
  for (int i = 0; i < samples; i = i + 1){
    sum = sum + (gForceSamples[i] / 1000);
  }
  
  average =  sum / samples;
  rawFactor = GForce / average;
  rawFactor /= 1000;
}

void convertToMetric(){
  aX = aX * (rawFactor);
  aY = aY * (rawFactor);
  aZ = aZ * (rawFactor);
}

void printComponents(){
  Serial.print("AcX = "); Serial.print(aX);
  Serial.print(" | AcY = "); Serial.print(aY);
  Serial.print(" | AcZ = "); Serial.print(aZ);
  Serial.println("\n++--------------------------------------++");
}

void getSpeed(){
  double temp = (255 / GForce);
  double ampFactor = 1.5;
  motorSpeed = (aX * temp) * ampFactor;
  //clipper filter
  if (motorSpeed > 255){
    motorSpeed = 255;
  } else if (motorSpeed < -255){
    motorSpeed = -255;
  }
  Serial.print("The motorSpeed is: ");
  Serial.println(motorSpeed);
}

void moveBot(){
  if (motorSpeed > 0){
    analogWrite(forward, motorSpeed);
    analogWrite(backward, zero);
  } else {
    analogWrite(backward, -motorSpeed);
    analogWrite(forward, zero);
  }
}


