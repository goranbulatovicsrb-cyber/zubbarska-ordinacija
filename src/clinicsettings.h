#pragma once
#include <QString>
#include <QPixmap>

// Singleton that holds clinic branding settings
// persisted via QSettings (registry on Windows)
class ClinicSettings
{
public:
    static ClinicSettings& instance();

    // Load from persistent storage
    void load();
    // Save to persistent storage
    void save();

    // ── Fields ──────────────────────────────────────────────────────────────
    QString clinicName;
    QString clinicSubtitle;   // e.g. "Stomatološka ordinacija"
    QString clinicAddress;
    QString clinicPhone;
    QString clinicEmail;
    QString logoPath;         // absolute path to logo image file

    // Cached pixmap (loaded on demand)
    QPixmap logo() const;
    bool    hasLogo() const;

private:
    ClinicSettings() = default;
};
