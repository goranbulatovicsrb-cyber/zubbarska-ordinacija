#pragma once
#include <QMainWindow>
#include "models.h"

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QStackedWidget;
class QLabel;
class QTabWidget;
class QTableWidget;
class QDateEdit;
class QComboBox;
class PatientHistoryWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    // Patients
    void onAddPatient();
    void onEditPatient();
    void onDeletePatient();
    void onPatientSelected(QListWidgetItem* item);
    void onSearchPatients(const QString& text);

    // Interventions
    void onAddIntervention();
    void onEditIntervention(int id);
    void onDeleteIntervention(int id);

    // History/PDF
    void onViewPdf(const QString& path);

    // Reports
    void onPrintPatientReport();
    void onPrintIntervention();

    // Search tab
    void onSearch();
    void onSearchResultDoubleClicked(int row, int col);

    // Dashboard
    void refreshDashboard();

    // Appointments
    void onAddAppointment();

private:
    void setupUi();
    void setupMenu();
    void setupToolbar();
    void setupLeftPanel();
    void setupRightPanel();
    void setupDashboard();
    void setupPatientsTab();
    void setupSearchTab();
    void setupAppointmentsTab();

    void loadPatientList();
    void selectPatientById(int id);
    void refreshPatientDetail();
    void showPatientCard(const Patient& p);

    // ── UI refs ───────────────────────────────────────────────────────────────
    QListWidget*           m_patientList        = nullptr;
    QLineEdit*             m_searchBox          = nullptr;
    QStackedWidget*        m_rightStack         = nullptr;

    // welcome / dashboard
    QWidget*               m_dashWidget         = nullptr;
    QLabel*                m_statPatients       = nullptr;
    QLabel*                m_statIvMonth        = nullptr;
    QLabel*                m_statRevMonth       = nullptr;
    QTableWidget*          m_upcomingTable      = nullptr;

    // patient detail area
    QWidget*               m_detailWidget       = nullptr;
    QTabWidget*            m_detailTabs         = nullptr;

    // Overview tab
    QLabel*                m_patientNameLbl     = nullptr;
    QLabel*                m_patientInfoLbl     = nullptr;
    QTableWidget*          m_allIvTable         = nullptr;

    // History tab
    PatientHistoryWidget*  m_historyWidget      = nullptr;

    // Search tab
    QWidget*               m_searchWidget       = nullptr;
    QLineEdit*             m_searchName         = nullptr;
    QDateEdit*             m_searchFrom         = nullptr;
    QDateEdit*             m_searchTo           = nullptr;
    QComboBox*             m_searchType         = nullptr;
    QTableWidget*          m_searchResults      = nullptr;
    QList<Intervention>    m_searchIvs;

    // Appointments tab
    QWidget*               m_appointmentsWidget = nullptr;
    QTableWidget*          m_appointTable       = nullptr;

    // State
    Patient                m_currentPatient;
};
