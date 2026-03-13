#include <QApplication>
#include <QMessageBox>
#include <QSplashScreen>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QSqlDatabase>
#include <QSystemTrayIcon>
#include <QFile>
#include "mainwindow.h"
#include "database.h"
#include "clinicsettings.h"
#include "exportmanager.h"

static QSplashScreen* createSplash()
{
    auto& cs = ClinicSettings::instance();

    const int W = 520, H = 300;
    QPixmap pix(W, H);
    pix.fill(QColor("#1565C0"));

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // Background card
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 20));
    p.drawRoundedRect(16, 16, W-32, H-32, 14, 14);

    // Decorative circles
    p.setBrush(QColor(255, 255, 255, 10));
    p.drawEllipse(W-120, H-120, 200, 200);
    p.setBrush(QColor(255, 255, 255, 8));
    p.drawEllipse(W-80, H-80, 140, 140);

    int contentY = 50;

    // Logo (ako postoji)
    if (cs.hasLogo()) {
        QPixmap logo = cs.logo();
        if (!logo.isNull()) {
            QPixmap scaled = logo.scaled(200, 80,
                Qt::KeepAspectRatio, Qt::SmoothTransformation);
            int lx = (W - scaled.width()) / 2;
            p.drawPixmap(lx, contentY, scaled);
            contentY += scaled.height() + 16;
        }
    }

    // Naziv ordinacije
    QString name = cs.clinicName.isEmpty() ? "DentaPro" : cs.clinicName;
    int fontSize = 32;
    if      (name.length() > 25) fontSize = 20;
    else if (name.length() > 18) fontSize = 24;
    else if (name.length() > 12) fontSize = 28;

    p.setPen(Qt::white);
    QFont fName("Segoe UI", fontSize, QFont::Bold);
    p.setFont(fName);
    p.drawText(QRect(24, contentY, W-48, fontSize+16), Qt::AlignCenter, name);
    contentY += fontSize + 20;

    // Podnaslov
    QString sub = cs.clinicSubtitle.isEmpty()
                  ? "Stomatoloska ordinacija"
                  : cs.clinicSubtitle;
    p.setPen(QColor(255, 255, 255, 210));
    QFont fSub("Segoe UI", 12);
    p.setFont(fSub);
    p.drawText(QRect(24, contentY, W-48, 28), Qt::AlignCenter, sub);
    contentY += 32;

    // Adresa i telefon
    QStringList contact;
    if (!cs.clinicAddress.isEmpty()) contact << cs.clinicAddress;
    if (!cs.clinicPhone.isEmpty())   contact << cs.clinicPhone;
    if (!contact.isEmpty()) {
        p.setPen(QColor(255, 255, 255, 160));
        QFont fC("Segoe UI", 9);
        p.setFont(fC);
        p.drawText(QRect(24, contentY, W-48, 20),
                   Qt::AlignCenter, contact.join("   |   "));
    }

    // Ucitavanje tekst
    p.setPen(QColor(255, 255, 255, 120));
    QFont fLoad("Segoe UI", 9);
    p.setFont(fLoad);
    p.drawText(QRect(0, H-34, W, 20), Qt::AlignCenter, "Ucitavanje...");

    // Progress bar
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 40));
    p.drawRoundedRect(40, H-16, W-80, 6, 3, 3);
    p.setBrush(QColor(255, 255, 255, 180));
    p.drawRoundedRect(40, H-16, (W-80)/2, 6, 3, 3);

    return new QSplashScreen(pix);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("DentaPro");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("DentaPro");

    // PRVO ucitaj postavke, TEK ONDA pravi splash
    ClinicSettings::instance().load();

    QSplashScreen* splash = createSplash();
    splash->show();
    app.processEvents();

    // Provjeri SQLite
    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        splash->hide();
        QMessageBox::critical(nullptr, "Greska - SQLite nedostaje",
            "SQLite driver nije pronadjen!\n\n"
            "Provjerite da folder 'sqldrivers' postoji pored DentaPro.exe\n"
            "i da sadrzi 'qsqlite.dll'.");
        return 1;
    }

    // Init baze
    if (!Database::instance().initialize()) {
        splash->hide();
        QMessageBox::critical(nullptr, "Greska baze podataka",
            "Nije moguce pokrenuti bazu podataka.\n"
            "Provjerite dozvole za pisanje u AppData folder.");
        return 1;
    }

    // Auto-backup
    ExportManager::autoBackup();

    // System tray
    auto& cs = ClinicSettings::instance();
    QSystemTrayIcon* tray = nullptr;
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        tray = new QSystemTrayIcon(&app);
        tray->setToolTip(cs.clinicName.isEmpty() ? "DentaPro" : cs.clinicName);
        tray->show();
    }

    MainWindow* window = nullptr;
    QTimer::singleShot(1600, [&](){
        window = new MainWindow();
        window->show();
        splash->finish(window);
        splash->deleteLater();

        if (tray) {
            auto todayApts = Database::instance().getTodayAppointments();
            if (!todayApts.isEmpty()) {
                tray->showMessage(
                    cs.clinicName + " - Termini danas",
                    QString("Imate %1 termin(a) zakazano danas.")
                        .arg(todayApts.size()),
                    QSystemTrayIcon::Information, 5000);
            }
        }
    });

    return app.exec();
}
