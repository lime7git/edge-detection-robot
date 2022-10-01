#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define PWMA   5  
#define PWMB   6                
#define AIN2   A0       
#define AIN1   A1       
#define BIN1   A2          
#define BIN2   A3     

#define ECHO   2
#define TRIG   3

#define OLED_RESET 9   
#define OLED_SA0   8
       
int Distance = 0;
int DistancesMean = 0;
int StepsOfCalibration = 10;
int DistanceFactor = 2;
int DistanceThreshold = 0;
int SpeedR = 43;
int SpeedL = 40;
unsigned long LastTime = 0;
unsigned int counter = 0;

int DistanceTest();
void Forward();
void Backward();
void Right();
void Left();
void Rotate180();
void RotateRandom();
void Stop();
void EdgeDetected();

Adafruit_SSD1306 oled(OLED_RESET, OLED_SA0);
void OledWriteDistance(int distance);

// PCF8547 gpio expander
void PCF8574Write(byte data);
byte PCF8574Read();
bool ButtonRead();

bool button;

int Distance_test()         // Measure the distance 
{
  digitalWrite(TRIG, LOW);   // set trig pin low 2μs
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);  // set trig pin 10μs , at last 10us 
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);    // set trig pin low
  float Fdistance = pulseIn(ECHO, HIGH);  // Read echo pin high level time(us)
  Fdistance= Fdistance/58;       //Y m=（X s*344）/2
  // X s=（ 2*Y m）/344 ==》X s=0.0058*Y m ==》cm = us /58       
  return (int)Fdistance;
}  
void Forward()
{
  analogWrite(PWMA,SpeedL);
  analogWrite(PWMB,SpeedR);
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,HIGH); 
  digitalWrite(BIN2,LOW); 
}

void Backward()
{
  analogWrite(PWMA,SpeedL);
  analogWrite(PWMB,SpeedR);
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,LOW);  
  digitalWrite(BIN2,HIGH); 
}

void Right()
{
  analogWrite(PWMA,25);
  analogWrite(PWMB,27);
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,HIGH); 
  digitalWrite(BIN2,LOW);  
}

void Left()
{
  analogWrite(PWMA,25);
  analogWrite(PWMB,27);
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,LOW); 
  digitalWrite(BIN2,HIGH);  
}
void Rotate180()
{
  Right();
  delay(700);
  Stop();
  delay(100);
}
void RotateRandom()
{
  long randomNumber;
  randomNumber = random(1,4);
  if(randomNumber == 1)
  {
    Right();
    delay(650);
  }
  else if(randomNumber == 2)
  {
    Left();
    delay(650);
  }
  else if(randomNumber == 3)
  {
    Right();
    delay(1300);
  }
  else
  {
    Right();
    delay(1350);
  }

  Stop();
  delay(100);
}
void Stop()
{
  analogWrite(PWMA,0);
  analogWrite(PWMB,0);
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,LOW); 
  digitalWrite(BIN2,LOW);  
}
void EdgeDetected()
{
  Stop();
    oled.clearDisplay();
    oled.setTextSize(2);
    oled.setTextColor(1);
    oled.setCursor(0,20);
    oled.println("EDGE!");
    oled.display();
  delay(200);
  Backward();
  delay(500);
  Stop();
  delay(100);
  RotateRandom(); 

  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(1);
  oled.setCursor(0,20);
  oled.println("RUNNING!");
  oled.display();
}


void OledWriteDistance(int distance)
{
  oled.clearDisplay();
  oled.setTextSize(3);
  oled.setTextColor(1);
  oled.setCursor(30,20);
  oled.println(distance);
  oled.display();
}

void PCF8574Write(byte data)
{
  Wire.beginTransmission(0x20);
  Wire.write(data);
  Wire.endTransmission(); 
}

byte PCF8574Read()
{
  int data = -1;
  Wire.requestFrom(0x20, 1);
  if(Wire.available()) {
    data = Wire.read();
  }
  return data;
}

bool ButtonRead()
{
  byte value = 0;
  bool pressed = false;

  PCF8574Write(0x1F | PCF8574Read());
  value = PCF8574Read() | 0xE0;

  if(value == 0xEF) pressed = true;

  return pressed;
}

void setup() 
{
  pinMode(PWMA,   OUTPUT);  
  pinMode(PWMB,  OUTPUT);                      
  pinMode(AIN1,   OUTPUT);      
  pinMode(AIN2,   OUTPUT);
  pinMode(BIN1,  OUTPUT);     
  pinMode(BIN2,  OUTPUT);  

  pinMode(ECHO, INPUT);    // Define the ultrasonic echo input pin
  pinMode(TRIG, OUTPUT);   // Define the ultrasonic trigger input pin   

  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(1);
  oled.setCursor(0,20);
  oled.println("Press center");
  oled.display();

  button = false;
  while(!button)
  {
    button = ButtonRead();
    delay(50);
  }
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(1);
  oled.setCursor(0,20);
  oled.println("CALIBRATION!");
  oled.display();

  for(int i = 0; i < StepsOfCalibration; i++)
  {
    DistancesMean += Distance_test();
  }
  DistanceThreshold = (DistancesMean / StepsOfCalibration) + DistanceFactor;
  
  delay(3500);
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(1);
  oled.setCursor(0,20);
  oled.println("RUNNING!");
  oled.display();
}

void loop() 
{
      Distance = Distance_test();     

        if(Distance > DistanceThreshold)
        {
          EdgeDetected();
        }
        else
        {
          Forward();
        }
} 
