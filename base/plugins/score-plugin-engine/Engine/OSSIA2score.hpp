#pragma once
#include <ossia/editor/scenario/time_value.hpp>
#include <State/Value.hpp>
#include <Process/TimeValue.hpp>

namespace Engine::ossia_to_score
{

template <typename>
struct MatchingType;
template <>
struct MatchingType<float>
{
  static constexpr const auto val = ossia::val_type::FLOAT;
  using type = float;
  static auto convert(float f)
  {
    return f;
  }
};
template <>
struct MatchingType<double>
{
  static constexpr const auto val = ossia::val_type::FLOAT;
  using type = float;
  static auto convert(double f)
  {
    return f;
  }
};
template <>
struct MatchingType<int>
{
  static constexpr const auto val = ossia::val_type::INT;
  using type = int32_t;
  static auto convert(int f)
  {
    return f;
  }
};
template <>
struct MatchingType<bool>
{
  static constexpr const auto val = ossia::val_type::BOOL;
  using type = bool;
  static auto convert(bool f)
  {
    return f;
  }
};
template <>
struct MatchingType<State::impulse>
{
  static constexpr const auto val = ossia::val_type::IMPULSE;
  using type = ossia::impulse;
  static auto convert(State::impulse)
  {
    return ossia::impulse{};
  }
};
template <>
struct MatchingType<std::string>
{
  static constexpr const auto val = ossia::val_type::STRING;
  using type = std::string;
  static auto convert(const std::string& f)
  {
    return f;
  }
  static auto convert(std::string&& f)
  {
    return std::move(f);
  }
};
template <>
struct MatchingType<QString>
{
  static constexpr const auto val = ossia::val_type::STRING;
  using type = std::string;
  static auto convert(const QString& f)
  {
    return f.toStdString();
  }
};
template <>
struct MatchingType<char>
{
  static constexpr const auto val = ossia::val_type::CHAR;
  using type = char;
  static auto convert(char f)
  {
    return f;
  }
};
template <>
struct MatchingType<QChar>
{
  static constexpr const auto val = ossia::val_type::CHAR;
  using type = char;
  static auto convert(QChar f)
  {
    return f.toLatin1();
  }
};
template <>
struct MatchingType<State::vec2f>
{
  static constexpr const auto val = ossia::val_type::VEC2F;
  using type = ossia::vec2f;
  static auto convert(const State::vec2f& t)
  {
    return t;
  }
};
template <>
struct MatchingType<State::vec3f>
{
  static constexpr const auto val = ossia::val_type::VEC3F;
  using type = ossia::vec3f;
  static auto convert(const State::vec3f& t)
  {
    return t;
  }
};
template <>
struct MatchingType<State::vec4f>
{
  static constexpr const auto val = ossia::val_type::VEC4F;
  using type = ossia::vec4f;
  static auto convert(const State::vec4f& t)
  {
    return t;
  }
};
template <>
struct MatchingType<State::list_t>
{
  static constexpr const auto val = ossia::val_type::LIST;
  using type = std::vector<ossia::value>;
  static auto convert(const State::list_t& t)
  {
    return t;
  }
  static auto convert(State::list_t&& t)
  {
    return std::move(t);
  }
};
template <>
struct MatchingType<::TimeVal>
{
  static constexpr const auto val = ossia::val_type::FLOAT;
  using type = float;
  static auto convert(const TimeVal& t)
  {
    return t.msec();
  }
};

inline ::TimeVal defaultTime(ossia::time_value t)
{
  return t.infinite() ? ::TimeVal::infinite()
                      : ::TimeVal::fromMsecs(double(t) / 1000.);
}
}
