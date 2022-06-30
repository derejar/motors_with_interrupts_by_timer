#define FIRST_MOTOR_STEP_PIN 45
#define FIRST_MOTOR_DIR_PIN 33
#define FIRST_MOTOR_5V 47
#define FIRST_MOTOR_ENABLE_PIN 35

volatile long firstMotorSteps = 0; // change for doing steps. Negative number = CW.
void configureFirstMotor()
{
  pinMode(FIRST_MOTOR_STEP_PIN, 1);
  pinMode(FIRST_MOTOR_DIR_PIN, 1);
  pinMode(FIRST_MOTOR_5V, 1);
  pinMode(FIRST_MOTOR_ENABLE_PIN, 1);
  configureTimer4();
}

void configureTimer4()
{
  cli();
  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= 0b00000100;
  TIMSK4 |= 0b00000010;
  OCR4A = 78; //interval between calls; interval = OCR4A * 16 = 1248 microseconds
  sei();
}

void doFirstMotorStep()
{
  digitalWrite(FIRST_MOTOR_ENABLE_PIN, 1);
  digitalWrite(FIRST_MOTOR_5V, 1);
  digitalWrite(FIRST_MOTOR_DIR_PIN, firstMotorSteps < 0); //Negative number == CW
  digitalWrite(FIRST_MOTOR_STEP_PIN, !digitalRead(FIRST_MOTOR_STEP_PIN));
  firstMotorSteps += (firstMotorSteps < 0) ? 1 : -1;
}

ISR(TIMER4_COMPA_vect)
{
  if(firstMotorSteps)
    doFirstMotorStep();
  TCNT4 = 0;
}

void setup() {
  firstMotorSteps = -800;
  configureFirstMotor();
  configureTimer4();
}

void loop() {

}
