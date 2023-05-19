#define PIN_METER_L 9
#define PIN_METER_R 10

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  pinMode(PIN_METER_R, OUTPUT);
  pinMode(PIN_METER_L, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  char buf[3];
  char levelL, levelR;
  while (Serial.available()) {
    char c = Serial.read();
    buf[0] = buf[1];
    buf[1] = buf[2];
    buf[2] = c;
    if (c == '\x00') {
      levelL = buf[0];
      levelR = buf[1];
      meterOut(levelL, levelR);
    }
  }
}

void meterOut(char levelL, char levelR) {
  analogWrite(PIN_METER_L, levelL);
  analogWrite(PIN_METER_R, levelR);
}
