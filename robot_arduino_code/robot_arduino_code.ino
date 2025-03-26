/**
 * @file       robot_arduino_code.ino
 * @author     Ruchira Silva
 * @version    1.0
 * @date       March 2025
 * @brief      Autonomous Food Delivery Robot - A project to automate food delivery in restaurants.
 * 
 * @details    This project demonstrates an autonomous robot capable of navigating to tables, avoiding obstacles,
 *             and delivering food while being controlled through a web-based interface. It integrates hardware
 *             components like ESP32, DC motors, ultrasonic sensors, and MPU6050 IMU with software for navigation
 *             and obstacle avoidance.
 * 
 * @copyright  Copyright (c) 2025 Ruchira Silva. All rights reserved.
 * 
 * Project Links:
 * - GitHub Repository: https://github.com/RuchiraSilva/autonomous_food_delivery_robot
 * - YouTube Demo: https://youtu.be/X6bn6PqycP4?si=IvlOGM1kDrbI3rAY 
 * 
 * Contact me:
 * - Email: ruchirasilva45@gmail.com
 * - LinkedIn: https://www.linkedin.com/in/ruchirasilva
 * 
 */

#include <Wire.h>
#include <MPU6050_light.h>
#include <WiFi.h>
#include <WebServer.h>

#define TRIG_PIN 5
#define ECHO_PIN 18
#define MOTOR1_PIN1 13
#define MOTOR1_PIN2 27
#define MOTOR2_PIN1 26
#define MOTOR2_PIN2 25
#define ENCODER1_PIN 33
#define ENCODER2_PIN 32

#define WHEEL_RADIUS 3.25
#define ENCODER_RESOLUTION 80
#define ROBOT_WIDTH 13
#define OBSTACLE_DISTANCE 30
#define SCAN_ANGLE 30
#define ANALOG_IN_PIN 36
#define REF_VOLTAGE 3.3
#define ADC_RESOLUTION 4096.0
#define R1 30000.0
#define R2 7500.0

const char* ssid = "wifi_ssid";
const char* password = "wifi_password";


float currentX = 0, currentY = 0;
float currentAngle = 0;
volatile long encoder1Ticks = 0, encoder2Ticks = 0;
float targetX = 0, targetY = 0;
float dis=50;
bool targetReached = false;
int speed=100;
String targetPlace;
int chargeP=100;

MPU6050 mpu(Wire);
WebServer server(80);

void encoder1ISR() {
  encoder1Ticks++;
}

void encoder2ISR() {
  encoder2Ticks++;
}

float calculateDistance() {
  float avgTicks = (encoder1Ticks + encoder2Ticks) / 2.0;
  float distance = (avgTicks / ENCODER_RESOLUTION) * (2 * PI * WHEEL_RADIUS);
  return distance;
}

void updatePosition() {
  float distance = calculateDistance();
  currentX += distance * cos(currentAngle * PI / 180);
  currentY += distance * sin(currentAngle * PI / 180);
  encoder1Ticks = 0;
  encoder2Ticks = 0;

  Serial.print(currentX);Serial.print(",");Serial.println(currentY);
  server.handleClient();
}

void rotateToAngle(float targetAngle) {
  Serial.print("Target angle: ");Serial.println(targetAngle);
  //delay(5000);
  float error = targetAngle - currentAngle;
  while (abs(error) > 2) {
    if (error > 0) {
      rotateLeft();
      //Serial.println("Rotating to the target angle...");
    } else {
      rotateRight();
      //Serial.println("Rotating to the target angle...");
    }
    updateIMU();
    server.handleClient();
    error = targetAngle - currentAngle;
  }
  stopRobot();
}

void moveForward() {
  attachInterrupt(digitalPinToInterrupt(ENCODER1_PIN), encoder1ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER2_PIN), encoder2ISR, RISING);
  analogWrite(MOTOR1_PIN1, speed);
  analogWrite(MOTOR1_PIN2, 0);
  analogWrite(MOTOR2_PIN1, speed);
  analogWrite(MOTOR2_PIN2, 0);
  updateIMU();
}

void rotateLeft() {
  detachInterrupt(digitalPinToInterrupt(ENCODER1_PIN));
  detachInterrupt(digitalPinToInterrupt(ENCODER2_PIN));
  analogWrite(MOTOR1_PIN1, (speed+10));
  analogWrite(MOTOR1_PIN2, 0);
  analogWrite(MOTOR2_PIN1, 0);
  analogWrite(MOTOR2_PIN2, (speed+10));
  updateIMU();
}

