int B = 9;
int R = 10;
int G = 11;

void setup() {
  // put your setup code here, to run once:
  for(int i=9 ; i<12 ; i++) pinMode(i, OUTPUT);
  for(int i=9 ; i<12 ; i++) analogWrite(i, 255);
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(B, 255);
  analogWrite(R, 255);
  analogWrite(G, 0);
  delay(80);
}
