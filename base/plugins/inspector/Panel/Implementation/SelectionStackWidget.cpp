#include "SelectionStackWidget.hpp"
#include <core/interface/selection/SelectionStack.hpp>
#include <QToolButton>
#include <QVBoxLayout>

SelectionStackWidget::SelectionStackWidget(SelectionStack* s, QWidget* parent):
    m_stack{s}
{
    m_prev = new QToolButton{this};
    m_prev->setArrowType(Qt::LeftArrow);
    m_prev->setEnabled(false);
    m_next = new QToolButton{this};
    m_next->setArrowType(Qt::RightArrow);
    m_next->setEnabled(false);

    auto lay = new QVBoxLayout{this};
    lay->addWidget(m_prev);
    lay->addWidget(m_next);
    setLayout(lay);

    if(!m_stack)
    { return; }

    connect(m_prev, &QToolButton::pressed,
            [&] () { m_stack->unselect(); });

    connect(m_next, &QToolButton::pressed,
            [&] () { m_stack->reselect(); });

    connect(s, &SelectionStack::currentSelectionChanged,
            [&] (Selection)
    {
        m_prev->setEnabled(m_stack->canUnselect());
        m_next->setEnabled(m_stack->canReselect());
    });

    m_prev->setEnabled(m_stack->canUnselect());
    m_next->setEnabled(m_stack->canReselect());
}
