/* ------Auto Connect or SmartConfig-------------- */



bool wifiConnect()
{
  WiFi.begin();
  Serial.print("WiFi Connecting.");
  for (unsigned int i = 0; i < 10; i++)//5s connecting time
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      printWifiStatus();
      //WiFi.printDiag(Serial);
      return true;
    }
    else
    {
      Serial.print(".");
      //Serial.println(wstatus);
      delay(500);
    }
  }
  Serial.println("\nWifi Connect Failed!" );
  return false;
  //WiFi.printDiag(Serial);
}





void printWifiStatus(){
  Serial.println("\nWiFi Connected.");
  Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
  Serial.printf("Password: %s\n", WiFi.psk().c_str());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}


void initSmartConfig()
{
  smartConfigTicker.attach(0.1, SMART_CONFIG_LED_TOGGLE);
  /** 連線到無線網路 */
  // 設定模式為 STA ( MODE 只能是 STA ) 才能使用 smartconfig
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();
  Serial.println("Start SmartConfig.");
  /* Wait for SmartConfig packet from mobile */
  Serial.println("Waiting for SmartConfig. Launch Mobile App (ex: ESP-TOUCH ) to progress SmartConfig.");
  unsigned int tick=0;
  while (!WiFi.smartConfigDone()) {    
    delay(500);
//    Serial.print(tick);
    tick++;
    if(tick > 120)  ESP.reset();//2 min smart config time
  }
  Serial.println("");
  Serial.println("SmartConfig done.");
  
  /* Wait for WiFi to connect to AP */
  Serial.print("WiFi Connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  exit_smartConfig();
  printWifiStatus();

  //WiFi.setAutoConnect(true);

}




//Working fine
void initSmartConfigV3()
{
  smartConfigTicker.attach(0.1, SMART_CONFIG_LED_TOGGLE);
  /** 連線到無線網路 */
  // 設定模式為 STA ( MODE 只能是 STA ) 才能使用 smartconfig
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();
  Serial.println("Start SmartConfig.");
  /* Wait for SmartConfig packet from mobile */
  Serial.println("Waiting for SmartConfig. Launch Mobile App (ex: ESP-TOUCH ) to progress SmartConfig.");
//  unsigned int tick=0;
  while (!WiFi.smartConfigDone()) {
    watchdogCount = 0;
    
    delay(500);//Necessary. otherwise SWD timer will reset the board
//    Serial.print(tick);
//    tick++;
//    if(tick > 120)  ESP.reset();//2 min smart config time
  }
  Serial.println("");
  Serial.println("SmartConfig done.");
  
  /* Wait for WiFi to connect to AP */
  Serial.print("WiFi Connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  exit_smartConfig();
  printWifiStatus();
  startmDNS();//1st
  api();//2nd
  startServer();//3rd
  setup_mqtt();//4th 

  //WiFi.setAutoConnect(true);

}





void initSmartConfigV2(){
  
  unsigned int cnt = 0;
  
  smartConfigTicker.attach(0.1, SMART_CONFIG_LED_TOGGLE);

  WiFi.mode(WIFI_STA);

  delay(1000);
  
    // if wifi cannot connect start smartconfig
  while(WiFi.status() != WL_CONNECTED) {
//    watchdogCount = 0; //not needed
    Serial.println("Waiting for SmartConfig");
    delay(500);
    Serial.print(".");
    if(cnt++ >= 15){
       WiFi.beginSmartConfig();
       while(1){
           watchdogCount = 0;
           delay(500);
           if(WiFi.smartConfigDone()){
             Serial.println("SmartConfig Success");
             exit_smartConfig();
             break;
           }
       }
    }
  }

  Serial.println("");
  Serial.println("");
  
  WiFi.printDiag(Serial);

  // Print the IP address
  Serial.println(WiFi.localIP());
}


void enter_smartconfig()
{
  if (in_smartconfig == false) {
    in_smartconfig = true;
    
    // Completely RESETS the existing SAVED configuration
    WiFi.disconnect();//very important ! or else smart config won't work
//    initSmartConfig();
//    initSmartConfigV2();
    initSmartConfigV3();
//    WiFi.beginSmartConfig();
  }
}


void exit_smartConfig()
{
  smartConfigTicker.detach();
  SMART_CONFIG_LED_ON();
  in_smartconfig = false;
}
