#include <ESP8266WiFi.h>
#include <espnow.h>
int test = 0;
// ### Steuerung ###
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
const uint8_t ERROR_SENDING = 0b11111100;

uint8_t broadcastAddress[] = {0xEC, 0xFA, 0xBC, 0xAF, 0x28, 0x06};

typedef struct struct_message {
    uint8_t cmdId;
    uint8_t cmdType;
    uint8_t cmdIndex;
    int16_t cmdValue;
    uint8_t cmdState;
} struct_message;

struct_message message;

bool messageReceived = false;

// ### Motoren ###
uint8_t CODE_PINS[] = {14, 12, 13, 15};
uint8_t MOTOR_PINS[] = {5, 4, 0};
uint8_t ENABLED_PIN = 16;
uint8_t CODES[] = {0b1001, 0b0001, 0b0101, 0b0100,
                0b0110, 0b0010, 0b1010, 0b1000 };

uint16_t delayTime = 200;

uint16_t steps[] = {0, 0, 0};
uint8_t indices[] = {0, 0, 0};

int16_t alphas[] = {0, 0, 0};

/*
* ###############
* Code fuer das WLAN
* ###############
*/

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
    showErrorCode(ERROR_SENDING);
  }
}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  if(messageReceived) return;

  memcpy(&message, incomingData, sizeof(message));
  Serial.println("Data received");
  Serial.print("cmdId: ");    Serial.println(message.cmdId);
  Serial.print("cmdType: ");  Serial.println(message.cmdType);
  Serial.print("cmdIndex: "); Serial.println(message.cmdIndex);
  Serial.print("cmdValue: "); Serial.println(message.cmdValue);
  Serial.print("cmdState: "); Serial.println(message.cmdState);
  messageReceived = true;
  
}

void sendMessage(){
  esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));
}

void processMessage(){
  
  message.cmdState = STATE_RECEIVED;
  sendMessage();
  
  if(!isEnabled()){
    message.cmdState = STATE_DISABLED;
    sendMessage();
    return;
  }

  message.cmdState = STATE_DONE;
  switch(message.cmdType){
    case TYPE_TEST:
      break;
    case TYPE_ROTATE:
      rotate(message.cmdIndex, message.cmdValue);
      break;
    case TYPE_HOME:
      home(message.cmdIndex);
      break;
    case TYPE_HOME_ALL:
      homeAll();
      break;
    case TYPE_RESET:
      reset(message.cmdIndex);
      break;
    case TYPE_RESET_ALL:
      resetAll();
      break;
    default:
      message.cmdState = STATE_ERROR;
  }

  sendMessage();
}

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

void setup() {
  for(int i = 0; i < sizeof(MOTOR_PINS) / sizeof(MOTOR_PINS[0]); i++){
    pinMode(MOTOR_PINS[i], OUTPUT);
    digitalWrite(MOTOR_PINS[i], LOW);
  }
  for(int i = 0; i < sizeof(CODE_PINS) / sizeof(CODE_PINS[0]); i++){
    pinMode(CODE_PINS[i], OUTPUT);
    digitalWrite(CODE_PINS[i], LOW);
  }
  pinMode(ENABLED_PIN, INPUT);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    showErrorCode(ERROR_ESP_INIT);
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  esp_now_register_send_cb(OnDataSent);

  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  int test2 = digitalRead(ENABLED_PIN);
  if( test2 != test){
    test = test2;
    Serial.println(test);
  }

  if(messageReceived){
    processMessage();
    messageReceived = false;
  }
}
