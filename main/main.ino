/*
 SMART CONFIG ON BUTTON PRESS, WORKS FINE
*/

/*Sketch uses 320444 bytes (38%) 
Global variables use 33564 bytes (39%)*/

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include "ESPAsyncWebServer.h"
  #include <ESP8266mDNS.h>        // Include the mDNS library

#elif defined(ESP32)
  #include <WiFi.h>
#endif


//#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <RCSwitch.h>
#include <Ticker.h>
#include <FS.h>
#include <WiFiUdp.h>







#define DEBUG 1

#if DEBUG
#define  P_R_I_N_T(x)   Serial.println(x)
#else
#define  P_R_I_N_T(x)
#endif


Ticker watchDogTicker, smartConfigTicker, mqttTicker, logTicker;
RCSwitch mySwitch = RCSwitch();
//for LED status

//enter your file name
const char* CONFIG_FILE = "/myconfig.json";


// Connect to the WiFi
//const char* ssid = "DataSoft_WiFi";//DataSoft_WiFi
//const char* password = "support123";
//const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_server = "broker.datasoft-bd.com";
const int mqttPort = 1883;
const char* hid = "HUB005";

//mqtt OUT topic
const char* responseTopic = "dsiot/smarthub/test/out/uid";
//mqtt IN topic
const char* configTopic = "dsiot/smarthub/test/in/uid";
const char* armTopic = "dsiot/smarthub/test/in/uid/arm";
const char* logTopic = "dsiot/smarthub/test/out/uid/log";


//unsigned long sensor1, sensor2, sensor3;
//for async server
//const char* PARAM_MESSAGE ="led";
//const char* PAIR_CMD ="PAIR";


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


WiFiClient espClient;
PubSubClient client(espClient);


//For Timer
//unsigned long lastWiFiCheckTime = 0;
unsigned long lastReconnectTime = 0;//MQTT reconnect

//unsigned long wifi_check_interval = 5000;



#define ALARM_PIN 14 //D5 Buzzer, stays HIGH at bootup and flashing for WEMOS
#define dataIn 2 //D4 wemos LED_BUILTIN
#define SOS_Button 5  //D1 reset/ON DEMAND WIFI CONFIG BUTTON
#define SMART_CONFIG_LED 13  //13(D7) SIGNAL ON CASING
#define MQTT_LED 15  //D8 STATUS ON CASING
#define ARM_LED 12  //D6 


#define ALARM_ON() digitalWrite(ALARM_PIN, HIGH)
#define ALARM_OFF() digitalWrite(ALARM_PIN, LOW)


//for RF msg
uint8_t pairMsgCounter=0, alarmMsgCounter=0;




// Holds the current button state.
//volatile int state = 1;
//volatile bool isPRESSED = false;

// Holds the last time debounce was evaluated (in millis).
//volatile long lastDebounceTime = 0;

// The delay threshold for debounce checking.
//const int debounceDelay = 30;// 30ms works good for push button


//for long press detection
//unsigned long lastPressedTime = 0;

//for alarm pause timeout detection
//unsigned long lastStopTime;
//volatile bool SNOOZE = false;
//unsigned long snoozeTime = 300000; //in ms.  5 minutes snooze time default

//For preventing realarming
bool isArmed = true;
//bool isAlarming = false;
bool isPairingRequested = false;


//bool autoConfig();
//void smartConfig();
void printWifiStatus();



//--------------------Watchdog----------------------//
volatile unsigned int watchdogCount = 0;

void ISRwatchdog() {
  watchdogCount++;
//  Serial.println(watchdogCount);    
  if (watchdogCount >= 150) {
    Serial.println("Watchdog bites!!!");    
    ESP.reset();    
  }  
}


