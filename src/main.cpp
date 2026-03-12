#include <QApplication>
#include <QMessageBox>
#include <QSplashScreen>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QSqlDatabase>
#include "mainwindow.h"
#include "database.h"

static QSplashScreen* createSplash()
{
    QPixmap pix(480, 280);
    pix.fill(QColor("#1565C0"));

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255,255,255,25));
    p.drawRoundedRect(16,16,448,248,12,12);

    p.setPen(Qt::white);
    QFont f("Segoe UI", 34, QFont::Bold);
    p.setFont(f);
    p.drawText(QRect(0,60,480,80), Qt::AlignCenter, "DentaPro");

    QFont f2("Segoe UI", 13);
    p.setFont(f2);
    p.setPen(QColor(255,255,255,200));
    p.drawText(QRect(0,148,480,36), Qt::AlignCenter,
               "Sistem za upravljanje zubarskom ordinacijom");

    QFont f3("Segoe UI", 10);
    p.setFont(f3);
    p.setPen(QColor(255,255,255,140));
    p.drawText(QRect(0,230,480,30), Qt::AlignCenter, "v1.0 - Ucitavanje...");

    return new QSplashScreen(pix);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("DentaPro");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("DentaPro");

    QSplashScreen* splash = createSplash();
    splash->show();
    app.processEvents();

    // Check SQLite driver
    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        splash->hide();
        QMessageBox::critical(nullptr, "Greska - SQLite nedostaje",
            "SQLite driver nije pronadjen!\n\n"
            "Provjerite da folder 'sqldrivers' postoji pored DentaPro.exe\n"
            "i da sadrzi 'qsqlite.dll'.");
        return 1;
    }

    // Init database
    if (!Database::instance().initialize()) {
        splash->hide();
        QMessageBox::critical(nullptr, "Greska baze podataka",
            "Nije moguce pokrenuti bazu podataka.\n"
            "Provjerite dozvole za pisanje u AppData folder.");
        return 1;
    }

    MainWindow* window = nullptr;
    QTimer::singleShot(1200, [&](){
        window = new MainWindow();
        window->show();
        splash->finish(window);
        splash->deleteLater();
    });

    return app.exec();
}
