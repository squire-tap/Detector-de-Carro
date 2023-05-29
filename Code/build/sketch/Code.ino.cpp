#line 1 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\Code.ino"
#include <protothreads.h>
#include <pt-sem.h>
#include <pt-sleep.h>
#include <pt.h>

#include <Arduino.h>
#include "sensors.h"

#define TIME_MAX 1 // Tempo máximo de espera do carro na cancela
#define TEMPO_MAX_STATE_C 1
#define TEMPO_MAX_STATE_V 1
#define TEMPO_MAX_STATE_J 1
#define TEMPO_MAX_STATE_W 1

#define NUM_SAMPLE 30
int ledPin = 13; // select the pin for the LED

float dist_t1;
float dist_t2;
bool objectDetected = false;

static bool state_c = false;
static bool state_v = false;
static bool state_j = false;
static bool state_w = false;

static struct pt pt_av;
static int average_thread_run = 1;

#line 30 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\Code.ino"
void setup();
#line 38 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\Code.ino"
static int averageThread(struct pt *pt);
#line 68 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\Code.ino"
void CarStopAlarm();
#line 98 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\Code.ino"
void isProximityState();
#line 152 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\Code.ino"
void loop();
#line 5 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
bool is_s1_active(void);
#line 10 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
bool is_s2_active(void);
#line 15 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
bool is_s1_s2_active(void);
#line 20 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
bool is_state_c(void);
#line 25 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
bool is_state_v(void);
#line 29 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
bool is_state_j(void);
#line 34 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
bool is_state_w(void);
#line 39 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
bool is_idle(void);
#line 44 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
float get_dist_s1();
#line 50 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
float get_dist_s2();
#line 30 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\Code.ino"
void setup()
{
  Serial.begin(115200); // Inicializa a comunicação serial com taxa de 9600 bps
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  PT_INIT(&pt_av);
}

static int averageThread(struct pt *pt)
{
  PT_BEGIN(pt);

  while(1)
  {
    //Waits until some part of program set's threadStop Flag
    PT_WAIT_UNTIL(pt, average_thread_run != 0);

    Serial.print("average thead perfoming\n");

    float dist_t1_aux = 0;
    float dist_t2_aux = 0;

    for(int count = 0; count < NUM_SAMPLE ; count++)
    {
      dist_t1_aux = dist_t1_aux + get_dist_s1();
      dist_t2_aux = dist_t2_aux + get_dist_s2();
      delay(20); //This delay is extreamly necessary for accurate measure
    }

    dist_t1 = dist_t1_aux/NUM_SAMPLE;
    dist_t2 = dist_t2_aux/NUM_SAMPLE;
    
    //Reset the flag
    average_thread_run = 0;
  }
  PT_END(pt);
}

void CarStopAlarm()
{
  static unsigned long timeStart;
  static unsigned long timeStop;

  if ((is_s1_active()) && (is_s2_active()))
  {
    if (!objectDetected)
    {
      objectDetected = true;
      timeStart = millis();
    }
    else
    {
      timeStop = millis() - timeStart;

      if (timeStop >= 1000 * TIME_MAX) // Car stoped too long verification
      {
        digitalWrite(ledPin, HIGH);
      }
    }
  }
  else
  {
    digitalWrite(ledPin, LOW);
    timeStop = 0;
    objectDetected = false;
  }
}

void isProximityState()
{

  static unsigned long start_state_c, start_state_v, start_state_j, start_state_w;
  volatile unsigned long time_state_c, time_state_v, time_state_j, time_state_w;


  if (!state_c && is_state_c())
  {
    state_c = true;
    start_state_c = millis();
  }
  else if (!state_v && is_state_v())
  {
    state_c = false;
    time_state_c = start_state_c - millis();
    if (time_state_c <= TEMPO_MAX_STATE_C * 1000)
    {
      state_v = true;
      start_state_v = millis();
    }
  }
  else if (!state_j && is_state_j())
  {
    state_v = false;
    time_state_v = start_state_v - millis();
    if (time_state_v <= TEMPO_MAX_STATE_V * 1000)
    {
      state_j = true;
      start_state_j = millis();
    }
  }
  else if (!state_w && is_state_w())
  {
    state_j = false;
    time_state_j = start_state_j - millis();
    if (time_state_j <= TEMPO_MAX_STATE_J * 1000)
    {
      state_w = true;
      start_state_w = millis();
    }
  }
  else if (is_idle())
  {
    time_state_w = start_state_w - millis();
    if (time_state_w <= TEMPO_MAX_STATE_W * 1000)
    {
    }
  }
  else
  {
  }
}

void loop()
{
  averageThread(&pt_av);
  

  // dist_t1 = get_dist_s1();
  // dist_t2 = get_dist_s2();  

  CarStopAlarm();
  isProximityState();

  Serial.print("S1: ");
  Serial.println(dist_t1);
  Serial.print("S2: ");
  Serial.println(dist_t2); 

  //Release thread of average to run
  average_thread_run = 1;

}
#line 1 "C:\\Users\\Hardware 1\\Desktop\\Detector-de-Carro\\Code\\sensors.ino"
#include "sensors.h"

extern float dist_t1, dist_t2;

inline bool is_s1_active(void)
{
    return (dist_t1 < DIST_MAX);
}

inline bool is_s2_active(void)
{
    return (dist_t2 < DIST_MAX);
}

inline bool is_s1_s2_active(void)
{
    return (is_s1_active() && is_s2_active());
}

inline bool is_state_c(void)
{
    return (is_s1_active() && !is_s2_active());
}

inline bool is_state_v(void)
{
    return (is_s1_s2_active());
}
inline bool is_state_j(void)
{
    return (!is_s1_active() && is_s2_active());
}

inline bool is_state_w(void)
{
    return (is_s1_s2_active());
}

inline bool is_idle(void)
{
    return !(is_s1_s2_active());
}

float get_dist_s1()
{
    float sensity_t1 = analogRead(PIN_S1);
    return sensity_t1 * MAX_RANG / ADC_SOLUTION;
}

float get_dist_s2()
{
    float sensity_t2 = analogRead(PIN_S2);
    return sensity_t2 * MAX_RANG / ADC_SOLUTION;
}
