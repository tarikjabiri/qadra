#include <glad/gl.h>
#include "Canvas.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cmath>
#include <exception>
#include <limits>
#include <unordered_map>
#include <utility>
#include <QDebug>
#include <QFileInfo>
#include <QOpenGLContext>
#include <QMessageBox>
#include <QMainWindow>
#include <QWheelEvent>
#include <QFile>
#include <QStringList>

#include "Program.hpp"
#include "Shader.hpp"
#include "TextLayout.hpp"

namespace {
  struct FontCandidate {
    const char *key;
    std::array<const char *, 3> paths;
  };

  struct ScenePrototypeSeed {
    const char *fontKey;
    const char *fallbackFontKey;
    std::string text;
    double height;
    double rotation;
    glm::vec4 color;
    double padding;
  };

  struct TextBounds {
    glm::dvec2 min{0.0};
    glm::dvec2 max{0.0};
  };

  struct ScenePrototype {
    std::string fontKey;
    std::string text;
    double height{};
    double rotation{};
    glm::vec4 color{1.0f};
    double padding{};
    TextBounds relativeBounds;
    glm::dvec2 relativeCenter{0.0};
  };

  struct SpatialHash {
    explicit SpatialHash(const double cellSize)
      : m_cellSize(cellSize) {
    }

    [[nodiscard]] bool overlaps(const TextBounds &bounds) const {
      const glm::ivec2 minimumCell = cellCoordinates(bounds.min);
      const glm::ivec2 maximumCell = cellCoordinates(bounds.max);

      for (int cellX = minimumCell.x; cellX <= maximumCell.x; ++cellX) {
        for (int cellY = minimumCell.y; cellY <= maximumCell.y; ++cellY) {
          const auto iterator = m_cells.find(cellKey(cellX, cellY));
          if (iterator == m_cells.end()) {
            continue;
          }

          for (const TextBounds &occupiedBounds: iterator->second) {
            if (bounds.min.x < occupiedBounds.max.x &&
                bounds.max.x > occupiedBounds.min.x &&
                bounds.min.y < occupiedBounds.max.y &&
                bounds.max.y > occupiedBounds.min.y) {
              return true;
            }
          }
        }
      }

      return false;
    }

    void insert(const TextBounds &bounds) {
      const glm::ivec2 minimumCell = cellCoordinates(bounds.min);
      const glm::ivec2 maximumCell = cellCoordinates(bounds.max);

      for (int cellX = minimumCell.x; cellX <= maximumCell.x; ++cellX) {
        for (int cellY = minimumCell.y; cellY <= maximumCell.y; ++cellY) {
          m_cells[cellKey(cellX, cellY)].push_back(bounds);
        }
      }
    }

  private:
    [[nodiscard]] glm::ivec2 cellCoordinates(const glm::dvec2 &point) const {
      return {
        static_cast<int>(std::floor(point.x / m_cellSize)),
        static_cast<int>(std::floor(point.y / m_cellSize))
      };
    }

