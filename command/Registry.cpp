#include "Registry.hpp"

#include "arc/Command.hpp"
#include "line/Command.hpp"

#include <cctype>
#include <string>

namespace
{
  [[nodiscard]] std::string_view trim ( std::string_view text ) noexcept
  {
    while ( ! text.empty () && std::isspace ( static_cast<unsigned char> ( text.front () ) ) )
      text.remove_prefix ( 1 );

    while ( ! text.empty () && std::isspace ( static_cast<unsigned char> ( text.back () ) ) )
      text.remove_suffix ( 1 );

    return text;
  }

  [[nodiscard]] std::string toLower ( const std::string_view text )
  {
    std::string normalized;
    normalized.reserve ( text.size () );

    for ( const char ch : text )
      normalized.push_back (
          static_cast<char> ( std::tolower ( static_cast<unsigned char> ( ch ) ) ) );

    return normalized;
  }
} // namespace

namespace Qadra::Command
{
  std::unique_ptr<SessionCommand> Registry::create ( const Tool::ToolKind kind ) const
  {
    switch ( kind )
    {
      case Tool::ToolKind::Line:
        return std::make_unique<LineCommand> ();
      case Tool::ToolKind::Arc:
        return std::make_unique<ArcCommand> ();

      case Tool::ToolKind::None:
      case Tool::ToolKind::Text:
        return nullptr;
    }

    return nullptr;
  }

  std::optional<Tool::ToolKind> Registry::resolveAlias ( const std::string_view text ) const
  {
    const std::string normalized = toLower ( trim ( text ) );
    if ( normalized == "arc" || normalized == "a" ) return Tool::ToolKind::Arc;
    if ( normalized == "line" || normalized == "l" ) return Tool::ToolKind::Line;
    return std::nullopt;
  }
} // namespace Qadra::Command
