#ifndef QADRA_DOCUMENT_CHANGE_HPP
#define QADRA_DOCUMENT_CHANGE_HPP

#include "Handle.hpp"

namespace Qadra::Cad
{
  struct DocumentChange
  {
    enum class Kind
    {
      Added,
      Removed,
      Modified,
      Reset,
    };

    Kind kind{};
    Core::Handle handle;
  };
} // namespace Qadra::Cad

#endif // QADRA_DOCUMENT_CHANGE_HPP
