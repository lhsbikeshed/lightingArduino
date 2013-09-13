#include <FastSPI_LED.h>

#define NUM_LEDS 60

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
// Sometimes chipsets wire in a backwards sort of way
//struct CRGB { unsigned char b; unsigned char r; unsigned char g; };
struct CRGB { 
  unsigned char r; 
  unsigned char g; 
  unsigned char b; 
};
struct CRGB *leds;

CRGB baseCol;

int sinCount = 0;
long lastWarpUpdate = 0;

/*
left
 back -> front
 0 - 10 
 21 - 30
 40 -> 49
 
 
 right
 20 -> 11
 31 -> 39
 50 -> 60
 */
byte leftLeds[] = {
  0,1,2,3,4,5,6,7,8,9,10,21,22,23,24,25,26,27,28,29,30,40,41,42,43,44,45,46,47,48,49}; // 31
byte rightLeds[] = {
  20,19,18,17,16,15,14,13,12,11,39,38,37,36,35,34,33,32,31,60,59,58,57,56,55,54,53,52,51,59}; // 30


boolean cabinState = false;


long redAlertTimer = 0;
boolean redToggle = false;

long damageTimer = 0;
boolean damageOn = false;
long nextDamageFlicker = 500;

boolean heartBeat = false;
long heartBeatTimer = 0;

byte state = LIGHTS_OFF;
byte effect = EFFECT_NONE;
byte mode = MODE_IDLE;

long flickerTimeOut = 0;
long flickRand = 0;
boolean flickerToggle = false;


