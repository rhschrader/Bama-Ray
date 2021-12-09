/* Created for ME 556 by Ross Schrader
 *  University of Alabama
 *  Dr. Vishesh Vikas
 *  December 2021
 *  
 *  Project done by Ross Schrader, Brenin Douglas, Joseph Lulka
 *  
 *  This is code to control a soft robotic manta ray. Controls fin movement using two servo motors.
 *  
 *  see https://github.com/rhschrader/Bama-Ray
 *  
 *  Contact Ross Schrader at rhschrader@crimson.ua.edu with any questions
*/

#include <Servo.h>

#include "Timer.h"

Timer t;

Servo servoLeft; // controls left servo
Servo servoRight; // controls right servo
char input = " ";

// timer events for movement of left and right servos
int leftControl;
int rightControl;

// the time delay between moving the left servo 1 degree (in milliseconds)
int left_rate = 25; 
int right_rate = 25;

 // Maximum angle you want servo to reach (180 is full vertical (90 degrees up))
int left_maxDeg = 150;
int right_maxDeg = 150;

 // Minimum angle you want servo to reach (0 is 90 degrees down)
int left_minDeg = 30;
int right_minDeg = 30; 

// Default values used to reset
int def_left_rate = left_rate;
int def_right_rate = right_rate;
int def_left_maxDeg = left_maxDeg;
int def_left_minDeg = left_minDeg;
int def_right_maxDeg = right_maxDeg;
int def_right_minDeg = right_minDeg;

// angular position of servos (0-180 degrees)
int left_pos = 0;
int right_pos = 0; 

// boolean to track whether the sweep of the servo is moving up or down
bool left_up = true;
bool right_up = false;

// starts and stops the flapping
bool isRunning = false; 

void setup() {
  Serial.begin(9600);

  // sets up the servos
  initializeServos();

  // reads the servo position and sets position variable to that
  left_pos = servoLeft.read(); 
  right_pos = servoRight.read();

  // initializes the Serial monitor
  initializeScreen();
  delay(500);

  // sets default timer events for moving the left and right servos
  leftControl = t.every(left_rate, left_move, 100000);
  rightControl = t.every(right_rate, right_move, 100000);
 
}

void loop() {

  // this loop executes the flapping of the fins
  while (isRunning == true){

    // gets an input from the serial monitor
    getSpd();

    // When pressing 'a' or 'd' to turn, the servos can get very out of sync. This function puts them back at the same position and speed
    if(input == 'e'){
      syncServos();
    }

    // Processing the input character and ajusting angular velocity of each servo accordingly
        /* Toy servo's like the ones we are using are typical controlled using the delay() function. 
         *  (see Examples -> servo -> sweep under File in Arduino IDE). The delay function is used to set the amount of time between 
         *  increasing or decreasing 1 degree of the servo, therefore controlling the angular velocity of the servo. 
         *  The delay function is not ideal for our situation because no other processes can occur during the delay. In order to 
         *  avoid this problem, I am using the "Timer.h" library (see https://playground.arduino.cc/Code/Timer/). This allows an event to occur 
         *  every x amount of time without interupting the rest of the code. Usually these timers are set in the setup loop. The problem there is 
         *  that you cannot change the time interval on which it occurs. To get around this problem, each time the user changes the angular velocity 
         *  of the servos, the previous timer event must be stopped using t.stop() (used in changeLeft() and changeRight()). Then, a new timer event
         *  must be initiated using the new time interval (left_rate and right_rate). This is also done in changeLeft() and changeRight() 
        */
    changeLeft();
    changeRight();

    printProperties();
   
    // updates all timer events
    t.update();

    // if q is pressed, flapping of the fins will quit
    if(input == 'q'){
     isRunning = false;
     initializeScreen();
    }
   }

   // this loop runs while there is no motion in the servos (after q has been pressed)
   while(isRunning == false){

    // receives an input from the serial monitor
    getSpd();

    // if the user presses b, the flapping will begin again
    if (input == 'b') {
      isRunning = true;
      propertyHeaders();
      printProperties();
    }

    
   }
}


