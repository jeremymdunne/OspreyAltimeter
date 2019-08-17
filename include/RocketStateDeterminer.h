/*
RocketStateDeterminer

TODO: Better name!

This handles the estimation of all states of the rocket. Attitude models, kinematic, etc. Also determines phase changes,
i.e. launch, coast, apogee, etc.


Models should be individual classes that are updated and, if accepted, placed in the global rocket state

*/


#ifndef _ROCKET_STATE_DETERMINER_H_
#define _ROCKET_STATE_DETERMINER_H_

#define PHASE_CHANGE_DETECTED 1

class RocketStateDeterminer{
public:

  int init();

  int update(RocketState *newState);

private:
  RocketState previous_state;


};


#endif
