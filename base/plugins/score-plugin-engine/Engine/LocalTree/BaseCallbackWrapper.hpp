#pragma once
#include <Engine/LocalTree/BaseProperty.hpp>
#include <ossia/network/base/parameter.hpp>

namespace Engine
{
namespace LocalTree
{
class SCORE_PLUGIN_ENGINE_EXPORT BaseCallbackWrapper
    : public BaseProperty
{
public:
  using BaseProperty::BaseProperty;
  ~BaseCallbackWrapper() override = default;

  ossia::net::parameter_base::callback_index callbackIt{};
};
}
}
