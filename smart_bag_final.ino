#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <WiFi.h>// Adafruit MQTT library
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
uint8_t id;
int getFingerprintIDez();
#define Light 36
#define RELAY 23
int state = 0;
const int pin = 5;
float gpslat;
float gpslon;
TinyGPS gps;
SoftwareSerial sgps(13, 12);
SoftwareSerial sgsm(4, 2);
WiFiClient  client;
void setup()
{
  sgsm.begin(9600);
  sgps.begin(9600);
  Serial.begin(9600);
  pinMode(Light, INPUT);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  Serial.println("fingertest");
  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }
  Serial.println("Waiting for valid finger...");
}
void loop()
{
  //float gpslat=11.3415;
  //float gpslon=24.5456;
  sgps.listen();
  while (sgps.available())
  {
    int c = sgps.read();
    if (gps.encode(c))
    {
      gps.f_get_position(&gpslat, &gpslon);
    }
  }
  if (digitalRead(pin) == HIGH && state == 0) {
    sgsm.listen();
    sgsm.print("\r");
    delay(1000);
    sgsm.print("AT+CMGF=1\r");
    delay(1000);
    /*Replace XXXXXXXXXX to 10 digit mobile number &
      ZZ to 2 digit country code*/
    sgsm.print("AT+CMGS=\"+919791650448\"\r");
    delay(1000);
    //The text of the message to be sent.
    sgsm.print("https://www.google.com/maps/?q=");
    sgsm.print(gpslat, 6);
    sgsm.print(",");
    sgsm.print(gpslon, 6);
    delay(1000);
    sgsm.write(0x1A);
    delay(1000);
    state = 1;
  }
  if (digitalRead(pin) == LOW) {
    state = 0;
  }
  getFingerprintIDez();
  delay(50);
}
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
}

// returns -1 if faiRELAY, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  {
    int val = analogRead(Light);
    Serial.println(val);
    if (val >= 1000)
    {
      digitalWrite(RELAY, LOW);
      Serial.println("HIGH");
    }
    else
    {
      digitalWrite(RELAY, HIGH);
      Serial.println("LOW");
    }
  }
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
