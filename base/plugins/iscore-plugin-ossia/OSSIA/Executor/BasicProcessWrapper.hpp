#pragma once
#include <memory>
#include <OSSIA/Executor/ProcessWrapper.hpp>

namespace ossia
{
class time_constraint;
class time_value;
class time_process;
}

namespace RecreateOnPlay
{
class BasicProcessWrapper : public ProcessWrapper
{
    public:
        BasicProcessWrapper(const std::shared_ptr<ossia::time_constraint>& cst,
                       const std::shared_ptr<ossia::time_process>& ptr,
                       ossia::time_value dur,
                       bool looping);

    private:
        std::shared_ptr<ossia::time_process> currentProcess() const;
        ossia::time_constraint& currentConstraint() const;

        std::shared_ptr<ossia::time_constraint> m_parent;
        std::shared_ptr<ossia::time_process> m_process;
};
}
