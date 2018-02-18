// OpenReflow controller
// Made by @CarlosGS
// https://github.com/CarlosGS/OpenReflow
// CC-BY-SA license


#include "ESP8266WiFi.h"
#include "firmware.h"

void init_buzzer() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}
void play_tone(int frequency, int duration_ms) {
  tone(BUZZER_PIN, frequency, duration_ms);
  delay(duration_ms+10);
  digitalWrite(BUZZER_PIN, LOW);
}
void play_startup() {
  play_tone(400, 200);
  play_tone(600, 200);
  play_tone(800, 600);
}
void play_error() {
  play_tone(800, 600);
  delay(200);
  play_tone(800, 600);
  delay(200);
  play_tone(800, 600);
  delay(200);
  play_tone(800, 600);
  delay(200);
}
void play_reflow_begin() {
  play_tone(400, 100);
  play_tone(600, 100);
  play_tone(800, 100);

  play_tone(400, 100);
  play_tone(600, 100);
  play_tone(800, 100);

  play_tone(400, 100);
  play_tone(600, 100);
  play_tone(800, 600);
}
void play_reflow_end() {
  play_tone(800, 200);
  delay(50);
  play_tone(800, 1000);
}


void init_relay() {
  pinMode(ZERO_CROSS_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}
void set_relay(bool state) {
  while(digitalRead(ZERO_CROSS_PIN) == HIGH) yield();
  while(digitalRead(ZERO_CROSS_PIN) == LOW) yield();
  digitalWrite(RELAY_PIN, state);
}

void firmwareSetup() {
  init_relay();
  init_buzzer();

  play_startup();
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float Vin = 3.3; // volts
float R2 = 50; // ohms

float temp() {
  double Vout_filt = 0;
  int N = 100;
  for(int i=0; i<N; i++) {
    Vout_filt += analogRead(A0);
    delay(1);
  }
  Vout_filt /= (double)N;
  float Vout = mapf(Vout_filt, 227,896.95, 0.202,0.833);// Calibrated ADC values
  float R1 = R2*((Vin/Vout)-1);
  float temp_degreesC = ((R1/100)-1)/0.00385;
  return temp_degreesC;
}

float value_filtered = 0.0;

int reflow_phase = -1;

float last_temperature = 0.0;

float temperature = 0.0;

float maxSpeedTemp[3]={3,0.8,1000};
float endTemp[3]={150,200,235};

void setParametersPhase(int phase, float maxSpeedT, float endT){
      maxSpeedTemp[phase] = maxSpeedT;
      endTemp[phase] = endT;
}

float getMaxSpeedTemp(int phase){
      return maxSpeedTemp[phase];
}

float getEndTemp(int phase){
      return endTemp[phase];
}

void setDefaultParameters(){
      maxSpeedTemp[RAMP_UP_PHASE] = 3; // max 3degC/s
      endTemp[RAMP_UP_PHASE] = 150;

      maxSpeedTemp[PREHEAT_PHASE] = 0.8; // max 0.8degC/s
      endTemp[PREHEAT_PHASE] = 200;

      maxSpeedTemp[REFLOW_PHASE] = 1000; // max always on
      endTemp[REFLOW_PHASE] = 235;
}

void firmwareLoop() {
  int heating = 0;
  temperature = temp();
  float temp_speed = (temperature-last_temperature)*2;// the 2 scales to degreesC/second

  switch(reflow_phase) {
    case RAMP_UP_PHASE: // Ramp-up
      heating = temp_speed < maxSpeedTemp[RAMP_UP_PHASE];
      if(temperature > endTemp[RAMP_UP_PHASE]) {
        reflow_phase = PREHEAT_PHASE;
        play_startup();
      }
      break;
    case PREHEAT_PHASE: // Preheat
      heating = temp_speed < maxSpeedTemp[PREHEAT_PHASE]; // max 0.8degC/s
      if(temperature > endTemp[PREHEAT_PHASE]) {
        reflow_phase = REFLOW_PHASE;
        play_reflow_begin();
      }
      break;
    case REFLOW_PHASE: // Reflow
      heating = 1;
      if(temperature > endTemp[REFLOW_PHASE]) {
        reflow_phase = COOL_PHASE;
        play_reflow_end();
      }
      break;
    case COOL_PHASE: // Cool
    default:
      heating = 0;
      break;
  }
  set_relay(heating);
  DBG_OUTPUT_PORT.print(millis());
  DBG_OUTPUT_PORT.print("\t");
  DBG_OUTPUT_PORT.print(reflow_phase);
  DBG_OUTPUT_PORT.print("\t");
  DBG_OUTPUT_PORT.print(temperature);
  DBG_OUTPUT_PORT.print("\t");
  DBG_OUTPUT_PORT.println(heating);
//  delay(500);
  last_temperature = temperature;
}
