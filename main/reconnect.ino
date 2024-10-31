//------------------------------------------------While client not conncected---------------------------------//




boolean reconnect() {
  
    mqttTicker.attach(0.25, MQTT_LED_TOGGLE);//end of WL_CONNECTED

    P_R_I_N_T("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266-";
    clientId += String(random(0xffff), HEX);
  
    if (client.connect(clientId.c_str(), "iotdatasoft", "brokeriot2o2o")) {
      P_R_I_N_T("connected");        
      client.subscribe(configTopic);
      client.subscribe(armTopic);
      mqttTicker.detach();
      MQTT_LED_ON();
      //      delay(250);   
    }else{  
      Serial.print("failed, rc=");
      Serial.print(client.state());
      P_R_I_N_T(" try again");
//      mqttTryCounter++;
//      Serial.println(mqttTryCounter);
      // Wait 6 seconds before retrying
//      delay(5000); //blocking
    }
  return client.connected();
}// reconnect() ends




void setup_mqtt(){      
   client.setServer(mqtt_server, mqttPort);
   client.setCallback(callback);
}


void setMqttReconnectInterval(){      
    if (!client.connected()) {
      
      unsigned long now = millis();
      if (now - lastReconnectTime > 30000) {
          lastReconnectTime = now;
          P_R_I_N_T("Ticking every 30 seconds");
          

//          P_R_I_N_T(WiFi.status());

          if(WiFi.status() == WL_CONNECTED){
              // Attempt to mqtt reconnect
              if (reconnect()) {
                
                lastReconnectTime = 0;//GOOD to make sure "now - lastReconnectTime" this line becomes true
              }
          }else{mqttTicker.attach(0.25, MQTT_LED_TOGGLE);}
          
//          if(WiFi.status() != WL_CONNECTED){mqttTicker.attach(0.25, MQTT_LED_TOGGLE);}//end of WL_CONNECTED       
 
      }//end of interval check
   }else{//if client connected
      client.loop();
   }
  
}//setMqttReconnect
