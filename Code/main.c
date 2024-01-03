/*
Jolin Lin
Christopher Ghim
Group 39
DESCRIPTION: User inputs x value by incrementing the left push button as well as the 
y value with the right button. User then pushes the middle button to go to coordinates (x,y).
*/


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
#include "lcd_driver.h"
#include "port_macros.h"

#define LEFTBUTTON 1
#define MIDBUTTON 4
#define RIGHTBUTTON 5
#define RIGHTMOTOR3 3
#define LEFTMOTOR5 5
#define LEFTMOTOR6 6

#define INCREMENT 25
#define PWM_TOP 100
#define TURN_TIME 400     //turn time for right side
#define TURN_TIMEL 395
#define duty_cycle1 26
#define duty_cycle2 26

//-------------------------------------------Basic Voids----------------------------------------------------------------------//

void brake(){
  PORTD |= (1<<LEFTMOTOR5)|(1<<LEFTMOTOR6)|(1<<RIGHTMOTOR3);
  PORTB |= (1<<RIGHTMOTOR3);
}

void forwardL(){
  PORTD &= ~((1<<LEFTMOTOR5)); 
  PORTD |= (1<<LEFTMOTOR6);
}

void forwardR(){
  PORTD &= ~((1<<RIGHTMOTOR3)); 
  PORTB |= (1<<RIGHTMOTOR3);
}

int straightLineForward(int distance){
  unsigned int pwm_counter=0;
  for(int i = 0; i<=distance; i++){                                 //---------MAY CAUSE ERROR!!!!!!!----------------//
    pwm_counter = pwm_counter + 1;
    if( pwm_counter >= PWM_TOP ){
      pwm_counter = 0;
    }
    if( pwm_counter < duty_cycle1 ){
      forwardL();
    }
    else{
      brake();
    }

    if( pwm_counter < duty_cycle2){
      forwardR();
    }
    else{
      brake();
    }
  }
  return 0;
}


int calculateDistance(float x){
  float estimatedDistance = (0.0308657581*abs(x)*abs(x)*abs(x))-(2.089208568*abs(x)*abs(x))+(73.05366369*abs(x))+250.8188;
  int distance = (int) estimatedDistance;
  return distance;
}

void backwardL(){
  PORTD |= ((1<<LEFTMOTOR5)); 
  PORTD &= ~(1<<LEFTMOTOR6);
}

void backwardR(){
  PORTB &= ~((1<<RIGHTMOTOR3)); 
  PORTD |= (1<<RIGHTMOTOR3);
}

int straightLineBackward(int distanceX){
  unsigned int pwm_counter=0;
  for(int i = 0; i<=distanceX; i++){
    pwm_counter = pwm_counter + 1;
    if( pwm_counter >= PWM_TOP ){
      pwm_counter = 0;
    }
    if( pwm_counter < duty_cycle1 ){
      backwardL();
    }
    else{
      brake();
    }
    if( pwm_counter < duty_cycle2){
      backwardR();
    }
    else{
      brake();
    }
  }
  return 0;
}

void turn_right(){
  unsigned int pwm_counter=0;
  for(int i = 0; i<=TURN_TIME; i++){
    pwm_counter = pwm_counter + 1;
    if( pwm_counter >= PWM_TOP ){
      pwm_counter = 0;
    }
    if( pwm_counter < duty_cycle1 ){
        PORTD &= ~(1<<LEFTMOTOR5);
        PORTD |= (1<<LEFTMOTOR6);
    }
    else{
    brake();
    }

    if( pwm_counter < duty_cycle2){
      PORTD |= (1<<RIGHTMOTOR3);
      PORTB &= ~(1<<RIGHTMOTOR3);
    }
    else{
      brake();
    }
  }
}

void turn_left(){
  unsigned int pwm_counter=0;
  for(int i = 0; i<=TURN_TIMEL; i++){
    pwm_counter = pwm_counter + 1;
    if( pwm_counter >= PWM_TOP ){
      pwm_counter = 0;
    }
    if( pwm_counter < duty_cycle1 ){
      PORTD &= ~(1<<LEFTMOTOR6); 
      PORTD |= (1<<LEFTMOTOR5);
    }
    else{
      brake();
    }
    if( pwm_counter < duty_cycle2){
      PORTB |= (1<<RIGHTMOTOR3);
      PORTD &= ~(1<<RIGHTMOTOR3);
    }
    else{
      brake();
    }
  }
}

