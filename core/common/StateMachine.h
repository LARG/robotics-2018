#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <vector>
#include <map>
#include <string>

typedef std::pair<std::string, int> StatePair;
 
class StateMachine {
 public:
  StateMachine();
  StateMachine(std::string name);

  std::string currentStateName();
  int currentStateID();

  bool transition(std::string);
  bool forceTransition(std::string);

  bool addState(std::string name, int ID);
  bool addTransition(std::string state1, std::string state2);
  
  bool printStates();
  bool printTransitions();

 private:
  bool stateExists(std::string name);
  bool transitionExists(std::string state1, std::string state2);

  std::vector<StatePair>::iterator findState(std::string name);

  std::vector<StatePair> states_;
  std::vector< std::pair<std::string,std::string> > transitions_;

  std::vector<StatePair>::iterator current_state_;

  std::string state_machine_name_;
};

#endif
