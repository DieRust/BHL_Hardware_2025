#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <FS.h>
#include <LittleFS.h>

#define NUMBER_OF_LED 5

#define SERVER_PORT 4080

typedef struct ledstruct{
  int pin;
  int samples_of_time;
  int state_pwm; 
} Ledstruct;

const char* ssid = "Oneplus";
const char* password = "Lubieroboty027";

String PARAM_MESSAGE = "status";
int polly_one = 390;
int polly_two = 430;
int light_level_of_darknes = 1800;
int set_light_level = 2500;
int time_light_led = 5;
Ledstruct led_array[NUMBER_OF_LED];

WiFiServer TCPserver(SERVER_PORT);
AsyncWebServer server(80);

String processor(const String& var);
String getLedStatus(int led_number);
String getPoll(int poll_number);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });

  server.on("/lamp1", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getLedStatus(0).c_str());
  });

  server.on("/lamp2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getLedStatus(1).c_str());
  });

  server.on("/lamp3", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getLedStatus(2).c_str());
  });

  server.on("/lamp4", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getLedStatus(3).c_str());
  });

  server.on("/lamp5", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getLedStatus(4).c_str());
  });

  server.on("/poll1", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getPoll(0).c_str());
  });

  server.on("/poll2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getPoll(1).c_str());
  });

  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request){
    String msg;
    if (request->hasParam(PARAM_MESSAGE)) {
            msg = request->getParam(PARAM_MESSAGE)->value();
            Serial.println(msg);
            int index_of_space = msg.indexOf(' ');
            int index_of_hash = msg.indexOf('.');
            light_level_of_darknes = msg.substring(0,index_of_space).toInt();
            time_light_led = msg.substring(index_of_space+1,index_of_hash).toInt();
            set_light_level = int(msg.substring(index_of_hash+1).toFloat()/100 * 3000);
            Serial.print("poziom czulosci swiatla: ");
            Serial.println(light_level_of_darknes);
            Serial.print("czas swiecenia swiatla: ");
            Serial.println(time_light_led);
            Serial.print("wymagany poziom jasnosci ulic: ");
            Serial.println(set_light_level);
        } else {
            msg = "No message sent";
        }
  });

  
  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  WiFiClient client = TCPserver.available();

  if (client) {
    uint8_t mess[35]; 
    int len = client.read(mess, 35);
    int id = mess[0];
    if (id < max_c) {
      user_array[id].id = mess[0];
      for (int i{0}; i<16; i++)
        user_array[id].name[i] = mess[1+i];

      uint32_t temp;
      temp = mess[20];
      temp |= mess[19]<<8;
      temp |= mess[18]<<16;
      temp |= mess[17]<<24;
      user_array[id].temp = ((float)temp);

      temp = mess[24];
      temp |= mess[23]<<8;
      temp |= mess[22]<<16;
      temp |= mess[21]<<24;
      user_array[id].presure = ((float)temp);

      user_array[id].battery = mess[25];

      user_array[id].saturation = mess[26];

      user_array[id].humidity = mess[27];

      user_array[id].heart_rate = mess[28];

      uint16_t CO2;
      CO2 = mess[30];
      CO2 |= mess[29];
      user_array[id].C02 = CO2;
      temp = mess[34];
      temp |= mess[33]<<8;
      temp |= mess[32]<<16;
      temp |= mess[31]<<24;
      user_array[id].rtctime = temp;
    }
    Serial.print(user_array[id].id);
    Serial.print(" ");
    Serial.print(id);
    Serial.print(" ");
    Serial.println(user_array[id].rtctime);
    client.stop();
  }

}

String processor(const String& var){
  return "sui";
}

String getLedStatus(int led_number){
  if(led_array[led_number].state_pwm > 0){
    return "on";
  }else{
    return "off";
  }
}

String getPoll(int poll_number){
  if(poll_number){
    return String(polly_one);
  }else{
    return String(polly_two);
  }
}
