#include <WiFi.h>
#include <HTTPClient.h>
#include "AsyncUDP.h"
#include <stdlib.h>

#define PHOTO_DETECTOR_PIN 0 
#define MAX_LIGHT_DETECt 3300
#define LED_PIN 100
#define MIN_PWM 10
#define FREQ 5000
#define RES 8
#define MAX_PWM 255
#define TRIGPIN 21
#define ECHOPIN 20
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define DETECT_DISTANCE 10.0 //value between 0-255
#define NUMBER_OF_LED 5

typedef struct ledstruct{
  int pin;
  int samples_of_time;
  int state_pwm; 
} Ledstruct;

const uint16_t   serverPort    = 4080;
const char* serverAddress = "192.168.137.186";
String serverNameForDarkness = "http://192.168.137.186:80/getLightLevelDarknes";
String serverNameForLight = "http://192.168.137.186:80/getLightLevel";
String serverNameForTimeLed = "http://192.168.137.186:80/getTimeLed";


const char* ssid = "Oneplus";
const char* password = "Lubieroboty027";

// WiFiClient TCPclient;
AsyncUDP udp;
AsyncUDP udp_in;
HTTPClient http;

void set_led_level(Ledstruct *led, int actual_light_level);
void setup_distance_sensor();
void smoth_startup(int led_pin, int aim_pwm);
void smoth_turnoff(int led_pin, int actual_pwm);
float measure_distance();

int sensorValue = 0; 
int light_level_of_darknes = 1800;
int set_light_level = 2500;
unsigned long currentTime;
unsigned long prevTime;
unsigned long prevTimeServer;
int time_light_led = 5; // time in seconds
int samples_of_time = 0;
String PARAM_MESSAGE = "status";

Ledstruct led_array[NUMBER_OF_LED];

void send_to_host() {
  byte mess[5];

  mess[0] = led_array[0].state_pwm > 0 ? '1' : '0';
  mess[1] = led_array[1].state_pwm > 0 ? '1' : '0';
  mess[2] = led_array[2].state_pwm > 0 ? '1' : '0';
  mess[3] = led_array[3].state_pwm > 0 ? '1' : '0';
  mess[4] = led_array[4].state_pwm > 0 ? '1' : '0';
  udp.broadcastTo(mess, 5, 1234);
  // TCPclient.write(mess,35);
  // TCPclient.flush();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  Serial.println("Connected to host");

  if (udp.connect(IPAddress(192, 168, 1, 100), 1234)) {
    Serial.println("UDP connected");
    udp.onPacket([](AsyncUDPPacket packet) {
      Serial.print("UDP Packet Type: ");
      Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
      Serial.print(", From: ");
      Serial.print(packet.remoteIP());
      Serial.print(":");
      Serial.print(packet.remotePort());
      Serial.print(", To: ");
      Serial.print(packet.localIP());
      Serial.print(":");
      Serial.print(packet.localPort());
      Serial.print(", Length: ");
      Serial.print(packet.length());
      Serial.print(", Data: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();
      //reply to the client
      packet.printf("Got %u bytes of data", packet.length());
    });}
    //Send unicast
    udp.print("Hello Server!");

  if (udp_in.listen(1232)) {
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.localIP());
    udp_in.onPacket([](AsyncUDPPacket packet) {
      Serial.write(packet.data(), packet.length());
      Serial.println();
      String msg="";
      for(int i =0; i<packet.length(); i++){
        msg.concat(char(packet.data()[i]));
      }
      int index_d = msg.indexOf('d');
      int index_t = msg.indexOf('t');
      int index_l = msg.indexOf('l');

      light_level_of_darknes = msg.substring(index_d+1,index_t).toInt();
      time_light_led = msg.substring(index_t+1,index_l).toInt();
      set_light_level = msg.substring(index_l+1).toInt();
      Serial.println(light_level_of_darknes);
      Serial.println(time_light_led);
      Serial.println(set_light_level);
      // for(int i=0;i<5;i++){
      //   led_array[i] = packet.data()[i]-'0';
      // }
      //reply to the client
      packet.printf("Got %u bytes of data", packet.length());
    });
  }

  
  setup_distance_sensor();
  Serial.println("im alive");
  led_array[0].pin = 10;
  led_array[1].pin = 9;
  led_array[2].pin = 8;
  led_array[3].pin = 7;
  led_array[4].pin = 6;
  for(int i = 0; i < NUMBER_OF_LED; i++){
    led_array[i].samples_of_time = 0;
    led_array[i].state_pwm = 0;
    ledcAttach(led_array[i].pin, FREQ, RES);
  }
  prevTime = millis();
}

