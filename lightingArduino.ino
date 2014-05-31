#include <FastLED.h>

#define NUM_LEDS 61

//brightness modifiers
#define LIGHTS_OFF      0
#define LIGHTS_ON       1
//effects
#define EFFECT_NONE       0
#define EFFECT_DAMAGE     1
#define EFFECT_HEARTBEAT  2

//lighting patterns
#define MODE_IDLE     0
#define MODE_REDALERT 1
#define MODE_WARP     2
#define MODE_BRIEF    3

#define SEATBELT   0
#define PRAY       1

#define MAX_BRIGHT  30

#define NUM_LEDS 99

#define PIN_AIRLOCK  15

CRGB right_effect[99];
CRGB left_effect[99];

CRGB praySeatLights[2];  //first 3 are the front cabin light that isnt in use yet

CRGB baseCol;

int sinCount = 0;
long lastWarpUpdate = 0;

boolean cabinState = false;


long redAlertTimer = 0;
boolean redToggle = false;

long damageTimer = 0;
boolean damageOn = false;
long nextDamageFlicker = 500;

boolean airlockBlink = false;
boolean airlockLight = false;
long airlockBlinkTime = 0;

boolean heartBeat = false;
long heartBeatTimer = 0;

byte state = LIGHTS_OFF;
byte effect = EFFECT_NONE;
byte mode = MODE_IDLE;

long flickerTimeOut = 0;
long flickRand = 0;
boolean flickerToggle = false;

boolean prayLight = false;
boolean seatbeltLight = false;


void setup()
{
  Serial.begin(9600);
  FastLED.addLeds<WS2811, 13, GRB>(left_effect, 99); 
  FastLED.addLeds<WS2811, 11, GRB>(right_effect, 99); 
  general_lighting(CRGB::Black);
  
  baseCol.r = 0;
  baseCol.g = 0;
  baseCol.b = 0;

  for (int i = 0; i < 2; i++){
    praySeatLights[i].r = 0;
    praySeatLights[i].g = 0;
    praySeatLights[i].b = 0;
  }
  FastLED.addLeds<WS2811, 8, GRB>(praySeatLights, 2); 
  FastLED.show();
  
  for (int i = 0; i < 4; i++){
    pinMode(19 - i, OUTPUT);
  }
  pinMode(PIN_AIRLOCK, OUTPUT);
  digitalWrite(PIN_AIRLOCK, LOW);


  cabinLightState(false);
}

void cabinLightState(boolean state){
  for (int i = 0; i < 4; i++){
    digitalWrite(19 -  i, state == true ? HIGH : LOW);
  }
}

void loop() { 
  if(state == LIGHTS_ON){
    cabinState = true;

    if(mode == MODE_IDLE){
      baseCol = CRGB(0, 0, MAX_BRIGHT);
    } 
    else if (mode == MODE_REDALERT){
      if(redAlertTimer + 800 < millis()){
        redToggle = !redToggle;
        redAlertTimer = millis();
      }
      baseCol = CRGB(redToggle == true ? 128 : 0, 0, 0);
    } 
    else if (mode == MODE_WARP){
      baseCol = CRGB(0, 0, MAX_BRIGHT);
    } 
    else if (mode == MODE_BRIEF){
      baseCol = CRGB::White;
    }
  } 
  else {
    cabinState = false;
    baseCol = CRGB::Black;
    airlockLight = false;
  }

  //effects
  if(effect == EFFECT_HEARTBEAT){
    if(heartBeatTimer + 400 < millis() && heartBeat){
      heartBeat = false;
      effect = EFFECT_NONE;
    } 
    else {
      baseCol = CRGB::Black;
      cabinState = false;
    }

  } 
  if (effect == EFFECT_DAMAGE){
    if(damageTimer + 1500 < millis()){
      effect = EFFECT_NONE;
    } 
    else {
      if(flickerTimeOut + flickRand < millis()){
        flickRand = 30 + random(150);
        flickerTimeOut = millis();
        flickerToggle = ! flickerToggle;
      }
      float rand = flickerToggle == true ? 1.0f : 0.3f;  //random(100) / 10.0f;
      baseCol.r = (int)baseCol.r * rand;
      baseCol.g = (int)baseCol.g * rand;
      baseCol.b = (int)baseCol.b * rand;
      cabinState = flickerToggle;
    }

  }
  if(mode == MODE_WARP){
    warp();
  } 
  else {
    general_lighting(baseCol);
  }
  cabinLightState (cabinState);
  if(airlockBlink && airlockBlinkTime + 500 < millis()){
    airlockLight = ! airlockLight;
    airlockBlinkTime = millis();
  }
  digitalWrite(PIN_AIRLOCK, airlockLight == true ? HIGH : LOW );



  if(seatbeltLight){
    praySeatLights[SEATBELT].r = 255;
    praySeatLights[SEATBELT].g = 255;
    praySeatLights[SEATBELT].b = 255;
  } 
  else {
    praySeatLights[SEATBELT].r = 0;
    praySeatLights[SEATBELT].g = 0;
    praySeatLights[SEATBELT].b = 0;
  }
  if(prayLight){
    praySeatLights[PRAY].r = 255;
    praySeatLights[PRAY].g = 255;
    praySeatLights[PRAY].b = 255;
  } 
  else {
    praySeatLights[PRAY].r = 0;
    praySeatLights[PRAY].g = 0;
    praySeatLights[PRAY].b = 0;
  }

  handleSerialCommands();
  FastLED.show();
}

