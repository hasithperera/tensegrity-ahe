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

#define strut_id 0  // 0,1,2 - change before upload
// #define debug 1 

char ssid[] = "ahe";
char password[] = "Morgantown";
char print[40];

const unsigned int UDP_PORT = 52311;

//encorder counts

long enc_A_cnt = 500;
long enc_B_cnt = 0;

int m0_current = 75;
int m1_current = 75;


double Setpoint_a, Input_a, Output_a;

//Specify the links and initial tuning parameters
double Kp=.8,Ki=0, Kd=.02;
PID mypid_a(&Input_a, &Output_a, &Setpoint_a, Kp, Ki, Kd, DIRECT);

void isr_a();
void internal_PID_loop(int m0_len,int m1_len);


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
  #ifdef debug
    sprintf(print,"set_mtr:%d,%d,%d",motor,dir,speed);
    Serial.println(print);
  #endif
  
  if (motor){
    //m1 cmds

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


void print_enc_a(){
  Serial.print("enc:");
  Serial.print(enc_A_cnt);
  Serial.print(',');
  Serial.println(enc_B_cnt);
}

void reset_enc_cnt(){
  enc_A_cnt = 0;
  enc_B_cnt = 0;
}

void init_encorders(){
  pinMode(enc_A1,INPUT);
  pinMode(enc_A2,INPUT);

  #ifdef v1
    reset_enc_cnt();
  #else
    enc_A_cnt = 500;
  #endif

  attachInterrupt(enc_A1,isr_a,RISING);
}

// #define v1 1
WiFiUDP udp;

char packet[12];


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

  m0_current = 75;

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
}

void extract_udp_cmd(){

  // use strct id and get motor length
  int mtr_a_len,mtr_b_len;
  mtr_a_len = int(packet[strut_id*2]);
  mtr_b_len = int(packet[strut_id*2+1]);

  internal_PID_loop(mtr_a_len,mtr_b_len);
    
}



void internal_PID_loop(int m0_len,int m1_len){

  // calculate counts for motors and direction
  long mtr_a_cnt = int(m0_len * 23.33);

  sprintf(print,"PID in: s:%d,c:%d",mtr_a_cnt,enc_A_cnt);
  Serial.println(print);


  Setpoint_a = 0;
  Input_a = (mtr_a_cnt - enc_A_cnt)*1.0;

  unsigned long pid_st = millis();
  while(abs(mtr_a_cnt - enc_A_cnt)>10){
      Input_a = abs(mtr_a_cnt - enc_A_cnt)*-1.0;

      mypid_a.Compute();

      // sprintf(print,"%04d,%04d",enc_A_cnt,mtr_a_cnt);
      // Serial.println(print);
      delay(3);
      // send_udp();

      set_motor(0,mtr_a_cnt>enc_A_cnt,map(int(abs(Output_a)),int(10),int(255),int(30),int(255)));
      // delay(1);
  }
  stop_motor(0);
  
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