void rotateRight() {
  detachInterrupt(digitalPinToInterrupt(ENCODER1_PIN));
  detachInterrupt(digitalPinToInterrupt(ENCODER2_PIN));
  analogWrite(MOTOR1_PIN1, 0);
  analogWrite(MOTOR1_PIN2, (speed+10));
  analogWrite(MOTOR2_PIN1, (speed+10));
  analogWrite(MOTOR2_PIN2, 0);
  updateIMU();
}

void stopRobot() {
  analogWrite(MOTOR1_PIN1, 0);
  analogWrite(MOTOR1_PIN2, 0);
  analogWrite(MOTOR2_PIN1, 0);
  analogWrite(MOTOR2_PIN2, 0);
}
/*
bool checkObstacle() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;
  if (distance>1){
    dis=distance;
  }
  Serial.print("Obsticle distance: ");Serial.println(dis);
  return dis < OBSTACLE_DISTANCE;
}*/

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;
    if (distance>1){
    dis=distance;
  }
  Serial.print("Obsticle distance: ");Serial.println(dis);
  return dis;
}

float chooseBestDirection() {
  float maxDistance = 30;
  float bestAngle = currentAngle;
/*
  for (float angle = -90; angle <= 90; angle += SCAN_ANGLE) {
    rotateToAngle(currentAngle + angle);
    float distance = measureDistance();
    Serial.print("Scanning at angle: "); Serial.print(angle);
    Serial.print(", Distance: "); Serial.println(distance);

    if (distance > maxDistance) {
      //maxDistance = distance;
      bestAngle = currentAngle + angle;
      break;
    }
  }
*/
  while(measureDistance()<maxDistance){
    bestAngle=bestAngle+30;
    rotateToAngle(bestAngle);
  }
  Serial.print("Best angle: "); Serial.println(bestAngle);
  return bestAngle;
}

/*
void avoidObstacle() {
  stopRobot();
  rotateToAngle(currentAngle + 30);
}*/

void avoidObstacle() {
  stopRobot();
  float bestAngle = chooseBestDirection();
  //rotateToAngle(bestAngle);
  updatePosition();
  moveForward();
  updatePosition();
  delay(1500);
  stopRobot();
  rotateToAngle(0);
  navigateTo(targetX, targetY);
}

void updateIMU() {
  mpu.update();
  currentAngle=mpu.getAngleZ();
  yield();
  Serial.print("Current angle: ");Serial.println(currentAngle);
  //delay(100);
}


void navigateTo(float targetX, float targetY) {
  float angle = atan2(targetY - currentY, targetX - currentX) * 180 / PI;
  float distance = sqrt(pow(targetX - currentX, 2) + pow(targetY - currentY, 2));

  rotateToAngle(angle);

  while (distance > 20) {
    if (measureDistance() < OBSTACLE_DISTANCE) {
      avoidObstacle();
    } else {
      updatePosition();
      moveForward();
      updatePosition();
      //angle = atan2(targetY - currentY, targetX - currentX) * 180 / PI;
      //rotateToAngle(angle);
      distance = sqrt(pow(targetX - currentX, 2) + pow(targetY - currentY, 2));
    }
    server.handleClient();
    updateLocationString();
  }

  if (abs(currentX - targetX) < 20 && abs(currentY - targetY) < 20) {
    stopRobot();
    targetReached = true;
    rotateToAngle(0);
    Serial.println("Reached Table!");
    updateLocationString();
  }
}

void chargeMeasure(){
  int adc_value = analogRead(ANALOG_IN_PIN);
  float voltage_adc = ((float)adc_value * REF_VOLTAGE) / ADC_RESOLUTION;
  float voltage_in = voltage_adc * (R1 + R2) / R2;
  Serial.print("Charge: ");Serial.print(voltage_in);Serial.println("V");
  //chargeP=(int)(map(voltage_in,6.0,8.4,0.0,100.0));
  chargeP = (int)((voltage_in - 6.0) * (100.0 / (8.4 - 6.0)));
  Serial.print("Charge precentage: ");Serial.println(chargeP);
}

void updateLocationString(){
  if(targetX==0 && targetY==0 && targetReached == false){
    targetPlace="Navigating to Robot Home";
  }
  else if(targetX==100 && targetY==0 && targetReached == false){
    targetPlace="Navigating to Table 1";
  }
  else if(targetX==200 && targetY==0 && targetReached == false){
    targetPlace="Navigating to Table 2";
  }
  else if(targetX==100 && targetY==100 && targetReached == false){
    targetPlace="Navigating to Table 3";
  }
  else if(targetX==200 && targetY==100 && targetReached == false){
    targetPlace="Navigating to Table 4";
  }

  if(targetX==0 && targetY==0 && targetReached == true){
    targetPlace="Robot at home";
  }
  else if(targetX==100 && targetY==0 && targetReached == true){
    targetPlace="Robot reached table 1";
  }
  else if(targetX==200 && targetY==0 && targetReached == true){
    targetPlace="Robot reached table 2";
  }
  else if(targetX==100 && targetY==100 && targetReached == true){
    targetPlace="Robot reached table 3";
  }
  else if(targetX==200 && targetY==100 && targetReached == true){
    targetPlace="Robot reached table 4";
  }
}

