#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Paderesqt";
const char* password = "qt123456";

const int ledPins[] = {2, 4, 5, 18, 19};
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]);

WebServer server(80);
int mode = 0; 
bool isOn = true;

void handleRoot() {                   
    String html = "<!DOCTYPE html><html><head><title>ESP32 LED Control</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; text-align: center; }";
    html += "button { background-color: #008CBA; color: white; padding: 15px 30px; ";
    html += "border: none; border-radius: 10px; font-size: 20px; cursor: pointer; margin: 10px; }";
    html += "button:hover { background-color: #005f73; }";
    html += "input[type='checkbox'] { width: 50px; height: 25px; }";
    html += "</style></head><body>";
    html += "<h1>ESP32 LED Control</h1>";

    
    html += "<button onclick=\"fetch('/toggle?mode=1')\">On</button>";
    html += "<button onclick=\"fetch('/toggle?mode=2')\">Alternating</button>";
    html += "<button onclick=\"fetch('/toggle?mode=3')\">Center-to-Side</button>";

    html += "<br><br>";

    
    html += "<label>Power: <input type='checkbox' onchange=\"fetch('/power?state=' + (this.checked ? '1' : '0'))\" " + String(isOn ? "checked" : "") + "></label>";

    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleToggle() {
    if (server.hasArg("mode")) {
        mode = server.arg("mode").toInt();
    }
    server.send(200, "text/plain", "Mode changed");
}

void handlePower() {
    isOn = server.arg("state") == "1";
    server.send(200, "text/plain", "Power toggled");
}

void updateLeds() {
    if (!isOn) {
        for (int i = 0; i < numLeds; i++) digitalWrite(ledPins[i], LOW);
        return;
    }

    static unsigned long lastUpdate = 0;
    static int step = 0;
    const int pattern[] = {0, 2, 4, 1, 3};

    if (millis() - lastUpdate < 200) return;
    lastUpdate = millis();

    switch (mode) {
        case 1: 
            for (int i = 0; i < numLeds; i++) digitalWrite(ledPins[i], HIGH);
            break;
        case 2: 
            for (int i = 0; i < numLeds; i++) digitalWrite(ledPins[i], LOW);
            digitalWrite(ledPins[pattern[step % (sizeof(pattern) / sizeof(pattern[0]))]], HIGH);
            step++;
            break;
        case 3: 
            for (int i = 0; i < numLeds; i++) digitalWrite(ledPins[i], LOW);
            int center = numLeds / 2;
            int offset = step;
            if (center - offset >= 0) digitalWrite(ledPins[center - offset], HIGH);
            if (center + offset < numLeds) digitalWrite(ledPins[center + offset], HIGH);
            step++;
            if (step > center) step = 0;
            break;
    }
}

void setup() {
    Serial.begin(115200);
    for (int i = 0; i < numLeds; i++) {
        pinMode(ledPins[i], OUTPUT);
        digitalWrite(ledPins[i], LOW);
    }
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());
    server.on("/", handleRoot);
    server.on("/toggle", handleToggle);
    server.on("/power", handlePower);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    updateLeds();
}
