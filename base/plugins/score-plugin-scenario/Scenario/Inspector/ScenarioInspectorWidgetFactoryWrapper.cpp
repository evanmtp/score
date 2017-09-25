// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "ScenarioInspectorWidgetFactoryWrapper.hpp"

#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <Scenario/Document/Event/EventModel.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <Scenario/Document/Synchronization/SynchronizationModel.hpp>
#include <Scenario/Process/ScenarioInterface.hpp>

#include <Scenario/Inspector/Interval/IntervalInspectorFactory.hpp>
#include <Scenario/Inspector/Summary/SummaryInspectorWidget.hpp>
#include <Scenario/Inspector/Synchronization/SynchronizationInspectorWidget.hpp>
#include <Scenario/Inspector/Event/EventInspectorWidget.hpp>
#include <Scenario/Inspector/State/StateInspectorWidget.hpp>

namespace Scenario
{
QWidget*
ScenarioInspectorWidgetFactoryWrapper::make(
    const QList<const QObject*>& sourceElements,
    const score::DocumentContext& doc,
    QWidget* parent) const
{
  std::set<const IntervalModel*> intervals;
  std::set<const SynchronizationModel*> synchronizations;
  std::set<const EventModel*> events;
  std::set<const StateModel*> states;

  if (sourceElements.empty())
    return nullptr;

  auto scenar = dynamic_cast<ScenarioInterface*>(sourceElements[0]->parent());
  auto abstr = safe_cast<const IdentifiedObjectAbstract*>(sourceElements[0]);
  SCORE_ASSERT(scenar); // because else, matches should have return false

  for (auto elt : sourceElements)
  {
    if (auto st = dynamic_cast<const StateModel*>(elt))
    {
      if (auto ev = scenar->findEvent(st->eventId()))
      {
        auto tn = scenar->findSynchronization(ev->synchronization());
        if (!tn)
          continue;
        states.insert(st);
        events.insert(ev);
        synchronizations.insert(tn);
      }
    }
    else if (auto ev = dynamic_cast<const EventModel*>(elt))
    {
      auto tn = scenar->findSynchronization(ev->synchronization());
      if (!tn)
        continue;
      events.insert(ev);
      synchronizations.insert(tn);
    }
    else if (auto tn = dynamic_cast<const SynchronizationModel*>(elt))
    {
      synchronizations.insert(tn);
    }
    else if (auto cstr = dynamic_cast<const IntervalModel*>(elt))
    {
      intervals.insert(cstr);
    }
  }

  if (states.size() == 1 && intervals.empty())
      return new StateInspectorWidget{**states.begin(), doc, parent};
  if (events.size() == 1 && intervals.empty())
      return new EventInspectorWidget{**events.begin(), doc, parent};
  if (synchronizations.size() == 1 && intervals.empty())
    return new SynchronizationInspectorWidget{**synchronizations.begin(), doc, parent};

  if (intervals.size() == 1 && synchronizations.empty())
  {
    return IntervalInspectorFactory{}.make(
        {*intervals.begin()}, doc, parent);
  }

  return new SummaryInspectorWidget{
      abstr, intervals, synchronizations, events, states, doc, parent}; // the default InspectorWidgetBase need
                                       // an only IdentifiedObject : this will
                                       // be "abstr"
}

bool ScenarioInspectorWidgetFactoryWrapper::matches(
    const QList<const QObject*>& objects) const
{
  return std::any_of(objects.begin(), objects.end(), [](const QObject* obj) {
    return dynamic_cast<const StateModel*>(obj)
           || dynamic_cast<const EventModel*>(obj)
           || dynamic_cast<const SynchronizationModel*>(obj)
           || dynamic_cast<const IntervalModel*>(obj);
  });
}
}
