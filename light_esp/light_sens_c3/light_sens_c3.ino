#define PHOTO_DETECTOR_PIN 0 
#define MAX_LIGHT_DETECt 3300
#define LED_PIN 10
#define FREQ 5000
#define RES 8
#define MAX_PWM 255

void set_led_level(int actual_light_level);

int sensorValue = 0; 
int light_level_of_darknes = 1800;
int set_light_level = 2500;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ledcAttach(LED_PIN, FREQ, RES);
  Serial.println("im alive");
  // pinMode(PHOTO_DETECTOR_PIN, OUTPUT);
}

void loop() {

  sensorValue = analogRead(PHOTO_DETECTOR_PIN);
  Serial.print("value of detect light: ");
  Serial.println(sensorValue);
  set_led_level(sensorValue);
  delay(100);
}

void set_led_level(int actual_light_level){
  int error = set_light_level - actual_light_level;
  if (error <= 0){
    ledcWrite(LED_PIN, 10);
  }else{
    float pulse_width_f = float(error) / float(set_light_level - light_level_of_darknes);
    int pulse_width = pulse_width_f * 255;
    if(pulse_width > MAX_PWM)
      pulse_width = MAX_PWM;
    ledcWrite(LED_PIN, pulse_width);
  }
}
