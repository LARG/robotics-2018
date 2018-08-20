#include <common/StateMachine.h>

#include <algorithm>
#include <iostream>

StateMachine::StateMachine() {
  StateMachine("unnamed");
}

StateMachine::StateMachine(std::string name) {
  state_machine_name_ = name;
  //current_state_= states_.begin();
}

std::string StateMachine::currentStateName() {
  if (states_.size() > 0) 
    return (*current_state_).first;
  else
    return "Undefined";
}

int StateMachine::currentStateID() {
  if (states_.size() > 0) 
    return (*current_state_).second;
  else
    return -1;
}

bool StateMachine::transition(std::string name) {
  if (!transitionExists((*current_state_).first,name)) {
    std::cout << "Error - Failed to transition from " << (*current_state_).first << " -> " << name << " because transition does not exist" << std::endl;
    return false;
  }
  current_state_=findState(name);
  return true;
}

bool StateMachine::forceTransition(std::string name) {
  std::vector<StatePair>::iterator result = findState(name);
  if (result==states_.end()) {
    std::cout << "Error - Failed to transition from " << (*current_state_).first << " -> " << name << " because " << name << " does not exist" << std::endl;
    return false;
  }
  current_state_ = result;
  return true;
}

bool StateMachine::addState(std::string name, int ID) {
  StatePair p(name,ID);
  if (!stateExists(name)) states_.push_back(p);
  current_state_ =  states_.begin();
  return true;
}

bool StateMachine::addTransition(std::string state1, std::string state2) {
  if (!stateExists(state1)) {
    std::cout << "Error - Failed to create transition " << state1 << " -> " << state2 << " because " << state1 << " does not exist" << std::endl;
    return false;
  }
  if (!stateExists(state2)) {
    std::cout << "Error - Failed to create transition " << state1 << " -> " << state2 << " because " << state2 << " does not exist" << std::endl;
    return false;
  }
  std::pair<std::string, std::string> new_pair(state1,state2);
  transitions_.push_back(new_pair);
  return true;
}

bool StateMachine::printStates() {
  std::cout << "States in " << state_machine_name_ << ":" << std::endl;
  std::vector<StatePair>::iterator it;
  for (it = states_.begin(); it!=states_.end(); ++it) {
    std::cout << "  " << (*it).second << ":" << (*it).first << std::endl;
  }
  return true;
}


bool StateMachine::printTransitions() {
  std::cout << "Transitions in " << state_machine_name_ << ":" << std::endl;
  std::vector< std::pair<std::string,std::string> > ::iterator it;
  for (it = transitions_.begin(); it!=transitions_.end(); ++it) {
    std::cout << "  " << (*it).first << " -> " << (*it).second << std::endl;
  }
  return true;
}

std::vector<StatePair>::iterator StateMachine::findState(std::string name) {
  std::vector<StatePair>::iterator it;
  for (it = states_.begin(); it!=states_.end(); ++it) {
    if ((*it).first == name) return it;
  }
  return states_.end();
}

bool StateMachine::stateExists(std::string name) {
  if (states_.empty())  return false;
  std::vector<StatePair>::iterator result = findState(name);
  if (result==states_.end()) return false;
  else return true;
}


bool StateMachine::transitionExists(std::string state1, std::string state2) {
  if (transitions_.empty())  return false;
  std::pair<std::string, std::string> test_pair(state1,state2);
  std::vector< std::pair<std::string,std::string> > ::iterator result = find(transitions_.begin(), transitions_.end(), test_pair);
  if (result==transitions_.end()) return false;
  else return true;
}
