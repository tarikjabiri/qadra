#include <algorithm>

#include <QApplication>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QPalette>
#include <QProgressBar>
#include <QScreen>
#include <QStyleHints>
#include <QSvgWidget>
#include <QTimer>

#include "Window.hpp"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  bool isDarkTheme = app.styleHints()->colorScheme() == Qt::ColorScheme::Dark;
  if (app.styleHints()->colorScheme() == Qt::ColorScheme::Unknown) {
    isDarkTheme = app.palette().color(QPalette::Window).lightness() < 128;
  }

  const QString splashPath = isDarkTheme
    ? ":/assets/qadra-splash-dark.svg"
    : ":/assets/qadra-splash-light.svg";

  QSvgWidget splash(splashPath);
  splash.setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  splash.setFixedSize(640, 480);

  QProgressBar bar(&splash);
  bar.setGeometry(200, 425, 240, 4);
  bar.setRange(0, 0);
  bar.setTextVisible(false);
  bar.setStyleSheet(isDarkTheme
    ? "QProgressBar { background: rgba(148, 163, 184, 0.22); border: none; border-radius: 2px; }"
      "QProgressBar::chunk { background: #38BDF8; border-radius: 2px; }"
    : "QProgressBar { background: rgba(14, 116, 144, 0.16); border: none; border-radius: 2px; }"
      "QProgressBar::chunk { background: #0369A1; border-radius: 2px; }");

  if (QScreen *screen = QGuiApplication::primaryScreen()) {
    splash.move(screen->availableGeometry().center() - splash.rect().center());
  }

  splash.show();
  app.processEvents();

  QElapsedTimer splashTimer;
  splashTimer.start();

  Window w;

  constexpr int minimumSplashDurationMilliseconds = 3900;
  const int remainingSplashDurationMilliseconds = std::max(
    0,
    minimumSplashDurationMilliseconds - static_cast<int>(splashTimer.elapsed())
  );

  QTimer::singleShot(remainingSplashDurationMilliseconds, [&w, &splash]() {
    w.show();
    splash.close();
  });

  return app.exec();
}
