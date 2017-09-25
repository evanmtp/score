// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <Scenario/Process/ScenarioModel.hpp>
#include <score/tools/std/Optional.hpp>

#include "StandardCreationPolicy.hpp"
#include <Process/TimeValue.hpp>
#include <Scenario/Document/Interval/IntervalDurations.hpp>
#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <Scenario/Document/Event/EventModel.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <Scenario/Document/Synchronization/SynchronizationModel.hpp>
#include <Scenario/Document/VerticalExtent.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>
#include <Scenario/Process/Algorithms/ProcessPolicy.hpp>
#include <score/document/DocumentContext.hpp>
#include <score/model/EntityMap.hpp>
#include <score/model/Identifier.hpp>

namespace Scenario
{
void ScenarioCreate<CommentBlockModel>::undo(
    const Id<CommentBlockModel>& id, Scenario::ProcessModel& s)
{
  s.comments.remove(id);
}

CommentBlockModel& ScenarioCreate<CommentBlockModel>::redo(
    const Id<CommentBlockModel>& id,
    const TimeVal& date,
    double y,
    Scenario::ProcessModel& s)
{
  auto comment = new CommentBlockModel{id, date, y, &s};
  s.comments.add(comment);

  return *comment;
}

void ScenarioCreate<SynchronizationModel>::undo(
    const Id<SynchronizationModel>& id, Scenario::ProcessModel& s)
{
  s.synchronizations.remove(id);
}

SynchronizationModel& ScenarioCreate<SynchronizationModel>::redo(
    const Id<SynchronizationModel>& id,
    const VerticalExtent& extent,
    const TimeVal& date,
    Scenario::ProcessModel& s)
{
  auto synchronization = new SynchronizationModel{id, extent, date, &s};
  s.synchronizations.add(synchronization);

  return *synchronization;
}

void ScenarioCreate<EventModel>::undo(
    const Id<EventModel>& id, Scenario::ProcessModel& s)
{
  auto& ev = s.event(id);
  s.synchronization(ev.synchronization()).removeEvent(id);
  s.events.remove(&ev);
}

EventModel& ScenarioCreate<EventModel>::redo(
    const Id<EventModel>& id,
    SynchronizationModel& synchronization,
    const VerticalExtent& extent,
    Scenario::ProcessModel& s)
{
  auto ev = new EventModel{id, synchronization.id(), extent, synchronization.date(), &s};

  s.events.add(ev);
  synchronization.addEvent(id);

  return *ev;
}

void ScenarioCreate<StateModel>::undo(
    const Id<StateModel>& id, Scenario::ProcessModel& s)
{
  auto& state = s.state(id);
  auto& ev = s.event(state.eventId());

  ev.removeState(id);

  s.states.remove(&state);
}

StateModel& ScenarioCreate<StateModel>::redo(
    const Id<StateModel>& id,
    EventModel& ev,
    double y,
    Scenario::ProcessModel& s)
{
  auto& stack = score::IDocument::documentContext(s).commandStack;
  auto state = new StateModel{id, ev.id(), y, stack, &s};

  s.states.add(state);
  ev.addState(state->id());

  return *state;
}

void ScenarioCreate<IntervalModel>::undo(
    const Id<IntervalModel>& id, Scenario::ProcessModel& s)
{
  auto& cst = s.intervals.at(id);

  SetNoNextInterval(startState(cst, s));
  SetNoPreviousInterval(endState(cst, s));

  s.intervals.remove(&cst);
}

IntervalModel& ScenarioCreate<IntervalModel>::redo(
    const Id<IntervalModel>& id,
    StateModel& sst,
    StateModel& est,
    double ypos,
    Scenario::ProcessModel& s)
{
  auto interval = new IntervalModel{id, ypos, &s};

  interval->setStartState(sst.id());
  interval->setEndState(est.id());

  s.intervals.add(interval);

  SetNextInterval(sst, *interval);
  SetPreviousInterval(est, *interval);

  const auto& sev = s.event(sst.eventId());
  const auto& eev = s.event(est.eventId());
  const auto& tn = s.synchronization(eev.synchronization());

  IntervalDurations::Algorithms::changeAllDurations(
      *interval, eev.date() - sev.date());
  interval->setStartDate(sev.date());

  if (tn.active())
  {
    interval->duration.setRigid(false);
    const auto& dur = interval->duration.defaultDuration();
    interval->duration.setMinDuration(
        TimeVal::fromMsecs(0.8 * dur.msec()));
    interval->duration.setMaxDuration(
        TimeVal::fromMsecs(1.2 * dur.msec()));
  }

  return *interval;
}
}
