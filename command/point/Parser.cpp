#include "Parser.hpp"

#include <cctype>
#include <cerrno>
#include <cstdlib>
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

  [[nodiscard]] std::optional<double> parseNumber ( const std::string_view text )
  {
    const std::string_view trimmed = trim ( text );
    if ( trimmed.empty () ) return std::nullopt;

    const std::string buffer ( trimmed );
    char *end = nullptr;
    errno = 0;

    const double value = std::strtod ( buffer.c_str (), &end );
    if ( errno == ERANGE ) return std::nullopt;
    if ( end != buffer.c_str () + buffer.size () ) return std::nullopt;
    return value;
  }

  [[nodiscard]] std::optional<Qadra::Command::PointExpression>
  parseCartesian ( const std::string_view text, const bool relative )
  {
    const std::size_t separator = text.find ( ',' );
    if ( separator == std::string_view::npos ) return std::nullopt;
    if ( text.find ( ',', separator + 1 ) != std::string_view::npos ) return std::nullopt;

    const auto x = parseNumber ( text.substr ( 0, separator ) );
    const auto y = parseNumber ( text.substr ( separator + 1 ) );
    if ( ! x.has_value () || ! y.has_value () ) return std::nullopt;

    Qadra::Command::PointExpression expression;
    expression.relative = relative;
    expression.value = Qadra::Command::CartesianValue{ *x, *y };
    return expression;
  }

  [[nodiscard]] std::optional<Qadra::Command::PointExpression>
  parsePolar ( const std::string_view text, const bool relative )
  {
    const std::size_t separator = text.find ( '<' );
    if ( separator == std::string_view::npos ) return std::nullopt;
    if ( text.find ( '<', separator + 1 ) != std::string_view::npos ) return std::nullopt;

    const auto distance = parseNumber ( text.substr ( 0, separator ) );
    const auto angle = parseNumber ( text.substr ( separator + 1 ) );
    if ( ! distance.has_value () || ! angle.has_value () ) return std::nullopt;

    Qadra::Command::PointExpression expression;
    expression.relative = relative;
    expression.value = Qadra::Command::PolarValue{ *distance, *angle };
    return expression;
  }
} // namespace

namespace Qadra::Command
{
  std::optional<PointExpression> PointParser::parse ( std::string_view text ) const
  {
    text = trim ( text );
    if ( text.empty () ) return std::nullopt;

    bool relative = false;
    if ( text.front () == '@' )
    {
      relative = true;
      text.remove_prefix ( 1 );
      text = trim ( text );
    }

    if ( text.empty () ) return std::nullopt;

    const bool hasCartesianSeparator = text.find ( ',' ) != std::string_view::npos;
    const bool hasPolarSeparator = text.find ( '<' ) != std::string_view::npos;

    if ( hasCartesianSeparator == hasPolarSeparator ) return std::nullopt;
    if ( hasCartesianSeparator ) return parseCartesian ( text, relative );
    return parsePolar ( text, relative );
  }
} // namespace Qadra::Command
