/*
 * --> Sketch_Paolo.ino
 * Thanks Dualvim for your availability. 
 * I am in Italy my telephone company is 'coopvoce'. 
 * I am using a LILYGO A7670G card that seems perfectly compatible. I attach some excerpts of the source.
* Thanks again for your help. 
*/

/*
** --> CoopVoce Parameters: https://apn.how/it/web-coopvoce/asus-zenfone-4 
**  - apn[]: "internet.coopvoce.it"
**  - gprsUser[]: ""   // Should be none
**  - gprsPass[]: ""   // Should be none
** 
** --> GSM_PIN: 
**  - Look for the default pin in the verse of the card which came with your GSM chip.
**  - I tryed "0000" or "". It didn't work with my chip
**
** --> Selected board: 'Esp32 Wrover Module'
** 
** --> Github TSIM 7600X: https://github.com/Xinyuan-LilyGO/T-SIM7600X
** --> https://github.com/Xinyuan-LilyGO/T-SIM7600X/blob/master/examples/ATdebug/ATdebug.ino
*/
	

// Select your modem
// #define TINY_GSM_MODEM_SIM7000SSL  // --> Probably the problem is here
#define TINY_GSM_MODEM_SIM7600   // --> Use this and check the pinout below
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb

//#define TINY_GSM_MODEM_SIM7600 // suggerimento Dualvim

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
#define SerialAT Serial1

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <UrlEncode.h>

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
// #define LOGGING // <- Logging is for the HTTP library

// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define TINY_GSM_YIELD() { delay(2); }

// set GSM PIN, if any
// #define GSM_PIN "0000" // --> You put this
#define GSM_PIN "8486" // Vivo's (BR) default PIN

// flag to force SSL client authentication, if needed
// #define TINY_GSM_SSL_CLIENT_AUTHENTICATION

// Set your APN Details / GPRS credentials
//const char apn[] = "internet.coopvoce.it";
//const char gprsUser[] = "";
//const char gprsPass[] = "";
// Vivo - BR
const char apn[] = "zap.vivo.com.br";
const char gprsUser[] = "vivo";
const char gprsPass[] = "vivo";

// Server details
const char server[] = "api.callmebot.com";
//const int port = 443;
const int port = 80;

// The phone number should be in international format (including the + sign)
//String phone_number = "+34684723962";
//String phone_number = "+393286673687"; // --> Your cell phene
String phone_number = "+5527997824908";

//String api_key = "1536749"; // --> Don't show this 
String api_key = "3544729";

TinyGsm modem(SerialAT);

TinyGsmClient client(modem);  // --> Also, I changed here
HttpClient http(client, server, port);

// LilyGo T-SIM7670 Pinout: https://randomnerdtutorials.com/lilygo-ttgo-t-a7670g-a7670e-a7670sa-esp32/
#define UART_BAUD         115200
#define PIN_DTR           25
#define PIN_TX            26
#define PIN_RX            27
// The modem boot pin needs to follow the startup sequence.
#define PWR_PIN           4
#define BAT_ADC           35
// The modem power switch must be set to HIGH for the modem to supply power.
#define LED_PIN            12
#define SD_MISO           2
#define SD_MOSI           15
#define SD_SCLK           14
#define SD_CS             13


void modemPowerOn(){
	pinMode(PWR_PIN, OUTPUT);
	digitalWrite(PWR_PIN, LOW);
	delay(1000);
	digitalWrite(PWR_PIN, HIGH);
}

void modemPowerOff(){
	pinMode(PWR_PIN, OUTPUT);
	digitalWrite(PWR_PIN, LOW);
	delay(1500);
	digitalWrite(PWR_PIN, HIGH);
}

void modemRestart(){
	modemPowerOff();
	delay(1000);
	modemPowerOn();
}

void setup() {
	// Set Serial Monitor baud rate
	SerialMon.begin(115200);
	delay(10);

	// Set LED OFF
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);

	modemPowerOn();

	SerialMon.println("Wait…");

	// Set GSM module baud rate and Pins
	SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
	delay(6000);

	// Restart takes quite some time
	// To skip it, call init() instead of restart()
	SerialMon.println("Initializing modem…");
	modem.restart();
	// modem.init();

	String modemInfo = modem.getModemInfo();
	SerialMon.print("Modem Info: ");
	SerialMon.println(modemInfo);

	// Unlock your SIM card with a PIN if needed
	if (GSM_PIN && modem.getSimStatus() != 3) {
		modem.simUnlock(GSM_PIN);
	}

	Serial.println("Make sure your LTE antenna has been connected to the SIM interface on the board.");
	delay(10000);
}

void loop() {
	modem.gprsConnect(apn, gprsUser, gprsPass);

	SerialMon.print("Waiting for network…");
	if (!modem.waitForNetwork()) {
		SerialMon.println(" fail");
		delay(10000);
		return;
	}
	SerialMon.println(" success");

	if (modem.isNetworkConnected()) {
		SerialMon.println("Network connected");
	}

	String message = "Hello from ESP32!";
	String url_callmebot = "/whatsapp.php?phone=" + phone_number + "&apikey=" + api_key + "&text=" + urlEncode(message);

	SerialMon.print(F("Performing HTTPS GET request… "));
	http.connectionKeepAlive(); // Currently, this is needed for HTTPS

	int err = http.get(url_callmebot);
	if (err != 0) {
		SerialMon.println(F("failed to connect"));
		delay(10000);
		return;
	}

	int status = http.responseStatusCode();
	SerialMon.print(F("Response status code: "));
	SerialMon.println(status);
	if (!status) {
		delay(10000);
		return;
	}

	SerialMon.println(F("Response Headers:"));
	while (http.headerAvailable()) {
		String headerName = http.readHeaderName();
		String headerValue = http.readHeaderValue();
		SerialMon.println(" " + headerName + " : " + headerValue);
	}

	int length = http.contentLength();
	if (length >= 0) {
		SerialMon.print(F("Content length is: "));
		SerialMon.println(length);
	}
	if (http.isResponseChunked()) {
		SerialMon.println(F("The response is chunked"));
	}

	String body = http.responseBody();
	SerialMon.println(F("Response:"));
	SerialMon.println(body);

	SerialMon.print(F("Body length is: "));
	SerialMon.println(body.length());

	// Shutdown
	http.stop();
	SerialMon.println(F("Server disconnected"));

	modem.gprsDisconnect();
	SerialMon.println(F("GPRS disconnected"));

	// Do nothing forevermore
	while (true) {
		delay(1000);
	}
}
