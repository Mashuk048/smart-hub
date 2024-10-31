//-----------------------Callback function-------------------------------------//

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  P_R_I_N_T(topic);
  

  
//-------------------------------------Getting CONFIG data---------------------//

  if(strcmp(topic, configTopic) == 0){
    
      StaticJsonDocument<128> doc;
      deserializeJson( doc,payload,length );
      const char* newhid =  doc["hid"];
//      unsigned long newStime = doc["stime"];
      uint8_t rst = doc["reset"];
      uint8_t pair = doc["pair"];
      uint8_t alarm = doc["alarm"];

      
      P_R_I_N_T(newhid);
      P_R_I_N_T(rst);
      P_R_I_N_T(pair);
      P_R_I_N_T(alarm);

    

      if(strcmp(hid,newhid) == 0){
        //tone alert
        ALARM_ON();
        delay(250);
        ALARM_OFF();
        delay(100); 
        
        //RESET cmd check
        if(rst){
        P_R_I_N_T("RESET");

        StaticJsonDocument<32> doc;
        doc["hid"] = hid;
        doc["msg"] = "200";//reset
    
        char buffer[32];
        size_t n = serializeJson(doc, buffer);//stringify
        client.publish(responseTopic,buffer, false);

        delay(500); 
        ESP.restart();
      }else{   P_R_I_N_T("NO RESET");}//rst validation done

      






      //Pair command check
      if(pair){
     
//        char buffer[128];
//        doc["pair"] = "success";
//        size_t n = serializeJson(doc, buffer);
//        client.publish(responseTopic,buffer, false);
        delay(200);
        
        isPairingRequested = true;
        smartConfigTicker.attach(0.1, SMART_CONFIG_LED_TOGGLE);
        P_R_I_N_T("PAIRING");
        
      }else if(!pair){
        stopPairingMode();
        P_R_I_N_T("STOP PAIRING");
      }





       //Alarm command check
      if(alarm){
     
//        char buffer[128];
//        doc["pair"] = "success";
//        size_t n = serializeJson(doc, buffer);
//        client.publish(responseTopic,buffer, false);

        P_R_I_N_T("ALARMING");
        ALARM_ON();
        
      }else{        
        P_R_I_N_T("STOP ALARMING");
        ALARM_OFF();
      }//ALARM validation done
      
      
    }//HID VALIDATION DONE
    
    else{
            P_R_I_N_T("HID INVALID");     
//          doc["msg"] = "hid invalid";   
//          char buffer[128];
//          size_t n = serializeJson(doc, buffer);               
//          client.publish(responseTopic, buffer, false);
    }
  }//config topic validation ends






//ARM topic validation and data parsing

    if(strcmp(topic, armTopic) == 0){
    
      StaticJsonDocument<64> doc;
      deserializeJson( doc,payload,length );
      const char* newhid =  doc["hid"];
      int arm = doc["arm"];
      
      P_R_I_N_T(newhid);
      P_R_I_N_T(arm);


     if(strcmp(hid,newhid) == 0){
        //tone alert
        ALARM_ON();
        delay(250);
        ALARM_OFF();
        delay(100); 

        
            //Arm command check
            if(arm == 0){
              isArmed = false;      
              P_R_I_N_T("DISARMED!");
              ARM_LED_OFF();
              P_R_I_N_T(writeConfigFile(isArmed));
      
              StaticJsonDocument<32> doc;
              doc["hid"] = hid;
              doc["msg"] = "300";//disarmed
          
              char buffer[32];
              size_t n = serializeJson(doc, buffer);//stringify
              client.publish(responseTopic,buffer, false);
              delay(250);        
              
            }else if(arm == 1){             
              ARM_LED_ON();
              isArmed = true;
              P_R_I_N_T("ARMED!");                           
              P_R_I_N_T(writeConfigFile(isArmed));
              
              StaticJsonDocument<32> doc;
              doc["hid"] = hid;
              doc["msg"] = "200";//armed
          
              char buffer[32];
              size_t n = serializeJson(doc, buffer);//stringify
              client.publish(responseTopic,buffer, false);
              delay(250);  
            }else{
              StaticJsonDocument<32> doc;
              doc["hid"] = hid;
              doc["msg"] = "400";//Failed
          
              char buffer[32];
              size_t n = serializeJson(doc, buffer);//stringify
              client.publish(responseTopic,buffer, false);
              delay(250); 
            }//ARM validation done       
     }else{
          doc["did"] = "failed";   
          char buffer[128];
          size_t n = serializeJson(doc, buffer);
          P_R_I_N_T("did mismatch");          
          client.publish(responseTopic, buffer, false);
      }//DID validation for armtopic is done
    }//armTopic validation done
  
     
}//mqtt msg Callback ends
