int SensorPin_1 = A0;
int SensorPin_2 = A1;
int SensorPin_3 = A2;
int SensorPin_4 = A3;

int B = 9;
int R = 10;
int G = 11;

float weight;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for(int i=9 ; i<12 ; i++) pinMode(i,OUTPUT);
  for(int i=9 ; i<12 ; i++) analogWrite(i,255);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  int SensorReading_1 = analogRead(SensorPin_1);
  int SensorReading_2 = analogRead(SensorPin_2);
  int SensorReading_3 = analogRead(SensorPin_3);
  int SensorReading_4 = analogRead(SensorPin_4);

  int mfsr_r18_1 = map(SensorReading_1, 0, 1024, 0, 255);
  int mfsr_r18_2 = map(SensorReading_2, 0, 1024, 0, 255);
  int mfsr_r18_3 = map(SensorReading_3, 0, 1024, 0, 255);
  int mfsr_r18_4 = map(SensorReading_4, 0, 1024, 0, 255);

  weight = (mfsr_r18_1+mfsr_r18_2+mfsr_r18_3+mfsr_r18_4)/4.00+1.00;

  if(weight > 2.350){
    analogWrite(B, 0);
    analogWrite(R, 255);
    analogWrite(G, 0);
  }else if(weight > 1.900){
    analogWrite(B, 255);
    analogWrite(R, 0);
    analogWrite(G, 0);
  }else{
    analogWrite(B, 0);
    analogWrite(R, 0);
    analogWrite(G, 255);
  }
  
  Serial.println(weight); 

  delay(100);
}
