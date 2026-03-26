#ifndef QADRA_TOOL_CONTEXT_HPP
#define QADRA_TOOL_CONTEXT_HPP

namespace Qadra::Cad
{
  class Document;
}

namespace Qadra::Core
{
  class Camera;
  class Font;
} // namespace Qadra::Core

namespace Qadra::Tool
{
  struct ToolContext
  {
    Cad::Document &document;
    Core::Camera &camera;
    Core::Font *font = nullptr;

    [[nodiscard]] bool hasFont () const noexcept { return font != nullptr; }
  };
} // namespace Qadra::Tool

#endif // QADRA_TOOL_CONTEXT_HPP