// takes input character and changes the speed of the right servo accordingly
void changeRight(){
  // This block increases speed. By sending a 'w' to the serial monitor, the speed will increase
  if(input == 'w'){
    right_rate = right_rate - 2; // decreases the amount of time in between executing right_move
    t.stop(rightControl); // stops the timer event, a new one will be created
    rightControl = t.every(right_rate, right_move, 100000);
  }
  // This block slows the speed down. By sending an 's' to the serial monitor, the speed will decrease
  if(input == 's'){
    right_rate = right_rate + 2;
    t.stop(rightControl);
    rightControl = t.every(right_rate, right_move, 100000);
  }
  // This block turns the robot left by decreasing the speed of the left fin and increasing the speed of the right fin
  if(input == 'a'){
    right_rate = right_rate - 2;
    t.stop(rightControl);
    rightControl = t.every(right_rate, right_move, 100000);
  }
  // This block turns the robot right by decreasing the speed of the left fin and increasing the speed of the right fin
  if(input == 'd'){
    right_rate = right_rate + 2;
    t.stop(rightControl);
    rightControl = t.every(right_rate, right_move, 100000);
  }
  
  // changes the amplitude or 'range of degrees' that the servo travels in one sweep
  // this results in a change of 10 degrees wider
  if(input == 'z') {
    right_maxDeg = right_maxDeg + 5;
    right_minDeg = right_minDeg - 5;
  }

  // changes the amplitude or 'range of degrees' that the servo travels in one sweep
  // this results in a change of 10 degrees narrower
  if(input == 'x') {
    right_maxDeg = right_maxDeg - 5;
    right_minDeg = right_minDeg + 5;
  }

  // reverts all settings on the right servo to defaults
  if(input == 'r') {
    right_rate = def_right_rate;
    t.stop(rightControl);
    rightControl = t.every(right_rate, right_move, 100000);
    
    right_maxDeg = def_right_maxDeg;
    right_minDeg = def_right_minDeg;
  }
}


// takes input character and adjusts the speed of the left servo accordingly
void changeLeft(){
  // This block increases speed. By sending a 'w' to the serial monitor, the speed will increase
  if(input == 'w'){
    
    // subtracts 2 milliseconds
    left_rate = left_rate - 2;
    // this function moves the servo 1 degree every left_rate milliseconds, so angular velocity can be calculated as below
    t.stop(leftControl);

    leftControl = t.every(left_rate, left_move, 100000);
  }
  // This block slows the speed down. By sending an 's' to the serial monitor, the speed will decrease
  if(input == 's'){

    // adds 2 milliseconds
    left_rate = left_rate + 2;

    // calculates angular velocity in terms of deg/s
    // this function moves the servo 1 degree every left_rate milliseconds, so angular velocity can be calculated as below
    t.stop(leftControl);

    leftControl = t.every(left_rate, left_move, 100000);
  }
  // This block turns the stingray left by decreasing the speed of the left fin and increasing the speed of the right fin
  if(input == 'a'){
    left_rate = left_rate + 2;

    t.stop(leftControl);

    leftControl = t.every(left_rate, left_move, 100000);
  }
  // This block turns the stingray right by decreasing the speed of the left fin and increasing the speed of the right fin
  if(input == 'd'){
    left_rate = left_rate - 2;

    t.stop(leftControl);

    leftControl = t.every(left_rate, left_move, 100000);
  }

   // changes the amplitude or 'range of degrees' that the servo travels in one sweep
  // this results in a change of 10 degrees wider
  if(input == 'z') {
    left_maxDeg = left_maxDeg + 5;
    left_minDeg = left_minDeg - 5;
  }

  // changes the amplitude or 'range of degrees' that the servo travels in one sweep
  // this results in a change of 10 degrees narrower
  if(input == 'x') {
    left_maxDeg = left_maxDeg - 5;
    left_minDeg = left_minDeg + 5;
  }

  // reverts all settings of left servo to defaults
  if(input == 'r'){
    left_rate = def_left_rate;
    t.stop(leftControl);
    leftControl = t.every(left_rate, left_move, 100000);
    
    left_maxDeg = def_left_maxDeg;
    left_minDeg = def_left_minDeg;
    
  }
}

