void SMART_CONFIG_LED_ON(){ digitalWrite(SMART_CONFIG_LED, HIGH);  }
void SMART_CONFIG_LED_OFF() {digitalWrite(SMART_CONFIG_LED, LOW);}
void SMART_CONFIG_LED_TOGGLE() {digitalWrite(SMART_CONFIG_LED, digitalRead(SMART_CONFIG_LED) ^ 0x01);}



#define MQTT_LED_ON() digitalWrite(MQTT_LED, HIGH)
#define MQTT_LED_OFF() digitalWrite(MQTT_LED, LOW)
void MQTT_LED_TOGGLE() {digitalWrite(MQTT_LED, digitalRead(MQTT_LED) ^ 0x01);}


void ARM_LED_ON(){ digitalWrite(ARM_LED, HIGH);}
void ARM_LED_OFF(){ digitalWrite(ARM_LED, LOW);}




////for power and net status
//void toggleLed()
//{
//  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));     // set pin to the opposite state
//
////  digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
//}
//
//void alwaysOnLed(){
////    digitalWrite (LED_BUILTIN, LOW);
//    digitalWrite(LED_BUILTIN, HIGH);
//}




