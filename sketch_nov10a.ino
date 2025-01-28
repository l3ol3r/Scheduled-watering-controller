#include <ESP8266WiFi.h>
#include <WifiClient.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include <typeinfo>

#define water1 D0


const char* local_ssid = "SmartPlant";
const char* local_password = "1644164416441";
const char* home_ssid = "";
const char* home_password = "";
String local_ip = "";
String timeSchedule = "";
String dateSchedule = "";
int valueSchedule = 0;
int waterValue = 0;
bool watering = false;
RtcDS3231<TwoWire> rtcObject(Wire);

AsyncWebServer server(80);


void setup() {

  pinMode(water1, OUTPUT);
  digitalWrite(water1, HIGH);

  WiFi.mode(WIFI_AP_STA);
  
  Serial.begin(115200);
  delay(10);
  WiFi.softAP(local_ssid, local_password);
  
  server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/plain", readSchedule(request).c_str());
});
  server.on("/water", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/plain", Water(request->getParam(0)->value().toInt()).c_str());
});
  server.on("/ipcheck", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/plain", local_ip.c_str());
});
  server.on("/homeconnect", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", HomeWifiConnection(request->getParam("ssid"), request->getParam("pass")).c_str());
});

 server.begin();
 rtcObject.Begin();
}


void loop() {

  RtcDateTime currentTime = rtcObject.GetDateTime();

  String time = getTime(currentTime);
  String date = getDate(currentTime);

  if (dateSchedule.indexOf(date) != -1 & timeSchedule == time & !watering){
      Water(valueSchedule);
      watering = true;
  }
  if (timeSchedule != time){
    watering = false;
  }

  if (WiFi.status() == WL_CONNECTED){
      local_ip = WiFi.localIP().toString();
    }
  if (waterValue > 0){
    delay(1000);
    waterValue --;
  }
  else{
    digitalWrite(water1, HIGH);
    }
  
}

String readSchedule(AsyncWebServerRequest* request){
  watering = false;
  timeSchedule = request->getParam("time")->value();
  dateSchedule = request->getParam("dates")->value();
  valueSchedule = request->getParam("value")->value().toInt();
  Serial.println(timeSchedule);
  Serial.println(dateSchedule);
  return "OK";
}

String Water(int value){
    digitalWrite(water1, LOW);
    waterValue = value / 100 * 2;
    return "OK";
}

String HomeWifiConnection(AsyncWebParameter* ssid, AsyncWebParameter* pass){
  home_ssid = ssid->value().c_str();
  home_password = pass->value().c_str();
  WiFi.begin(home_ssid, home_password);
  Serial.println(WiFi.localIP().toString());
  return "OK";
}

String getTime(RtcDateTime currentTime){
  char current_hour_ch[2];
  sprintf(current_hour_ch, "%d",
    currentTime.Hour()
  );
  String current_hour = current_hour_ch;
  if (current_hour.length() < 2){
    current_hour = "0" + current_hour;
  }

  char current_minute_ch[2];
  sprintf(current_minute_ch, "%d",
    currentTime.Minute()
  );
  String current_minute = current_minute_ch;
  if (current_minute.length() < 2){
    current_minute = "0" + current_minute;
  }
  return current_hour + current_minute;
}

String getDate(RtcDateTime currentTime){
  char current_day_ch[2];
  sprintf(current_day_ch, "%d",
    currentTime.Day()
  );
  String current_day = current_day_ch;
  if (current_day.length() < 2){
    current_day = "0" + current_day;
  }
  char current_month_ch[2];
  sprintf(current_month_ch, "%d",
    currentTime.Month()
  );
  String current_month = current_month_ch;
  if (current_month.length() < 2){
    current_month = "0" + current_month;
  }
  return current_day + current_month;
}