// The getSpd function receives an input character from the serial monitor
void getSpd() {
  if(Serial.available()){
        input = Serial.read();
    }
    else {
      input = ' ';
    }
}


// This function is what actually moves the right servo
void right_move(){
  
  // moving 0 -> 180
  if ((right_pos < right_maxDeg) && (right_up == false)){
    
    // increasing the degree by an increment of 1 and writing that position to the servo
    right_pos++;
    servoRight.write(right_pos);

    // checking if right position is equal to the maximum position
    if (right_pos == right_maxDeg){
    right_up = true;
    }
  }

  // moving 180 -> 0
  if ((right_pos > right_minDeg) && (right_up == true)){
    
    servoRight.write(right_pos);
    right_pos--;
    
    // if position is equal to minimum degree, setting bool right_up to true
    if (right_pos == right_minDeg){
    right_up = false;
    }
  }
}

void left_move(){

  // moving 0 -> 180
  if ((left_pos < left_maxDeg) && (left_up == true)){

    left_pos++;
    servoLeft.write(left_pos);

    if (left_pos == left_maxDeg){
    left_up = false;
    }
    
  }
  
  // moving 180 -> 0
  if ((left_pos > left_minDeg) && (left_up == false)){
    
    servoLeft.write(left_pos);
    left_pos--;

    if (left_pos == left_minDeg){
    left_up = true;
    }
  }
}

// Sets screen on initialization
void initializeScreen() {
  Serial.print("--------------------------------------------------\n");
  Serial.print("\t\tControls\n--------------------------------------------------");
  Serial.print("\n\nSpeed Up\tw\tIncrease Amplitude\tz\nSlow Down\ts\tDecrease Amplitude\tx\nTurn Right\td\tReset to Defaults\tr\nTurn Left\ta\tRe-Sync (after turn)\te\n\n");
  Serial.print("*PRESS b TO BEGIN, PRESS q TO QUIT*\n");
}

void propertyHeaders() {
  Serial.print("\n---------------------------------------------------------\n");
  Serial.print("\t\tProperties\nLeft Ang. Velo. \tRight Ang. Velo.\tAmplitude\n---------------------------------------------------------\n");
}

void printProperties() {
  if (input == 'w' || input == 's' || input == 'd' || input == 'a' || input == 'z' || input == 'x' || input == 'r' || input == 'b' || input == 'e'){
    
    /* The left_move() and right_move() functions work by moving the servos 1 degree every left_rate or right_rate milliseconds
    Therefore, the angular velocity of each servo can be calculated by dividing 1 degree by left_rate or 
    right_rate milliseconds to achieve degrees per second*/
    int left_angVelo = 1 / (float(left_rate) / 1000);
    int right_angVelo = 1 / (float(right_rate) / 1000);

    // the way I have coded this the amplitude of both left and right servos will always be the same
    int amplitude = left_maxDeg - left_minDeg;

    Serial.print(left_angVelo);
    Serial.print(" deg/s\t\t");
    Serial.print(right_angVelo);
    Serial.print(" deg/s\t\t");
    Serial.print(amplitude);
    Serial.print(" deg\n");
    
  }
}

void syncServos(){
  int avgSpd = (left_rate + right_rate) / 2;
  t.stop(leftControl);
  t.stop(rightControl);
  if(left_pos <= 90){
    servoLeft.write(90);
    servoRight.write(90);
    right_up = true;
    left_up = true;
  }
  if(left_pos > 90) {
    servoLeft.write(90);
    servoRight.write(90);
    right_up = false;
    left_up = false;
  }
  delay(250);
  left_rate = avgSpd;
  right_rate = avgSpd;
  right_pos = servoRight.read();
  left_pos = servoLeft.read();
  leftControl = t.every(left_rate, left_move, 100000);
  rightControl = t.every(right_rate, right_move, 100000);

}

void initializeServos() {

  // attaching left servo to PWM pin 9 and right servo to PWM pin 10
  servoLeft.attach(9);
  servoRight.attach(3);

  // initializing both servos to 90 degrees, this is horizontal
  servoLeft.write(90); 
  servoRight.write(90);

  // delays 2 seconds to ensure the servo reaches 90
  delay(2000); 
  
}
