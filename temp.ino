#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include "time.h"
#include <GyverOLED.h>
#include <charMap.h>
#include <icons_7x7.h>
#include <icons_8x8.h>
#include <DHT.h>
#include <DHT_U.h>

DHT dht(4, DHT11);

int blue = 16;
int red = 17;
int buzzpin = 12;

int mq2 = 34, mq9 = 35, mq135 = 32;
int gas2, gas9, gas135;
float R02, R09, R0135;
const float VCC = 3.3;
const int RL = 10000;
  //Gas Sensor calibration function
  float calculateRS(int analogValue) {
    float sensorVoltage = (analogValue / 4095.0) * VCC;
    return ((VCC - sensorVoltage) / sensorVoltage) * RL;
  }


  //Buzzer Functions
  void buzz_on(int repeat = 5, int onTime = 200, int offTime = 100) 
  {
    for (int i = 0; i < repeat; i++) {
      digitalWrite(buzzpin, HIGH); // Turn buzzer ON
      delay(onTime);
      digitalWrite(buzzpin, LOW);  // Turn buzzer OFF
      delay(offTime);
    }
  }

  void buzz_off() {
    digitalWrite(buzzpin, LOW);
  }


int button = 27;
int screen_state = 0;
bool last_button_state = HIGH;
int sos=26;
bool sos_triggered = false;
bool last_sos_state = HIGH;

//OLED
GyverOLED<SSH1106_128x64> oled;
const uint8_t degreeSymbol[] PROGMEM = {
  0b01110,
  0b10001,
  0b10001,
  0b01110,
  0b00000
};    //Bit-Image of degree symbol


//SD Card
const int SD_CS = 5;
unsigned long lastLogTime = 0;
const unsigned long logInterval = 10000;

//ᯤ<<<-----Wifi----->>>ᯤ
// WiFi Credentials
const char* ssid = "ABCD1234";
const char* password = "abcd1234";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

WiFiServer server(80);

void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");
}

String getFormattedDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "N/A,N/A";
  }
  char dateStr[11];
  char timeStr[9];
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  return String(dateStr) + "," + String(timeStr);
}


//Dashboard ᯤᯤᯤ
String generateHTML(float temperature, float humidity, float CO2_ppm, float CH4_ppm, float Butane_ppm,
                    float LPG_ppm, float CO_ppm, float NH4_ppm, float Benzene_ppm,
                    float Smoke_ppm, float FGAS_ppm) {

  String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5'>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #ece9e6, #ffffff); margin: 0; padding: 20px; text-align: center; animation: fadeIn 1s ease-in-out; }";
  html += "h1 { color: #2c3e50; margin-bottom: 30px; text-shadow: 2px 2px 5px #ccc; }";
  html += ".card { display: inline-block; width: 200px; background: linear-gradient(135deg, #f9f9f9, #e0e0e0); padding: 20px; margin: 15px; border-radius: 15px; box-shadow: 0 6px 15px rgba(0,0,0,0.1); transition: transform 0.3s ease, box-shadow 0.3s ease; }";
  html += ".card:hover { transform: translateY(-5px); box-shadow: 0 12px 20px rgba(0,0,0,0.15); }";
  html += ".value { font-size: 1.8em; color: #2980b9; margin-top: 8px; font-weight: bold; }";

  // Danger alert style without text shadow, with bigger font and blinking background/border
  html += ".alert { font-size: 3em; color: red; font-weight: bold; margin-bottom: 20px; padding: 20px; border: 5px solid red; border-radius: 15px; animation: blink 1s infinite, blinkBackground 1s infinite; }";
  html += "@keyframes fadeIn { from {opacity: 0;} to {opacity: 1;} }";
  html += "@keyframes blink { 0% { opacity: 1; } 50% { opacity: 0; } 100% { opacity: 1; } }";
  html += "@keyframes blinkBackground { 0% { background-color: rgba(255, 0, 0, 0.2); } 50% { background-color: rgba(255, 0, 0, 0.5); } 100% { background-color: rgba(255, 0, 0, 0.2); } }";
  
  html += "</style></head><body>";
  html += "<h1> SMART MINER HELMET DASHBOARD </h1>";

  // Show danger alert if sos_triggered is true
  if (sos_triggered) {
    html += "<div class='alert'> DANGER ALERT </div>";
  }

  html += "<div class='card'><b> Temperature</b><div class='value'>" + String(temperature) + " Deg. C</div></div>";
  html += "<div class='card'><b> Humidity</b><div class='value'>" + String(humidity) + " %</div></div>";
  html += "<div class='card'><b> CO2</b><div class='value'>" + String(CO2_ppm) + " ppm</div></div>";
  html += "<div class='card'><b> CH4</b><div class='value'>" + String(CH4_ppm) + " ppm</div></div>";
  html += "<div class='card'><b> Butane</b><div class='value'>" + String(Butane_ppm) + " ppm</div></div>";
  html += "<div class='card'><b> LPG</b><div class='value'>" + String(LPG_ppm) + " ppm</div></div>";
  html += "<div class='card'><b> CO</b><div class='value'>" + String(CO_ppm) + " ppm</div></div>";
  html += "<div class='card'><b> Ammonia</b><div class='value'>" + String(NH4_ppm) + " ppm</div></div>";
  html += "<div class='card'><b> Benzene</b><div class='value'>" + String(Benzene_ppm) + " ppm</div></div>";
  html += "<div class='card'><b> Smoke</b><div class='value'>" + String(Smoke_ppm) + " ppm</div></div>";
  html += "<div class='card'><b> FGas</b><div class='value'>" + String(FGAS_ppm) + " ppm</div></div>";

  html += "</body></html>";
  return html;
}



