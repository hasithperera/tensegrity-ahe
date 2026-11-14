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

long enc_A_cnt = 0;
long enc_B_cnt = 0;

int m0_current = 75;
int m1_current = 75;


double Setpoint_a, Input_a, Output_a;

//Specify the links and initial tuning parameters
double Kp=.45,Ki=0, Kd=.02;
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

  reset_enc_cnt();

  attachInterrupt(enc_A1,isr_a,RISING);
}


WiFiUDP udp;

char packet[12];


void wifi_firmware_check(){
  // Print a welcome message
  Serial.println("WiFiNINA firmware check.");
  Serial.println();

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // Print firmware version on the module
  String fv = WiFi.firmwareVersion();
  String latestFv;
  Serial.print("Firmware version installed: ");
  Serial.println(fv);

  latestFv = WIFI_FIRMWARE_LATEST_VERSION;

  // Print required firmware version
  Serial.print("Latest firmware version available : ");
  Serial.println(latestFv);

  // Check if the latest version is installed
  Serial.println();
  if (fv >= latestFv) {
    Serial.println("Check result: PASSED");
  } else {
    Serial.println("Check result: NOT PASSED");
    Serial.println(" - The firmware version on the module does not match the");
    Serial.println("   version required by the library, you may experience");
    Serial.println("   issues or failures.");
  }
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
  // put your setup code here, to run once:
  // int result = myFunction(2, 3);
  // attachInterrupt(1,isr,RISING);  // https://docs.arduino.cc/language-reference/en/functions/external-interrupts/attachInterrupt/

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

  #ifdef debug
    Serial.print("mtr cmd:");
    Serial.print(mtr_a_len);
    Serial.print(',');
    Serial.println(mtr_b_len);
  #endif

  // if (mtr_a_len==65){
  //   stop_motor(0);
  //   print_enc_a();
  //   reset_enc_cnt();
  // }
    
  
  // if(mtr_a_len > 65){
  //   analogWrite(PWMA,255);
  //   motor_fwd();
  // }

  // if(mtr_a_len < 65){
  //   analogWrite(PWMA,255);
  //   motor_rev();
  // }

  internal_PID_loop(mtr_a_len,mtr_b_len);
    
}



void internal_PID_loop(int m0_len,int m1_len){

  // external values are setpoints
  


  // calculate counts for motors and direction
  float mtr_a_pending_cnt = (m0_len - m0_current) * 23.33;
  sprintf(print,"PID in:%d,%d\nm0c:%d\n m_pnd_cnt:%.2f\n",m0_len,m1_len,m0_current,mtr_a_pending_cnt);
  Serial.println(print);

  reset_enc_cnt();
  // fwd if mtr_a_pending_cnt > 0
  // rev if mtr_a_pending_cnt < 0

  Setpoint_a = abs(mtr_a_pending_cnt);
  Input_a = enc_A_cnt;

  
  while(abs(abs(mtr_a_pending_cnt) - enc_A_cnt)>5){
      Input_a = enc_A_cnt*1.0;
      mypid_a.Compute();

      // sprintf(print,"%04d,%06.2f,%06.2f,000",enc_A_cnt,mtr_a_pending_cnt,Output_a);
      // // // Serial.println(print);
      // send_udp();

      
      set_motor(0,mtr_a_pending_cnt>0,map(int(Output_a),int(0),int(255),int(15),int(255)));
      delay(4);
  }
  stop_motor(0);

  // tellling you its perfectly there, this accumilates error
  m0_current = m0_len;

}

long enc_a_old,enc_b_old=0;

void loop() {
  // put your main code here, to run repeatedly:

  //test_speeds(0);
  
  // for(int i=0;i<20;i++){
  //   Serial.print('.');
  //   send_udp();
  //   delay(200);
  // }
  //Serial.print('.');

  // if (enc_a_old != enc_A_cnt){
  //   send_udp();
  //   enc_a_old = enc_A_cnt;
  // }


  // // hardcoded motor limit
  // if(enc_A_cnt > 7000){
  //   stop_motor(0);
  //   reset_enc_cnt();
  // }
  
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
  enc_A_cnt++;

}