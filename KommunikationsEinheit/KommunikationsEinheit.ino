#include <ESP8266WiFi.h>
#include <espnow.h>
#include <string>
using namespace std;

#define LED_PIN 2

const uint8_t TYPE_TEST = 1;
const uint8_t TYPE_ROTATE = 2;
const uint8_t TYPE_HOME = 3;
const uint8_t TYPE_HOME_ALL = 4;
const uint8_t TYPE_RESET = 5;
const uint8_t TYPE_RESET_ALL = 6;
const uint8_t TYPE_CHANGE_SETTING = 7;

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

String receivedMessage;

void showErrorCode(uint8_t code){
  uint8_t tmpCode = code;

  for(int i = 0; i < 5; i++){
    for(int j = 0; j < 8; j++){
      digitalWrite(LED_PIN, (tmpCode & 1) ? HIGH : LOW);
      tmpCode >>= 1;
      delay(1000);
    }
    tmpCode = code;
    delay(2000);
  }
}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&message, incomingData, sizeof(message));
  char buffer[64]; 

  snprintf(buffer, sizeof(buffer), "%d, %d, %d, %d, %d\n", message.cmdId, message.cmdType, message.cmdIndex, message.cmdValue, message.cmdState);
  Serial.write(buffer);
}


void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    showErrorCode(ERROR_ESP_INIT);
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  //esp_now_register_send_cb(OnDataSent);

  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  esp_now_register_recv_cb(OnDataRecv);

  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  while (Serial.available()) {
    receivedMessage = Serial.readStringUntil('\n');
    
    if(receivedMessage.equals("pingESP")){
      Serial.write("pongESP");
    }
    else{
      uint8_t first = receivedMessage.indexOf(",");
      uint8_t second = receivedMessage.indexOf(",", first + 1);
      uint8_t third = receivedMessage.indexOf(",", second + 1);
      uint8_t fourth = receivedMessage.indexOf(",", third + 1);

      message.cmdId = receivedMessage.substring(0, first).toInt();
      message.cmdType = receivedMessage.substring(first+1, second).toInt();
      message.cmdIndex = receivedMessage.substring(second + 1, third).toInt();
      message.cmdValue = receivedMessage.substring(third + 1, fourth).toInt();
      message.cmdState = receivedMessage.substring(fourth).toInt();

      esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));
    }

    receivedMessage = "";
  }

}
