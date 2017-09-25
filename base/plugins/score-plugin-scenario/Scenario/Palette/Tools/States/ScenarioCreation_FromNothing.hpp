#pragma once
#include "ScenarioCreationState.hpp"
#include <Scenario/Document/Synchronization/SynchronizationModel.hpp>

#include <Scenario/Commands/Scenario/Displacement/MoveNewEvent.hpp>
#include <Scenario/Commands/Scenario/Displacement/MoveNewState.hpp>

#include <Scenario/Commands/Scenario/Creations/CreateEvent_State.hpp>
#include <Scenario/Commands/Scenario/Creations/CreateState.hpp>

#include <Scenario/Palette/Transitions/AnythingTransitions.hpp>
#include <Scenario/Palette/Transitions/IntervalTransitions.hpp>
#include <Scenario/Palette/Transitions/EventTransitions.hpp>
#include <Scenario/Palette/Transitions/NothingTransitions.hpp>
#include <Scenario/Palette/Transitions/StateTransitions.hpp>
#include <Scenario/Palette/Transitions/SynchronizationTransitions.hpp>

#include <QFinalState>

namespace Scenario
{
template <typename Scenario_T, typename ToolPalette_T>
class Creation_FromNothing final
    : public CreationState<Scenario_T, ToolPalette_T>
{
public:
  Creation_FromNothing(
      const ToolPalette_T& stateMachine,
      const Scenario_T& scenarioPath,
      const score::CommandStackFacade& stack,
      QState* parent)
      : CreationState<Scenario_T, ToolPalette_T>{stateMachine, stack,
                                                 scenarioPath, parent}
  {
    this->setObjectName("ScenarioCreation_FromNothing");
    using namespace Scenario::Command;
    auto finalState = new QFinalState{this};
    QObject::connect(
        finalState, &QState::entered, [&]() { this->clearCreatedIds(); });

    auto mainState = new QState{this};
    mainState->setObjectName("Main state");
    {
      auto pressed = new QState{mainState};
      auto released = new QState{mainState};
      auto move_nothing = new StrongQState<MoveOnNothing>{mainState};
      auto move_state = new StrongQState<MoveOnState>{mainState};
      auto move_event = new StrongQState<MoveOnEvent>{mainState};
      auto move_synchronization = new StrongQState<MoveOnSynchronization>{mainState};

      pressed->setObjectName("Pressed");
      released->setObjectName("Released");
      move_nothing->setObjectName("Move on Nothing");
      move_state->setObjectName("Move on State");
      move_event->setObjectName("Move on Event");
      move_synchronization->setObjectName("Move on Synchronization");

      // General setup
      mainState->setInitialState(pressed);
      released->addTransition(finalState);

      // Release
      score::make_transition<ReleaseOnAnything_Transition>(
          mainState, released);

      // Pressed -> ...
      score::make_transition<MoveOnNothing_Transition<Scenario_T>>(
          pressed, move_nothing, *this);

      /// MoveOnNothing -> ...
      // MoveOnNothing -> MoveOnNothing.
      score::make_transition<MoveOnNothing_Transition<Scenario_T>>(
          move_nothing, move_nothing, *this);

      // MoveOnNothing -> MoveOnState.
      this->add_transition(move_nothing, move_state, [&]() {
        this->rollback();
        createToState();
      });

      // MoveOnNothing -> MoveOnEvent.
      this->add_transition(move_nothing, move_event, [&]() {
        this->rollback();
        createToEvent();
      });

      // MoveOnNothing -> MoveOnSynchronization
      this->add_transition(move_nothing, move_synchronization, [&]() {
        this->rollback();
        createToSynchronization();
      });

      /// MoveOnState -> ...
      // MoveOnState -> MoveOnNothing
      this->add_transition(move_state, move_nothing, [&]() {
        this->rollback();
        createToNothing();
      });

      // MoveOnState -> MoveOnState
      // We don't do anything, the interval should not move.

      // MoveOnState -> MoveOnEvent
      this->add_transition(move_state, move_event, [&]() {
        this->rollback();
        createToEvent();
      });

      // MoveOnState -> MoveOnSynchronization
      this->add_transition(move_state, move_synchronization, [&]() {
        this->rollback();
        createToSynchronization();
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
        createToState();
      });

      // MoveOnEvent -> MoveOnEvent
      score::make_transition<MoveOnEvent_Transition<Scenario_T>>(
          move_event, move_event, *this);

      // MoveOnEvent -> MoveOnSynchronization
      this->add_transition(move_event, move_synchronization, [&]() {
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
        createToState();
      });

      // MoveOnSynchronization -> MoveOnEvent
      this->add_transition(move_synchronization, move_event, [&]() {
        this->rollback();
        createToEvent();
      });

      // MoveOnSynchronization -> MoveOnSynchronization
      score::make_transition<MoveOnSynchronization_Transition<Scenario_T>>(
          move_synchronization, move_synchronization, *this);

      // What happens in each state.
      QObject::connect(pressed, &QState::entered, [&]() {
        this->m_clickedPoint = this->currentPoint;
        this->clickedEvent = this->m_parentSM.model().startEvent().id();
        createToNothing();
      });

      QObject::connect(move_nothing, &QState::entered, [&]() {
        if (this->createdIntervals.empty() || this->createdEvents.empty())
        {
          this->rollback();
          return;
        }

        if (this->clickedEvent != this->m_parentSM.model().startEvent().id()
         && this->currentPoint.date <= this->m_clickedPoint.date)
        {
          this->currentPoint.date
              = this->m_clickedPoint.date + TimeVal::fromMsecs(10);
        }
        else if (this->clickedEvent == this->m_parentSM.model().startEvent().id()
              && this->currentPoint.date <= TimeVal::fromMsecs(10))
        {
          this->currentPoint.date = TimeVal::fromMsecs(10);
        }

        this->m_dispatcher.template submitCommand<MoveNewEvent>(
            this->m_scenario,
            this->createdIntervals.last(),
            this->createdEvents.last(),
            this->currentPoint.date,
            this->currentPoint.y,
            stateMachine.editionSettings().sequence());
      });

      QObject::connect(move_synchronization, &QState::entered, [&]() {
        if (this->createdStates.empty())
        {
          this->rollback();
          return;
        }

        if (this->currentPoint.date <= this->m_clickedPoint.date)
        {
          return;
        }

        this->m_dispatcher.template submitCommand<MoveNewState>(
            this->m_scenario,
            this->createdStates.last(),
            this->currentPoint.y);
      });

      QObject::connect(move_event, &QState::entered, [&]() {
        if (this->createdStates.empty())
        {
          this->rollback();
          return;
        }

        if (this->currentPoint.date <= this->m_clickedPoint.date)
        {
          return;
        }

        this->m_dispatcher.template submitCommand<MoveNewState>(
            this->m_scenario,
            this->createdStates.last(),
            this->currentPoint.y);
      });

      QObject::connect(
          released, &QState::entered, this, &Creation_FromNothing::commit);
    }

    auto rollbackState = new QState{this};
    rollbackState->setObjectName("Rollback");
    score::make_transition<score::Cancel_Transition>(
        mainState, rollbackState);
    rollbackState->addTransition(finalState);

    QObject::connect(
        rollbackState, &QState::entered, this,
        &Creation_FromNothing::rollback);

    this->setInitialState(mainState);
  }

private:
  void createInitialState()
  {
    if (this->clickedEvent)
    {
      auto cmd = new Scenario::Command::CreateState{
          this->m_scenario, *this->clickedEvent, this->currentPoint.y};
      this->m_dispatcher.submitCommand(cmd);

      this->createdStates.append(cmd->createdState());
    }
  }

  void createToNothing()
  {
    createInitialState();
    this->createToNothing_base(this->createdStates.first());
  }
  void createToState()
  {
    SCORE_ASSERT(bool(this->hoveredState));

    const auto& state
        = this->m_parentSM.model().states.at(*this->hoveredState);
    if (!bool(state.previousInterval()))
    {
      createInitialState();
      this->createToState_base(this->createdStates.first());
    }
  }
  void createToEvent()
  {
    if (this->hoveredEvent != this->clickedEvent)
    {
      createInitialState();
      this->createToEvent_base(this->createdStates.first());
    }
  }
  void createToSynchronization()
  {
    createInitialState();
    this->createToSynchronization_base(this->createdStates.first());
  }
};
}
