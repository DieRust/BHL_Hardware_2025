#include <WiFi.h>
#include <HTTPClient.h>

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

const int   serverPort    = 4080;
const char* serverAddress = "192.168.4.1";
String serverNameForDarkness = "http://192.168.4.1:80/getLightLevelDarknes";
String serverNameForLight = "http://192.168.4.1:80/getLightLevel";
String serverNameForTimeLed = "http://192.168.4.1:80/getTimeLed";


const char* ssid = "Oneplus";
const char* password = "Lubieroboty027";

WiFiClient TCPclient;
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

int get_time_led() {
  http.begin(TCPclient, serverNameForTimeLed.c_str());
  int httpResponseCode = http.GET();
  String payload = http.getString();
  http.end();
  return payload.toInt();
}

int get_darknes_level() {
  http.begin(TCPclient, serverNameForDarkness.c_str());
  int httpResponseCode = http.GET();
  String payload = http.getString();
  http.end();
  return payload.toInt();
}

int get_light_level() {
  http.begin(TCPclient, serverNameForLight.c_str());
  int httpResponseCode = http.GET();
  String payload = http.getString();
  http.end();
  return payload.toInt();
}

void send_to_host() {
  byte mess[5];

  mess[0] = led_array[0].state_pwm > 0 ? 1 : 0;
  mess[1] = led_array[1].state_pwm > 0 ? 1 : 0;
  mess[2] = led_array[2].state_pwm > 0 ? 1 : 0;
  mess[3] = led_array[3].state_pwm > 0 ? 1 : 0;
  mess[4] = led_array[4].state_pwm > 0 ? 1 : 0;

  TCPclient.write(mess,35);
  TCPclient.flush();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  while(!TCPclient.connect(serverAddress, serverPort)) {
    Serial.println("Failed to connect to host, retrying in 1s");
    delay(1000);
  }
  Serial.println("Connected to host");

  
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
    if(WiFi.status()== WL_CONNECTED ){ 
      light_level_of_darknes = get_darknes_level();
      set_light_level = get_light_level();
      time_light_led = get_time_led();
      Serial.println("wziolem dane");
    }else{
      Serial.println("nie ma fal");
    }
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

