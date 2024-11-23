//WebServer Libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>

//Oled librarys
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

//RTD library
#include <Adafruit_MAX31865.h>

// Web server
WebServer server(80);
//Oled display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// RTD setup. Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(13, 14,27, 26);

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  100.0

/*
 * This example serves a "hello world" on a WLAN and a SoftAP at the same time.
 * The SoftAP allow you to configure WLAN parameters at run time. They are not setup in the sketch but saved on EEPROM.
 * 
 * Connect your computer or cell phone to wifi network ESP_ap with password 12345678. A popup may appear and it allow you to go to WLAN config. If it does not then navigate to http://192.168.4.1/wifi and config it there.
 * Then wait for the module to connect to your wifi and take note of the WLAN IP it got. Then you can disconnect from ESP_ap and return to your regular WLAN.
 * 
 * Now the ESP8266/ESP32 is in your network. You can reach it through http://192.168.x.x/ (the IP you took note of) or maybe at http://esp8266.local too.
 * 
 * This is a captive portal because through the softAP it will redirect any http request to http://192.168.4.1/
 */

// temperature and time arrays to store data for graph
#define ARRAY_SIZE 20  // Define the fixed size of the arrays
float temperatures[ARRAY_SIZE]; // Array to store temperatures
String runTimes[ARRAY_SIZE];    // Array to store runtimes in "HH:MM" format

/* Set these to your desired softAP credentials. They are not configurable at runtime */
const char *softAP_ssid = "JustChillinFreezer";
const char *softAP_password = "A6";
/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "freezer";

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[32] = "";
char password[32] = "";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

/* Storage for SSID and password */
Preferences preferences;

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);


/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
long lastConnectTry = 0;

/** Current WLAN status */
int status = WL_IDLE_STATUS;

//current temp (in current unit preset)
float temp=0.0f;
bool isCelcius=false;

//setpoint temperature
float newtemp=0.0f;

//variables to keep track of temperature change
float oldtemp=temp;
float diff=0.1f;

//variable for turning on and off the freezer controller
bool is_active=false;
const int freezer = 25;

//variable to keep track of time to determine when
// to save the temperature data
unsigned long lasttime=0;

void setup() {
  //delay to allow user to start serial monitor
  delay(1000);
  Serial.begin(115200);
  Serial.println("Freezer starting...");

  //Setup Freezer control pin
  pinMode(freezer,OUTPUT);

  //Setup wifi and DNS servers
  preferences.begin("CapPortAdv", false);
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid,softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */  
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  //bind wifi request pages to function handler.
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.on("/set_temp",handleSetTemp);
  server.on("/save_temp",handleSaveTemp);
  server.on("/update_units",handleUnits);
  server.on("/get_graph",handleGraph);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/get_temperature_data",handleTemperatureData);
  server.onNotFound ( handleNotFound );
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  //if we have saved wifi LAN credentials, load and display
  loadCredentials(); // Load WLAN credentials from network
  //if there are credentials, try to connect
  connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID

  //initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  //Initialize RTD temperature sensor
  thermo.begin(MAX31865_3WIRE);  // set to 2WIRE or 4WIRE as necessary
  //Update the screen
  updateOLED();
}

//try to connect to wifi network
void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin ( ssid, password );
  int connRes = WiFi.waitForConnectResult();
  Serial.print ( "connRes: " );
  Serial.println ( connRes );
}

void loop() {
  //if flag is set, try to connect to wlan
  if (connect) {
    Serial.println ( "Connect requested" );
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  else{
    int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000) ) {
      /* If WLAN disconnected and idle try to connect */
      /* Don't set retry time too low as retry interfere the softAP operation */
      connect = true;
    }
    if (status != s) { // WLAN status change
      Serial.print ( "Status: " );
      Serial.println ( s );
      status = s;
      if (s == WL_CONNECTED) {
        /* Just connected to WLAN */
        Serial.println ( "" );
        Serial.print ( "Connected to " );
        Serial.println ( ssid );
        Serial.print ( "IP address: " );
        Serial.println ( WiFi.localIP() );

        // Setup MDNS responder
        if (!MDNS.begin(myHostname)) {
          Serial.println("Error setting up MDNS responder!");
        } else {
          Serial.println("mDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
        }
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
  }
  
  //DNS processing
  dnsServer.processNextRequest();
  //HTTP processing
  server.handleClient();

  //get temperature update from RTD module
  updateRDT();
  //only update the OLED if the temperature has substantially changed
  if(temp>=oldtemp+diff||temp<=oldtemp-diff){
    updateOLED();
    oldtemp=temp;
  }
  //if we are above the setpoint, turn the freezer on.
  is_active=temp>newtemp;
  if (is_active){
    digitalWrite(freezer, HIGH);
  }
  else{
    digitalWrite(freezer, LOW);
  }
  //every minute or so update the graph array with current temp and runtime
  if(lasttime<millis()-60000){
    lasttime=millis();
    appendTemperatureAndRuntime();
  }
}

/*update OLED display with current info*/
void updateOLED(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  //display welcome message
  display.print("Just Chillin' Freezer");
  display.setCursor(0,10);
  //display current setpoint
  display.print("Setpoint:");
  display.print(newtemp);
  display.print(isCelcius?" *C":" *F");
  display.setCursor(0,20);
  //display current temp
  display.print("Curent Temp:");
  display.print(temp);
  display.print(isCelcius?" *C":" *F");
  display.display();
}

/*update RTD sensor and get current temp*/
void updateRDT(){
  //get temperature (and convert units to farenheit if nessesary)
  temp=isCelcius?thermo.temperature(RNOMINAL, RREF):thermo.temperature(RNOMINAL, RREF)*9/5+32;
  // Check and print any faults
  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold"); 
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold"); 
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias"); 
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage"); 
    }
    thermo.clearFault();
  }
}

/*function to push new temperatures and runtimes to arrays and remove old values*/
void shiftAndAdd(float newTemperature, String newRuntime) {
    // Shift all elements in the arrays to the left by one
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        temperatures[i] = temperatures[i + 1];
        runTimes[i] = runTimes[i + 1];
    }

    // Add the new values at the end of the arrays
    temperatures[ARRAY_SIZE - 1] = newTemperature;
    runTimes[ARRAY_SIZE - 1] = newRuntime;
}

/*function to handle updating temperature and time arrays for graph*/
void appendTemperatureAndRuntime() {
    // Calculate hours and minutes from the runtime in milliseconds
    unsigned long totalMinutes = millis() / 60000; // Convert milliseconds to minutes
    unsigned int hours = totalMinutes / 60;            // Calculate hours
    unsigned int minutes = totalMinutes % 60;          // Calculate remaining minutes

    // Format runtime as "HH:MM"
    String formattedTime = String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes);

    // Add the current temperature and runtime, maintaining array size
    shiftAndAdd(temp, formattedTime);
    // Debug output
    Serial.print("Data Added - Temperature: ");
    Serial.print(temp);
    Serial.print(", Runtime: ");
    Serial.println(formattedTime);
}