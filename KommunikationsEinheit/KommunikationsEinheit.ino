#include <GxEPD2.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>
#include <GxEPD2_7C.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_BW_SHM.h>
#include <GxEPD2_EPD.h>
#include <GxEPD2_EPD_friends.h>
#include <GxEPD2_GFX.h>

#include <ESP8266WiFi.h>
#include <espnow.h>

const uint8_t TYPE_TEST = 1;
const uint8_t TYPE_ROTATE = 2;
const uint8_t TYPE_HOME = 3;
const uint8_t TYPE_HOME_ALL = 4;
const uint8_t TYPE_RESET = 5;
const uint8_t TYPE_RESET_ALL = 6;

const uint8_t STATE_RECEIVED = 1;
const uint8_t STATE_DISABLED = 2;
const uint8_t STATE_DONE = 3;
const uint8_t STATE_ERROR = 4;

const uint8_t ERROR_ESP_INIT = 0b00110011;

uint8_t broadcastAddress[] = {0xE8, 0xDB, 0x84, 0xB1, 0x07, 0x83};


typedef struct struct_message {
    uint8_t cmdId;
    uint8_t cmdType;
    uint8_t cmdIndex;
    int16_t cmdValue;
    uint8_t cmdState;
} struct_message;

struct_message message;

void showErrorCode(uint8_t code){
  uint8_t tmpCode = code;

  for(int i = 0; i < 5; i++){
    for(int j = 0; j < 8; j++){
      digitalWrite(2, (tmpCode & 1) ? HIGH : LOW);
      tmpCode >>= 1;
      delay(1000);
    }
    tmpCode = code;
  }
}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&message, incomingData, sizeof(message));
  Serial.println("Data received");
  Serial.print("cmdId: ");    Serial.println(message.cmdId);
  Serial.print("cmdType: ");  Serial.println(message.cmdType);
  Serial.print("cmdIndex: "); Serial.println(message.cmdIndex);
  Serial.print("cmdValue: "); Serial.println(message.cmdValue);
  Serial.print("cmdState: "); Serial.println(message.cmdState);
}


void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    showErrorCode(ERROR_ESP_INIT);
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  //esp_now_register_send_cb(OnDataSent);

  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  esp_now_register_recv_cb(OnDataRecv);

}

void loop() {
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    
    if(command.equals("pingESP")){
      Serial.write("pongESP");
    }
    else{
      Serial.print("You sent: ");
      Serial.println(receivedMessage);
      

      int first = receivedMessage.indexOf(",");
      int second = receivedMessage.indexOf(",", first + 1);

      message.cmdIndex = receivedMessage.substring(0, first).toInt();
      message.cmdValue = receivedMessage.substring(first+1, second).toInt();
      message.cmdType = receivedMessage.substring(second+1).toInt();
      Serial.print("You sent: ");
      Serial.print(message.cmdIndex);
      Serial.print(message.cmdValue);
      Serial.print(message.cmdType);
      receivedMessage = "";

      esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));
    }
  }

}
