#ifndef QADRA_COMMAND_CONTEXT_HPP
#define QADRA_COMMAND_CONTEXT_HPP

namespace Qadra::Cad
{
  class Document;
}

namespace Qadra::Core
{
  class Camera;
  class Font;
} // namespace Qadra::Core

namespace Qadra::Command
{
  struct Context
  {
    Cad::Document &document;
    Core::Camera &camera;
    Core::Font *font = nullptr;

    [[nodiscard]] bool hasFont () const noexcept { return font != nullptr; }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_CONTEXT_HPP
