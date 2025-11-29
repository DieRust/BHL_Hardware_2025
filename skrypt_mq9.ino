// Odczyt czujnika MQ-9 na ESP32-C6
// AOUT -> GPIO4 (ADC1_CH3)
#define PACKAGE_SIZE 10

const int MQ9_PIN = 4;  // Pin ADC

float data_tab[PACKAGE_SIZE] = {0};
int i_data_tab = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Konfiguracja ADC dla ESP32-C6
  analogReadResolution(12);    // 0 - 4095
  analogSetAttenuation(ADC_0db);  
  // ADC_0db = wejście max ~1.0 V (ESP32-C6 tak ma)

  Serial.println("Start pomiaru MQ-9 (ESP32-C6)");
}

void loop() {
  int raw = analogRead(MQ9_PIN);

  // Napięcie wejściowe (max 1.0V)
  
  float voltage = raw * (1.0 / 4095.0);
  data_tab[i] = voltage;
  i_data_tab++;
  if (i_data_tab >= PACKAGE_SIZE){
    i_data_tab = 0;
    send_data();
    for (int j = 0; j > PACKAGE_SIZE; j++){
      data_tab[j] = 0;
    }
  }

  delay(500);
}

void send_data(){
  for (int a = 0; a <= PACKAGE_SIZE; a++){
    Serial.println(data_tab[a],3);
  }
  
}
