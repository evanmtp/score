#include "InspectorPlugin.hpp"
#include "Panel/InspectorPanelFactory.hpp"
using namespace iscore;

#include <interface/inspector/InspectorWidgetFactoryInterface.hpp>
#include <InspectorControl.hpp>
InspectorPlugin::InspectorPlugin() :
	QObject {},
	iscore::Autoconnect_QtInterface {},
	iscore::PanelFactoryInterface_QtInterface {},
	m_inspectorControl{new InspectorControl}
{
	setObjectName ("InspectorPlugin");
}





QList<Autoconnect> InspectorPlugin::autoconnect_list() const
{
	return
	{
		/// Common
		{{iscore::Autoconnect::ObjectRepresentationType::QObjectName,
		  "Presenter",			 SIGNAL(elementSelected(QObject*))},
		 {iscore::Autoconnect::ObjectRepresentationType::QObjectName,
		  "InspectorPanelModel", SLOT(newItemInspected(QObject*))}},

		{{iscore::Autoconnect::ObjectRepresentationType::Inheritance,
		  "InspectorWidgetBase", SIGNAL(submitCommand(iscore::SerializableCommand*))},
		 {iscore::Autoconnect::ObjectRepresentationType::QObjectName,
		  "Presenter", SLOT(applyCommand(iscore::SerializableCommand*))}},
	};
}



QStringList InspectorPlugin::panel_list() const
{
	return {"Inspector Panel"};
}

PanelFactoryInterface* InspectorPlugin::panel_make (QString name)
{
	if (name == "Inspector Panel")
	{
		return new InspectorPanelFactory;
	}

	return nullptr;
}

QVector<FactoryFamily> InspectorPlugin::factoryFamilies_make()
{
	return {{"Inspector",
			std::bind(&InspectorControl::on_newInspectorWidgetFactory,
					  m_inspectorControl,
					  std::placeholders::_1)}};
}


QStringList InspectorPlugin::control_list() const
{
	return {"InspectorControl"};
}

PluginControlInterface* InspectorPlugin::control_make(QString s)
{
	if(s == "InspectorControl")
	{
		return m_inspectorControl; // TODO what happens on deletion ?
	}

	return nullptr;
}
