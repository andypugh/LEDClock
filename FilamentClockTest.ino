
int count = 0;

void setup()
{
  Serial.begin(9600);

  Serial.println("Starting");
  // Minutes
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  // 10s of minutes
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  // Hours
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(A5, OUTPUT);
  // Tens of Hours
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  // Blanking
  pinMode(A4, OUTPUT);
  digitalWrite(A4, 1);
}

void loop()
{
  Serial.println(count);
  //Tens of Minutes
  digitalWrite(3, count & 1);
  digitalWrite(4, count & 2);
  digitalWrite(6, count & 4);
  digitalWrite(5, count & 8);
  // Hours
  digitalWrite(7, count & 1);
  digitalWrite(8, count & 2);
  digitalWrite(9, count & 4);
  digitalWrite(10, count & 8);
  //Tens of Hours
  digitalWrite(11, count & 1);
  digitalWrite(12, count & 2);
  digitalWrite(13, count & 4);
  digitalWrite(A5, count & 8);
  // Minutes
  digitalWrite(A0, count & 1);
  digitalWrite(A1, count & 2);
  digitalWrite(A2, count & 4);
  digitalWrite(A3, count & 8);

  count = (count < 9) ? count + 1 : 0;
  
  delay(1000);
}
