#pragma once
#include <QByteArray>
#include <QPair>
#include <QVector>
#include <Scenario/Commands/ScenarioCommandFactory.hpp>
#include <score/command/Command.hpp>
#include <score/model/path/Path.hpp>
#include <score/model/Identifier.hpp>
#include <score/tools/std/Optional.hpp>

#include <score/selection/Selection.hpp>

struct DataStreamInput;
struct DataStreamOutput;

namespace Scenario
{
class IntervalModel;
class EventModel;
class StateModel;
class SynchronizationModel;
class CommentBlockModel;
class ProcessModel;
namespace Command
{
/**
 * @brief The RemoveSelection class
 *
 * Tries to remove what is selected in a scenario.
 */
class RemoveSelection final : public score::Command
{
  SCORE_COMMAND_DECL(
      ScenarioCommandFactoryName(), RemoveSelection,
      "Remove selected elements")
public:
  RemoveSelection(
      const Scenario::ProcessModel&,
      Selection sel);

  void undo(const score::DocumentContext& ctx) const override;
  void redo(const score::DocumentContext& ctx) const override;

protected:
  void serializeImpl(DataStreamInput&) const override;
  void deserializeImpl(DataStreamOutput&) override;

private:
  Path<Scenario::ProcessModel> m_path;

  // For synchronizations that may be removed when there is only a single event
  QVector<QPair<Id<SynchronizationModel>, QByteArray>> m_maybeRemovedSynchronizations;

  QVector<QPair<Id<CommentBlockModel>, QByteArray>> m_removedComments;
  QVector<QPair<Id<StateModel>, QByteArray>> m_removedStates;
  QVector<QPair<Id<EventModel>, QByteArray>> m_removedEvents;
  QVector<QPair<Id<SynchronizationModel>, QByteArray>> m_removedSynchronizations;
  QVector<QPair<Id<IntervalModel>, QByteArray>> m_removedIntervals;
};
}
}
