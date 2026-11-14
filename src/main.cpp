#include <Arduino.h>


#include <SPI.h>
#include <WiFiNINA.h>
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

//check and fix - place holders for now
#define BIN1 1
#define BIN2 2
#define enc_B1 3
#define enc_B2 4

#define strut_id 0  // 0,1,2 - change before upload
// #define debug 1 

char ssid[] = "ahe";
char password[] = "Morgantown";
char print[40];

char mtr_state[7] ="------"; //global motor state

const unsigned int UDP_PORT = 52311;

//encorder counts

long enc_A_cnt = 500;
long enc_B_cnt = 500;

double Setpoint_a, Input_a, Output_a;
double Setpoint_b, Input_b, Output_b;

//Specify the links and initial tuning parameters
double Kp=.8,Ki=0, Kd=.02;
PID mypid_a(&Input_a, &Output_a, &Setpoint_a, Kp, Ki, Kd, DIRECT);
PID mypid_b(&Input_b, &Output_b, &Setpoint_b, Kp, Ki, Kd, DIRECT);

WiFiUDP udp;
char packet[12];

void isr_a();
void isr_b();
void internal_PID_loop(int m0_len,int m1_len);


void init_motors(){
  pinMode(stby,OUTPUT);
  pinMode(PWMA,OUTPUT);
  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);

  pinMode(PWMB,OUTPUT);
  pinMode(BIN1,OUTPUT);
  pinMode(BIN2,OUTPUT);

  digitalWrite(stby,HIGH);
  analogWrite(PWMA,128);
}

void set_motor(int motor,int dir,int speed){
  // set speed,direction for motor 0,1
  #ifdef debug
    sprintf(print,"set_mtr:%d,%d,%d",motor,dir,speed);
    Serial.println(print);
  #endif
  
  if (motor){
    //m1 cmds
    analogWrite(PWMB,speed);
    if(dir){
      digitalWrite(BIN1,HIGH);
      digitalWrite(BIN2,LOW);
      return;
    }
    digitalWrite(BIN1,LOW);
    digitalWrite(BIN2,HIGH);
    return;

  }else{
    //m0 cmds
    analogWrite(PWMA,speed);
    if(dir){
      digitalWrite(AIN1,HIGH);
      digitalWrite(AIN2,LOW);
      return;
    }
    digitalWrite(AIN1,LOW);
    digitalWrite(AIN2,HIGH);

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
    digitalWrite(BIN1,LOW);
    digitalWrite(BIN2,LOW);
    mtr_state[strut_id*2]='S';

  }else{
    //m0-a
    digitalWrite(AIN1,LOW);
    digitalWrite(AIN2,LOW);
    mtr_state[strut_id*2+1]='S';

  }
}



void reset_enc_cnt(){
  enc_A_cnt = 0;
  enc_B_cnt = 0;
}

void init_encorders(){
  pinMode(enc_A1,INPUT);
  pinMode(enc_A2,INPUT);

  pinMode(enc_B1,INPUT);
  pinMode(enc_B2,INPUT);


  // need to replace with udp init packet
  enc_A_cnt = 500;
  enc_B_cnt = 500;

  attachInterrupt(enc_A1,isr_a,RISING);
  attachInterrupt(enc_B1,isr_b,RISING);
}

int init_wifi(){
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  return 1;
}

int udp_listen(){
  if (udp.begin(UDP_PORT)) {
    Serial.print("UDP listening on port ");
    Serial.println(UDP_PORT);
    return 1;
  } else {
    Serial.println("Failed to start UDP listener!");
    return 0;
  }
}

void send_udp(){
    udp.beginPacket("192.168.1.255", 5000);
    // sprintf(packet,"%04d,%04d:",enc_A_cnt,enc_B_cnt);
    udp.write((uint8_t*)print, 22);
    udp.endPacket();
}

void setup() {

  init_encorders();
  init_motors();
  Serial.begin(9600);
 

  while (!Serial && millis() < 5000);
  Serial.println("Tensegrity AHE - 1"); 

  init_wifi();
  udp_listen();

  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  delay(500);
  digitalWrite(13,LOW);


  mypid_a.SetMode(AUTOMATIC);
  mypid_b.SetMode(AUTOMATIC);
}

void update_mtr_state(){
  mtr_state[strut_id*2]='M';
  mtr_state[strut_id*2+1]='M';
}

void extract_udp_cmd(){

  //startup packet
  if(packet[10]=='I'){
    //xxxxyzzzzI

    //init data
    sprintf(print,"udp:%s\n",packet);
    Serial.print(print);

  }



  // use strct id and get motor length
  int mtr_a_len,mtr_b_len;
  mtr_a_len = int(packet[strut_id*2]);
  mtr_b_len = int(packet[strut_id*2+1]);

  update_mtr_state();

  internal_PID_loop(mtr_a_len,mtr_b_len);
    
}



void internal_PID_loop(int m0_len,int m1_len){

  // calculate counts for motors and direction
  long mtr_a_cnt = int(m0_len * 23.33);
  long mtr_b_cnt = int(m1_len * 23.33);

  sprintf(print,"PID a: s:%d,c:%d",mtr_a_cnt,enc_A_cnt);
  Serial.println(print);
  sprintf(print,"PID b: s:%d,c:%d",mtr_b_cnt,enc_B_cnt);
  Serial.println(print);


  Setpoint_a = 0;
  Input_a = (mtr_a_cnt - enc_A_cnt)*-1.0;
  Input_b = (mtr_b_cnt - enc_B_cnt)*-1.0;

  unsigned long pid_st = millis();
  while(abs(mtr_a_cnt - enc_A_cnt)>10 || abs(mtr_a_cnt - enc_A_cnt)>10){
      Input_a = abs(mtr_a_cnt - enc_A_cnt)*-1.0;
      Input_b = abs(mtr_b_cnt - enc_B_cnt)*-1.0;

      mypid_a.Compute();
      mypid_b.Compute();

      // sprintf(print,"%04d,%04d",enc_A_cnt,mtr_a_cnt);
      // Serial.println(print);
      
    // send_udp();
    if(abs(mtr_a_cnt - enc_A_cnt)>10){
      set_motor(0,mtr_a_cnt>enc_A_cnt,map(int(abs(Output_a)),int(10),int(255),int(30),int(255)));
    }else{
      stop_motor(0);
    }

    if(abs(mtr_b_cnt - enc_B_cnt)>10){
      set_motor(1,mtr_b_cnt>enc_B_cnt,map(int(abs(Output_b)),int(10),int(255),int(30),int(255)));
    }else{
      stop_motor(1);
    }
      
    delay(3);

  }

  
  Serial.print("run time:");
  Serial.println((millis()-pid_st));


}




long enc_a_old,enc_b_old=0;

void loop() {
  
  
 int packetSize = udp.parsePacket();
  if (packetSize) {
    digitalWrite(13,HIGH);
    udp.read(packet,10);
    #ifdef debug
      Serial.println(packet);
    #endif
    extract_udp_cmd();
  }
  digitalWrite(13,LOW);

}

 
void isr_a(){
  if(digitalRead(enc_A2))
    enc_A_cnt++;
  else
    enc_A_cnt--;

}

void isr_b(){
  if(digitalRead(enc_A2))
    enc_B_cnt++;
  else
    enc_B_cnt--;

}
