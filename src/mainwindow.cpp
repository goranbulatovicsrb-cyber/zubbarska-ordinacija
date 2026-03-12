#include "mainwindow.h"
#include "database.h"
#include "patientdialog.h"
#include "interventiondialog.h"
#include "patienthistorywidget.h"
#include "reportgenerator.h"

#include <QApplication>
#include <QSplitter>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLabel>
#include <QDateEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QToolBar>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QDialog>
#include <QDateTimeEdit>
#include <QTextEdit>
#include <QFormLayout>
#include <QScrollArea>
#include <QFrame>
#include <QSizePolicy>

// ── Helpers ───────────────────────────────────────────────────────────────────

static QTableWidgetItem* tItem(const QString& text, bool center=false) {
    auto* i = new QTableWidgetItem(text);
    i->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    if (center) i->setTextAlignment(Qt::AlignCenter);
    return i;
}

// ── Constructor ────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("DentaPro — Sistem za upravljanje zubarskom ordinacijom");
    setMinimumSize(1100, 720);
    resize(1300, 820);

    setupUi();
    setupMenu();
    loadPatientList();
    refreshDashboard();

    statusBar()->showMessage("DentaPro spreman za rad  🦷");
}

// ── UI Setup ──────────────────────────────────────────────────────────────────

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* mainLay = new QHBoxLayout(central);
    mainLay->setContentsMargins(0,0,0,0);
    mainLay->setSpacing(0);

    // App-wide stylesheet
    setStyleSheet(R"(
        QMainWindow { background:#F5F5F5; }
        QMenuBar { background:#1565C0; color:white; font-size:11px; }
        QMenuBar::item { padding:6px 14px; }
        QMenuBar::item:selected { background:#1976D2; }
        QMenu { background:white; color:#212121; border:1px solid #E0E0E0; }
        QMenu::item:selected { background:#E3F2FD; color:#1565C0; }
        QToolBar { background:#1565C0; border:none; spacing:4px; padding:2px 6px; }
        QStatusBar { background:#F5F5F5; color:#757575; font-size:10px; border-top:1px solid #E0E0E0; }
        QSplitter::handle { background:#E0E0E0; }
        QScrollBar:vertical { border:none; background:#F5F5F5; width:8px; }
        QScrollBar::handle:vertical { background:#BDBDBD; border-radius:4px; min-height:30px; }
        QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical { height:0; }
    )");

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);
    mainLay->addWidget(splitter);

    setupLeftPanel();
    splitter->addWidget(findChild<QWidget*>("leftPanel"));
    setupRightPanel();
    splitter->addWidget(findChild<QWidget*>("rightPanel"));
    splitter->setSizes({280, 1020});

    setupMenu();
    setupToolbar();
}

void MainWindow::setupLeftPanel()
{
    auto* panel = new QWidget;
    panel->setObjectName("leftPanel");
    panel->setFixedWidth(280);
    panel->setStyleSheet("QWidget#leftPanel{background:#263238; border-right:1px solid #37474F;}");

    auto* lay = new QVBoxLayout(panel);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    // Logo area
    auto* logoWidget = new QWidget;
    logoWidget->setStyleSheet("background:#1565C0; padding:16px 12px;");
    auto* logoLay = new QVBoxLayout(logoWidget);
    logoLay->setContentsMargins(0,0,0,0);
    auto* logoTitle = new QLabel("🦷 DentaPro");
    logoTitle->setStyleSheet("color:white; font-size:20px; font-weight:bold;");
    auto* logoSub = new QLabel("Zubarska Ordinacija");
    logoSub->setStyleSheet("color:rgba(255,255,255,0.7); font-size:10px;");
    logoLay->addWidget(logoTitle);
    logoLay->addWidget(logoSub);
    lay->addWidget(logoWidget);

    // Search
    auto* searchWidget = new QWidget;
    searchWidget->setStyleSheet("background:#37474F; padding:8px;");
    auto* sLay = new QHBoxLayout(searchWidget);
    sLay->setContentsMargins(0,0,0,0);
    m_searchBox = new QLineEdit;
    m_searchBox->setPlaceholderText("🔍 Pretraži pacijente...");
    m_searchBox->setStyleSheet("QLineEdit{background:#455A64;border:none;border-radius:4px;"
                               "padding:6px 10px;color:white;font-size:11px;}"
                               "QLineEdit::placeholder{color:#90A4AE;}");
    sLay->addWidget(m_searchBox);
    lay->addWidget(searchWidget);

    // Patient list
    m_patientList = new QListWidget;
    m_patientList->setStyleSheet(R"(
        QListWidget { background:#263238; border:none; outline:none; }
        QListWidget::item { color:#CFD8DC; padding:10px 16px; border-bottom:1px solid #37474F;
                            font-size:12px; }
        QListWidget::item:selected { background:#1565C0; color:white; }
        QListWidget::item:hover:!selected { background:#37474F; }
    )");
    lay->addWidget(m_patientList, 1);

    // Add patient button
    auto* addBtn = new QPushButton("➕  Novi Pacijent");
    addBtn->setStyleSheet("QPushButton{background:#00695C;color:white;border:none;"
                          "padding:12px;font-size:12px;font-weight:bold;}"
                          "QPushButton:hover{background:#00897B;}");
    lay->addWidget(addBtn);

    connect(m_searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearchPatients);
    connect(m_patientList, &QListWidget::itemClicked, this, &MainWindow::onPatientSelected);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddPatient);
}

void MainWindow::setupRightPanel()
{
    auto* panel = new QWidget;
    panel->setObjectName("rightPanel");
    panel->setStyleSheet("background:#F5F5F5;");

    auto* lay = new QVBoxLayout(panel);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    m_rightStack = new QStackedWidget;
    lay->addWidget(m_rightStack);

    setupDashboard();
    setupPatientsTab();
    setupSearchTab();
    setupAppointmentsTab();

    m_rightStack->setCurrentIndex(0);
    m_rightStack->setObjectName("rightStack");
}

void MainWindow::setupDashboard()
{
    m_dashWidget = new QWidget;
    m_dashWidget->setStyleSheet("background:#F5F5F5;");
    auto* lay = new QVBoxLayout(m_dashWidget);
    lay->setContentsMargins(24,24,24,24);
    lay->setSpacing(20);

    // Welcome header
    auto* welcome = new QLabel("Dobrodošli u DentaPro 🦷");
    welcome->setStyleSheet("font-size:24px; font-weight:bold; color:#1565C0;");
    lay->addWidget(welcome);
    auto* sub = new QLabel("Sistem za upravljanje zubarskom ordinacijom");
    sub->setStyleSheet("font-size:12px; color:#757575; margin-top:-16px;");
    lay->addWidget(sub);

    // Stats row
    auto* statsRow = new QHBoxLayout;
    statsRow->setSpacing(16);

    auto makeStatCard = [&](const QString& icon, const QString& label,
                             QLabel*& valueLbl, const QString& bg) {
        auto* card = new QWidget;
        card->setStyleSheet(QString("QWidget{background:%1;border-radius:10px;}").arg(bg));
        auto* cl = new QVBoxLayout(card);
        cl->setContentsMargins(20,16,20,16);
        auto* il = new QLabel(icon);
        il->setStyleSheet("font-size:28px;");
        valueLbl = new QLabel("...");
        valueLbl->setStyleSheet("font-size:28px; font-weight:bold; color:white;");
        auto* ll = new QLabel(label);
        ll->setStyleSheet("font-size:11px; color:rgba(255,255,255,0.8);");
        cl->addWidget(il);
        cl->addWidget(valueLbl);
        cl->addWidget(ll);
        return card;
    };

    statsRow->addWidget(makeStatCard("👥","Ukupno pacijenata",m_statPatients,"#1565C0"));
    statsRow->addWidget(makeStatCard("🦷","Intervencije ovog mjeseca",m_statIvMonth,"#00695C"));
    statsRow->addWidget(makeStatCard("💰","Prihod ovog mjeseca (BAM)",m_statRevMonth,"#E65100"));
    lay->addLayout(statsRow);

    // Upcoming appointments
    auto* apTitle = new QLabel("📅  Nadolazeći termini (30 dana)");
    apTitle->setStyleSheet("font-size:15px; font-weight:bold; color:#1565C0;");
    lay->addWidget(apTitle);

    m_upcomingTable = new QTableWidget(0, 4);
    m_upcomingTable->setHorizontalHeaderLabels({"Pacijent","Datum/Vrijeme","Razlog","Status"});
    m_upcomingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_upcomingTable->setStyleSheet("QTableWidget{border:1px solid #E0E0E0;border-radius:6px;"
                                   "background:white;gridline-color:#F5F5F5;}"
                                   "QHeaderView::section{background:#1565C0;color:white;"
                                   "font-weight:bold;padding:8px;border:none;}");
    m_upcomingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_upcomingTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_upcomingTable->setAlternatingRowColors(true);
    m_upcomingTable->verticalHeader()->setVisible(false);
    lay->addWidget(m_upcomingTable);

    m_rightStack->addWidget(m_dashWidget); // index 0
}

void MainWindow::setupPatientsTab()
{
    m_detailWidget = new QWidget;
    m_detailWidget->setStyleSheet("background:#F5F5F5;");
    auto* lay = new QVBoxLayout(m_detailWidget);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    // Patient header bar
    auto* hBar = new QWidget;
    hBar->setStyleSheet("background:white; border-bottom:1px solid #E0E0E0;");
    auto* hLay = new QHBoxLayout(hBar);
    hLay->setContentsMargins(20,12,20,12);

    m_patientNameLbl = new QLabel("Odaberite pacijenta");
    m_patientNameLbl->setStyleSheet("font-size:20px; font-weight:bold; color:#1565C0;");
    m_patientInfoLbl = new QLabel("");
    m_patientInfoLbl->setStyleSheet("font-size:11px; color:#757575;");

    auto* nameCol = new QVBoxLayout;
    nameCol->addWidget(m_patientNameLbl);
    nameCol->addWidget(m_patientInfoLbl);
    hLay->addLayout(nameCol, 1);

    auto makeBtn = [](const QString& lbl, const QString& bg, const QString& hov) {
        auto* b = new QPushButton(lbl);
        b->setMinimumHeight(34);
        b->setStyleSheet(QString("QPushButton{background:%1;color:white;border-radius:4px;"
                                 "border:none;padding:0 14px;font-weight:bold;}"
                                 "QPushButton:hover{background:%2;}").arg(bg,hov));
        return b;
    };
    auto* editPatBtn  = makeBtn("✏️ Uredi","#1565C0","#1976D2");
    auto* delPatBtn   = makeBtn("🗑 Briši","#EF5350","#E53935");
    auto* addIvBtn    = makeBtn("➕ Nova Intervencija","#00695C","#00897B");
    auto* printBtn    = makeBtn("🖨 Štampaj Izvještaj","#E65100","#EF6C00");

    hLay->addWidget(editPatBtn);
    hLay->addWidget(delPatBtn);
    hLay->addSpacing(12);
    hLay->addWidget(addIvBtn);
    hLay->addWidget(printBtn);
    lay->addWidget(hBar);

    // Tabs
    m_detailTabs = new QTabWidget;
    m_detailTabs->setStyleSheet(R"(
        QTabWidget::pane { border:none; background:#F5F5F5; }
        QTabBar::tab { padding:10px 20px; font-size:11px; font-weight:bold;
                       background:#EEEEEE; color:#757575; border:none; margin-right:2px; }
        QTabBar::tab:selected { background:#F5F5F5; color:#1565C0;
                                border-bottom:3px solid #1565C0; }
        QTabBar::tab:hover:!selected { background:#E3F2FD; color:#1565C0; }
    )");
    lay->addWidget(m_detailTabs, 1);

    // ── Tab 1: Overview ───────────────────────────────────────────────────────
    auto* overviewWidget = new QWidget;
    overviewWidget->setStyleSheet("background:#F5F5F5;");
    auto* ovLay = new QVBoxLayout(overviewWidget);
    ovLay->setContentsMargins(20,16,20,16);
    ovLay->setSpacing(12);

    auto* ivTitle = new QLabel("🦷  Sve intervencije");
    ivTitle->setStyleSheet("font-size:15px; font-weight:bold; color:#00695C;");
    ovLay->addWidget(ivTitle);

    m_allIvTable = new QTableWidget(0, 6);
    m_allIvTable->setHorizontalHeaderLabels({"Datum","Vrsta","Zub","Doktor","Cijena","Status"});
    m_allIvTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_allIvTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_allIvTable->setStyleSheet("QTableWidget{border:1px solid #E0E0E0;border-radius:6px;"
                                "background:white;gridline-color:#F5F5F5;}"
                                "QHeaderView::section{background:#00695C;color:white;"
                                "font-weight:bold;padding:8px;border:none;}");
    m_allIvTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_allIvTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_allIvTable->setAlternatingRowColors(true);
    m_allIvTable->verticalHeader()->setVisible(false);
    ovLay->addWidget(m_allIvTable);

    m_detailTabs->addTab(overviewWidget, "📋 Pregled");

    // ── Tab 2: Calendar History ───────────────────────────────────────────────
    m_historyWidget = new PatientHistoryWidget;
    m_detailTabs->addTab(m_historyWidget, "📅 Historija");

    connect(m_historyWidget, &PatientHistoryWidget::editIntervention, this, &MainWindow::onEditIntervention);
    connect(m_historyWidget, &PatientHistoryWidget::deleteIntervention, this, &MainWindow::onDeleteIntervention);
    connect(m_historyWidget, &PatientHistoryWidget::viewPdf, this, &MainWindow::onViewPdf);

    // Connections
    connect(editPatBtn, &QPushButton::clicked, this, &MainWindow::onEditPatient);
    connect(delPatBtn,  &QPushButton::clicked, this, &MainWindow::onDeletePatient);
    connect(addIvBtn,   &QPushButton::clicked, this, &MainWindow::onAddIntervention);
    connect(printBtn,   &QPushButton::clicked, this, &MainWindow::onPrintPatientReport);

    m_rightStack->addWidget(m_detailWidget); // index 1
}

void MainWindow::setupSearchTab()
{
    m_searchWidget = new QWidget;
    m_searchWidget->setStyleSheet("background:#F5F5F5;");
    auto* lay = new QVBoxLayout(m_searchWidget);
    lay->setContentsMargins(24,24,24,24);
    lay->setSpacing(16);

    auto* title = new QLabel("🔍  Napredna Pretraga");
    title->setStyleSheet("font-size:22px; font-weight:bold; color:#1565C0;");
    lay->addWidget(title);

    // Filter row
    auto* filterCard = new QWidget;
    filterCard->setStyleSheet("background:white; border-radius:8px; border:1px solid #E0E0E0;");
    auto* filterLay = new QHBoxLayout(filterCard);
    filterLay->setContentsMargins(16,12,16,12);
    filterLay->setSpacing(16);

    auto fieldStyle = "QLineEdit,QDateEdit,QComboBox{border:1px solid #BDBDBD;border-radius:4px;"
                      "padding:4px 8px;min-height:30px;}"
                      "QLineEdit:focus,QDateEdit:focus,QComboBox:focus{border:2px solid #1565C0;}";

    m_searchName = new QLineEdit;
    m_searchName->setPlaceholderText("Ime ili prezime pacijenta");
    m_searchName->setMinimumWidth(180);
    m_searchName->setStyleSheet(fieldStyle);

    m_searchFrom = new QDateEdit(QDate::currentDate().addMonths(-6));
    m_searchFrom->setCalendarPopup(true);
    m_searchFrom->setDisplayFormat("dd.MM.yyyy");
    m_searchFrom->setStyleSheet(fieldStyle);

    m_searchTo = new QDateEdit(QDate::currentDate());
    m_searchTo->setCalendarPopup(true);
    m_searchTo->setDisplayFormat("dd.MM.yyyy");
    m_searchTo->setStyleSheet(fieldStyle);

    m_searchType = new QComboBox;
    m_searchType->addItems({"Sve vrste","Pregled","Vađenje zuba","Plomba","Devitalizacija",
                             "Protetika","Ortodoncija","Rendgen","Čišćenje kamenca","Implant"});
    m_searchType->setStyleSheet(fieldStyle);

    auto* searchBtn = new QPushButton("🔍 Pretraži");
    searchBtn->setMinimumHeight(34);
    searchBtn->setStyleSheet("QPushButton{background:#1565C0;color:white;border-radius:4px;"
                             "border:none;padding:0 20px;font-weight:bold;}"
                             "QPushButton:hover{background:#1976D2;}");

    auto* resetBtn = new QPushButton("↺ Reset");
    resetBtn->setMinimumHeight(34);
    resetBtn->setStyleSheet("QPushButton{background:white;color:#1565C0;border-radius:4px;"
                            "border:1px solid #1565C0;padding:0 12px;font-weight:bold;}"
                            "QPushButton:hover{background:#E3F2FD;}");

    filterLay->addWidget(new QLabel("Pacijent:"));
    filterLay->addWidget(m_searchName, 2);
    filterLay->addWidget(new QLabel("Od:"));
    filterLay->addWidget(m_searchFrom);
    filterLay->addWidget(new QLabel("Do:"));
    filterLay->addWidget(m_searchTo);
    filterLay->addWidget(new QLabel("Vrsta:"));
    filterLay->addWidget(m_searchType, 1);
    filterLay->addWidget(searchBtn);
    filterLay->addWidget(resetBtn);
    lay->addWidget(filterCard);

    // Results table
    m_searchResults = new QTableWidget(0, 7);
    m_searchResults->setHorizontalHeaderLabels(
        {"Pacijent","Datum","Vrsta","Zub","Doktor","Cijena","Status"});
    m_searchResults->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_searchResults->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_searchResults->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_searchResults->setStyleSheet("QTableWidget{border:1px solid #E0E0E0;border-radius:8px;"
                                   "background:white;gridline-color:#F5F5F5;}"
                                   "QHeaderView::section{background:#1565C0;color:white;"
                                   "font-weight:bold;padding:8px;border:none;}");
    m_searchResults->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_searchResults->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_searchResults->setAlternatingRowColors(true);
    m_searchResults->verticalHeader()->setVisible(false);
    lay->addWidget(m_searchResults, 1);

    auto* hint = new QLabel("💡 Dvaput kliknite na red da vidite detalje pacijenta");
    hint->setStyleSheet("font-size:10px; color:#9E9E9E;");
    lay->addWidget(hint);

    connect(searchBtn, &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(resetBtn, &QPushButton::clicked, [this](){
        m_searchName->clear(); m_searchResults->setRowCount(0); m_searchIvs.clear();
    });
    connect(m_searchResults, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onSearchResultDoubleClicked);

    m_rightStack->addWidget(m_searchWidget); // index 2
}

void MainWindow::setupAppointmentsTab()
{
    m_appointmentsWidget = new QWidget;
    m_appointmentsWidget->setStyleSheet("background:#F5F5F5;");
    auto* lay = new QVBoxLayout(m_appointmentsWidget);
    lay->setContentsMargins(24,24,24,24);
    lay->setSpacing(16);

    auto* hLay = new QHBoxLayout;
    auto* title = new QLabel("📅  Zakazani Termini");
    title->setStyleSheet("font-size:22px; font-weight:bold; color:#1565C0;");
    hLay->addWidget(title, 1);
    auto* addApBtn = new QPushButton("➕ Zakaži Termin");
    addApBtn->setStyleSheet("QPushButton{background:#00695C;color:white;border-radius:4px;"
                            "border:none;padding:8px 16px;font-weight:bold;}"
                            "QPushButton:hover{background:#00897B;}");
    hLay->addWidget(addApBtn);
    lay->addLayout(hLay);

    m_appointTable = new QTableWidget(0, 5);
    m_appointTable->setHorizontalHeaderLabels({"Pacijent","Datum/Vrijeme","Razlog","Status","Akcija"});
    m_appointTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_appointTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_appointTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_appointTable->setStyleSheet("QTableWidget{border:1px solid #E0E0E0;border-radius:8px;"
                                  "background:white;gridline-color:#F5F5F5;}"
                                  "QHeaderView::section{background:#00695C;color:white;"
                                  "font-weight:bold;padding:8px;border:none;}");
    m_appointTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_appointTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_appointTable->setAlternatingRowColors(true);
    m_appointTable->verticalHeader()->setVisible(false);
    lay->addWidget(m_appointTable, 1);

    connect(addApBtn, &QPushButton::clicked, this, &MainWindow::onAddAppointment);

    m_rightStack->addWidget(m_appointmentsWidget); // index 3
}

// ── Menu & Toolbar ────────────────────────────────────────────────────────────

void MainWindow::setupMenu()
{
    auto* mb = menuBar();
    mb->setStyleSheet("QMenuBar{background:#1565C0;color:white;font-size:11px;}"
                      "QMenuBar::item{padding:6px 14px;}"
                      "QMenuBar::item:selected{background:#1976D2;}");

    auto* fileMenu = mb->addMenu("📁 Fajl");
    fileMenu->addAction("Novi pacijent", this, &MainWindow::onAddPatient, QKeySequence::New);
    fileMenu->addSeparator();
    fileMenu->addAction("Izlaz", qApp, &QApplication::quit, QKeySequence::Quit);

    auto* viewMenu = mb->addMenu("🖥 Prikaz");
    viewMenu->addAction("Dashboard", [this](){ m_rightStack->setCurrentIndex(0); refreshDashboard(); });
    viewMenu->addAction("Pacijenti",  [this](){ if (m_currentPatient.id > 0) m_rightStack->setCurrentIndex(1); });
    viewMenu->addAction("Pretraga",   [this](){ m_rightStack->setCurrentIndex(2); });
    viewMenu->addAction("Termini",    [this](){ m_rightStack->setCurrentIndex(3); refreshDashboard(); });

    auto* reportMenu = mb->addMenu("🖨 Izvještaji");
    reportMenu->addAction("Štampaj izvještaj pacijenta", this, &MainWindow::onPrintPatientReport);

    auto* helpMenu = mb->addMenu("❓ Pomoć");
    helpMenu->addAction("O programu", [this](){
        QMessageBox::about(this, "O DentaPro",
            "<b>DentaPro v1.0</b><br/>Sistem za upravljanje zubarskom ordinacijom<br/><br/>"
            "Razvijeno u C++ / Qt<br/>"
            "<small>© 2025 DentaPro</small>");
    });
}

void MainWindow::setupToolbar()
{
    auto* tb = addToolBar("Toolbar");
    tb->setMovable(false);
    tb->setIconSize(QSize(20,20));

    auto addTbBtn = [&](const QString& lbl, const QString& tip, auto slot) {
        auto* btn = new QPushButton(lbl);
        btn->setToolTip(tip);
        btn->setStyleSheet("QPushButton{background:rgba(255,255,255,0.15);color:white;"
                           "border:1px solid rgba(255,255,255,0.3);border-radius:4px;"
                           "padding:5px 12px;font-size:11px;font-weight:bold;}"
                           "QPushButton:hover{background:rgba(255,255,255,0.25);}");
        connect(btn, &QPushButton::clicked, this, slot);
        tb->addWidget(btn);
    };

    addTbBtn("🏠 Dashboard","Početna stranica", &MainWindow::refreshDashboard);
    tb->addSeparator();
    addTbBtn("👤 Novi Pacijent","Dodaj novog pacijenta", &MainWindow::onAddPatient);
    addTbBtn("🦷 Nova Intervencija","Dodaj intervenciju", &MainWindow::onAddIntervention);
    tb->addSeparator();
    addTbBtn("🔍 Pretraga","Napredna pretraga", [this](){ m_rightStack->setCurrentIndex(2); });
    addTbBtn("📅 Termini","Zakazani termini", [this](){ m_rightStack->setCurrentIndex(3); refreshDashboard(); });
    tb->addSeparator();
    addTbBtn("🖨 Štampaj","Štampaj izvještaj", &MainWindow::onPrintPatientReport);

    // Spacer
    auto* spacer = new QWidget; spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setStyleSheet("background:transparent;");
    tb->addWidget(spacer);

    auto* clockLbl = new QLabel(QDate::currentDate().toString("dddd, dd. MMMM yyyy."));
    clockLbl->setStyleSheet("color:rgba(255,255,255,0.8); font-size:10px; padding:0 12px;");
    tb->addWidget(clockLbl);
}

// ── Load / Refresh ────────────────────────────────────────────────────────────

void MainWindow::loadPatientList()
{
    m_patientList->clear();
    auto patients = m_searchBox->text().isEmpty()
                    ? Database::instance().getAllPatients()
                    : Database::instance().searchPatients(m_searchBox->text());

    for (const auto& p : patients) {
        auto* item = new QListWidgetItem("👤 " + p.fullName());
        item->setData(Qt::UserRole, p.id);
        m_patientList->addItem(item);
    }
    statusBar()->showMessage(QString("Pacijenata: %1").arg(patients.size()));
}

void MainWindow::refreshDashboard()
{
    m_statPatients->setText(QString::number(Database::instance().totalPatients()));
    m_statIvMonth->setText(QString::number(Database::instance().totalInterventionsThisMonth()));
    m_statRevMonth->setText(QString::number(Database::instance().totalRevenueThisMonth(), 'f', 2));

    auto appointments = Database::instance().getUpcomingAppointments(30);
    m_upcomingTable->setRowCount(appointments.size());
    for (int i = 0; i < appointments.size(); ++i) {
        const auto& ap = appointments[i];
        auto p = Database::instance().getPatient(ap.patientId);
        QString dt = ap.dateTime.length() >= 16 ?
                     QDateTime::fromString(ap.dateTime,"yyyy-MM-dd HH:mm").toString("dd.MM.yyyy HH:mm")
                     : ap.dateTime;
        m_upcomingTable->setItem(i,0,tItem(p.fullName()));
        m_upcomingTable->setItem(i,1,tItem(dt,true));
        m_upcomingTable->setItem(i,2,tItem(ap.reason));
        m_upcomingTable->setItem(i,3,tItem(ap.status,true));
    }

    m_rightStack->setCurrentIndex(0);
}

void MainWindow::selectPatientById(int id)
{
    for (int i = 0; i < m_patientList->count(); ++i) {
        if (m_patientList->item(i)->data(Qt::UserRole).toInt() == id) {
            m_patientList->setCurrentRow(i);
            break;
        }
    }
}

void MainWindow::refreshPatientDetail()
{
    if (m_currentPatient.id == 0) return;
    m_currentPatient = Database::instance().getPatient(m_currentPatient.id);
    showPatientCard(m_currentPatient);
    m_historyWidget->setPatient(m_currentPatient);

    auto ivs = Database::instance().getPatientInterventions(m_currentPatient.id);
    m_allIvTable->setRowCount(ivs.size());
    for (int i = 0; i < ivs.size(); ++i) {
        const auto& iv = ivs[i];
        QString dt = iv.dateTime.length() >= 10 ?
                     QDate::fromString(iv.dateTime.left(10),"yyyy-MM-dd").toString("dd.MM.yyyy.") : iv.dateTime;
        m_allIvTable->setItem(i,0,tItem(dt,true));
        m_allIvTable->setItem(i,1,tItem(iv.interventionType));
        m_allIvTable->setItem(i,2,tItem(iv.tooth,true));
        m_allIvTable->setItem(i,3,tItem(iv.doctor));
        m_allIvTable->setItem(i,4,tItem(QString::number(iv.cost,'f',2)+" "+iv.currency,true));
        QString statusLabel = iv.status=="completed" ? "✅ Završeno" :
                              iv.status=="scheduled" ? "📅 Zakazano" : "❌ Otkazano";
        m_allIvTable->setItem(i,5,tItem(statusLabel,true));
        m_allIvTable->item(i,0)->setData(Qt::UserRole, iv.id);
    }
}

void MainWindow::showPatientCard(const Patient& p)
{
    m_patientNameLbl->setText("👤 " + p.fullName());
    QString info;
    if (!p.dateOfBirth.isEmpty())
        info += "📅 " + QDate::fromString(p.dateOfBirth,"yyyy-MM-dd").toString("dd.MM.yyyy.") + "  ";
    if (!p.phone.isEmpty())   info += "📞 " + p.phone + "  ";
    if (!p.email.isEmpty())   info += "📧 " + p.email + "  ";
    if (!p.bloodType.isEmpty() && p.bloodType != "Nepoznata") info += "🩸 " + p.bloodType;
    if (!p.allergies.isEmpty()) info += "  ⚠️ Alergije: " + p.allergies;
    m_patientInfoLbl->setText(info);
}

// ── Patient Slots ─────────────────────────────────────────────────────────────

void MainWindow::onPatientSelected(QListWidgetItem* item)
{
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    m_currentPatient = Database::instance().getPatient(id);
    refreshPatientDetail();
    m_rightStack->setCurrentIndex(1);
}

void MainWindow::onSearchPatients(const QString& text)
{
    Q_UNUSED(text);
    loadPatientList();
}

void MainWindow::onAddPatient()
{
    PatientDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Patient p = dlg.getPatient();
        if (Database::instance().addPatient(p)) {
            loadPatientList();
            m_currentPatient = p;
            refreshPatientDetail();
            selectPatientById(p.id);
            m_rightStack->setCurrentIndex(1);
            statusBar()->showMessage("Pacijent uspješno dodan.", 3000);
        }
    }
}

void MainWindow::onEditPatient()
{
    if (m_currentPatient.id == 0) {
        QMessageBox::information(this,"Napomena","Odaberite pacijenta iz liste.");
        return;
    }
    PatientDialog dlg(this, m_currentPatient);
    if (dlg.exec() == QDialog::Accepted) {
        Patient p = dlg.getPatient();
        if (Database::instance().updatePatient(p)) {
            loadPatientList();
            selectPatientById(p.id);
            m_currentPatient = p;
            refreshPatientDetail();
            statusBar()->showMessage("Podaci pacijenta ažurirani.", 3000);
        }
    }
}

void MainWindow::onDeletePatient()
{
    if (m_currentPatient.id == 0) return;
    auto reply = QMessageBox::question(this, "Brisanje pacijenta",
        QString("Da li ste sigurni da želite obrisati pacijenta '%1'?\n"
                "Sve intervencije i termini će biti obrisani.")
                .arg(m_currentPatient.fullName()),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        Database::instance().deletePatient(m_currentPatient.id);
        m_currentPatient = {};
        loadPatientList();
        m_rightStack->setCurrentIndex(0);
        refreshDashboard();
        statusBar()->showMessage("Pacijent obrisan.", 3000);
    }
}

// ── Intervention Slots ────────────────────────────────────────────────────────

void MainWindow::onAddIntervention()
{
    if (m_currentPatient.id == 0) {
        QMessageBox::information(this,"Napomena","Odaberite pacijenta iz liste.");
        return;
    }
    InterventionDialog dlg(this, m_currentPatient.id);
    if (dlg.exec() == QDialog::Accepted) {
        Intervention iv = dlg.getIntervention();
        if (Database::instance().addIntervention(iv)) {
            refreshPatientDetail();
            m_detailTabs->setCurrentIndex(1); // switch to history
            statusBar()->showMessage("Intervencija uspješno dodana.", 3000);
        }
    }
}

void MainWindow::onEditIntervention(int id)
{
    Intervention iv = Database::instance().getIntervention(id);
    InterventionDialog dlg(this, m_currentPatient.id, iv);
    if (dlg.exec() == QDialog::Accepted) {
        Intervention updated = dlg.getIntervention();
        if (Database::instance().updateIntervention(updated)) {
            refreshPatientDetail();
            statusBar()->showMessage("Intervencija ažurirana.", 3000);
        }
    }
}

void MainWindow::onDeleteIntervention(int id)
{
    if (Database::instance().deleteIntervention(id)) {
        refreshPatientDetail();
        statusBar()->showMessage("Intervencija obrisana.", 3000);
    }
}

void MainWindow::onViewPdf(const QString& path)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

// ── Print Slots ───────────────────────────────────────────────────────────────

void MainWindow::onPrintPatientReport()
{
    if (m_currentPatient.id == 0) {
        QMessageBox::information(this,"Napomena","Odaberite pacijenta iz liste.");
        return;
    }
    auto ivs = Database::instance().getPatientInterventions(m_currentPatient.id);
    ReportGenerator::printPatientReport(m_currentPatient, ivs, this);
}

void MainWindow::onPrintIntervention()
{
    // Print currently selected intervention
}

// ── Search Slots ──────────────────────────────────────────────────────────────

void MainWindow::onSearch()
{
    QString name = m_searchName->text().trimmed();
    QDate from   = m_searchFrom->date();
    QDate to     = m_searchTo->date();
    QString type = m_searchType->currentIndex() == 0 ? QString() : m_searchType->currentText();

    m_searchIvs = Database::instance().searchInterventions(name, from, to, type);
    m_searchResults->setRowCount(m_searchIvs.size());

    for (int i = 0; i < m_searchIvs.size(); ++i) {
        const auto& iv = m_searchIvs[i];
        Patient p = Database::instance().getPatient(iv.patientId);
        QString dt = iv.dateTime.length() >= 10 ?
                     QDate::fromString(iv.dateTime.left(10),"yyyy-MM-dd").toString("dd.MM.yyyy.") : iv.dateTime;
        m_searchResults->setItem(i,0,tItem(p.fullName()));
        m_searchResults->setItem(i,1,tItem(dt,true));
        m_searchResults->setItem(i,2,tItem(iv.interventionType));
        m_searchResults->setItem(i,3,tItem(iv.tooth,true));
        m_searchResults->setItem(i,4,tItem(iv.doctor));
        m_searchResults->setItem(i,5,tItem(QString::number(iv.cost,'f',2)+" "+iv.currency,true));
        QString sl = iv.status=="completed"?"✅ Završeno":iv.status=="scheduled"?"📅 Zakazano":"❌ Otkazano";
        m_searchResults->setItem(i,6,tItem(sl,true));
    }
    statusBar()->showMessage(QString("Pronađeno: %1 rezultata").arg(m_searchIvs.size()));
}

void MainWindow::onSearchResultDoubleClicked(int row, int)
{
    if (row < 0 || row >= m_searchIvs.size()) return;
    int patientId = m_searchIvs[row].patientId;
    m_currentPatient = Database::instance().getPatient(patientId);
    refreshPatientDetail();
    loadPatientList();
    selectPatientById(patientId);
    m_rightStack->setCurrentIndex(1);
}

// ── Appointment Slots ─────────────────────────────────────────────────────────

void MainWindow::onAddAppointment()
{
    // Simple appointment dialog
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("Zakaži Termin");
    dlg->setMinimumWidth(440);
    dlg->setModal(true);

    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(20,20,20,20);
    lay->setSpacing(12);

    auto* title = new QLabel("📅 Zakazivanje termina");
    title->setStyleSheet("font-size:16px; font-weight:bold; color:#1565C0;");
    lay->addWidget(title);

    auto fs = "QComboBox,QLineEdit,QDateTimeEdit,QTextEdit{border:1px solid #BDBDBD;"
              "border-radius:4px;padding:4px 8px;min-height:30px;}"
              "QComboBox:focus,QLineEdit:focus,QDateTimeEdit:focus,QTextEdit:focus{"
              "border:2px solid #00695C;}";

    auto* form = new QFormLayout;
    auto makeLbl = [](const QString& t){
        auto* l = new QLabel(t); l->setStyleSheet("font-weight:bold; color:#424242;"); return l;
    };

    // Patient combo
    auto* patCombo = new QComboBox;
    patCombo->setStyleSheet(fs);
    auto allP = Database::instance().getAllPatients();
    for (const auto& p : allP)
        patCombo->addItem(p.fullName(), p.id);
    if (m_currentPatient.id > 0) {
        for (int i = 0; i < patCombo->count(); ++i)
            if (patCombo->itemData(i).toInt() == m_currentPatient.id) { patCombo->setCurrentIndex(i); break; }
    }

    auto* dtEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1));
    dtEdit->setCalendarPopup(true);
    dtEdit->setDisplayFormat("dd.MM.yyyy HH:mm");
    dtEdit->setStyleSheet(fs);

    auto* reasonEdit = new QLineEdit;
    reasonEdit->setPlaceholderText("Razlog dolaska...");
    reasonEdit->setStyleSheet(fs);

    auto* notesEdit = new QTextEdit;
    notesEdit->setPlaceholderText("Napomene...");
    notesEdit->setFixedHeight(70);
    notesEdit->setStyleSheet(fs);

    form->addRow(makeLbl("Pacijent:"), patCombo);
    form->addRow(makeLbl("Datum/Vrijeme:"), dtEdit);
    form->addRow(makeLbl("Razlog:"), reasonEdit);
    form->addRow(makeLbl("Napomene:"), notesEdit);
    lay->addLayout(form);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* cancelB = new QPushButton("Odustani");
    cancelB->setStyleSheet("QPushButton{border:1px solid #BDBDBD;border-radius:4px;"
                           "background:white;color:#424242;padding:6px 16px;font-weight:bold;}"
                           "QPushButton:hover{background:#F5F5F5;}");
    auto* saveB = new QPushButton("Zakaži");
    saveB->setStyleSheet("QPushButton{background:#00695C;color:white;border-radius:4px;"
                         "border:none;padding:6px 20px;font-weight:bold;}"
                         "QPushButton:hover{background:#00897B;}");
    btnRow->addWidget(cancelB); btnRow->addWidget(saveB);
    lay->addLayout(btnRow);

    connect(cancelB, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(saveB, &QPushButton::clicked, [&](){
        Appointment ap;
        ap.patientId = patCombo->currentData().toInt();
        ap.dateTime  = dtEdit->dateTime().toString("yyyy-MM-dd HH:mm");
        ap.reason    = reasonEdit->text().trimmed();
        ap.status    = "scheduled";
        ap.notes     = notesEdit->toPlainText().trimmed();
        Database::instance().addAppointment(ap);
        dlg->accept();
    });

    if (dlg->exec() == QDialog::Accepted) {
        refreshDashboard();
        statusBar()->showMessage("Termin zakazan.", 3000);
    }
    dlg->deleteLater();
}