//-------------------------------------------------Going to Coordinates-----------------------------------------------------------//
int gotocoord(int distanceX, int distanceY, int x, int y ){
  for(int k = 0; k <=1000; k++){
    brake();
    LCD_execute_command(CLEAR_DISPLAY);
  }
  for (int i = 0; i<=distanceY; i++){
    if(y >= 0){
      straightLineForward(distanceY);
    }
    else{
      straightLineBackward(distanceY);
    }
  }
  for(int k = 0; k <=500; k++){
    brake();
    LCD_execute_command(CLEAR_DISPLAY);
  }
  for(int r = 0; r<=TURN_TIME; r++){
    if(x >= 0){
      turn_right();
    }
    else{
      turn_left();
    }
  }
  for(int k = 0; k <=500; k++){
    brake();
    LCD_execute_command(CLEAR_DISPLAY);
  }
  for(int i = 0; i<=distanceX; i++){
    straightLineForward(distanceX);
  }
  brake();

  return 0;
}


//----------------------------------------------int main------------------------------------------------------------//
int main(){

  unsigned int last_mid_button_state = (PINB & (1<<MIDBUTTON));
  unsigned int mid_button_pressed=0;
  unsigned int last_left_button_state = (PINB & (1<<LEFTBUTTON));
  unsigned int left_button_pressed=0;
  unsigned int last_right_button_state = (PINB & (1<<RIGHTBUTTON));
  unsigned int right_button_pressed=0;
  int x = -30;            //starting x value is -30 and increments onwards
  int y = -30;            
  char xchar[2];          //displays x value on lcd
  char ychar[2];          //displays y value on lcd

  initialize_LCD_driver();
  LCD_execute_command(TURN_ON_DISPLAY);
  LCD_execute_command(CLEAR_DISPLAY);

  DDRB &= ~((1<<RIGHTBUTTON)|(1<<LEFTBUTTON)|(1<<MIDBUTTON)) ; //sets to INPUT (PB1,PB4,PB5,PB0)
  DDRB |= (1<<RIGHTMOTOR3); //sets motor to OUTPUT (PB3,PB4,PB5,PB1)
  DDRD |= (1<<LEFTMOTOR5)|(1<<LEFTMOTOR6)|(1<<RIGHTMOTOR3);//outputs (PD5,PD6,PD3,PD7)

  //motors remain in place
  PINB &= ~((1<<LEFTBUTTON)|(1<<RIGHTBUTTON)|(1<<MIDBUTTON));
  brake();

  while(1){
    
    if( (PINB & (1<<1)) != last_left_button_state ){
      if( (PINB & (1<<1)) == 0 ){
        left_button_pressed=1;
      }
      last_left_button_state = (PINB & (1<<1));
    }
    else{
      left_button_pressed=0;
    }
    _delay_us(300);

    if(left_button_pressed == 1){
      x = x+1;
      itoa(x,xchar,10);
      LCD_execute_command(CLEAR_DISPLAY);
      LCD_print_String(xchar);
    }

    if( (PINB & (1<<5)) != last_right_button_state ){
      if( (PINB & (1<<5)) == 0 ){
        right_button_pressed=1;
      }
      last_right_button_state = (PINB & (1<<5));
    }
    else{
      right_button_pressed=0;
    }
    _delay_us(300);

    if(right_button_pressed == 1){
  		y = y+1;
  		itoa(y,ychar,10);
  		LCD_execute_command(CLEAR_DISPLAY);
      LCD_move_cursor_to_col_row(5,0);
  		LCD_print_String(ychar);
    }

    int distanceX = calculateDistance(x);
    int distanceY= calculateDistance(y);

    if( (PINB & (1<<MIDBUTTON)) != last_mid_button_state ){
      if( (PINB & (1<<MIDBUTTON)) == 0 ){
        mid_button_pressed=1;
      }
      last_mid_button_state = (PINB & (1<<MIDBUTTON));
    }
    else{
      mid_button_pressed=0;
    }

    if(mid_button_pressed == 1){
      LCD_execute_command(CLEAR_DISPLAY);
      gotocoord(distanceX,distanceY,x,y);
      itoa(x,xchar,10);
      LCD_print_String(xchar);
      itoa(y,ychar,10);    
      LCD_move_cursor_to_col_row(0,1);
      LCD_print_String(ychar);
      brake();
      break;
    }
  }
  return 0;
}
