#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

const char* ssid = "2470";
const char* password = "/6800f6V";
const char* url = "https://iot-smarttankmonitor.onrender.com/api/data";

const int pins[] = {25, 26, 27, 14}; 
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

// Animation State Variables
int lastBookingState = 0;
String animText = "OPEN";
String animDesc = "Tanker Available";

void setup() {
  u8g2.begin();
  for (int p : pins) pinMode(p, INPUT_PULLUP);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

// Function to handle the OLED animation sequence
void playBookingAnimation() {
  const char* stages[] = {"INITIATING...", "FINDING TANKER", "BOOKING...", "BOOKED!"};
  const char* descs[] = {"Process Started", "Searching Fleet", "Finalizing", "On the Way"};

  for (int i = 0; i < 4; i++) {
    animText = stages[i];
    animDesc = descs[i];
    
    // Update Display during animation
    u8g2.firstPage();
    do {
      drawUI(0, "WAIT", 1); // Pass current level and dummy status
    } while (u8g2.nextPage());
    
    delay(1500); // Animation speed
  }
}

void drawUI(int level, String srvStatus, int isBooked) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "POSEIDON MONITOR V1.1.0");
  
  // Water Level
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.setCursor(0, 40);
  u8g2.print(level); u8g2.print("%");
  
  // Booking Animation/Status Area
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(65, 32);
  u8g2.print(animText);
  u8g2.setCursor(65, 45);
  u8g2.print(animDesc);
  
  // Server Status
  u8g2.setCursor(0, 60);
  u8g2.print("Srv: "); u8g2.print(srvStatus);
}

void loop() {
  int level = 0;
  if (digitalRead(pins[0]) == LOW) level = 25;
  if (digitalRead(pins[1]) == LOW) level = 50;
  if (digitalRead(pins[2]) == LOW) level = 75;
  if (digitalRead(pins[3]) == LOW) level = 100;

  int currentBooking = (level < 25) ? 1 : 0;
  
  // Trigger animation only on state CHANGE (0 to 1)
  if (currentBooking == 1 && lastBookingState == 0) {
    playBookingAnimation();
  } else if (currentBooking == 0) {
    animText = "OPEN";
    animDesc = "Available";
  }
  
  lastBookingState = currentBooking;
  String srvStatus = "OK";

  // HTTPS Post
  NetworkClientSecure *client = new NetworkClientSecure;
  if (client) {
    client->setInsecure();
    HTTPClient http;
    if (http.begin(*client, url)) {
      http.addHeader("Content-Type", "application/json");
      StaticJsonDocument<128> doc;
      doc["id"] = "ESP32_TANK";
      doc["w_lvl"] = level;
      doc["booking"] = currentBooking;
      String json;
      serializeJson(doc, json);
      int code = http.POST(json);
      srvStatus = (code == 200) ? "Sent" : String(code);
      http.end();
    }
    delete client;
  }

  // Final Screen Update
  u8g2.firstPage();
  do {
    drawUI(level, srvStatus, currentBooking);
  } while (u8g2.nextPage());

  delay(500); 
}