void handleRoot() {
  String page = "<html><head><meta http-equiv='refresh' content='10' charset='utf-8' />";
  page += "<style>";
  page += "body { font-family: Arial, sans-serif; text-align: center; background-color: #e1fafa; padding: 5px; }";
  page += "h1 { color: #333; }";
  page += "p { font-size: 18px; color: #555; }";
  page += "button { padding: 10px 20px; margin: 10px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; transition: 0.3s;}";
  page += "button:hover { opacity: 0.8; transition: background-color 0.5s ease; }";
  page += "#btn1{ background-color: #4CAF50; color: white; margin-top: -10px; }";
  page += "#btn2{ background-color: #2196F3; color: white; margin-top: -10px; }";
  page += "#btn3{ background-color: #FF9800; color: white; margin-top: -10px; }";
  page += "#btn4{ background-color: #A020F0; color: white; margin-top: -10px; }";
  page += "#btn5{ background-color: #f44336; color: white; }";
  page += "#speedBtn{ background-color: #04db37; color: white; }";
  page += "#rstBtn{ background-color: #f046e4; color: white; }";
  page += "#stopBtn{ background-color: #ecfc03; color: red; }";
  page += ".flex-form { display: flex; align-items: center; justify-content: center; margin-top: -10px;}";
  page += ".flex-form label, .flex-form input { margin-right: 10px; }";
  page += "input[type='button']:hover { background-color: #45a049; }";
  page += "</style>";

  page += "<script>";
  page += "function sendCommand(command) {";
  page += "fetch('/' + command)";
  page += ".then(response => response.text())";
  page += ".then(message => {";
  page += "alert(message);";
  page += "});";
  page += "}";
  page += "function setSpeed() {";
  page += "let speedValue = document.getElementById('speedInput').value;";
  page += "fetch('/setspeed?speed=' + speedValue)";
  page += ".then(response => response.text())";
  page += ".then(message => {";
  page += "alert(message);";
  page += "});";
  page += "}";
  page += "function setTarget() {";
  page += "const targetX = document.getElementById('targetX').value;";
  page += "const targetY = document.getElementById('targetY').value;";
  page += "fetch('/setTarget?targetX=' + targetX + '&targetY=' + targetY)";
  page += ".then(response => response.text())";
  page += ".then(message => alert(message))";
  page += ".catch(error => console.error('Error:', error));";
  page += "}";
  page += "function updateStatus() {";
  page += "fetch('/status')";
  page += ".then(response => response.json())";
  page += ".then(data => {";
  page += "document.getElementById('battery').textContent = data.battery + '%';";
  page += "document.getElementById('posX').textContent = data.posX;";
  page += "document.getElementById('posY').textContent = data.posY;";
  page += "document.getElementById('target').textContent = data.target;";
  page += "});";
  page += "}";
  page += "setInterval(updateStatus, 2000);";
  page += "</script>";
  page +="</head><body>";

  page += "<h1>🤖Robot Control🤖</h1>";
  page += "<h4>Battery Level🪫🔋➡️"+String(chargeP)+"%</h4>";
  page += "<p>Current Position: X=" + String(currentX) + " Y=" + String(currentY) + "  |  Current Angle: " + String(currentAngle) +"</p>";
  page +="<p>"+String(targetPlace)+"</p>";
  page += "<button id=\"btn5\" onclick=\"sendCommand('move?table=5')\">Return Robot Home</button>";

  page += "<h4>Select Table Destination</h4>";
  page += "<button id=\"btn1\" onclick=\"sendCommand('move?table=1')\">Table 1</button>";
  page += "<button id=\"btn2\" onclick=\"sendCommand('move?table=2')\">Table 2</button>";
  page += "<button id=\"btn3\" onclick=\"sendCommand('move?table=3')\">Table 3</button>";
  page += "<button id=\"btn4\" onclick=\"sendCommand('move?table=4')\">Table 4</button>";

  page += "<h4>Set Custom Target Coordinates</h4>";
  page += "<form id='targetForm' class='flex-form'>";
  page += "<label for='targetX'>Target X:</label>";
  page += "<input type='number' id='targetX' name='targetX' value='" + String(targetX) + "' step='1' style='width: 80px; padding: 5px; font-size: 16px; border-radius: 5px; border: 1px solid #ccc;'>";
  page += "<label for='targetY'>Target Y:</label>";
  page += "<input type='number' id='targetY' name='targetY' value='" + String(targetY) + "' step='1' style='width: 80px; padding: 5px; font-size: 16px; border-radius: 5px; border: 1px solid #ccc;'>";
  page += "<input type='button' value='Set Target' onclick='setTarget()' style='background-color: #4CAF50;color: white;'>";
  page += "</form>";

  page += "<h4>Set Robot Speed</h4>";
  page += "<input type='number' id='speedInput' min='0' max='255' value='" + String(speed) + "' oninput='document.getElementById(\"speedValue\").textContent = this.value' style='margin-top: -10px; width: 80px; padding: 5px; font-size: 16px; border-radius: 5px; border: 1px solid #ccc;'>";
  page += "<button id=\"speedBtn\" onclick='setSpeed()' style='margin-top:-10px;'>Set Speed</button>";

  page +="<div></div>";
  page += "<button id=\"rstBtn\" onclick=\"sendCommand('reset')\">Restart ESP32</button><button id=\"stopBtn\" onclick=\"sendCommand('stop')\">Stop Robot</button>";
  page += "<h6>A Project by Ruchira Silva</h6></body></html>";
  
  server.send(200, "text/html", page);
}

