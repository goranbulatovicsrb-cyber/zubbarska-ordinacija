#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QSplashScreen>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include "mainwindow.h"
#include "database.h"

// ── Splash screen helper ─────────────────────────────────────────────────────
static QSplashScreen* createSplash()
{
    QPixmap pix(500, 300);
    pix.fill(QColor("#1565C0"));

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // White rounded rect
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255,255,255,30));
    p.drawRoundedRect(20, 20, 460, 260, 12, 12);

    // Title
    p.setPen(Qt::white);
    QFont titleFont("Segoe UI", 36, QFont::Bold);
    p.setFont(titleFont);
    p.drawText(QRect(0, 60, 500, 80), Qt::AlignCenter, "🦷 DentaPro");

    QFont subFont("Segoe UI", 14);
    p.setFont(subFont);
    p.setPen(QColor(255,255,255,200));
    p.drawText(QRect(0, 150, 500, 40), Qt::AlignCenter, "Sistem za upravljanje zubarskom ordinacijom");

    QFont verFont("Segoe UI", 10);
    p.setFont(verFont);
    p.setPen(QColor(255,255,255,150));
    p.drawText(QRect(0, 240, 500, 30), Qt::AlignCenter, "v1.0 — Učitavanje...");

    return new QSplashScreen(pix);
}

// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("DentaPro");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("DentaPro");

    // High-DPI support
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    // Splash
    auto* splash = createSplash();
    splash->show();
    app.processEvents();

    // Initialize database
    if (!Database::instance().initialize()) {
        QMessageBox::critical(nullptr, "Greška baze podataka",
            "Nije moguće pokrenuti bazu podataka.\n"
            "Provjerite dozvole za pisanje.");
        return 1;
    }

    // Short delay for splash
    QTimer::singleShot(1500, [&](){
        MainWindow* window = new MainWindow();
        window->show();
        splash->finish(window);
        splash->deleteLater();
    });

    return app.exec();
}