/*

// Gets called by the interrupt.
void   ICACHE_RAM_ATTR   ISR() {
  // Get the pin reading.
  int reading = digitalRead(sosButton);

  // Ignore dupe readings.
  if(reading == state) return;

  boolean debounce = false;
  
  // Check to see if the change is within a debounce delay threshold.
  if((millis() - lastDebounceTime) <= debounceDelay) {
    debounce = true;
  }

  // This update to the last debounce check is necessary regardless of debounce state.
  lastDebounceTime = millis();

  // Ignore reads within a debounce delay threshold.
  if(debounce) return;

  // All is good, persist the reading as the state.
  state = reading;
//  isPRESSED = state;// if TRUE(HIGH),  then start counting after resetting counter
//  counter = 0;

  if(!state){//if button pin is read low
//    if(isAlarming){P_R_I_N_T("Already Alarming!");}else{setAlarm_and_Publish("SOS",1); isAlarming = true;}         
    lastPressedTime = millis();//will be reused for long press hard reset
  }
  
// Work with the value now.
  Serial.println("button: " + String(reading));
  
}//ISR ENDS

*/

//smart config setup
bool in_smartconfig = false;



void setup() {
    
    Serial.begin(115200);
    Serial.setDebugOutput(true);

//    while(WiFi.SSID()==""){
//      initSmartConfig_2();
//    }
    
    


    
    pinMode(ALARM_PIN, OUTPUT);
    pinMode(SMART_CONFIG_LED, OUTPUT);
    //BUZZER indicator not implemented
    pinMode(MQTT_LED, OUTPUT);
    pinMode(ARM_LED,OUTPUT);
    pinMode(SOS_Button, INPUT);
//    pinMode(SOS_Button, INPUT_PULLUP);

//    pinMode(alarmPin, LOW);//(gpiO-0) TO avoid making sound at startup but it always stays  HIGH((gpio goes HIGH at bootup by design), so can't use it here in this mode, better keep its state undefined
  
//    attachInterrupt(sosButton, ISR,  CHANGE);


    delay(500);
    
    mySwitch.enableReceive(dataIn);  // Receiver on interrupt 0 => that is pin #2 
    
    P_R_I_N_T("DEVICE READY");
    
//    setup_wifi();
//    checkWiFi();
//    callWiFiManager();

//    if (!wifiConnect())//changes from if to while to try to solve problem
//    {
//        enter_smartconfig();
//    }


    
     if(wifiConnect()){
        startmDNS();//1st
        api();//2nd
        startServer();//3rd
        setup_mqtt();//4th 
    };
    
    
    


    

    watchDogTicker.attach(1, ISRwatchdog);// ISRwatchdog ticking every 1 second
    logTicker.attach(300, logPublish);
    
    if(mountSPIFFS()){      loadConfigFile();    }else{Serial.println("An Error has occurred while mounting SPIFFS");}
    
}//void SETUP ends




void loop() {
    
    watchdogCount = 0;
   


    //smartConfig
    if (longPress()) {
      Serial.println("Enter smartconfig");
      enter_smartconfig();      
    }
    else if (WiFi.status() == WL_CONNECTED && in_smartconfig && WiFi.smartConfigDone()) {
      exit_smartConfig();
      Serial.println("Connected, Exit smartconfig");
    }
  
    if (WiFi.status() == WL_CONNECTED) {   
      //only run if connected to router    
      MDNS.update();   
      if(isPairingRequested){startPairingMode();}
      else if(isArmed){readRadioData();}
      setMqttReconnectInterval();
    }

    
    
}//LOOP ENDS


bool longPress()
{
  static unsigned int lastPress = 0;
  if (millis() - lastPress > 3000 && digitalRead(SOS_Button) == 0) {
    return true;
  } else if (digitalRead(SOS_Button) == 1) {
    lastPress = millis();
  }
  return false;
}


void startServer(){
  server.begin();
  Serial.println("TCP server started");
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void api(){  
    server.on("/getid", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", hid);
    });
}




void stopPairingMode(){
//  Serial.println("Stopped");
  smartConfigTicker.detach();
  SMART_CONFIG_LED_OFF();
  isPairingRequested = false;
}

/*
void setAlarm_and_Publish(const char *type, const int state){
  
  digitalWrite(alarmPin, state);  
//  digitalWrite(ledPin, state);

  if(state){   sirenTicker.attach(0.25, rapidBlinkAlarmLed);  } else { sirenTicker.detach();    turnOffAlarmLed(); }
  
  
  
  StaticJsonDocument<128> doc;
  char buffer[128];
  doc["did"] = hid;
  doc["alarm"] = state;
  doc["src"] = type;
  size_t n = serializeJson(doc, buffer);
  client.publish(responseTopic, buffer, n);  
}
*/