void general_lighting(CRGB colour) {
  for(int i = 0 ; i < NUM_LEDS; i++ ) {
    left_effect[i] = colour;
    right_effect[i] = colour;
  }
}

void on(){
  general_lighting(CRGB::Blue);
  cabinLightState(true);

}

void damage(){
  unsigned char c = random(255);
  general_lighting(CRGB(0,0,c));
  if(random(10) < 5){
    cabinLightState(false);
  } 
  else {
    cabinLightState(true);
  }

}

void kill(){
  general_lighting(CRGB::Black);
  cabinLightState(false);
  airlockLight = false;
  airlockBlink = false;
}

void redAlert(){
  unsigned char red = 0;
  if(redAlertTimer + 1500 < millis()){
    redAlertTimer = millis();
    redToggle = !redToggle;
    if (redToggle){
      red = 128;
    } 
    else {
      red = 0;
    }
    general_lighting(CRGB(red, 0, 0));
  }
  cabinLightState(true);
}

void warp(){
  //apply sin wave over the top of the base colour
  if(lastWarpUpdate + 25 < millis()){
    sinCount ++;
    sinCount %= 628;
    lastWarpUpdate = millis();
  }
  for(int i = 0; i < NUM_LEDS; i++){
    float sinMod = sin(sinCount / 3 + i);
    if(sinMod < 0){ 
      sinMod = 0; 
    }
    CRGB colour = CRGB((int)(baseCol.r * sinMod), (int)(baseCol.g * sinMod), (int)(baseCol.b * sinMod));
    left_effect[i] = colour;
    right_effect[i] = colour;
  }
}

void handleSerialCommands(){
  if(Serial.available() == 0){
    return;
  }
  char c = Serial.read();
  switch (c) {
  case 'k':
    state = LIGHTS_OFF;
    airlockBlink = false;
    airlockLight = false;
    break;
  case 'o':
    state = LIGHTS_ON;
    break;
  case 'r':
    mode = MODE_REDALERT;
    break;
  case 'w':
    mode = MODE_WARP;
    break;
  case 'd':
    damageTimer = millis();
    flickerTimeOut = millis();

    damageOn = true;
    effect = EFFECT_DAMAGE;
    break;
  case 'h':
    effect = EFFECT_HEARTBEAT;
    heartBeatTimer = millis();
    heartBeat = true;
    break;
  case 'b':
    mode = MODE_BRIEF;
    break;
  case 'R':
    mode = MODE_IDLE;
    effect = EFFECT_NONE;
    state = LIGHTS_OFF;
    break;
  case 'i':
    mode = MODE_IDLE;
    break;
  case 'S':
    seatbeltLight = true;
    break; 
  case 's':
    seatbeltLight = false;
    break;
  case 'P': 
    prayLight = true;
    break;
  case 'p':
    prayLight = false;
    break;
  case 'A':  //airlock light
    airlockBlink = true;
    airlockLight = true;
    break;
  case 'a':
    airlockBlink = false;
    airlockLight = false;
    break;
  case '\n':
  case '\r':
    return;
  default:
    Serial.write("Unknown,");
    return;
  }
  Serial.write("OK,");
}







