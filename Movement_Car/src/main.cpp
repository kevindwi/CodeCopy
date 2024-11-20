#include <Arduino.h>

class MotorDriver {

  private:
    uint8_t _RPWM;
    uint8_t _LPWM;
    uint8_t _PWM;
    uint8_t _pwmVal;
  
  public:
    typedef enum {
      FORWARD = 0,
        BACKWARD = 1,
        STOP = -1
    } Direction;

    MotorDriver(uint8_t PWM, uint8_t RPWM, uint8_t LPWM) {
      _RPWM = RPWM;
      _LPWM = LPWM;
      _PWM = PWM;

      pinMode(_RPWM, OUTPUT);
      pinMode(_LPWM, OUTPUT);
      pinMode(_PWM, OUTPUT);
    }

    void setSpeed(unsigned short pwmVal) {
      _pwmVal = pwmVal;
    }

    void forward() {
      digitalWrite(_LPWM, HIGH);
      digitalWrite(_RPWM, LOW);

      analogWrite(_PWM, _pwmVal);
    }

    void backward() {
      digitalWrite(_LPWM, LOW);
      digitalWrite(_RPWM, HIGH);

      analogWrite(_PWM, _pwmVal);
    }

    void stop() {
      digitalWrite(_LPWM, LOW);
      digitalWrite(_RPWM, LOW);

      analogWrite(_PWM, 255);
    }

    void run(MotorDriver::Direction direction) {
      switch (direction) {
      case BACKWARD:
        this -> backward();
        break;
      case FORWARD:
        this -> forward();
        break;
      case STOP:
        this -> stop();
        break;
      }
    }
};


// receiver
#define CH1 A8
#define CH2 A9
#define CH3 A10
#define CH4 A11
#define CH5 A12
#define CH6 A13

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

int ch1Value = 0;
int ch3Value = 0;
int ch5Value = 0;

int readChannel(int channelInput, int minLimit, int maxLimit, int defaultValue){
  int ch = pulseIn(channelInput, HIGH, 30000);
  if (ch < 100) return defaultValue;
  return map(ch, 1000, 2000, minLimit, maxLimit);
}

// Read the channel and return a boolean value
bool readSwitch(byte channelInput, bool defaultValue){
  int intDefaultValue = (defaultValue)? 100: 0;
  int ch = readChannel(channelInput, 0, 100, intDefaultValue);
  return (ch > 50);
}

int SPEED = 0;

// inisialisasi motor
MotorDriver motor1(PWM_1, RPWM_1, LPWM_1);
MotorDriver motor2(PWM_2, RPWM_2, LPWM_2);
MotorDriver motor3(PWM_3, RPWM_3, LPWM_3);

void setup()
{
  Serial.begin(115200);

  // remote receiver
  pinMode(CH1, INPUT);
  pinMode(CH3, INPUT);
  pinMode(CH5, INPUT);
}

void loop()
{
  ch1Value = readChannel(CH1, -100, 100, 0);
  ch3Value = readChannel(CH3, -100, 100, -100);
  ch5Value = readSwitch(CH5, false);

  // Serial.print("Ch1: ");
  // Serial.println(ch1Value);
  Serial.print("Ch3: ");
  Serial.println(ch3Value);
  // Serial.print("Ch5: ");
  // Serial.println(ch5Value);

  if(ch3Value < -80) {
    SPEED = 0;
  } else {
    // kecepatan
    SPEED = map(ch3Value, -100, 100, 0, 255);
    Serial.print("Speed: ");
    Serial.println(SPEED);
  }
  if(SPEED > 100){
    Serial.println("Terjadi");
  }
  

  motor1.setSpeed(SPEED);
  motor2.setSpeed(SPEED);

  motor3.setSpeed(150);

  if(ch5Value == false) {
    motor1.forward();
    motor2.backward();
    Serial.println("maju");
  } else if(ch5Value == true) {
    motor1.backward();
    motor2.forward();
    Serial.println("mundur");
  } else {
    motor1.stop();
    motor2.stop();
  }

  // kanan kiri
  if(ch1Value > 1) {
    motor3.forward();
    Serial.print("kanan");
  } else if(ch1Value < -5) {
    motor3.backward();
    Serial.print("kiri");
  } else {
    motor3.stop();
  }
}