    [[nodiscard]] static std::uint64_t cellKey(const int cellX, const int cellY) {
      return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(cellX)) << 32) |
             static_cast<std::uint32_t>(cellY);
    }

    double m_cellSize{};
    std::unordered_map<std::uint64_t, std::vector<TextBounds>> m_cells;
  };

  int viewportPixels(const int logicalPixels, const qreal devicePixelRatio) {
    return static_cast<int>(std::lround(static_cast<double>(logicalPixels) * devicePixelRatio));
  }

  glm::dvec2 viewportPixels(const QPointF &logicalPosition, const qreal devicePixelRatio) {
    return {
      logicalPosition.x() * devicePixelRatio,
      logicalPosition.y() * devicePixelRatio
    };
  }

  std::optional<QString> firstExistingFontPath(const std::array<const char *, 3> &candidates) {
    for (const char *candidate: candidates) {
      const QString path = QString::fromLatin1(candidate);
      if (QFileInfo::exists(path)) {
        return path;
      }
    }

    return std::nullopt;
  }

  std::string utf8String(const char8_t *text) {
    return {
      reinterpret_cast<const char *>(text),
      std::char_traits<char8_t>::length(text)
    };
  }

  TextBounds expandBounds(const TextBounds &bounds, const double padding) {
    return {
      .min = bounds.min - glm::dvec2(padding, padding),
      .max = bounds.max + glm::dvec2(padding, padding)
    };
  }

  [[nodiscard]] bool intersects(const TextBounds &left, const TextBounds &right) {
    return left.min.x < right.max.x &&
           left.max.x > right.min.x &&
           left.min.y < right.max.y &&
           left.max.y > right.min.y;
  }

  [[nodiscard]] double halton(std::uint64_t index, const int base) {
    double result = 0.0;
    double factor = 1.0 / static_cast<double>(base);

    while (index > 0) {
      result += factor * static_cast<double>(index % static_cast<std::uint64_t>(base));
      index /= static_cast<std::uint64_t>(base);
      factor /= static_cast<double>(base);
    }

    return result;
  }

  [[nodiscard]] std::uint64_t mixBits(std::uint64_t value) {
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ull;
    value ^= value >> 27;
    value *= 0x94d049bb133111ebull;
    value ^= value >> 31;
    return value;
  }

  const std::array<FontCandidate, 10> showcaseFontCandidates{{
    {"latinRegular", {"C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/calibri.ttf", "C:/Windows/Fonts/arial.ttf"}},
    {"latinBold", {"C:/Windows/Fonts/segoeuib.ttf", "C:/Windows/Fonts/calibrib.ttf", "C:/Windows/Fonts/arialbd.ttf"}},
    {"unicodeRegular", {"C:/Windows/Fonts/ARIALUNI.ttf", "C:/Windows/Fonts/arial.ttf", "C:/Windows/Fonts/segoeui.ttf"}},
    {"arabicBold", {"C:/Windows/Fonts/arialbd.ttf", "C:/Windows/Fonts/segoeuib.ttf", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"chineseRegular", {"C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"chineseBold", {"C:/Windows/Fonts/msyhbd.ttc", "C:/Windows/Fonts/simsunb.ttf", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"japaneseRegular", {"C:/Windows/Fonts/YuGothM.ttc", "C:/Windows/Fonts/msgothic.ttc", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"japaneseBold", {"C:/Windows/Fonts/YuGothB.ttc", "C:/Windows/Fonts/msgothic.ttc", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"koreanBold", {"C:/Windows/Fonts/malgunbd.ttf", "C:/Windows/Fonts/malgun.ttf", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"thaiRegular", {"C:/Windows/Fonts/LeelawUI.ttf", "C:/Windows/Fonts/LeelaUIb.ttf", "C:/Windows/Fonts/ARIALUNI.ttf"}},
  }};

  constexpr std::size_t minimumShowcaseEntityCount = 50000;
}

namespace Qadra::Ui {
  Canvas::Canvas() {
    setSurfaceType(OpenGLSurface);

    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 6);
    format.setAlphaBufferSize(8);
    format.setSwapInterval(0);
    format.setSamples(4);
    setFormat(format);

    m_context = new QOpenGLContext(this);
    m_context->setFormat(requestedFormat());
  }

  QString Canvas::loadShaderSource(const QString &filename) {
    const QString path = QCoreApplication::applicationDirPath() + "/shaders/" + filename;
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      throw std::runtime_error("Failed to open shader: " + path.toStdString());
    }

    return QTextStream(&file).readAll();
  }

  void Canvas::exposeEvent(QExposeEvent *expose_event) {
    if (isExposed()) {
      if (!m_initialized) {
        initialize();
        m_initialized = true;

        m_buffer.emplace();
        m_vertexArray.emplace();
        m_program.emplace();

        m_programGrid.emplace();
        m_grid.emplace();

        m_gridPass.emplace();
        m_orientedBoxRenderer.emplace();
        m_textRenderer.emplace();

        const auto gridVertexSource = loadShaderSource("grid.vertex.glsl");
        const auto gridFragmentSource = loadShaderSource("grid.fragment.glsl");
        const auto orientedBoxVertexSource = loadShaderSource("oriented-box.vertex.glsl");
        const auto orientedBoxFragmentSource = loadShaderSource("oriented-box.fragment.glsl");
        const auto textVertexSource = loadShaderSource("text.vertex.glsl");
        const auto textFragmentSource = loadShaderSource("text.fragment.glsl");

        m_gridPass->init(gridVertexSource, gridFragmentSource);
        m_orientedBoxRenderer->init(orientedBoxVertexSource, orientedBoxFragmentSource);
        m_textRenderer->init(textVertexSource, textFragmentSource);

        loadShowcaseFonts();
        buildShowcaseScene();

        GL::Shader gridVertexShader(GL::Shader::Type::Vertex);
        GL::Shader gridFragmentShader(GL::Shader::Type::Fragment);

        if (!gridVertexShader.compile(gridVertexSource) || !gridFragmentShader.compile(gridFragmentSource)) {
          throw std::runtime_error("Vertex shader or fragment shaders are not compiled");
        }

        if (!m_programGrid->link(gridVertexShader, gridFragmentShader)) {
          throw std::runtime_error("Gird program shader or fragment shaders are not linked");
        }

        constexpr float vertices[] = {
          -0.5f * 1000, -0.5f * 1000, 0.0f * 1000,
          0.5f * 1000, -0.5f * 1000, 0.0f * 1000,
          0.0f * 1000, 0.5f * 1000, 0.0f * 1000
        };

        m_vertexArray->bind();

        m_buffer->bind();
        m_buffer->allocate(std::span(vertices));

        const auto vertexShaderSource = loadShaderSource("vertex.glsl");
        const auto fragmentShaderSource = loadShaderSource("fragment.glsl");

        GL::Shader vertexShader(GL::Shader::Type::Vertex);
        GL::Shader fragmentShader(GL::Shader::Type::Fragment);

        if (!vertexShader.compile(vertexShaderSource) || !fragmentShader.compile(fragmentShaderSource)) {
          throw std::runtime_error("Vertex shader or fragment shaders are not compiled");
        }


        if (!m_program->link(vertexShader, fragmentShader)) {
          throw std::runtime_error("Program shader or fragment shaders are not linked");
        }

        m_program->bind();

        constexpr GL::VertexArray::Attribute position{
          .index = 0,
          .size = 3,
          .type = GL_FLOAT,
          .relativeOffset = 0,
        };
        m_vertexArray->attribute(position);

        m_vertexArray->attachVertexBuffer(0, m_buffer.value(), 0, 3 * sizeof(float));
      }
      render();
    }
  }

  bool Canvas::event(QEvent *event) {
    if (event->type() == QEvent::UpdateRequest) {
      render();
      return true;
    }
    return QWindow::event(event);
  }

  void Canvas::initialize() {
    if (!m_context->create()) {
      delete m_context;
      QMessageBox::critical(nullptr, tr("Context is not created"), tr("Context is not created"));
    }

    if (m_context->makeCurrent(this)) {
      if (gladLoadGL(&Canvas::getProcAddress) == 0) {
        QMessageBox::critical(nullptr, tr("QMessage"), tr("QMessage"));
      }
    }
  }

  void Canvas::loadShowcaseFonts() {
    m_fonts.clear();

    for (const FontCandidate &fontCandidate: showcaseFontCandidates) {
      const std::optional<QString> fontPath = firstExistingFontPath(fontCandidate.paths);
      if (!fontPath) {
        continue;
      }

      try {
        LoadedFont loadedFont{
          .key = fontCandidate.key,
          .path = *fontPath,
          .font = std::make_unique<Core::Font>(fontPath->toStdString())
        };
        m_fonts.push_back(std::move(loadedFont));
      } catch (const std::exception &exception) {
        qWarning() << "Font initialization failed for" << fontCandidate.key << "at" << *fontPath << ":" << exception.what();
      }
    }

    if (m_fonts.empty()) {
      qWarning() << "No showcase fonts were loaded for text rendering";
    }
  }

  Core::Font *Canvas::findFont(const std::string_view fontKey) {
    const auto iterator = std::find_if(m_fonts.begin(), m_fonts.end(), [&](const LoadedFont &loadedFont) {
      return loadedFont.key == fontKey;
    });

    return iterator != m_fonts.end() ? iterator->font.get() : nullptr;
  }

  const Core::Font *Canvas::findFont(const std::string_view fontKey) const {
    const auto iterator = std::find_if(m_fonts.begin(), m_fonts.end(), [&](const LoadedFont &loadedFont) {
      return loadedFont.key == fontKey;
    });

    return iterator != m_fonts.end() ? iterator->font.get() : nullptr;
  }

  void Canvas::buildShowcaseScene() {
    m_textEntities.clear();
    if (m_fonts.empty()) {
      return;
    }

    const std::array<ScenePrototypeSeed, 24> seeds{{
      {"latinBold", "latinRegular", "Qadra Type Atlas", 124.0, -0.08, glm::vec4(0.98f, 0.73f, 0.56f, 0.98f), 40.0},
      {"latinBold", "latinRegular", "Bezier Notes", 96.0, 0.11, glm::vec4(0.93f, 0.66f, 0.52f, 0.97f), 34.0},
      {"arabicBold", "unicodeRegular", utf8String(u8"النص العربي يتصل ويتغير مع السياق"), 82.0, -0.05, glm::vec4(0.95f, 0.90f, 0.78f, 0.98f), 30.0},
      {"unicodeRegular", "latinRegular", utf8String(u8"السَّلَامُ عَلَيْكُمْ"), 62.0, 0.09, glm::vec4(0.91f, 0.82f, 0.69f, 0.96f), 24.0},
      {"chineseBold", "unicodeRegular", utf8String(u8"矢量文本在缩放时应保持清晰"), 86.0, 0.04, glm::vec4(0.58f, 0.84f, 0.80f, 0.97f), 30.0},
      {"chineseRegular", "unicodeRegular", utf8String(u8"复杂脚本与字形替换需要稳定"), 46.0, 0.00, glm::vec4(0.76f, 0.92f, 0.81f, 0.94f), 20.0},
      {"chineseRegular", "unicodeRegular", utf8String(u8"注释、标注、尺寸、层级"), 28.0, -0.08, glm::vec4(0.73f, 0.88f, 0.79f, 0.92f), 16.0},
      {"japaneseBold", "japaneseRegular", utf8String(u8"回転したラベルとかな漢字の混在"), 74.0, 0.18, glm::vec4(0.93f, 0.70f, 0.80f, 0.96f), 28.0},
      {"japaneseRegular", "unicodeRegular", utf8String(u8"図面ビューに重ならない注釈"), 40.0, -0.22, glm::vec4(0.80f, 0.77f, 0.94f, 0.94f), 18.0},
      {"japaneseRegular", "unicodeRegular", utf8String(u8"細いラベルでも輪郭を保つ"), 22.0, 0.15, glm::vec4(0.74f, 0.72f, 0.90f, 0.92f), 14.0},
      {"koreanBold", "unicodeRegular", utf8String(u8"확대해도 가장자리가 깨지지 않아야 합니다"), 60.0, -0.09, glm::vec4(0.68f, 0.84f, 0.95f, 0.96f), 24.0},
      {"unicodeRegular", "latinRegular", utf8String(u8"곡선, 주석, 자간, 밀도"), 28.0, 0.12, glm::vec4(0.67f, 0.81f, 0.91f, 0.93f), 16.0},
      {"thaiRegular", "unicodeRegular", utf8String(u8"ข้อความภาษาไทยกับวรรณยุกต์และสระ"), 58.0, 0.11, glm::vec4(0.84f, 0.87f, 0.64f, 0.95f), 24.0},
      {"thaiRegular", "unicodeRegular", utf8String(u8"ตัวอย่างข้อความไทยหลายขนาด"), 26.0, -0.14, glm::vec4(0.77f, 0.82f, 0.59f, 0.92f), 14.0},
      {"unicodeRegular", "latinRegular", utf8String(u8"שלום עולם טקסט מימין לשמאל"), 56.0, 0.03, glm::vec4(0.86f, 0.79f, 0.95f, 0.96f), 24.0},
      {"unicodeRegular", "latinRegular", utf8String(u8"ימין לשמאל עם 2026"), 24.0, -0.17, glm::vec4(0.79f, 0.73f, 0.90f, 0.92f), 14.0},
      {"latinRegular", "unicodeRegular", "office affinity shuffle waffle", 52.0, 0.00, glm::vec4(0.89f, 0.93f, 1.00f, 0.95f), 22.0},
      {"latinBold", "latinRegular", "ffi  ffl  AVA  WAVE  kerning", 36.0, 0.07, glm::vec4(0.94f, 0.96f, 1.00f, 0.92f), 18.0},
      {"unicodeRegular", "latinRegular", utf8String(u8"RTL العربية + English + 2026"), 48.0, -0.16, glm::vec4(0.98f, 0.83f, 0.63f, 0.96f), 22.0},
      {"unicodeRegular", "latinRegular", utf8String(u8"漢字 العربية 한국어 mix"), 34.0, -0.05, glm::vec4(0.84f, 0.91f, 0.94f, 0.93f), 18.0},
      {"latinRegular", "unicodeRegular", "caps, numerals, punctuation [ ] { } / 0OIl1", 24.0, -0.19, glm::vec4(0.90f, 0.87f, 0.78f, 0.92f), 16.0},
      {"latinRegular", "unicodeRegular", "tiny note 12 px", 12.0, 0.00, glm::vec4(0.78f, 0.83f, 0.89f, 0.90f), 12.0},
      {"latinRegular", "unicodeRegular", "micro 8 px", 8.0, 0.00, glm::vec4(0.70f, 0.74f, 0.80f, 0.88f), 10.0},
      {"latinRegular", "unicodeRegular", "anchor / trim / glyph / atlas", 18.0, 0.13, glm::vec4(0.84f, 0.88f, 0.92f, 0.91f), 14.0},
    }};

    std::vector<ScenePrototype> prototypes;
    prototypes.reserve(seeds.size());

    for (const ScenePrototypeSeed &seed: seeds) {
      const std::string resolvedFontKey = [&]() -> std::string {
        if (findFont(seed.fontKey)) {
          return seed.fontKey;
        }
        if (seed.fallbackFontKey && *seed.fallbackFontKey && findFont(seed.fallbackFontKey)) {
          return seed.fallbackFontKey;
        }
        return {};
      }();

      if (resolvedFontKey.empty()) {
        continue;
      }

      Core::Font *font = findFont(resolvedFontKey);
      if (!font) {
        continue;
      }

      const Core::TextLayoutBounds localBounds = Core::measureTextLocalBounds(*font, seed.text, seed.height);
      prototypes.push_back({
        .fontKey = resolvedFontKey,
        .text = seed.text,
        .height = seed.height,
        .rotation = seed.rotation,
        .color = seed.color,
        .padding = seed.padding,
        .relativeBounds = {
          .min = localBounds.minimum,
          .max = localBounds.maximum
        },
        .relativeCenter = (localBounds.minimum + localBounds.maximum) * 0.5,
      });
    }

    if (prototypes.empty()) {
      qWarning() << "No showcase prototypes were generated";
      return;
    }

    m_textEntities.reserve(minimumShowcaseEntityCount);

    SpatialHash occupiedBounds(256.0);
    std::uint64_t placementSequence = 1;

    while (m_textEntities.size() < minimumShowcaseEntityCount) {
      const std::size_t entityIndex = m_textEntities.size();
      const std::uint64_t prototypeSelector = mixBits(static_cast<std::uint64_t>(entityIndex) + 0x9e3779b97f4a7c15ull);
      const ScenePrototype &prototype = prototypes[prototypeSelector % prototypes.size()];

      bool placed = false;
      std::size_t placementAttempt = 0;
      while (!placed) {
        const std::uint64_t sequenceValue = placementSequence++;
        const std::uint64_t mixedSequence = mixBits(sequenceValue + prototypeSelector);
        const double extent = 900.0 +
                              170.0 * std::sqrt(static_cast<double>(entityIndex + 1)) +
                              160.0 * std::sqrt(static_cast<double>(placementAttempt + 1));

        double candidateX = (halton(sequenceValue, 2) * 2.0 - 1.0) * extent;
        double candidateY = (halton(sequenceValue, 3) * 2.0 - 1.0) * extent * 0.82;

        const double phaseA = halton(sequenceValue, 5) * 6.283185307179586;
        const double phaseB = halton(sequenceValue, 7) * 6.283185307179586;
        candidateX += std::sin(candidateY * 0.0017 + phaseA) * (110.0 + prototype.height * 0.45);
        candidateY += std::cos(candidateX * 0.0013 + phaseB) * (95.0 + prototype.height * 0.32);

        if (placementAttempt > 48) {
          const double spiralAngle = static_cast<double>(mixedSequence % 8192ull) * 0.013;
          const double spiralRadius = extent + 180.0 * static_cast<double>(placementAttempt - 48);
          candidateX = std::cos(spiralAngle) * spiralRadius;
          candidateY = std::sin(spiralAngle) * spiralRadius * 0.78;
        }

        const glm::dvec2 worldCenter(candidateX, candidateY);
        const glm::dvec2 worldPosition = worldCenter - prototype.relativeCenter;
        const Core::TextLayoutBounds localBounds{
          .minimum = prototype.relativeBounds.min,
          .maximum = prototype.relativeBounds.max
        };
        const std::array<glm::dvec2, 4> orientedCorners = Core::orientedTextCorners(
          worldPosition,
          localBounds,
          prototype.rotation
        );
        const Core::TextLayoutBounds worldAxisAlignedBounds = Core::axisAlignedBounds(orientedCorners);
        const TextBounds worldBounds{
          .min = worldAxisAlignedBounds.minimum,
          .max = worldAxisAlignedBounds.maximum
        };
        const TextBounds occupiedWorldBounds = expandBounds(worldBounds, prototype.padding);

        if (!occupiedBounds.overlaps(occupiedWorldBounds)) {
          const float tint = static_cast<float>(0.86 + 0.18 * halton(sequenceValue, 11));
          const float alpha = static_cast<float>(0.84 + 0.12 * halton(sequenceValue, 13));

          m_textEntities.push_back({
            .fontKey = prototype.fontKey,
            .text = prototype.text,
            .position = worldPosition,
            .height = prototype.height,
            .rotation = prototype.rotation,
            .color = glm::vec4(
              std::clamp(prototype.color.r * tint, 0.0f, 1.0f),
              std::clamp(prototype.color.g * tint, 0.0f, 1.0f),
              std::clamp(prototype.color.b * tint, 0.0f, 1.0f),
              std::clamp(prototype.color.a * alpha, 0.0f, 1.0f)
            ),
            .localBoundsMin = prototype.relativeBounds.min,
            .localBoundsMax = prototype.relativeBounds.max,
            .worldBoundsMin = worldBounds.min,
            .worldBoundsMax = worldBounds.max,
          });
          occupiedBounds.insert(occupiedWorldBounds);
          placed = true;
        }

        ++placementAttempt;
      }
    }
  }

  void Canvas::render() {
    m_context->makeCurrent(this);

    const qreal devicePixelRatioValue = devicePixelRatio();
    const GLsizei viewportWidthPixels = viewportPixels(width(), devicePixelRatioValue);
    const GLsizei viewportHeightPixels = viewportPixels(height(), devicePixelRatioValue);

    if (m_camera.width() != viewportWidthPixels || m_camera.height() != viewportHeightPixels) {
      if (m_hasInitializedCameraViewport) {
        m_camera.resizePreserveViewportOrigin(viewportWidthPixels, viewportHeightPixels);
      } else {
        m_camera.resize(viewportWidthPixels, viewportHeightPixels);
        m_hasInitializedCameraViewport = true;
      }
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    glViewport(0, 0, viewportWidthPixels, viewportHeightPixels);
    glClearColor(0.09f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    m_gridPass->render(m_camera, glm::vec2(static_cast<float>(viewportWidthPixels),
                                           static_cast<float>(viewportHeightPixels)));

    if (m_textRenderer && m_orientedBoxRenderer && !m_fonts.empty()) {
      constexpr double minimumReadableTextHeightPixels = 10.0;
      const glm::dvec2 topLeftWorld = m_camera.screenToWorld(glm::dvec2(0.0, 0.0));
      const glm::dvec2 bottomRightWorld = m_camera.screenToWorld(glm::dvec2(viewportWidthPixels, viewportHeightPixels));
      const TextBounds visibleWorldBounds = expandBounds({
        .min = glm::dvec2(topLeftWorld.x, bottomRightWorld.y),
        .max = glm::dvec2(bottomRightWorld.x, topLeftWorld.y)
      }, 220.0 / std::max(m_camera.zoom(), 1e-6));

      m_orientedBoxRenderer->begin(m_camera);
      for (const LoadedFont &loadedFont: m_fonts) {
        m_textRenderer->begin(*loadedFont.font, m_camera);
        for (const auto &textEntity: m_textEntities) {
          if (textEntity.fontKey != loadedFont.key) {
            continue;
          }

          if (!intersects({
                .min = textEntity.worldBoundsMin,
                .max = textEntity.worldBoundsMax
              }, visibleWorldBounds)) {
            continue;
          }

          const double pixelHeight = textEntity.height * m_camera.zoom();
          if (pixelHeight < minimumReadableTextHeightPixels) {
            const std::array<glm::dvec2, 4> orientedCorners = Core::orientedTextCorners(
              textEntity.position,
              {
                .minimum = textEntity.localBoundsMin,
                .maximum = textEntity.localBoundsMax
              },
              textEntity.rotation
            );
            m_orientedBoxRenderer->draw(
              orientedCorners,
              glm::vec4(textEntity.color.r, textEntity.color.g, textEntity.color.b, textEntity.color.a * 0.55f)
            );
            continue;
          }

          m_textRenderer->draw(
            textEntity.text,
            textEntity.position,
            textEntity.height,
            textEntity.rotation,
            textEntity.color
          );
        }
        m_textRenderer->end();
      }
      m_orientedBoxRenderer->end();
    }

    m_context->swapBuffers(this);
  }

  QFunctionPointer Canvas::getProcAddress(const char *procName) {
    return QOpenGLContext::currentContext()->getProcAddress(procName);
  }

  void Canvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
      m_panning = true;
      const glm::dvec2 positionPixels = viewportPixels(event->position(), devicePixelRatio());
      m_lastMousePosition = QPointF(positionPixels.x, positionPixels.y);
      setCursor(QCursor(Qt::ClosedHandCursor));
      event->accept();
    } else {
      QWindow::mousePressEvent(event);
    }
  }

  void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
      m_panning = false;
      setCursor(QCursor(Qt::ArrowCursor));
      event->accept();
    } else {
      QWindow::mouseReleaseEvent(event);
    }
  }

  void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if (m_panning) {
      const glm::dvec2 positionPixels = viewportPixels(event->position(), devicePixelRatio());
      const QPointF currentMousePosition(positionPixels.x, positionPixels.y);
      const QPointF delta = currentMousePosition - m_lastMousePosition;
      m_camera.pan(glm::dvec2(-delta.x(), delta.y()));
      m_lastMousePosition = currentMousePosition;
      requestUpdate();
      event->accept();
    } else {
      QWindow::mouseMoveEvent(event);
    }
  }

  void Canvas::wheelEvent(QWheelEvent *event) {
    const float delta = event->angleDelta().y() / 105.0f;
    const glm::dvec2 mouseScreenPixels = viewportPixels(event->position(), devicePixelRatio());
    const glm::dvec2 mouseWorld = m_camera.screenToWorld(mouseScreenPixels);
    m_camera.zoom(std::pow(1.1f, delta), mouseWorld);

    requestUpdate();
    event->accept();
  }

  void Canvas::resizeEvent(QResizeEvent *event) {
    const qreal devicePixelRatioValue = devicePixelRatio();
    const int viewportWidthPixels = viewportPixels(event->size().width(), devicePixelRatioValue);
    const int viewportHeightPixels = viewportPixels(event->size().height(), devicePixelRatioValue);

    if (m_hasInitializedCameraViewport) {
      m_camera.resizePreserveViewportOrigin(viewportWidthPixels, viewportHeightPixels);
    } else {
      m_camera.resize(viewportWidthPixels, viewportHeightPixels);
      m_hasInitializedCameraViewport = true;
    }

    requestUpdate();
    QWindow::resizeEvent(event);
  }
} // Ui
// Qadra