void setup() 
{
  Serial.begin(9600);
  dht.begin();
  delay(2000);

  pinMode(blue, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(buzzpin, OUTPUT);

  pinMode(mq2, INPUT);
  pinMode(mq9, INPUT);
  pinMode(mq135, INPUT);
  R02 = calculateRS(analogRead(mq2));
  R09 = calculateRS(analogRead(mq9));
  R0135 = calculateRS(analogRead(mq135));

  pinMode(button, INPUT_PULLUP);
  pinMode(sos, INPUT_PULLUP);

  oled.init();
  Wire.setClock(400000L);
  oled.clear();
  oled.update();

  connectToWiFi();  // ᯤᯤᯤ
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  server.begin();
  Serial.print("Web server started at: ");
  Serial.println(WiFi.localIP());

  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
  } else {
    Serial.println("SD card initialized.");
  }

  /*Serial.print("Web server started at: ");
  Serial.println(WiFi.localIP()); //*/
}

void loop() 
{
  oled.clear();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  gas2 = analogRead(mq2);
  float RS2 = calculateRS(gas2);
  float ratio2 = RS2 / R02;
  float CH4_ppm = pow(10, (-0.38 * log10(ratio2)) + 1.5);
  float LPG_ppm = pow(10, (-0.47 * log10(ratio2)) + 1.68);
  float Butane_ppm = pow(10, (-0.50 * log10(ratio2)) + 1.8);
  float Smoke_ppm = pow(10, (-0.48 * log10(ratio2)) + 1.6);

  gas9 = analogRead(mq9);
  float RS9 = calculateRS(gas9);
  float ratio9 = RS9 / R09;
  float CO_ppm = pow(10, (-0.47 * log10(ratio9)) + 1.5);
  float FGAS_ppm = pow(10, (-0.45 * log10(ratio9)) + 1.7);

  gas135 = analogRead(mq135);
  float RS135 = calculateRS(gas135);
  float ratio135 = RS135 / R0135;
  float NH4_ppm = pow(10, (-0.47 * log10(ratio135)) + 1.68);
  float Benzene_ppm = pow(10, (-0.50 * log10(ratio135)) + 1.8);
  float CO2_ppm = pow(10, (-0.38 * log10(ratio135)) + 1.7);

  bool current_sos_state = digitalRead(sos);
  if (current_sos_state == LOW && last_sos_state == HIGH) 
    {
      delay(50);
      if (digitalRead(sos) == LOW)
        sos_triggered = !sos_triggered;  // Toggle emergency mode
    }
  last_sos_state = current_sos_state;

  if(temperature>55 || CH4_ppm>=150 || LPG_ppm>=150 || Butane_ppm>=150 || Smoke_ppm>=150 || CO_ppm>=150 || FGAS_ppm>=150 || NH4_ppm>=150 || Benzene_ppm>=150 || CO2_ppm>=150 || sos_triggered)
  {
    digitalWrite(blue,LOW);
    digitalWrite(red,HIGH);
    buzz_on();
  }
  else
  {
    buzz_off();
    digitalWrite(blue,HIGH);
    digitalWrite(red,LOW);
  }

  //SD Card Data Storage
  if (millis() - lastLogTime >= logInterval) {
    bool fileExists = SD.exists("/data.csv");
    File dataFile = SD.open("/data.csv", FILE_APPEND);
    if (dataFile) {
      if (!fileExists) {
        dataFile.println("Date,Time,Temperature(deg. C),Humidity(%),CO2(ppm),CH4(ppm),Butane(ppm),LPG(ppm),CO(ppm),Ammonia(ppm),Benzene(ppm),Smoke(ppm),FGas(ppm)");
      }
      String dateTime = getFormattedDate();
      dataFile.print(dateTime); dataFile.print(",");
      dataFile.print(temperature); dataFile.print(",");
      dataFile.print(humidity); dataFile.print(",");
      dataFile.print(CO2_ppm); dataFile.print(",");
      dataFile.print(CH4_ppm); dataFile.print(",");
      dataFile.print(Butane_ppm); dataFile.print(",");
      dataFile.print(LPG_ppm); dataFile.print(",");
      dataFile.print(CO_ppm); dataFile.print(",");
      dataFile.print(NH4_ppm); dataFile.print(",");
      dataFile.print(Benzene_ppm); dataFile.print(",");
      dataFile.print(Smoke_ppm); dataFile.print(",");
      dataFile.println(FGAS_ppm);
      dataFile.close();
    }
    lastLogTime = millis();
  }

  //Toggle Button-->OLED
  bool current_button_state = digitalRead(button);
  if (current_button_state == LOW && last_button_state == HIGH) {
    delay(50);
    if (digitalRead(button) == LOW) {
      screen_state = (screen_state + 1) % 3;
    }
  }
  last_button_state = current_button_state;

  if (screen_state == 0) {
    oled.setScale(1);
    oled.setCursorXY(10, 10);
    oled.print("Temp.   : "); oled.print(temperature);
    oled.drawBitmap(98, 10, degreeSymbol, 5, 5, 0);
    oled.setCursorXY(104, 10); oled.print("C");

    oled.setCursorXY(10, 25);
    oled.print("Humid.  : "); oled.print(humidity); oled.print("%");

    oled.setCursorXY(10, 40);
    oled.print("CO2     : "); oled.print(CO2_ppm); oled.print(" ppm");

    oled.setCursorXY(10, 55);
    oled.print("WEB-"); oled.print(WiFi.localIP());
  } 
  else if (screen_state == 1) {
    oled.setCursorXY(10, 10);
    oled.print("Methane : "); oled.print(CH4_ppm); oled.print(" ppm");

    oled.setCursorXY(10, 25);
    oled.print("Butane  : "); oled.print(Butane_ppm); oled.print(" ppm");

    oled.setCursorXY(10, 40);
    oled.print("LPG     : "); oled.print(LPG_ppm); oled.print(" ppm");

    oled.setCursorXY(10, 55);
    oled.print("CO      : "); oled.print(CO_ppm); oled.print(" ppm");
  } 
  else if (screen_state == 2) {
    oled.setCursorXY(10, 10);
    oled.print("Ammonia : "); oled.print(NH4_ppm); oled.print(" ppm");

    oled.setCursorXY(10, 25);
    oled.print("Benzene : "); oled.print(Benzene_ppm); oled.print(" ppm");

    oled.setCursorXY(10, 40);
    oled.print("Smoke   : "); oled.print(Smoke_ppm); oled.print(" ppm");

    oled.setCursorXY(10, 55);
    oled.print("FGas    : "); oled.print(FGAS_ppm); oled.print(" ppm");
  }
  oled.update();
  delay(1000);
  //ᯤᯤᯤ
  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        client.readStringUntil('\r');
        client.flush();
        String html = generateHTML(temperature, humidity, CO2_ppm, CH4_ppm, Butane_ppm,
                                   LPG_ppm, CO_ppm, NH4_ppm, Benzene_ppm, Smoke_ppm, FGAS_ppm);
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println();
        client.println(html);
        break;
      }
    }
    delay(1);
    client.stop();
  }
}
