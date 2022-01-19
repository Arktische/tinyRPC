//
// Created by tyx on 1/14/22.
//

#ifndef TINYRPC_FSM_HPP
#define TINYRPC_FSM_HPP
#include <vector>
#include <initializer_list>
#include <functional>
template<class InputT,class StateT>
class IFiniteStateMachie {
public:
  virtual void addState(std::initializer_list<InputT> state) = 0;
  virtual void addTransition(StateT src, StateT dst, std::function<void(std::vector<InputT>)> event) = 0;
  virtual StateT output(std::vector<InputT> input) = 0;
private:
  std::vector<StateT> state;
  virtual void solve() = 0;
};

template<class InputT,class StateT>
class CommonFSM : public IFiniteStateMachie<InputT,StateT> {

};
#endif // TINYRPC_FSM_HPP
