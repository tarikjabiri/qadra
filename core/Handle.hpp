#ifndef QADRA_HANDLE_HPP
#define QADRA_HANDLE_HPP

#include <QString>
#include <qhashfunctions.h>

namespace Qadra::Core
{
  class Handle
  {
  public:
    Handle () : m_value ( 0 ) { }

    explicit Handle ( quint64 value );

    quint64 value () const { return m_value; }

    bool isValid () const { return m_value != 0; }

    QString hex () const;

    static Handle fromHex ( const QString &hex );

    auto operator<=> ( const Handle &other ) const = default;

  private:
    quint64 m_value;
  };

} // namespace Qadra::Core

template <>
struct std::hash<Qadra::Core::Handle>
{
  size_t operator() ( const Qadra::Core::Handle &handle ) const noexcept
  {
    return std::hash<quint64>{}( handle.value () );
  }
};

inline size_t qHash ( const Qadra::Core::Handle &h, const size_t seed = 0 )
{
  return qHashMulti ( seed, h.value () );
}

#endif // QADRA_HANDLE_HPP
