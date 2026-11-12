#include <Arduino.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <PID_v1.h>



// put function declarations here:


#define stby 6
#define PWMA A2
#define PWMB 3

#define AIN1 A1
#define AIN2 A3

#define enc_A1 9
#define enc_A2 10

char ssid[] = "ahe";
char password[] = "Morgantown";

//encorder counts

long enc_A_cnt = 0;
long enc_B_cnt = 0;

void isr_a();


void init_motors(){
  pinMode(stby,OUTPUT);
  pinMode(PWMA,OUTPUT);
  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);


  digitalWrite(stby,HIGH);
  analogWrite(PWMA,128);
}

void set_motor(int motor,int dir,int speed){
  // set speed,direction for motor 0,1
  if (motor){
    // m1 cmds

  }else{
    //m0 cmds
  }
  
}


void motor_fwd(){
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
}

void motor_rev(){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
}


void stop_motor(int motor){
  if(motor){
    //m1-b

  }else{
    //m0-a
    digitalWrite(AIN1,LOW);
    digitalWrite(AIN2,LOW);
  }
}

// write a function to find min rotation speed

void test_speeds(int motor){
  int mtr_pwm = PWMA;
  if(motor){
    mtr_pwm = PWMB;
  }

  Serial.println('Note the last speed you see the motor mover');
  for(int i=128;i>0;i=i-5){
    motor_fwd();
    analogWrite(mtr_pwm,i);
    Serial.println(i);
    delay(500);
    stop_motor(0);
    delay(500);
    Serial.print("enc a:");
    Serial.println(enc_A_cnt);

  }
  


}


void reset_enc_cnt(){
  enc_A_cnt = 0;
  enc_B_cnt = 0;
}

void init_encorders(){
  pinMode(enc_A1,INPUT);
  pinMode(enc_A2,INPUT);

  reset_enc_cnt();

  attachInterrupt(enc_A1,isr_a,RISING);
}

double Setpoint, Input, Output;
double Kp=2, Ki=5, Kd=1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);


void setup() {
  // put your setup code here, to run once:
  // int result = myFunction(2, 3);
  // attachInterrupt(1,isr,RISING);  // https://docs.arduino.cc/language-reference/en/functions/external-interrupts/attachInterrupt/


  init_encorders();
  init_motors();
  Serial.begin(9600);
  delay(500);
  Serial.println("Tensegrity AHE - 1"); 

}

void loop() {
  // put your main code here, to run repeatedly:

  test_speeds(0);

  
}



void isr_a(){
  enc_A_cnt++;

}