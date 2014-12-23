#pragma once
#include <core/presenter/command/SerializableCommand.hpp>
#include <tools/ObjectPath.hpp>

#include <QPointF>
#include <QMap>
#include <tuple>

struct EventModelData;

/**
 * @brief The CreateEventCommand class
 *
 * This command creates an Event, which is linked to the first event in the
 * scenario.
 */
class CreateEventCommand : public iscore::SerializableCommand
{
		friend class CreateEventAfterEventCommandTest;
		friend class MoveEventCommandTest;
	public:
		CreateEventCommand();
		CreateEventCommand(ObjectPath&& scenarioPath, int time, double heightPosition);
		virtual void undo() override;
		virtual void redo() override;
		virtual int id() const override;
		virtual bool mergeWith(const QUndoCommand* other) override;

	protected:
		virtual void serializeImpl(QDataStream&) override;
		virtual void deserializeImpl(QDataStream&) override;

	private:
		ObjectPath m_path;

		int m_createdConstraintId{};
		int m_createdBoxId{};
		int m_createdEventId{};
		// Map between the scenario view model ID and the constraint view model
		QMap<std::tuple<int,int,int>, int> m_createdConstraintViewModelIDs;

		int m_time{};
		double m_heightPosition{};
};
