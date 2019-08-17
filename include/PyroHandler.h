/*
PyroHandler

helper class for determining pyro programming and execution. There will likely be ~4 of these on the flight controller

Standard flags are used to determine conditions when to deploy

Flags are shared with the Standard Rocket Phases

*/

#ifndef _PYRO_HANDLER_H_
#define _PYRO_HANDLER_H_

#include <RocketState.h>

class PyroHandler{
public:

  struct PyroOptions{
    RocketFlightPhase mainFlag = APOGEE_PHASE;
    int standard_delay = 0; //ms
    int firing_length = 1000;  //ms
    float min_altitude = 20; //m
    float max_velocity = 20; //m/s
    float min_velocity = 10; //m/s

  };

  int init(PyroOptions options);

  int update(RocketState *state)

  int fire();

  int safe();


private:
  int pin = -1;
  PyroOptions options;
  bool phase_reached = False;
  ulong long phase_reached_timestamp = 0;



};


#endif
