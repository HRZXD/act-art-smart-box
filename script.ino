#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define OLED_RESET 4
#define R_PIN 18
#define G_PIN 19
#define B_PIN 2
#define R_PIN_2 27
#define G_PIN_2 26
#define B_PIN_2 25
#define TRIG_PIN 23
#define ECHO_PIN 13
#define debounce_time 50
#define buzzer 5
#define BUTTON_PIN 14

Adafruit_SSD1306 display(OLED_RESET);
Servo myservo;

float duration_us, distance_cm; 
int num_of_letter = 0;
bool checkDb = true;
bool checkSv = true;
const int sensor = 34;
int val = 0;
int btn;
// wifi connect
WiFiClient   espClient;
PubSubClient client(espClient);
const char* ssid = "HRZ";
const char* password = "harislolza12";
const char* mqtt_broker = "broker.hivemq.com";
const int   mqtt_port = 1883;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
  payload[length]='\0';
  // if(strcmp(topic,"actart/index")==0)
  // {
  //   char str[1000];
  //   sprintf(str, "%d", num_of_letter);
  //   client.publish("actart/index",str);
  // }
  if(strcmp(topic,"actart/button")==0){
    num_of_letter = 0;
    display.clearDisplay();
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(num_of_letter);
    display.display();
    client.publish("actart/index","0");
  }
     if(val < 3000){
    client.publish("actart/weather","Raining");
    delay(1000);
   }else{
    client.publish("actart/weather","Sunny");
    delay(1000);
   }
}

void reconnect() {
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected())
  {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str()))
      Serial.println("Public emqx mqtt broker connected");
    else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() { 

  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  //  buzzer and rgb
   pinMode(buzzer,OUTPUT);
   pinMode(TRIG_PIN, OUTPUT); 
   pinMode(ECHO_PIN, INPUT);
   pinMode(R_PIN, OUTPUT);
   pinMode(G_PIN, OUTPUT);
   pinMode(B_PIN, OUTPUT);
   pinMode(R_PIN_2, OUTPUT);
   pinMode(G_PIN_2, OUTPUT);
   pinMode(B_PIN_2, OUTPUT);

   pinMode(BUTTON_PIN, INPUT_PULLUP);
   //servo
   myservo.attach(12);
   //wifi connect
   setup_wifi();
   reconnect();

   client.subscribe("actart/#");
}


void loop() {
  btn = digitalRead(BUTTON_PIN);
  Serial.println(btn);
  client.loop();
  //button
  if(btn == 0){
    num_of_letter = 0;
    client.publish("actart/index","0");
    display.clearDisplay();
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(num_of_letter);
    display.display();
  }

  //  ultrasonic check
   digitalWrite(TRIG_PIN, HIGH); 
   delayMicroseconds(10); 
   digitalWrite(TRIG_PIN, LOW); 
   duration_us = pulseIn(ECHO_PIN, HIGH); 
   distance_cm = 0.017 * duration_us;
   if (checkDb == false && (distance_cm >= 14 && distance_cm <= 60)){
      checkDb = true;
   }

  val = analogRead(sensor); 
   Serial.println(val);
   if(val < 3000){
    checkSv == false;
    analogWrite(R_PIN_2, 255);
    analogWrite(G_PIN_2, 0);
    analogWrite(B_PIN_2, 0);
    myservo.write(0);
    delay(1000);
   }else{
    checkSv == true;
    analogWrite(R_PIN_2, 173);
    analogWrite(G_PIN_2, 216);
    analogWrite(B_PIN_2, 255);
    myservo.write(90);
    delay(1000);
   }

   if (distance_cm >= 14 && distance_cm <= 60) {
      analogWrite(R_PIN, 0);
      analogWrite(G_PIN, 255);
      analogWrite(B_PIN, 0);
      digitalWrite(buzzer,HIGH);
   } else if(checkDb == true && checkSv == true) {
      checkDb = false;
      analogWrite(R_PIN, 255);
      analogWrite(G_PIN, 0);
      analogWrite(B_PIN, 0);
      digitalWrite(buzzer,LOW);
      num_of_letter += 1;
      display.clearDisplay();
      display.setTextSize(4);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println(num_of_letter);
      display.display();
      char str[1000];
      sprintf(str, "%d", num_of_letter);
      client.publish("actart/index",str);
   }
   Serial.print(distance_cm); 
   Serial.println(" cm");
   Serial.print(num_of_letter);
   Serial.println("s");

}