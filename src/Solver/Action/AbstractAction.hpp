#ifndef PACE2026_ABSTRACT_ACTION_HPP
#define PACE2026_ABSTRACT_ACTION_HPP

namespace solver
{

/// \brief This is a base class for actions.
/// Actions are atomic operations on a forest.
/// They are intended to be both doable and undoable.
class AbstractAction
{
  public:
    virtual ~AbstractAction() = default;

    /// \brief do the action
    virtual void doAction() = 0;

    /// \brief undo the action
    virtual void undoAction() = 0;
};

}  //namespace solver

#endif  //PACE2026_ABSTRACT_ACTION_HPP
