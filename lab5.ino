

// these constants describe the pins. They won't change:

const int xpin = A9;                  // x-axis of the accelerometer
const int ypin = A8;                  // y-axis
const int zpin = A10;                  // z-axis (only on 3-axis models)

const int red = PB_2;

 const int x = 2068;
  const int y = 2048;
  const int z = 1420;
  const float k = 0.015;
  
#define RED 15
#define YELLOW 18
#define GREEN 19
void setup()
{
  // initialize the serial communications:
  //Serial.begin(9600);
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
}

void loop()
{
  
  int x0 = analogRead(xpin);
  int y0 = analogRead(ypin);
  int z0 = analogRead(zpin);
  
  int T_Vector = (k * sqrt (pow((x-x0),2)+ pow((y-y0),2)+ pow((z-z0),2)));
 
 
  //Serial.print(T_Vector);
  //Serial.println();
  // delay before next reading:
  delay(10);
  
  if(T_Vector <= 9,8)
  {
    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, LOW);
    digitalWrite(GREEN, HIGH);
  }
  else if (T_Vector <= 12)
  {
    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, HIGH);
    digitalWrite(GREEN, HIGH);
  }
  else 
  {
    digitalWrite(RED, HIGH);
    digitalWrite(YELLOW, HIGH);
    digitalWrite(GREEN, HIGH);
  }
}
