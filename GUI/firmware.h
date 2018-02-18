#ifndef _OPEN_REFLOW_CONTROLLER_H
#define _OPEN_REFLOW_CONTROLLER_H

#define DBG_OUTPUT_PORT Serial

#define RAMP_UP_PHASE 0
#define PREHEAT_PHASE 1
#define REFLOW_PHASE 2
#define COOL_PHASE 3

#define RELAY_PIN 13
#define ZERO_CROSS_PIN 14

#define BUZZER_PIN 12

/** Public variables */
extern int reflow_phase;
extern float last_temperature;
extern float temperature;

/** Parameters getters and setters */
void setDefaultParameters();
void setParametersPhase(int phase, float maxSpeedT, float endT);
float getMaxSpeedTemp(int phase);
float getEndTemp(int phase);


void firmwareSetup();
/** control function - must be called every 500ms during the reflow process !*/
void firmwareLoop();


#endif
