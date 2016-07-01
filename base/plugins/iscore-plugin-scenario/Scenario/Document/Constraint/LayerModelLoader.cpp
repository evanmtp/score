#include <Process/Process.hpp>
#include <Scenario/Document/Constraint/ConstraintModel.hpp>
#include <iscore/tools/std/Optional.hpp>
#include <QJsonObject>
#include <algorithm>

#include "LayerModelLoader.hpp"
#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/serialization/JSONValueVisitor.hpp>
#include <iscore/serialization/JSONVisitor.hpp>
#include <iscore/tools/NotifyingMap.hpp>
#include <iscore/tools/SettableIdentifier.hpp>

template <typename VisitorType> class Visitor;
namespace Process
{
template<>
LayerModel* createLayerModel(
        Deserializer<DataStream>& deserializer,
        const Scenario::ConstraintModel& constraint,
        QObject* parent)
{
    Id<ProcessModel> sharedProcessId;
    deserializer.m_stream >> sharedProcessId;

    auto& process = constraint.processes.at(sharedProcessId);
    auto viewmodel = process.loadLayer(deserializer.toVariant(),
                                       parent);

    deserializer.checkDelimiter();

    return viewmodel;
}


template<>
LayerModel* createLayerModel(
        Deserializer<JSONObject>& deserializer,
        const Scenario::ConstraintModel& constraint,
        QObject* parent)
{
    auto proc_id = fromJsonValue<Id<ProcessModel>>(deserializer.m_obj["SharedProcessId"]);
    if(!proc_id)
        return nullptr;

    auto process_it = constraint.processes.find(proc_id);
    if(process_it == constraint.processes.end())
        return nullptr;

    return process_it->loadLayer(deserializer.toVariant(), parent);
}

}
