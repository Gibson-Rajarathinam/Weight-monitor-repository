#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>

const char* ssid = "Redmi";
const char* password = "12345678";

WiFiClientSecure client;
String receivedData = "";
unsigned long lastSendTime = 0;
const unsigned long SEND_DELAY = 3000;
int connectionFailures = 0;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("\nConnecting WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    // Sync time with NTP server (critical for SSL)
    Serial.println("Syncing time with NTP...");
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    
    time_t now = time(nullptr);
    int retries = 0;
    while (now < 24 * 3600 && retries < 20) {
      delay(500);
      Serial.print(".");
      now = time(nullptr);
      retries++;
    }
    Serial.println();
    Serial.print("Time: ");
    Serial.println(ctime(&now));
  } else {
    Serial.println("\nWiFi Connection Failed");
  }
}

void loop()
{
  // Reconnect WiFi if needed
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting WiFi...");
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }

  if (Serial.available())
  {
    receivedData = Serial.readStringUntil('\n');
    receivedData.trim();
    
    if (receivedData.length() > 0) {
      Serial.print("Received: ");
      Serial.println(receivedData);
      
      // Rate limit the sends
      if (millis() - lastSendTime > SEND_DELAY) {
        processData(receivedData);
        lastSendTime = millis();
      } else {
        Serial.println("Rate limited - please wait before sending again");
      }
    }
  }
}

// =============================================
// PROCESS UART DATA
// FORMAT: Gibson,72.5
// =============================================
void processData(String data)
{
  int commaIndex = data.indexOf(',');

  if (commaIndex == -1)
  {
    Serial.println("Invalid Data - No comma found");
    return;
  }

  String name = data.substring(0, commaIndex);
  name.trim();
  
  String weightString = data.substring(commaIndex + 1);
  weightString.trim();

  float weight = weightString.toFloat();

  if (weight == 0.0 && weightString != "0") {
    Serial.println("Invalid weight value");
    return;
  }

  Serial.print("Name: ");
  Serial.println(name);
  Serial.print("Weight: ");
  Serial.println(weight);

  sendData(name, weight);
}

// =============================================
// SEND DATA TO FIREBASE
// =============================================
void sendData(String name, float weight)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("✗ WiFi Disconnected - cannot send data");
    connectionFailures++;
    return;
  }

  Serial.print("Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // Reset client connection
  if (client.connected()) {
    client.stop();
    delay(500);
  }

  client.setInsecure();

  HTTPClient http;

  String url = "https://weight-monitoring-database-default-rtdb.europe-west1.firebasedatabase.app/weights.json";

  Serial.println("Attempting Firebase connection...");

  if (!http.begin(client, url))
  {
    Serial.println("✗ HTTP Begin Failed");
    connectionFailures++;
    return;
  }

  http.setTimeout(15000); // 15 seconds timeout

  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["name"] = name;
  doc["value"] = weight;

  String json;
  serializeJson(doc, json);

  Serial.print("JSON Size: ");
  Serial.print(json.length());
  Serial.println(" bytes");
  Serial.println("Sending: " + json);

  int httpCode = http.POST(json);

  Serial.print("HTTP Response Code: ");
  Serial.println(httpCode);

  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println("Response: " + payload);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
      Serial.println("✓ Data sent successfully!");
      connectionFailures = 0;
    } else {
      Serial.print("⚠ Unexpected HTTP code: ");
      Serial.println(httpCode);
      connectionFailures++;
    }
  }
  else
  {
    Serial.print("✗ HTTP Error: ");
    Serial.println(http.errorToString(httpCode));
    connectionFailures++;
    
    if (connectionFailures > 3) {
      Serial.println("Too many failures - restarting WiFi...");
      WiFi.disconnect();
      delay(2000);
      WiFi.reconnect();
      connectionFailures = 0;
    }
  }

  http.end();
  client.stop();
  delay(1000);
}
