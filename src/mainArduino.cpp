#ifdef ARDUINO

#include <memory>
#include <string>
#include <SD.h>
#include <WiFiNINA.h>
#include "mainArduino.h"
#include "HttpResponseFileArduino.h"
#include "HttpServerControl.h"
#include "TimerControl.h"

const int PIN_LED_GREEN = 4;
const int PIN_LED_RED = 6;
const int PIN_RELAY_OPEN = 3;
const int PIN_RELAY_CLOSE = 5;

const int PIN_TEMP = A0;
const int PIN_HUMI = A1;
const int SDCardPinCS = 10;

bool ArduinoApp::Setup()
{
    Serial.begin(115200);

	// Initializing output pins
	pinMode(PIN_LED_GREEN, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_RELAY_OPEN, OUTPUT);
	pinMode(PIN_RELAY_CLOSE, OUTPUT);

	// Initializing SD Card
	Serial.print("Initializing SD card...");
	if (!SD.begin(SDCardPinCS)) {
		Serial.println(" Error!\r\nCard failed or not present");
		return false;
	}
	Serial.println(" Ok!");
  
	// Reading SSID and password from "wifi.txt" file in SD
    std::vector<char> fileContent = HttpResponseFile::ReadFileContent("/wifi.txt");
	if (fileContent.size() == 0) {
		Serial.println("wifi.txt not found or no data inside");
		return false;
	}
    std::string wifiSSID;
    std::string wifiPassword;
	bool readingSSID = true;
    for (auto & it : fileContent)
    {
        if (ch == '\n' && readingSSID) readingSSID = false;
		else if (ch == '\n' && !readingSSID) break;
		else if (ch != '\r' && readingSSID) wifiSSID += ch;
		else if (ch != '\r' && !readingSSID) wifiPassword += ch;
    }

    // Connecting to the WIFI
    int status = WL_IDLE_STATUS;
	int counter = 0;
	while (status != WL_CONNECTED) {
		Serial.print("Attempting to connect to network: ");
		Serial.println(WiFiSSID);
		// Connect to WPA/WPA2 network:
		status = WiFi.begin(WiFiSSID, WiFiPassword);
		if (++counter >= 5) break;
		// wait 5 seconds for new attempt:
		if (status != WL_CONNECTED) delay(5000);
	}
	if (status != WL_CONNECTED) return false;
    Serial.println("Connected to WIFI.");
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // Initializing HTTP Server
    httpServer = std::unique_ptr<HttpServerControl>(new HttpServerControl());
    httpServer->Initialize();
}

CUSTOM_TIME lastSecondCount;
int secondsCounter = 0;
void ArduinoApp::Loop()
{
    CUSTOM_TIME timeNow = GetCustomCurrentTime();
    double timeEllapsed = TIME_DIFF_TO_MSECS_DOUBLE(timeNow - lastMessageSent);
    // Counting seconds and blinking leds every second
    if (timeEllapsed >= 1)
    {
        secondsCounter++;
        digitalWrite(PIN_LED_RED, !digitalRead(PIN_LED_RED));
        digitalWrite(PIN_LED_GREEN, !digitalRead(PIN_LED_GREEN));
    }

    // Ticking server
    httpServer->Tick();

    // Adding a message to be sent to all WebSockets every 5 seconds
    if (secondsCounter >= 5)
    {
        secondsCounter = 0;
        httpServer->SendTextToWSClients(std::string("{\"Text\":\"hi again!\"}"));
    }
}

#endif
