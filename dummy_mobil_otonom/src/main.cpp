#include <Arduino.h>

// motor driver 1
#define PWM_1 5
#define RPWM_1 6
#define LPWM_1 7

// motor driver 2
#define PWM_2 8
#define RPWM_2 9
#define LPWM_2 10

// motor driver 3
#define PWM_3 11
#define RPWM_3 12
#define LPWM_3 13

int SPEED = 0;

unsigned long timer[7];
byte last_channel[6];
int input[6];
int last_input[6] = {1500,0,0,0,0,0};
bool direct_c1[6];
int count_left = 0;
int count_right = 0;

ISR(PCINT2_vect){
  timer[0] = micros();
  if((last_channel[0] == 0) && (PINK & B00000100)){
    last_channel[0] = 1;
    timer[1] = timer[0];
  }else if((last_channel[0] == 1) && !(PINK & B00000100)){
    last_channel[0] = 0;
    input[0] = timer[0] - timer[1];
    direct_c1[0] = (input[0] - last_input[0]) > 0 ;
    last_input[0] = input[0];
  }
  
  timer[0] = micros();
  if((last_channel[1] == 0) && (PINK & B00000010)){
    last_channel[1] = 1;
    timer[2] = timer[0];
  }else if((last_channel[1] == 1) && !(PINK & B00000010)){
    last_channel[1] = 0;
    input[1] = timer[0] - timer[2];
  }
  
  timer[0] = micros();
  if((last_channel[2] == 0) && (PINK & B00000001)){
    last_channel[2] = 1;
    timer[3] = timer[0];
  }else if((last_channel[2] == 1) && !(PINK & B00000001)){
    last_channel[2] = 0;
    input[2] = timer[0] - timer[3];
  }

  timer[0] = micros();
  if((last_channel[3] == 0) && (PINK & B00001000)){
    last_channel[3] = 1;
    timer[4] = timer[0];
  }else if((last_channel[3] == 1) && !(PINK & B00001000)){
    last_channel[3] = 0;
    input[3] = timer[0] - timer[4];
  }

  timer[0] = micros();
  if((last_channel[4] == 0) && (PINK & B00010000)){
    last_channel[4] = 1;
    timer[5] = timer[0];
  }else if((last_channel[4] == 1) && !(PINK & B00010000)){
    last_channel[4] = 0;
    input[4] = timer[0] - timer[5];
  }

  timer[0] = micros();
  if((last_channel[5] == 0) && (PINK & B00100000)){
    last_channel[5] = 1;
    timer[6] = timer[0];
  }else if((last_channel[5] == 1) && !(PINK & B00100000)){
    last_channel[5] = 0;
    input[5] = timer[0] - timer[6];
  }
}

void menampilkan_pwm_remote(){
  for(int channel = 0; channel < 6 ; channel++){
    Serial.print(input[channel]);
    Serial.print("\t");
  }
  Serial.println();
}

float errorM1, lastErrorM1, integralM1, derivativeM1, outputM1;
float setpointM1,Kp1,Ki1,Kd1;
int posisi_ki = 0;
int posisi_ka = 0;
int posisi = 0;

unsigned long waktu_sekarang_gps_kompas = 0;
unsigned long waktu_sebelumnya_gps_kompas = 0;

String inputString;

void Split(char* e) {
  char* v[4];
  char *p;
  int i = 0;
  p = strtok(e, ",");
  while (p && i < 4) {
    v[i] = p;
    p = strtok(NULL, ",");
    i++;
  }

  Kp1 = atof(v[0]);
  Ki1 = atof(v[1]);
  Kd1 = atof(v[2]);
  setpointM1 = atof(v[3]);
}

void setup(){
  Serial.begin(115200);
  Serial3.begin(9600);

  Kp1 = 30;
  Ki1 = 0.003;
  Kd1 = 0.005;

  //motor1
  pinMode(PWM_1,OUTPUT);
  pinMode(RPWM_1,OUTPUT);
  pinMode(LPWM_1,OUTPUT);

  //motor2
  pinMode(PWM_2,OUTPUT);
  pinMode(RPWM_2,OUTPUT);
  pinMode(LPWM_2,OUTPUT);

  //motor3
  pinMode(PWM_3,OUTPUT);
  pinMode(RPWM_3,OUTPUT);
  pinMode(LPWM_3,OUTPUT);

  //Channel input receiver remote = A8-A13
  PCICR |= (1<<PCIE2);
  PCMSK2 |= (1<<PCINT16); //A8
  PCMSK2 |= (1<<PCINT17); //A9
  PCMSK2 |= (1<<PCINT18); //A10
  PCMSK2 |= (1<<PCINT19); //A11
  PCMSK2 |= (1<<PCINT20); //A12
  PCMSK2 |= (1<<PCINT21); //A13
}

void loop(){

  if (Serial3.available() > 0) {
    inputString = Serial3.readStringUntil('\n'); // Read the input as a string
    char inputCharArray[inputString.length() + 1]; // Create a char array
    inputString.toCharArray(inputCharArray, inputString.length() + 1); // Convert string to char array
    integralM1 = 0;
    Split(inputCharArray); // Pass the char array to Split function

    // Serial3.println("Parameter PID di UPDATE");
  }

  // setpointM1 = map(input[0],1000,2000,73,116);
  // if(setpointM1 > 85 && setpointM1 < 95){
  //   setpointM1 = 90;
  // }

  posisi_ka = analogRead(A0);
  posisi_ki = analogRead(A1);

  posisi = map(posisi_ka,590,739,73,116);

  errorM1 = (setpointM1 - posisi);
  integralM1 += errorM1;
  derivativeM1 = errorM1 - lastErrorM1;
  outputM1 = Kp1 * errorM1 + Ki1 * integralM1 + Kd1 * derivativeM1;

  lastErrorM1 = errorM1;

  if(outputM1 > 255){
    outputM1 = 255;
  }else if(outputM1 < -255){
    outputM1 = -255;
  }

  if (outputM1>0){
    digitalWrite(LPWM_3, HIGH);
    digitalWrite(RPWM_3, LOW);
  } else if (outputM1<0) {
    digitalWrite(LPWM_3, LOW);
    digitalWrite(RPWM_3, HIGH);
  }

  analogWrite(PWM_3, abs(outputM1));

  //kirim serial1
  waktu_sekarang_gps_kompas = millis();
  if(waktu_sekarang_gps_kompas - waktu_sebelumnya_gps_kompas >= 100){
    waktu_sebelumnya_gps_kompas = waktu_sekarang_gps_kompas;
    Serial3.print(outputM1);
    Serial3.print(',');
    Serial3.print(errorM1);
    Serial3.print(',');
    Serial3.print(setpointM1);
    Serial3.print(',');
    Serial3.println(posisi);
  }
}