void handleMove() {
  String tableNum = server.arg("table");
  
  if (tableNum == "1") {
    targetX = 100;
    targetY = 0;
    targetReached = false;
  } else if (tableNum == "2") {
    targetX = 200;
    targetY = 0;
    targetReached = false;
  } else if (tableNum == "3") {
    targetX = 100;
    targetY = 100;
    targetReached = false;
  }
  else if (tableNum=="4"){
    targetX=200;
    targetY=100;
    targetReached = false;
  }
  else if (tableNum=="5"){
    targetX=0;
    targetY=0;
    targetReached = false;
  }

  server.send(200, "text/plain", "Moving to Table " + tableNum);
}

void handleStatus() {
  String status = "{";
  status += "\"battery\":" + String(chargeP) + ",";
  status += "\"posX\":" + String(currentX) + ",";
  status += "\"posY\":" + String(currentY) + ",";
  status += "\"target\":\"" + String(targetPlace) + "\"";
  status += "}";
  server.send(200, "application/json", status);
}

void handleSetSpeed() {
  String speedStr = server.arg("speed");
  int newSpeed = speedStr.toInt();

  if (newSpeed >= 0 && newSpeed <= 255) {
    speed = newSpeed;
    server.send(200, "text/plain", "Speed set to " + String(speed));
  } else {
    server.send(400, "text/plain", "Invalid speed value. Must be between 0 and 255.");
  }
}

void handleSetTarget() {
  String targetXStr = server.arg("targetX");
  String targetYStr = server.arg("targetY");

  targetX = targetXStr.toFloat();
  targetY = targetYStr.toFloat();
  targetReached = false;

  server.send(200, "text/plain", "Target set to X=" + targetXStr + ", Y=" + targetYStr);
}

void handleReset() {
  server.send(200, "text/plain", "Restarting ESP32...");
  delay(100);
  ESP.restart();
}

void handleStop() {
  server.send(200, "text/plain", "Robot Stopped");
  delay(100);
  stopRobot();
  targetX=currentX;targetY=currentY;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/move", handleMove);
  server.on("/status", handleStatus);
  server.on("/setspeed", handleSetSpeed);
  server.on("/setTarget", handleSetTarget);
  server.on("/reset", handleReset);
  server.on("/stop", handleStop);
  server.begin();
  Serial.println("HTTP server started");

  Wire.begin();
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ }

  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets();
  Serial.println("Done!\n");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(MOTOR1_PIN1, OUTPUT);
  pinMode(MOTOR1_PIN2, OUTPUT);
  pinMode(MOTOR2_PIN1, OUTPUT);
  pinMode(MOTOR2_PIN2, OUTPUT);

  digitalWrite(MOTOR1_PIN1, LOW);
  digitalWrite(MOTOR1_PIN2, LOW);
  digitalWrite(MOTOR2_PIN1, LOW);
  digitalWrite(MOTOR2_PIN2, LOW);

  pinMode(ENCODER1_PIN, INPUT);
  pinMode(ENCODER2_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODER1_PIN), encoder1ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER2_PIN), encoder2ISR, RISING);

}

void loop() {
  chargeMeasure();
  measureDistance();
  server.handleClient();
  yield();
  updateLocationString();

  Serial.print("Current Location(x,y): ");Serial.print(currentX);Serial.print(",");Serial.println(currentY);
  Serial.print("Target Location(x,y): ");Serial.print(targetX);Serial.print(",");Serial.println(targetY);

  if (!targetReached) {
    navigateTo(targetX, targetY);
  }
  delay(100);
}
