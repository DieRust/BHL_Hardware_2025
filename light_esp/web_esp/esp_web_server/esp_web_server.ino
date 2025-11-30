#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <FS.h>
#include <LittleFS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"

// DHT     - adafruit
// Dallas  - Miles Burton
// Onewire - Paul Stof..

#define NUMBER_OF_LED 5

#define MIC_PIN 2
#define DHT_PIN 32
#define DALLAS_PIN 26

#define SERVER_PORT 4080

DHT dht(DHT_PIN, DHT11);

// temp
OneWire oneWire(DALLAS_PIN);
DallasTemperature sensors(&oneWire);

const char* ssid = "Oneplus";
const char* password = "Lubieroboty027";

String PARAM_MESSAGE = "status";
int polly_one = 390;
int polly_two = 430;
int light_level_of_darknes = 1800;
int set_light_level = 2500;
int time_light_led = 5;
int led_array[NUMBER_OF_LED];
int micData=0;
float humidity=0;
float temp=0;
float tempdht=0;

WiFiServer TCPserver(SERVER_PORT);
AsyncWebServer server(80);

String processor(const String& var);
String getLedStatus(int led_number);
String getPoll(int poll_number);

void setup() {
  pinMode(MIC_PIN, INPUT);
  pinMode(DHT_PIN, INPUT);
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
    //request->send(200, "text/plain", "mlem");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send(LittleFS, "/style.css", "text/css");
    request->send(200, "text/plain", "mlem");
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
  
  server.on("/getLightLevelDarknes", HTTP_GET, [](AsyncWebServerRequest *request){
	char buff[5];
	sprintf(buff, "%d", light_level_of_darknes);
    request->send(200, "text/plain", buff);
  });
  
  server.on("/getLightLevel", HTTP_GET, [](AsyncWebServerRequest *request){
	char buff[5];
	sprintf(buff, "%d", set_light_level);
    request->send(200, "text/plain", buff);
  });
  
  server.on("/getTimeLed", HTTP_GET, [](AsyncWebServerRequest *request){
	char buff[5];
	sprintf(buff, "%d", time_light_led);
    request->send(200, "text/plain", buff);
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

  sensors.begin();
  dht.begin();
  server.begin();
}

void loop() {
  micData = analogRead(MIC_PIN);
  humidity = dht.readHumidity();
  tempdht = dht.readTemperature();
  temp = sensors.getTempCByIndex(0);
  
  WiFiClient client = TCPserver.available();

  if (client) {
    uint8_t mess[5]; 
    int len = client.read(mess, 35);
    
    for (int i=0; i< NUMBER_OF_LED; i++) {
		  led_array[i] = mess[i];
    }
  }

}

String processor(const String& var){
  return "sui";
}

String getLedStatus(int led_number){
  if(led_array[led_number] > 0){
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