void setup()
{
  FastSPI_LED.setLeds(NUM_LEDS);

  FastSPI_LED.setChipset(CFastSPI_LED::SPI_LPD6803);


  FastSPI_LED.init();
  FastSPI_LED.start();

  leds = (struct CRGB*)FastSPI_LED.getRGBData(); 
  memset(leds, 0, NUM_LEDS * 3);
  for(int i = 0 ; i < NUM_LEDS; i++ ) {

    leds[i].r = 0; 
    leds[i].g = 0; 
    leds[i].b = 0; 
  }
  Serial.begin(9600);
  baseCol.r = 0;
  baseCol.g = 0;
  baseCol.b = 0;
  for (int i = 0; i < 4; i++){
    pinMode(19 - i, OUTPUT);
  }

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
      baseCol.r = 0;
      baseCol.g = 0;
      baseCol.b = 255;
    } 
    else if (mode == MODE_REDALERT){

      if(redAlertTimer + 800 < millis()){
        redToggle = !redToggle;
        redAlertTimer = millis();
      }
      baseCol.r = redToggle == true ? 255 : 0;
      baseCol.g = 0;
      baseCol.b = 0;
    } 
    else if (mode == MODE_WARP){
      baseCol.r = 0;
      baseCol.g = 0;
      baseCol.b = 255;
    } 
    else if (mode == MODE_BRIEF){
      baseCol.r = 255;
      baseCol.g = 255;
      baseCol.b = 255;
    }
  } 
  else {
    cabinState = false;
    baseCol.r = 0;
    baseCol.g = 0;
    baseCol.b = 0;
  }


  //effects
  if(effect == EFFECT_HEARTBEAT){
    if(heartBeatTimer + 400 < millis() && heartBeat){

      heartBeat = false;
      effect = EFFECT_NONE;
    } 
    else {

      baseCol.r = 0;
      baseCol.g = 0;
      baseCol.b = 0;
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


      // if(random(100) < 25){
      cabinState = flickerToggle;
      //}

    }

  }
  if(mode == MODE_WARP){
    //apply sin wave over the top of the base colour
    if(lastWarpUpdate + 25 < millis()){
      sinCount ++;
      sinCount %= 628;
      lastWarpUpdate = millis();
    }
    for(int i = 0 ; i < 10; i++){
      //left eds
      float sinMod = sin(sinCount / 3 + i);
      byte ledIndex = leftLeds[i];
      if(sinMod < 0){ 
        sinMod = 0; 
      }
      leds[ledIndex].r = (int)(baseCol.r * sinMod); 
      leds[ledIndex].g = (int)(baseCol.g * sinMod); 
      leds[ledIndex].b = (int)(baseCol.b * sinMod); 
      ledIndex = leftLeds[i+10];
      leds[ledIndex].r = (int)(baseCol.r * sinMod); 
      leds[ledIndex].g = (int)(baseCol.g * sinMod); 
      leds[ledIndex].b = (int)(baseCol.b * sinMod); 
      ledIndex = leftLeds[i+20];
      leds[ledIndex].r = (int)(baseCol.r * sinMod); 
      leds[ledIndex].g = (int)(baseCol.g * sinMod); 
      leds[ledIndex].b = (int)(baseCol.b * sinMod); 

    }
    for(int i = 0 ; i < 10; i++){
      //left eds
      float sinMod = sin(sinCount / 3 + i);
      byte ledIndex = rightLeds[i];
      if(sinMod < 0){ 
        sinMod = 0; 
      }
      leds[ledIndex].r = (int)(baseCol.r * sinMod); 
      leds[ledIndex].g = (int)(baseCol.g * sinMod); 
      leds[ledIndex].b = (int)(baseCol.b * sinMod); 
      ledIndex = rightLeds[i+10];
      leds[ledIndex].r = (int)(baseCol.r * sinMod); 
      leds[ledIndex].g = (int)(baseCol.g * sinMod); 
      leds[ledIndex].b = (int)(baseCol.b * sinMod); 
      ledIndex = rightLeds[i+20];
      leds[ledIndex].r = (int)(baseCol.r * sinMod); 
      leds[ledIndex].g = (int)(baseCol.g * sinMod); 
      leds[ledIndex].b = (int)(baseCol.b * sinMod);
    }

  } 
  else {
    for(int i = 0 ; i < NUM_LEDS; i++ ) {

      leds[i].r = baseCol.r; 
      leds[i].g = baseCol.g; 
      leds[i].b = baseCol.b; 
    }
  }
  cabinLightState (cabinState);


  if(Serial.available() > 0){
    char c = Serial.read();
    if(c == 'k'){        //kill
      state = LIGHTS_OFF;
    } 
    else if (c == 'o'){   //on
      state = LIGHTS_ON;
    } 
    else if (c == 'r'){  //red alert
      mode = MODE_REDALERT;
    } 
    else if (c == 'w'){
      mode = MODE_WARP;
    } 
    else if (c == 'd'){
      damageTimer = millis();
      flickerTimeOut = millis();

      damageOn = true;
      effect = EFFECT_DAMAGE;
    } 
    else if(c == 'h'){
      effect = EFFECT_HEARTBEAT;
      heartBeatTimer = millis();
      heartBeat = true;

    } 
    else if (c == 'b'){
      //reset
      mode = MODE_BRIEF;
    }
    else if (c == 'R'){
      //reset
      mode = MODE_IDLE;
      effect = EFFECT_NONE;
      state = LIGHTS_OFF;
    } 
    else if (c == 'i'){
      //idle
      mode = MODE_IDLE;
    }
  }

  //redAlert();
  FastSPI_LED.show();
  //delay(10);


}

void on(){
  for(int i = 0 ; i < NUM_LEDS; i++ ) {

    leds[i].r = 0; 
    leds[i].g = 0; 
    leds[i].b = 255; 
  }
  cabinLightState(true);

}

void damage(){
  unsigned char c = random(255);
  for(int i = 0 ; i < NUM_LEDS; i++ ) {

    leds[i].r = 0; 
    leds[i].g = 0; 
    leds[i].b = c; 
  }
  if(random(10) < 5){
    cabinLightState(false);
  } 
  else {
    cabinLightState(true);
  }

}

void kill(){
  for(int i = 0 ; i < NUM_LEDS; i++ ) {

    leds[i].r = 0; 
    leds[i].g = 0; 
    leds[i].b = 0; 
  }
  cabinLightState(false);

}


void redAlert(){
  unsigned char red = 0;
  if(redAlertTimer + 1500 < millis()){
    redAlertTimer = millis();
    redToggle = !redToggle;
    if (redToggle){
      red = 255;
    } 
    else {
      red = 0;
    }
    for(int i = 0 ; i < NUM_LEDS; i++ ) {

      leds[i].r = red; 
      leds[i].g = 0; 
      leds[i].b = 0; 
    }
  }
  cabinLightState(true);


}








