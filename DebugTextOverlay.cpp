#include "DebugTextOverlay.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

#include "TextLayout.hpp"

namespace {
  struct ScenePrototypeSeed {
    const char *fontKey;
    const char *fallbackFontKey;
    std::string text;
    double height;
    double rotation;
    glm::vec4 color;
    double padding;
  };

  struct OverlayBounds {
    glm::dvec2 minimum{0.0};
    glm::dvec2 maximum{0.0};
  };

  struct ScenePrototype {
    std::string fontKey;
    std::string text;
    double height{};
    double rotation{};
    glm::vec4 color{1.0f};
    double padding{};
    OverlayBounds relativeBounds;
    glm::dvec2 relativeCenter{0.0};
  };

  struct SpatialHash {
    explicit SpatialHash(const double cellSize)
      : m_cellSize(cellSize) {
    }

    [[nodiscard]] bool overlaps(const OverlayBounds &bounds) const {
      const glm::ivec2 minimumCell = cellCoordinates(bounds.minimum);
      const glm::ivec2 maximumCell = cellCoordinates(bounds.maximum);

      for (int cellX = minimumCell.x; cellX <= maximumCell.x; ++cellX) {
        for (int cellY = minimumCell.y; cellY <= maximumCell.y; ++cellY) {
          const auto iterator = m_cells.find(cellKey(cellX, cellY));
          if (iterator == m_cells.end()) {
            continue;
          }

          for (const OverlayBounds &occupiedBounds: iterator->second) {
            if (bounds.minimum.x < occupiedBounds.maximum.x &&
                bounds.maximum.x > occupiedBounds.minimum.x &&
                bounds.minimum.y < occupiedBounds.maximum.y &&
                bounds.maximum.y > occupiedBounds.minimum.y) {
              return true;
            }
          }
        }
      }

      return false;
    }

    void insert(const OverlayBounds &bounds) {
      const glm::ivec2 minimumCell = cellCoordinates(bounds.minimum);
      const glm::ivec2 maximumCell = cellCoordinates(bounds.maximum);

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
    std::unordered_map<std::uint64_t, std::vector<OverlayBounds>> m_cells;
  };

  [[nodiscard]] std::string utf8String(const char8_t *text) {
    return {
      reinterpret_cast<const char *>(text),
      std::char_traits<char8_t>::length(text)
    };
  }

  [[nodiscard]] OverlayBounds expandBounds(const OverlayBounds &bounds, const double padding) {
    return {
      .minimum = bounds.minimum - glm::dvec2(padding, padding),
      .maximum = bounds.maximum + glm::dvec2(padding, padding)
    };
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

  [[nodiscard]] Qadra::Core::Font *findFont(std::span<const Qadra::Ui::DebugTextOverlay::FontEntry> fonts,
                                            const std::string_view fontKey) {
    const auto iterator = std::find_if(fonts.begin(), fonts.end(), [&](const auto &fontEntry) {
      return fontEntry.key == fontKey;
    });

    return iterator != fonts.end() ? iterator->font : nullptr;
  }

  constexpr std::size_t minimumShowcaseEntityCount = 50000;
}

namespace Qadra::Ui {
  void DebugTextOverlay::rebuild(std::span<const FontEntry> fonts) {
    clear();
    if (fonts.empty()) {
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
        if (findFont(fonts, seed.fontKey)) {
          return seed.fontKey;
        }
        if (seed.fallbackFontKey && *seed.fallbackFontKey && findFont(fonts, seed.fallbackFontKey)) {
          return seed.fallbackFontKey;
        }
        return {};
      }();

      if (resolvedFontKey.empty()) {
        continue;
      }

      Qadra::Core::Font *font = findFont(fonts, resolvedFontKey);
      if (!font) {
        continue;
      }

      const Qadra::Core::TextEntityBuildInput input{
        .heightWorld = seed.height,
        .rotationRadians = seed.rotation,
        .content = seed.text,
        .fontKey = resolvedFontKey,
        .color = seed.color,
      };
      const Qadra::Core::TextEntityData prototypeEntity = Qadra::Core::buildTextEntityData(0, input, *font);

      prototypes.push_back({
        .fontKey = resolvedFontKey,
        .text = seed.text,
        .height = seed.height,
        .rotation = seed.rotation,
        .color = seed.color,
        .padding = seed.padding,
        .relativeBounds = {
          .minimum = prototypeEntity.localBounds.minimum,
          .maximum = prototypeEntity.localBounds.maximum
        },
        .relativeCenter = (prototypeEntity.localBounds.minimum + prototypeEntity.localBounds.maximum) * 0.5,
      });
    }

    if (prototypes.empty()) {
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
        const Qadra::Core::TextLayoutBounds localBounds{
          .minimum = prototype.relativeBounds.minimum,
          .maximum = prototype.relativeBounds.maximum
        };
        const std::array<glm::dvec2, 4> orientedCorners = Qadra::Core::orientedTextCorners(
          worldPosition,
          localBounds,
          prototype.rotation
        );
        const Qadra::Core::TextLayoutBounds worldAxisAlignedBounds = Qadra::Core::axisAlignedBounds(orientedCorners);
        const OverlayBounds worldBounds{
          .minimum = worldAxisAlignedBounds.minimum,
          .maximum = worldAxisAlignedBounds.maximum
        };
        const OverlayBounds occupiedWorldBounds = expandBounds(worldBounds, prototype.padding);

        if (!occupiedBounds.overlaps(occupiedWorldBounds)) {
          Qadra::Core::Font *font = findFont(fonts, prototype.fontKey);
          if (!font) {
            break;
          }

          const float tint = static_cast<float>(0.86 + 0.18 * halton(sequenceValue, 11));
          const float alpha = static_cast<float>(0.84 + 0.12 * halton(sequenceValue, 13));

          const Qadra::Core::TextEntityBuildInput input{
            .position = worldPosition,
            .heightWorld = prototype.height,
            .rotationRadians = prototype.rotation,
            .content = prototype.text,
            .fontKey = prototype.fontKey,
            .color = glm::vec4(
              std::clamp(prototype.color.r * tint, 0.0f, 1.0f),
              std::clamp(prototype.color.g * tint, 0.0f, 1.0f),
              std::clamp(prototype.color.b * tint, 0.0f, 1.0f),
              std::clamp(prototype.color.a * alpha, 0.0f, 1.0f)
            )
          };
          m_textEntities.push_back(Qadra::Core::buildTextEntityData(0, input, *font));
          occupiedBounds.insert(occupiedWorldBounds);
          placed = true;
        }

        ++placementAttempt;
      }
    }
  }

  void DebugTextOverlay::clear() {
    m_textEntities.clear();
  }
} // Qadra::Ui
