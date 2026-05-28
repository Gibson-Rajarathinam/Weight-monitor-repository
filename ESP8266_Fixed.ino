#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "Redmi";
const char* password = "12345678";

String receivedData = "";
unsigned long lastSendTime = 0;
const unsigned long SEND_DELAY = 3000; // 3 seconds between sends (increased)
int connectionFailures = 0;

void setup()
{
  Serial.begin(115200); // Increased baud rate for ESP8266

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
    Serial.println("WiFi Disconnected - cannot send data");
    connectionFailures++;
    return;
  }

  // Create a new client for each request
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  
  // For Firebase, we skip certificate validation
  client->setInsecure();

  HTTPClient http;

  String url = "https://weight-monitoring-database-default-rtdb.europe-west1.firebasedatabase.app/weights.json";

  Serial.println("Connecting to Firebase...");

  // Begin connection with NO timeout (uses default longer timeout)
  if (!http.begin(*client, url))
  {
    Serial.println("HTTP Begin Failed");
    connectionFailures++;
    return;
  }

  // Set longer timeout (in milliseconds) - 10 seconds for HTTPS handshake
  http.setTimeout(10000);

  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["name"] = name;
  doc["value"] = weight;

  String json;
  serializeJson(doc, json);

  Serial.println("Sending JSON:");
  Serial.println(json);

  int httpCode = http.POST(json);

  Serial.print("HTTP Response Code: ");
  Serial.println(httpCode);

  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println("Response:");
    Serial.println(payload);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
      Serial.println("✓ Data sent successfully!");
      connectionFailures = 0; // Reset on success
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
    
    // If too many failures, restart WiFi
    if (connectionFailures > 3) {
      Serial.println("Too many failures - restarting WiFi...");
      WiFi.disconnect();
      delay(1000);
      WiFi.begin(ssid, password);
      connectionFailures = 0;
    }
  }

  http.end();
  client.reset(); // Clean up

  delay(1000); // Increased delay after request
}
