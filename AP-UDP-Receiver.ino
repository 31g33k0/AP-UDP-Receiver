#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiAP.h>
#include <thread> // For thread handling, maybe useless
#include <string> // For string handling, maybe useless
#include <iostream> // For input/output operations, maybe useless
#include <Arduino.h>

// Configuration
const char* SSID = "AP-UDP-Receiver";    // Access Point SSID
const char* PASSWORD = "12345678";       // Password (min 8 chars)
const uint16_t LOCAL_UDP_PORT = 12345;    // Local port to listen on
const size_t BUFFER_SIZE = 512;           // Increased buffer size
const IPAddress LOCAL_IP(192, 168, 4, 1);
const IPAddress GATEWAY(192, 168, 4, 1);
const IPAddress SUBNET(255, 255, 255, 0);
const unsigned long WIFI_RETRY_DELAY = 5000; // ms between connection retries
const uint8_t MAX_CONNECTIONS = 1;        // Maximum number of connected clients
const int ctrlPinX = 25;  // X-axis pin (GPIO25)
const int ctrlPinY = 26;  // Y-axis pin (GPIO26)
const int inPinX0 = 33; // Input X0-axis pin (GPIO33)
const int inPinY0 = 32; // Input Y0-axis pin (GPIO32)
const int inPinX1 = 35; // Input X1-axis pin (GPIO35)
const int inPinY1 = 34; // Input Y1-axis pin (GPIO34)
const int MidValue = 2047;

// DAC configuration
/*
const int dacResolution = 12; // DAC resolution (12 bits)
const int dacMaxValue = 4095; // DAC maximum value (12 bits)
const int dacMinValue = 0; // DAC minimum value (12 bits)
const int dacMidValue = 2047; // DAC mid value (12 bits)
*/

// Global objects
WiFiUDP udp;
char packetBuffer[BUFFER_SIZE];            // Buffer to hold incoming packets

// Global variables
bool isClientConnected = false;

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {
    delay(1); // Wait for serial port to connect with timeout
  }

  setupPins();
  
  Serial.println("\n=== UDP Access Point Server ===");
  
  // Set up WiFi Access Point
  if (!startAP()) {
    Serial.println("Failed to start Access Point. Restarting...");
    delay(1000);
    ESP.restart();
  }
  
  // Start UDP server
  if (!startUdpServer()) {
    Serial.println("Failed to start UDP server. Restarting...");
    delay(1000);
    ESP.restart();
  }
  
  Serial.println("System ready");
  printNetworkInfo();
}

void loop() {
  handleUdpTraffic();
  yield(); // Allow ESP8266/ESP32 background tasks to run
}

void handleUdpTraffic() {
  checkClientConnection();

  if (!isClientConnected) {
    resetPins();
    return;
  }
  
  int packetSize = udp.parsePacket();
  
  if (packetSize > 0) {
    // Ensure we don't overflow the buffer
    size_t len = min((size_t)packetSize, BUFFER_SIZE - 1);
    
    // Read the packet into packetBuffer
    len = udp.read(packetBuffer, len);
    if (len > 0) {
      packetBuffer[len] = '\0'; // Null-terminate the string
      
      // Set the pin values based on the received data
      int xValue = atoi(packetBuffer) - MidValue; // TODO could be replaced by std::stoi 
      char* spacePos = strchr(packetBuffer, ' ');  // Find the space between X and Y values
      int yValue = spacePos ? atoi(spacePos + 1) - MidValue : 0;
      // for debugging
      Serial.print("X : ");
      Serial.println(xValue);
      Serial.print("Y : ");
      Serial.println(yValue);
      // end debugging
      setPinValues(xValue, yValue);
      // Log the received message
      logPacket(udp.remoteIP(), udp.remotePort(), packetBuffer, len);
      
    if (len <= 0) {
      resetPins();
      return;
    }

    // Optional: Send a reply to the client
    // sendUdpResponse(udp.remoteIP(), udp.remotePort(), "ACK");
    }
  }
}

bool startAP() {
  // Configure WiFi mode and AP settings
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(LOCAL_IP, GATEWAY, SUBNET);
  
  // Start the access point
  bool success = WiFi.softAP(SSID, PASSWORD, 1, 0, MAX_CONNECTIONS);
  
  if (success) {
    Serial.println("\nAccess Point started successfully");
    return true;
  } else {
    Serial.println("\nFailed to start Access Point");
    return false;
  }
}

bool startUdpServer() {
  if (udp.begin(LOCAL_UDP_PORT)) {
    Serial.printf("UDP server started on port %d\n", LOCAL_UDP_PORT);
    return true;
  }
  Serial.println("UDP server failed to start");
  return false;
}

void printNetworkInfo() {
  Serial.println("\n=== Network Information ===");
  Serial.print("SSID: ");
  Serial.println(SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("===========================\n");
}

void checkClientConnection() {
  if (WiFi.softAPgetStationNum() == 0) {
    isClientConnected = false;
  } else {
    isClientConnected = true;
  }
}

void setupPins() {
  pinMode(ctrlPinX, OUTPUT);
  pinMode(ctrlPinY, OUTPUT);
  pinMode(inPinX0, OUTPUT);
  pinMode(inPinY0, OUTPUT);
  pinMode(inPinX1, OUTPUT);
  pinMode(inPinY1, OUTPUT);
}

void resetPins() {
  dacWrite(ctrlPinX, 0); // or ledcWrite(ctrlPinX, 0) or digitalWrite(ctrlPinX, LOW);
  dacWrite(ctrlPinY, 0); // or ledcWrite(ctrlPinY, 0) or digitalWrite(ctrlPinY, LOW);
  digitalWrite(inPinX0, LOW);
  digitalWrite(inPinY0, LOW);
  digitalWrite(inPinX1, LOW);
  digitalWrite(inPinY1, LOW);
}

void setPinValues(int xValue, int yValue) {
  ledcWrite(ctrlPinX, xValue); // TODO test it as a dac signal using dacWrite
  ledcWrite(ctrlPinY, yValue); // TODO test it as a dac signal using dacWrite
  digitalWrite(inPinX0, xValue < 0 ? HIGH : LOW); 
  digitalWrite(inPinY0, yValue < 0 ? HIGH : LOW);
  digitalWrite(inPinX1, xValue > 0 ? HIGH : LOW); 
  digitalWrite(inPinY1, yValue > 0 ? HIGH : LOW);
}

void logPacket(IPAddress remoteIp, uint16_t remotePort, const char* data, size_t len) {
  Serial.printf("\n[%lu] Received %u bytes from %s:%d\n", 
               millis(), len, 
               remoteIp.toString().c_str(), 
               remotePort);
  Serial.print("Data: ");
  // Print only printable characters for safety
  for (size_t i = 0; i < len; i++) {
    if (isPrintable(data[i])) {
      Serial.print(data[i]);
    } else {
      Serial.printf("\\x%02X", (uint8_t)data[i]);
    }

  }
//  Serial.println();
}
// Send a response to the client
// WIP
void sendUdpResponse(IPAddress ip, uint16_t port, const char* response) {
  udp.beginPacket(ip, port);
  udp.write((const uint8_t*)response, strlen(response));
  udp.endPacket();
}