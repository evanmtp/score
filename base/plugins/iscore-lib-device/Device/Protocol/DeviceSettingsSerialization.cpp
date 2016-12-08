#include <Device/Protocol/ProtocolList.hpp>

#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QVariant>
#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/serialization/JSONVisitor.hpp>

#include "DeviceSettings.hpp"
#include "ProtocolFactoryInterface.hpp"
#include <iscore/application/ApplicationContext.hpp>
#include <iscore/plugins/customfactory/FactoryFamily.hpp>

#include <iscore/plugins/customfactory/StringFactoryKey.hpp>
#include <iscore/plugins/customfactory/StringFactoryKeySerialization.hpp>
#include <iscore/serialization/JSONValueVisitor.hpp>

template <typename T>
class Reader;
template <typename T>
class Writer;

template <>
ISCORE_LIB_DEVICE_EXPORT void
Visitor<Reader<DataStream>>::readFrom(const Device::DeviceSettings& n)
{
  m_stream << n.name << n.protocol;

  // TODO try to see if this pattern is refactorable with the similar thing
  // usef for CurveSegmentData.

  auto& pl = components.interfaces<Device::ProtocolFactoryList>();
  auto prot = pl.get(n.protocol);
  if (prot)
  {
    prot->serializeProtocolSpecificSettings(
        n.deviceSpecificSettings, this->toVariant());
  }
  else
  {
    qDebug() << "Warning: could not serialize device " << n.name;
  }

  insertDelimiter();
}

template <>
ISCORE_LIB_DEVICE_EXPORT void
Visitor<Writer<DataStream>>::writeTo(Device::DeviceSettings& n)
{
  m_stream >> n.name >> n.protocol;

  auto& pl = components.interfaces<Device::ProtocolFactoryList>();
  auto prot = pl.get(n.protocol);
  if (prot)
  {
    n.deviceSpecificSettings
        = prot->makeProtocolSpecificSettings(this->toVariant());
  }
  else
  {
    qDebug() << "Warning: could not load device " << n.name;
  }

  checkDelimiter();
}
template <>
ISCORE_LIB_DEVICE_EXPORT void
Visitor<Reader<JSONObject>>::readFrom(const Device::DeviceSettings& n)
{
  m_obj[strings.Name] = n.name;
  m_obj[strings.Protocol] = toJsonValue(n.protocol);

  auto& pl = components.interfaces<Device::ProtocolFactoryList>();
  auto prot = pl.get(n.protocol);
  if (prot)
  {
    prot->serializeProtocolSpecificSettings(
        n.deviceSpecificSettings, this->toVariant());
  }
  else
  {
    qDebug() << "Warning: could not serialize device " << n.name;
  }
}

template <>
ISCORE_LIB_DEVICE_EXPORT void
Visitor<Writer<JSONObject>>::writeTo(Device::DeviceSettings& n)
{
  n.name = m_obj[strings.Name].toString();
  n.protocol = fromJsonValue<UuidKey<Device::ProtocolFactory>>(
      m_obj[strings.Protocol]);

  auto& pl = components.interfaces<Device::ProtocolFactoryList>();
  auto prot = pl.get(n.protocol);
  if (prot)
  {
    n.deviceSpecificSettings
        = prot->makeProtocolSpecificSettings(this->toVariant());
  }
  else
  {
    qDebug() << "Warning: could not load device " << n.name;
  }
}
