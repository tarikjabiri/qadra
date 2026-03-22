#ifndef QADRA_UI_CANVAS_HPP
#define QADRA_UI_CANVAS_HPP

#include <QWindow>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Grid.hpp"
#include "Program.hpp"
#include "VertexArray.hpp"
#include "Camera.hpp"
#include "Font.hpp"
#include "GridPass.hpp"
#include "OrientedBoxRenderer.hpp"
#include "TextRenderer.hpp"

namespace Qadra::Ui {
  class Canvas : public QWindow {
    Q_OBJECT

  public:
    explicit Canvas();

    static QString loadShaderSource(const QString& filename);

  protected:
    void exposeEvent(QExposeEvent *) override;

    bool event(QEvent *) override;

  private:
    void initialize();

    void render();

    static QFunctionPointer getProcAddress(const char *procName);

  protected:
    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

  private:
    struct TextEntity {
      std::string fontKey;
      std::string text;
      glm::dvec2 position{0.0};
      double height{};
      double rotation{};
      glm::vec4 color{1.0f};
      glm::dvec2 localBoundsMin{0.0};
      glm::dvec2 localBoundsMax{0.0};
      glm::dvec2 worldBoundsMin{0.0};
      glm::dvec2 worldBoundsMax{0.0};
    };

    struct LoadedFont {
      std::string key;
      QString path;
      std::unique_ptr<Core::Font> font;
    };

    void loadShowcaseFonts();

    void buildShowcaseScene();

    [[nodiscard]] Core::Font *findFont(std::string_view fontKey);

    [[nodiscard]] const Core::Font *findFont(std::string_view fontKey) const;

    QOpenGLContext *m_context = nullptr;
    bool m_initialized = false;
    bool m_hasInitializedCameraViewport = false;

    Core::Camera m_camera;
    QPointF m_lastMousePosition;
    bool m_panning{false};

    std::optional<GL::VertexArray> m_vertexArray;
    std::optional<GL::Buffer> m_buffer;
    std::optional<GL::Program> m_program;
    std::optional<GL::Program> m_programGrid;
    std::optional<Core::Grid> m_grid;
    std::optional<Render::GridPass> m_gridPass;
    std::optional<Core::OrientedBoxRenderer> m_orientedBoxRenderer;
    std::optional<Core::TextRenderer> m_textRenderer;
    std::vector<LoadedFont> m_fonts;
    std::vector<TextEntity> m_textEntities;
  };
} // Qadra::Ui

#endif // QADRA_UI_CANVAS_HPP
