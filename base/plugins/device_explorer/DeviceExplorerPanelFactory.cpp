#include "DeviceExplorerPanelFactory.hpp"
#include "Panel/DeviceExplorerModel.hpp"
#include "Panel/DeviceExplorerWidget.hpp"
#include "document/DocumentPresenter.hpp"
#include <core/document/DocumentModel.hpp>
#include <core/view/View.hpp>
using namespace iscore;

//@todo split this in multiple files.

DeviceExplorerPanelView::DeviceExplorerPanelView(View* parent):
	iscore::PanelViewInterface{parent},
	m_widget{new DeviceExplorerWidget{parent}}
{
	setObjectName("Device Explorer");
}

QWidget* DeviceExplorerPanelView::getWidget()
{
	return m_widget;
}


DeviceExplorerPanelModel::DeviceExplorerPanelModel(DocumentModel* parent):
	iscore::PanelModelInterface{"DeviceExplorerPanelModel", parent},
	m_model{new DeviceExplorerModel{this}}
{
}


DeviceExplorerPanelPresenter::DeviceExplorerPanelPresenter(iscore::Presenter* parent,
														   iscore::PanelViewInterface* view):
	iscore::PanelPresenterInterface{parent, view}
{

}

void DeviceExplorerPanelPresenter::on_modelChanged()
{
	auto v = static_cast<DeviceExplorerPanelView*>(m_view);
	auto m = static_cast<DeviceExplorerPanelModel*>(m_model);

	// TODO make a function to get the document here
	m->m_model->setCommandQueue(m_parentPresenter->document()->presenter()->commandQueue());
	v->m_widget->setModel(m->m_model);
}







iscore::PanelViewInterface*DeviceExplorerPanelFactory::makeView(iscore::View* parent)
{
	return new DeviceExplorerPanelView{parent};
}

iscore::PanelPresenterInterface*DeviceExplorerPanelFactory::makePresenter(iscore::Presenter* parent_presenter,
		iscore::PanelViewInterface* view)
{
	return new DeviceExplorerPanelPresenter{parent_presenter, view};
}

iscore::PanelModelInterface*DeviceExplorerPanelFactory::makeModel(DocumentModel* parent)
{
	return new DeviceExplorerPanelModel{parent};
}

