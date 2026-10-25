unsigned long lastStepTime = 0;

/*
 * ####################
 * Code für die Motoren
 * ####################
 */

void outputCode(uint8_t code){
  digitalWrite(CODE_PINS[0], (code & 1) ? HIGH : LOW);
  code >>= 1;
  digitalWrite(CODE_PINS[1], (code & 1) ? HIGH : LOW);
  code >>= 1;
  digitalWrite(CODE_PINS[2], (code & 1) ? HIGH : LOW);
  code >>= 1;
  digitalWrite(CODE_PINS[3], (code & 1) ? HIGH : LOW);
  code >>= 1;
  delay(delayTime);
  
}

void activateMotor(uint8_t k){
  if(k >= sizeof(MOTOR_PINS) / sizeof(MOTOR_PINS[0])) return;

  digitalWrite(MOTOR_PINS[k], HIGH);
  delay(delayTime); 
}

void deactivateMotor(uint8_t k){
  if(k >= sizeof(MOTOR_PINS) / sizeof(MOTOR_PINS[0])) return;

  digitalWrite(MOTOR_PINS[k], LOW);
  delay(delayTime);
}

void rotatePos(uint8_t k, uint16_t n){
  
  if(k >= sizeof(MOTOR_PINS) / sizeof(MOTOR_PINS[0])) return;
  if(n == 0) return;

  uint16_t s = steps[k];
  uint8_t i = indices[k];
  Serial.println(n);
  outputCode(CODES[i]);
  activateMotor(k);
  
  while(n > 0){
    
    if(s < 399) s++;
    else s = 0;

    i++;
    i &= 7;
    outputCode(CODES[i]);
    n--;
  }

  deactivateMotor(k);
  outputCode(0);
  steps[k] = s;
  indices[k] = i;
}

void rotateNeg(uint8_t k, uint16_t n){
  if(k >= sizeof(MOTOR_PINS) / sizeof(MOTOR_PINS[0])) return;
  if(n == 0) return;

  uint16_t s = steps[k];
  uint8_t i = indices[k];
  
  outputCode(CODES[i]);
  activateMotor(k);

  while(n > 0){
    if(s > 0) s--;
    else s = 399;

    i--;
    i &= 7;
    outputCode(CODES[i]);
    n--;
  }

  deactivateMotor(k);
  outputCode(0);
  steps[k] = s;
  indices[k] = i;
}

bool isEnabled(){
  return digitalRead(ENABLED_PIN);
}

/*
 * ####################
 * Code für die Befehle
 * ####################
 */

void rotate(uint8_t k, int16_t Dalpha){
  
  if(k > sizeof(MOTOR_PINS) / sizeof(MOTOR_PINS[0])) return;
  if(Dalpha < -10800 || Dalpha > 18000) return;

  int16_t a = alphas[k];
  a += Dalpha;

  if(Dalpha > 0){
    uint16_t n = round(a / 27.0) - steps[k];
    rotatePos(k, n);
    if(a >= 10800) a -= 10800;
  }
  if(Dalpha < 0){
    uint16_t n = steps[k] - round(a / 27.0);
    rotateNeg(k, n);
    if(a < 0) a += 10800;
  }

  alphas[k] = a;
}

void home(uint8_t k){
  if(k > sizeof(MOTOR_PINS) / sizeof(MOTOR_PINS[0])) return;

  int16_t Da = -alphas[k];
  if(Da <= -5400) Da += 10800;
  rotate(k, Da);
}

void homeAll(){
  for(uint8_t k = 0; k < sizeof(MOTOR_PINS) / sizeof(MOTOR_PINS[0]); k++){
    home(k);
  }
}

void reset(uint8_t k){
  steps[k] = 0;
  alphas[k] = 0;
}

void resetAll(){
  for(uint8_t k = 0; k < sizeof(MOTOR_PINS) / sizeof(MOTOR_PINS[0]); k++){
    reset(k);
  }
}