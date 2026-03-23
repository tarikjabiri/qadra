#include "Handle.hpp"

namespace Qadra::Core
{
  Handle::Handle ( const quint64 value ) : m_value ( value ) { }

  QString Handle::hex () const { return QString::number ( m_value, 16 ).toUpper (); }

  Handle Handle::fromHex ( const QString &hex )
  {
    bool ok = false;
    const uint64_t value = hex.toULongLong ( &ok, 16 );
    return ok ? Handle ( value ) : Handle ();
  }
} // namespace Qadra::Core
