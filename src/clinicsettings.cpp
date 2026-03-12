#include "clinicsettings.h"
#include <QSettings>
#include <QPixmap>
#include <QFile>

ClinicSettings& ClinicSettings::instance()
{
    static ClinicSettings s;
    return s;
}

void ClinicSettings::load()
{
    QSettings s("DentaPro", "DentaPro");
    s.beginGroup("Clinic");
    clinicName     = s.value("name",     "Moja Zubarska Ordinacija").toString();
    clinicSubtitle = s.value("subtitle", "Stomatoloska ordinacija").toString();
    clinicAddress  = s.value("address",  "").toString();
    clinicPhone    = s.value("phone",    "").toString();
    clinicEmail    = s.value("email",    "").toString();
    logoPath       = s.value("logoPath", "").toString();
    s.endGroup();
}

void ClinicSettings::save()
{
    QSettings s("DentaPro", "DentaPro");
    s.beginGroup("Clinic");
    s.setValue("name",     clinicName);
    s.setValue("subtitle", clinicSubtitle);
    s.setValue("address",  clinicAddress);
    s.setValue("phone",    clinicPhone);
    s.setValue("email",    clinicEmail);
    s.setValue("logoPath", logoPath);
    s.endGroup();
}

QPixmap ClinicSettings::logo() const
{
    if (logoPath.isEmpty() || !QFile::exists(logoPath))
        return {};
    QPixmap px(logoPath);
    return px;
}

bool ClinicSettings::hasLogo() const
{
    return !logoPath.isEmpty() && QFile::exists(logoPath);
}
