#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiAP.h>
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
const uint8_t MAX_CONNECTIONS = 4;        // Maximum number of connected clients

// Global objects
WiFiUDP udp;
char packetBuffer[BUFFER_SIZE];            // Buffer to hold incoming packets

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {
    delay(1); // Wait for serial port to connect with timeout
  }
  
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
  int packetSize = udp.parsePacket();
  
  if (packetSize > 0) {
    // Ensure we don't overflow the buffer
    size_t len = min((size_t)packetSize, BUFFER_SIZE - 1);
    
    // Read the packet into packetBuffer
    len = udp.read(packetBuffer, len);
    if (len > 0) {
      packetBuffer[len] = '\0'; // Null-terminate the string
      
      // Log the received message
      logPacket(udp.remoteIP(), udp.remotePort(), packetBuffer, len);
      
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
  Serial.println();
}

void sendUdpResponse(IPAddress ip, uint16_t port, const char* response) {
  udp.beginPacket(ip, port);
  udp.write((const uint8_t*)response, strlen(response));
  udp.endPacket();
}