void startmDNS(){
  if (!MDNS.begin("esp8266")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
//  MDNS.addService("http", "tcp", 80);
}




//****************************NOT REQUIRED FOR WIFI MANAGER***************************
/*
void setup_wifi() {
    // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    P_R_I_N_T(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
  randomSeed(micros());
  P_R_I_N_T("");
  P_R_I_N_T("WiFi connected");
  P_R_I_N_T("IP address: ");
  P_R_I_N_T(WiFi.localIP());
}*/




void startPairingMode(){

//  Serial.println("Pairing on");
  
  
  if (mySwitch.available()) {
    
    Serial.print("Received ");

    unsigned long data = mySwitch.getReceivedValue();
    
    Serial.print( data );
    Serial.print(" / ");
    Serial.print( mySwitch.getReceivedBitlength() );
    Serial.print("bit ");
    Serial.print("Protocol: ");
    P_R_I_N_T( mySwitch.getReceivedProtocol() );


    pairMsgCounter++;
//    P_R_I_N_T(msgCounter);
    if(pairMsgCounter > 3){
//        savePairedDevice(data);

          StaticJsonDocument<64> doc;
          doc["hid"] = hid;
          doc["paired"] = data;
      
          char buffer[64];
          size_t n = serializeJson(doc, buffer);//stringify
          if(client.publish(responseTopic,buffer, false)){
              delay(1000);//to solve the immediate sensor trigger issue after being paired
              stopPairingMode();    
              Serial.println("Paired");        
          }else{
            //I need to turn off pairing mode on my own automatically after 10s
          }

    
//        Serial.println("SAME");
        pairMsgCounter = 0;
    }    
    
    mySwitch.resetAvailable();
  }//END of rf data buffer check
  
}//END PAIR


void readRadioData(){
    if (mySwitch.available()) {
    
    Serial.print("Received ");

    unsigned long data = mySwitch.getReceivedValue();
    
    Serial.print( data );
    Serial.print(" / ");
    Serial.print( mySwitch.getReceivedBitlength() );
    Serial.print("bit ");
    Serial.print("Protocol: ");
    P_R_I_N_T( mySwitch.getReceivedProtocol() );


    alarmMsgCounter++;
    P_R_I_N_T(alarmMsgCounter);
    if(alarmMsgCounter > 4){  
          

        StaticJsonDocument<64> doc;
        doc["hid"] = hid;
        doc["sid"] = data;
    
        char buffer[64];
        size_t n = serializeJson(doc, buffer);//stringify
        client.publish(responseTopic,buffer, false);
        delay(250);        
        P_R_I_N_T("Alarming!");              

        alarmMsgCounter = 0;
    }
    mySwitch.resetAvailable();    
  }
}




void logPublish(){
//        P_R_I_N_T("Log Published");
        StaticJsonDocument<128> doc;
        doc["hid"] = hid;
        delay(500);
        doc["ip"] = WiFi.localIP().toString();
        delay(1000);
        doc["rssi"] = WiFi.RSSI();        
        doc["ssid"] = WiFi.SSID();
        
        
        char buffer[128];
        size_t n = serializeJson(doc, buffer);//stringify
        Serial.println(buffer);
        if(client.publish(logTopic,buffer, false)){delay(250);P_R_I_N_T("Log Published"); }
        
}


/*
void setLongPressTimer(){

  unsigned long currentTime = millis();
  
  if(currentTime - lastPressedTime > 4000){
      
//      setAlarm_and_Publish("SOS",0);
      Serial.println("Hard Resetting...");
//      lastStopTime = millis();
      state = 1;//button state made UNPRESSED to make sure while loop does not go on if MCU missed the state change

  }
        
}*/




/*
void checkResumeTimer(){
  unsigned long now = millis();
  if(now - lastStopTime > snoozeTime){
      SNOOZE = false;//resume radio by cancelling snooze
  }
}*/




