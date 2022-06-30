#define FIRST_MOTOR_STEP_PIN 45
#define FIRST_MOTOR_DIR_PIN 33
#define FIRST_MOTOR_5V 47
#define FIRST_MOTOR_ENABLE_PIN 35

#define SECOND_MOTOR_STEP_PIN 49
#define SECOND_MOTOR_DIR_PIN 51
#define SECOND_MOTOR_ENABLE_PIN 53

#define IN1 37
#define IN2 39
#define IN3 41
#define IN4 43

#define CW false
#define CCW true
////////////////////////////////////////////////////////////////////////
volatile long firstMotorSteps = 0; // change for doing steps. Negative number = CW. 800 = 1 rotation
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
////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////
bool dir;
long steps;

double speed;
volatile unsigned long interval;
const int actions[4] = {0b1001, 0b0101, 0b0110, 0b1010};
int action = 0;

double maxSpeed = 250.0;
int acceleration = 0;
long stepsBeforeDeaccelerate = 0;
int coefficient = 1;

void doStep(const bool (&modes)[4])
{
  digitalWrite(IN1, modes[0]); 
  digitalWrite(IN2, modes[1]);
  digitalWrite(IN3, modes[2]);
  digitalWrite(IN4, modes[3]); 
}

bool step()
{
  int current = actions[dir ? action : action ^ 0b11];
  bool modes[4];
  for(int i = 3; i >= 0; --i, current >>= 1)
    modes[i] = current & 1;
  doStep(modes);
  ++action;
  action %= 4;
  --steps;
  if(acceleration)
  {
    if(coefficient)
    {
      speed += coefficient * acceleration / 1000000.0 * interval;
      interval = 1000000.0 / speed;
    }
    if(speed >= maxSpeed && coefficient)
    {
      coefficient = 0;
      stepsBeforeDeaccelerate = stepsBeforeDeaccelerate * 2 - steps + 2;
    }
    if(steps == stepsBeforeDeaccelerate)
      coefficient = -1;
    configureTimer();
  }
  return steps;
}

void setSteps(long msteps = 0, bool mdir = CW, double mspeed = 50.0, double mmaxSpeed = 200.0, int maccel = 50)
{
  steps = msteps;
  dir = mdir;
  speed = mspeed;
  if(speed <= 1.0)
    speed = 1.0;
  maxSpeed = mmaxSpeed;
  if(maxSpeed < speed)
    maxSpeed = speed;
  acceleration = maccel;

  interval = 1000000.0 / speed;
  coefficient = 1;
  stepsBeforeDeaccelerate = steps / 2;
  action = 0;

  configureTimer();
}

void configureTimer()
{
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= 0b00000100;
  TIMSK1 |= 0b00000010;
  OCR1A = interval / 16;
//  configureTimer3();
  sei();
}

ISR(TIMER1_COMPA_vect)
{
  TCNT1 = 0;
  if(steps)
    step();
}

void configureThirdMotor()
{
  pinMode(IN1, 1);
  pinMode(IN2, 1);
  pinMode(IN3, 1);
  pinMode(IN4, 1);
}

void setup() {
  configureFirstMotor();
  configureSecondMotor();
  configureThirdMotor();
  Serial.begin(115200);
}

void loop() {
  if(Serial.available())
  {
    char input = Serial.read();
    switch(input)
    {
      case '1': firstMotorSteps = 800; break;
      case '2': firstMotorSteps = -800; break;
      case '3': secondMotorSteps = 1600; break;
      case '4': secondMotorSteps = -1600; break;
      case '5': setSteps(100, CW, 20, 200.0, 50); break;
      case '6': setSteps(100, CCW, 20, 200.0, 50); break;
    }
    Serial.println(input);
  }
}
