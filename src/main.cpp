#include <Arduino.h>
#include <WiFi101.h>
#include <WiFiUdp.h>

// put function declarations here:
int myFunction(int, int);
void isr();

int i;

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  attachInterrupt(1,isr,RISING);  // https://docs.arduino.cc/language-reference/en/functions/external-interrupts/attachInterrupt/
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

void isr(){
  i++;
}