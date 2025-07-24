#include <ESP8266WiFi.h>

#define FRONT_TRIG_PIN D1
#define FRONT_ECHO_PIN D2
#define BACK_TRIG_PIN  D3
#define BACK_ECHO_PIN  D4
#define LED_CLIENT     LED_BUILTIN
#define LED_AP         D7
#define LED_ALERT      D5
#define BUTTON_PIN     D6

WiFiServer server(80);
WiFiClient client;

bool isAP = false;
bool buttonPressed = false;
long frontDuration, backDuration;
int frontDistance, backDistance;

void setup() {
  Serial.begin(115200);
  pinMode(FRONT_TRIG_PIN, OUTPUT); pinMode(FRONT_ECHO_PIN, INPUT);
  pinMode(BACK_TRIG_PIN, OUTPUT);  pinMode(BACK_ECHO_PIN, INPUT);
  pinMode(LED_CLIENT, OUTPUT); pinMode(LED_AP, OUTPUT); pinMode(LED_ALERT, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
}

void loop() {
  int networks = WiFi.scanNetworks();
  if (networks >= 2) {
    becomeAP("Car2_AP", "12345678");
  } else {
    connectToAP("Car3_AP", "12345678")
  }

  if (isAP) {
    WiFiClient newClient = server.available();
    if (newClient) {
      handleCommunication(newClient);
    }
  } else {
    if (!client.connected()) {
      client.connect("192.168.4.1", 80);
    }
    if (client.connected() && client.available()) {
      Serial.println("Received: " + client.readStringUntil('\n'));
      digitalWrite(LED_CLIENT, HIGH);
    } else {
      digitalWrite(LED_CLIENT, LOW);
    }
  }

  delay(1000);
}

void becomeAP(const char* ssid, const char* pass) {
  if (!isAP) {
    WiFi.softAP(ssid, pass);
    server.begin();
    isAP = true;
    Serial.println("Acting as AP");
  }
  digitalWrite(LED_AP, HIGH);
}

void connectToAP(const char* ssid, const char* pass) {
  if (isAP) {
    WiFi.softAPdisconnect(true);
    isAP = false;
  }
  WiFi.begin(ssid, pass);
  digitalWrite(LED_AP, LOW);
}

void handleCommunication(WiFiClient& newClient) {
  int f = readUltrasonic(FRONT_TRIG_PIN, FRONT_ECHO_PIN);
  int b = readUltrasonic(BACK_TRIG_PIN, BACK_ECHO_PIN);

  if (f < 20 || b < 20) {
    digitalWrite(LED_ALERT, HIGH);
    newClient.println("Too close");
  } else {
    digitalWrite(LED_ALERT, LOW);
  }

  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
    newClient.println("I am overtaking!");
    buttonPressed = true;
  } else if (digitalRead(BUTTON_PIN) == HIGH) {
    buttonPressed = false;
  }
}

int readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}
