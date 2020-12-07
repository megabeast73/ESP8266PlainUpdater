#include "Arduino.h"
#include "PlainUpdater.h"
#include <ESP8266WiFi.h>

//Where to listen what to expect
#define UDP_LISTENER_PORT 18777
#define TCP_LISTENER_PORT 18778

//MUST HAVE ALL 4 bytes
#define UDP_HELLO 0xBAADF00D


void setupRS()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);
}

//You know these
#define STASSID "MikroTik-C40E2D"
#define STAPSK  ""

void setupWiFi()
{

	Serial.print("Setting up WiFi");
	WiFi.mode(WIFI_STA);
	WiFi.begin(STASSID, STAPSK);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(300);
		Serial.print(".");
	}
	Serial.println(WiFi.localIP());
	WiFi.setAutoConnect(true);
}

//Define the variable
PlainUpdater* updater = nullptr;

//The setup function is called once at startup of the sketch
void setup()
{
	setupRS();
	setupWiFi();
	//Set the IP address andport where to check for pull update (emergency).
	//NOTE: Octets of the IP Address are digits with  with commas
	//You have to execute this method, after the WiFi server initialization
	PlainUpdater::emergency(IPAddress(192,168,90,26), 1879);
	updater = new PlainUpdater(UDP_LISTENER_PORT,TCP_LISTENER_PORT,UDP_HELLO);
//  ...
}

// The loop function is called in an endless loop
void loop()
{
	updater->process();
	delay(100);
//Add your repeated code here
}
