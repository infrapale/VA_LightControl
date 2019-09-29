void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) ;  // Wait for serial terminal to open port before starting program
  Serial.println("TK Light Terminal");
  String meas = "ABCDEF";
  Serial.println(meas);
  meas += 12345;
  Serial.println(meas);

}

void loop() {
  // put your main code here, to run repeatedly:

}