void loop() {
  currentTime = millis();
  if((currentTime - prevTime) > 100){
    float distance = measure_distance();
    Serial.print("measure distance: ");
    Serial.println(distance);
    prevTime = currentTime;

    if(distance < DETECT_DISTANCE){
      for(int i = 0; i < NUMBER_OF_LED; i++){
        if(led_array[i].samples_of_time > 0){
          led_array[i].samples_of_time = time_light_led * 10;
        }else{
          led_array[i].samples_of_time = time_light_led * 10 + i*2;
        }
        
      }
    }

    sensorValue = analogRead(PHOTO_DETECTOR_PIN);

    for(int i = 0; i < NUMBER_OF_LED; i++){
      if(led_array[i].samples_of_time > 0){
        if(led_array[i].samples_of_time < time_light_led * 10){
          set_led_level(&led_array[i], sensorValue);
        }
        led_array[i].samples_of_time--;
        if(led_array[i].samples_of_time == 0){
          smoth_turnoff(led_array[i].pin, led_array[i].state_pwm);
          led_array[i].state_pwm = 0;
        }

      }else{
        ledcWrite(led_array[i].pin, MIN_PWM);
      }
      
    }
  }

  if((currentTime - prevTimeServer) > 1000){
    prevTimeServer = currentTime;
    send_to_host();
    Serial.println("nie ma fal");
    // if(WiFi.status()== WL_CONNECTED ){ 
    //   light_level_of_darknes = get_darknes_level();
    //   set_light_level = get_light_level();
    //   time_light_led = get_time_led();
    //   Serial.println("wziolem dane");
    // }else{
    //   Serial.println("nie ma fal");
    // }
  }
  
}

void set_led_level(Ledstruct *led, int actual_light_level){
  int error = set_light_level - actual_light_level;
  if (error <= 0){
    ledcWrite(led->pin, MIN_PWM);
  }else{
    float pulse_width_f = float(error) / float(set_light_level - light_level_of_darknes);
    int pulse_width = pulse_width_f * 255;
    if(pulse_width > MAX_PWM)
      pulse_width = MAX_PWM;
    if(led->state_pwm == 0){
      smoth_startup(led->pin, pulse_width);
    }
    led->state_pwm = pulse_width;
    ledcWrite(led->pin, pulse_width);
  }
}

void smoth_startup(int led_pin, int aim_pwm){
  for(int dutyCycle = MIN_PWM; dutyCycle <= aim_pwm; dutyCycle++){   
    // changing the LED brightness with PWM
    ledcWrite(led_pin, dutyCycle);
    delay(1);
  }
}

void smoth_turnoff(int led_pin, int actual_pwm){
for(int dutyCycle = actual_pwm; dutyCycle >= MIN_PWM; dutyCycle--){
    // changing the LED brightness with PWM
    ledcWrite(led_pin, dutyCycle);   
    delay(1);
  }
};


void setup_distance_sensor(){
  pinMode(TRIGPIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHOPIN, INPUT);
}

float measure_distance(){
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(ECHOPIN, HIGH);
  
  // Calculate the distance
  float distanceCm = duration * SOUND_SPEED/2;

  return distanceCm;
}

