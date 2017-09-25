#pragma once
#include "ScenarioCreationState.hpp"

#include <Scenario/Document/Synchronization/SynchronizationModel.hpp>

#include <Scenario/Commands/Scenario/Displacement/MoveEventMeta.hpp>
#include <Scenario/Commands/Scenario/Displacement/MoveNewEvent.hpp>

#include <Scenario/Commands/Scenario/Creations/CreateInterval.hpp>

#include <Scenario/Palette/Transitions/AnythingTransitions.hpp>
#include <Scenario/Palette/Transitions/IntervalTransitions.hpp>
#include <Scenario/Palette/Transitions/EventTransitions.hpp>
#include <Scenario/Palette/Transitions/NothingTransitions.hpp>
#include <Scenario/Palette/Transitions/StateTransitions.hpp>
#include <Scenario/Palette/Transitions/SynchronizationTransitions.hpp>

#include <QFinalState>
#include <Scenario/Palette/Tools/ScenarioRollbackStrategy.hpp>

namespace Scenario
{
template <typename Scenario_T, typename ToolPalette_T>
class Creation_FromSynchronization final
    : public CreationState<Scenario_T, ToolPalette_T>
{
public:
  Creation_FromSynchronization(
      const ToolPalette_T& stateMachine,
      const Scenario_T& scenarioPath,
      const score::CommandStackFacade& stack,
      QState* parent)
      : CreationState<Scenario_T, ToolPalette_T>{
            stateMachine, stack, std::move(scenarioPath), parent}
  {
    using namespace Scenario::Command;
    auto finalState = new QFinalState{this};
    QObject::connect(
        finalState, &QState::entered, [&]() { this->clearCreatedIds(); });

    auto mainState = new QState{this};
    {
      auto pressed = new QState{mainState};
      auto released = new QState{mainState};
      auto move_nothing = new StrongQState<MoveOnNothing>{mainState};
      auto move_state = new StrongQState<MoveOnState>{mainState};
      auto move_event = new StrongQState<MoveOnEvent>{mainState};
      auto move_synchronization = new StrongQState<MoveOnSynchronization>{mainState};

      // General setup
      mainState->setInitialState(pressed);
      released->addTransition(finalState);

      // Release
      score::make_transition<ReleaseOnAnything_Transition>(
          mainState, released);

      // Pressed -> ...
      auto t_pressed_moving_nothing
          = score::make_transition<MoveOnNothing_Transition<Scenario_T>>(
              pressed, move_nothing, *this);

      QObject::connect(
          t_pressed_moving_nothing, &QAbstractTransition::triggered, [&]() {
            this->rollback();
            createToNothing();
          });

      /// MoveOnNothing -> ...
      // MoveOnNothing -> MoveOnNothing.
      score::make_transition<MoveOnNothing_Transition<Scenario_T>>(
          move_nothing, move_nothing, *this);

      // MoveOnNothing -> MoveOnState.
      this->add_transition(move_nothing, move_state, [&]() {
        this->rollback();
        SCORE_TODO;
      });

      // MoveOnNothing -> MoveOnEvent.
      this->add_transition(move_nothing, move_event, [&]() {
        if (this->hoveredEvent
            && this->createdEvents.contains(*this->hoveredEvent))
        {
          return;
        }
        this->rollback();

        createToEvent();
      });

      // MoveOnNothing -> MoveOnSynchronization
      this->add_transition(move_nothing, move_synchronization, [&]() {
        if (this->hoveredSynchronization
            && this->createdSynchronizations.contains(*this->hoveredSynchronization))
        {
          return;
        }
        this->rollback();
        createToSynchronization();
      });

      /// MoveOnState -> ...
      // MoveOnState -> MoveOnNothing
      this->add_transition(move_state, move_nothing, [&]() {
        this->rollback();
        SCORE_TODO;
      });

      // MoveOnState -> MoveOnState
      // We don't do anything, the interval should not move.

      // MoveOnState -> MoveOnEvent
      this->add_transition(move_state, move_event, [&]() {
        this->rollback();
        SCORE_TODO;
      });

      // MoveOnState -> MoveOnSynchronization
      this->add_transition(move_state, move_synchronization, [&]() {
        this->rollback();
        SCORE_TODO;
      });

      /// MoveOnEvent -> ...
      // MoveOnEvent -> MoveOnNothing
      this->add_transition(move_event, move_nothing, [&]() {
        this->rollback();
        createToNothing();
      });

      // MoveOnEvent -> MoveOnState
      this->add_transition(move_event, move_state, [&]() {
        this->rollback();
        SCORE_TODO;
      });

      // MoveOnEvent -> MoveOnEvent
      score::make_transition<MoveOnEvent_Transition<Scenario_T>>(
          move_event, move_event, *this);

      // MoveOnEvent -> MoveOnSynchronization
      this->add_transition(move_event, move_synchronization, [&]() {

        if (this->hoveredSynchronization
            && this->createdSynchronizations.contains(*this->hoveredSynchronization))
        {
          return;
        }
        this->rollback();
        createToSynchronization();
      });

      /// MoveOnSynchronization -> ...
      // MoveOnSynchronization -> MoveOnNothing
      this->add_transition(move_synchronization, move_nothing, [&]() {
        this->rollback();
        createToNothing();
      });

      // MoveOnSynchronization -> MoveOnState
      this->add_transition(move_synchronization, move_state, [&]() {
        this->rollback();
        SCORE_TODO;
      });

      // MoveOnSynchronization -> MoveOnEvent
      this->add_transition(move_synchronization, move_event, [&]() {
        if (this->hoveredEvent
            && this->createdEvents.contains(*this->hoveredEvent))
        {
          this->rollback();
          return;
        }
        this->rollback();
        createToEvent();
      });

      // MoveOnSynchronization -> MoveOnSynchronization
      score::make_transition<MoveOnSynchronization_Transition<Scenario_T>>(
          move_synchronization, move_synchronization, *this);

      // What happens in each state.
      QObject::connect(pressed, &QState::entered, [&]() {
        this->m_clickedPoint = this->currentPoint;
        createInitialEventAndState();
      });

      QObject::connect(move_nothing, &QState::entered, [&]() {
        if (this->createdEvents.empty() || this->createdIntervals.empty())
        {
          this->rollback();
          return;
        }

        if (this->currentPoint.date <= this->m_clickedPoint.date)
        {
          this->currentPoint.date
              = this->m_clickedPoint.date + TimeVal::fromMsecs(10);
          ;
        }

        // Move the synchronization
        this->m_dispatcher.template submitCommand<MoveNewEvent>(
            this->m_scenario,
            this->createdIntervals.last(),
            this->createdEvents.last(),
            this->currentPoint.date,
            this->currentPoint.y,
            stateMachine.editionSettings().sequence());
      });

      QObject::connect(move_synchronization, &QState::entered, [&]() {
        if (this->createdEvents.empty())
        {
          this->rollback();
          return;
        }

        if (this->currentPoint.date <= this->m_clickedPoint.date)
        {
          return;
        }

        this->m_dispatcher.template submitCommand<MoveEventMeta>(
            this->m_scenario,
            this->createdEvents.last(),
            TimeVal::zero(),
            0.,
            stateMachine.editionSettings().expandMode(),
            LockMode::Free);
      });

      QObject::connect(
          released, &QState::entered, this, &Creation_FromSynchronization::commit);
    }

    auto rollbackState = new QState{this};
    score::make_transition<score::Cancel_Transition>(
        mainState, rollbackState);
    rollbackState->addTransition(finalState);
    QObject::connect(
        rollbackState, &QState::entered, this,
        &Creation_FromSynchronization::rollback);

    this->setInitialState(mainState);
  }

private:
  void createInitialEventAndState()
  {
    if (this->clickedSynchronization)
    {
      auto cmd = new Command::CreateEvent_State{
          this->m_scenario, *this->clickedSynchronization, this->currentPoint.y};
      this->m_dispatcher.submitCommand(cmd);

      this->createdStates.append(cmd->createdState());
      this->createdEvents.append(cmd->createdEvent());
    }
  }

  void createToNothing()
  {
    createInitialEventAndState();
    this->createToNothing_base(this->createdStates.first());
  }
  void createToState()
  {
    createInitialEventAndState();
    this->createToState_base(this->createdStates.first());
  }
  void createToEvent()
  {
    createInitialEventAndState();
    this->createToEvent_base(this->createdStates.first());
  }
  void createToSynchronization()
  {
    // TODO "if hoveredSynchronization != clickedSynchronization"
    createInitialEventAndState();
    this->createToSynchronization_base(this->createdStates.first());
  }
};
}
