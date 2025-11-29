#define PHOTO_DETECTOR_PIN 0 
#define MAX_LIGHT_DETECt 3300
#define LED_PIN 10
#define MIN_PWM 10
#define FREQ 5000
#define RES 8
#define MAX_PWM 255
#define TRIGPIN 21
#define ECHOPIN 20
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define DETECT_DISTANCE 10.0 //value between 0-255

void set_led_level(int actual_light_level);
void setup_distance_sensor();
float measure_distance();

int sensorValue = 0; 
int light_level_of_darknes = 1800;
int set_light_level = 2500;
int motion_detect = 0;
unsigned long currentTime;
unsigned long prevTime;
int time_light_led = 5; // time in seconds
int samples_of_time = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ledcAttach(LED_PIN, FREQ, RES);
  setup_distance_sensor();
  Serial.println("im alive");

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
      samples_of_time = time_light_led * 10;
    }

    if(samples_of_time > 0){
      sensorValue = analogRead(PHOTO_DETECTOR_PIN);
      Serial.print("value of detect light: ");
      Serial.println(sensorValue);
      set_led_level(sensorValue);
      samples_of_time--;
    }else{
      ledcWrite(LED_PIN, 0);
    }
  }
  
}

void set_led_level(int actual_light_level){
  int error = set_light_level - actual_light_level;
  if (error <= 0){
    ledcWrite(LED_PIN, MIN_PWM);
  }else{
    float pulse_width_f = float(error) / float(set_light_level - light_level_of_darknes);
    int pulse_width = pulse_width_f * 255;
    if(pulse_width > MAX_PWM)
      pulse_width = MAX_PWM;
    ledcWrite(LED_PIN, pulse_width);
  }
}

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
