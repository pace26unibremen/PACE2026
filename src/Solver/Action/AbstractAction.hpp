#ifndef PACE2026_ABSTRACT_ACTION_HPP
#define PACE2026_ABSTRACT_ACTION_HPP

namespace solver
{

class AbstractAction
{
  public:
    virtual void doAction() = 0;
    virtual void undoAction() = 0;
};

}  //namespace solver

#endif  //PACE2026_ABSTRACT_ACTION_HPP
