#define SECOND_MOTOR_STEP_PIN 49
#define SECOND_MOTOR_DIR_PIN 51
#define SECOND_MOTOR_ENABLE_PIN 53

volatile long secondMotorSteps = 0; // change for doing steps. Negative number = CW. 1600 = 1 rotation
void configureSecondMotor()
{
  pinMode(SECOND_MOTOR_STEP_PIN, 1);
  pinMode(SECOND_MOTOR_DIR_PIN, 1);
  pinMode(SECOND_MOTOR_ENABLE_PIN, 1);
  configureTimer3();
}

void configureTimer3()
{
  cli();
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3B |= 0b00000100;
  TIMSK3 |= 0b00000010;
  OCR3A = 39; //interval between calls; interval = OCR4A * 16 = 624 microseconds
  sei();
}

void doSecondMotorStep()
{
  digitalWrite(SECOND_MOTOR_ENABLE_PIN, 0);
  digitalWrite(SECOND_MOTOR_DIR_PIN, secondMotorSteps < 0);
  digitalWrite(SECOND_MOTOR_STEP_PIN, !digitalRead(SECOND_MOTOR_STEP_PIN));
  secondMotorSteps += (secondMotorSteps < 0) ? 1 : -1;
}

ISR(TIMER3_COMPA_vect)
{
  if(secondMotorSteps)
    doSecondMotorStep();
  TCNT3 = 0;
}

void setup() {
  secondMotorSteps = 1600;
  configureSecondMotor();
}

void loop() {

}
