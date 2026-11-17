// main.cpp
#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <QDir>
#include "mainclient.h"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // App meta info
    app.setApplicationName("Container Application");
    app.setApplicationVersion("1.0");

    // ----- First Splash Screen -----
    QString splashImagePath1 = ":/icons/splashFirstState.png";
    QPixmap pixmap1(splashImagePath1);

    if (pixmap1.isNull()) {
        // Fallback if image not found
        pixmap1 = QPixmap(225, 225);
        pixmap1.fill(Qt::darkGray);
    } else {
        pixmap1 = pixmap1.scaled(225, 225, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QSplashScreen splash1(pixmap1);
    splash1.show();
    QTimer::singleShot(1200, &splash1, &QSplashScreen::close); // Show for 1.2 sec

    // ----- Second Splash Screen -----
    QString splashImagePath = ":/icons/splashMiddleState.png";
    QPixmap pixmap(splashImagePath);

    if (pixmap.isNull()) {
        pixmap = QPixmap(225, 225);
        pixmap.fill(Qt::darkGray);
    }

    QSplashScreen splash(pixmap);
    splash.show();
    QTimer::singleShot(1400, &splash, &QSplashScreen::close); // Show for 1.4 sec

    // ----- Third Splash Screen -----
    QString splashImagePath2 = ":/icons/splashFinalState.png";
    QPixmap pixmap2(splashImagePath2);

    if (pixmap2.isNull()) {
        pixmap2 = QPixmap(225, 225);
        pixmap2.fill(Qt::darkGray);
    } else {
        pixmap2 = pixmap2.scaled(225, 225, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QSplashScreen splash2(pixmap2);
    splash2.show();
    QTimer::singleShot(800, &splash2, &QSplashScreen::close); // Show for 0.8 sec

    // Launch the main window after splash screens
    MainClient window;
    QTimer::singleShot(1000, &window, &MainClient::show); // Show after 1 sec

    return app.exec(); // Start event loop
}
