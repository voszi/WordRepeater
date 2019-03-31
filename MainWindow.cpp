#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ConfigForm.h"
#include "DbForm.h"
#include "PlayForm.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->tabSettings, &ConfigForm::settingsChanged, this, &MainWindow::onSettingsChanged);
    connect(ui->tabSettings, &ConfigForm::settingsChanged, ui->tabWords, &DbForm::onSettingsChanged);
    connect(ui->tabSettings, &ConfigForm::settingsChanged, ui->tabPlay, &PlayForm::onSettingsChanged);
    connect(ui->tabWords, &DbForm::play, this, &MainWindow::onPlay);
    connect(ui->tabWords, &DbForm::play, ui->tabList, &ListForm::onPlay);
    connect(ui->tabWords, &DbForm::play, ui->tabPlay, &PlayForm::onPlay);
    connect(ui->tabList, &ListForm::play, this, &MainWindow::onPlay);
    connect(ui->tabList, &ListForm::play, ui->tabPlay, &PlayForm::onPlay);
    connect(ui->tabPlay, &PlayForm::finished, this, &MainWindow::onFinished);
    connect(ui->tabPlay, &PlayForm::finished, ui->tabList, &ListForm::onFinished);
    ui->tabWidget->setCurrentWidget(ui->tabSettings);
    ui->tabWidget->removeTab(1);
    ui->tabWidget->removeTab(1);
    ui->tabWidget->removeTab(1);
    ui->tabSettings->load();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSettingsChanged()
{
    ui->tabWidget->addTab(ui->tabWords, "Szószedet");
    ui->tabWidget->addTab(ui->tabList, "Listák");
    ui->tabWidget->setCurrentWidget(ui->tabWords);
}

void MainWindow::onPlay(QString fname)
{
    Q_UNUSED(fname)
    ui->tabSettings->setEnabled(false);
    ui->tabWords->setEnabled(false);
    ui->tabList->setEnabled(false);
    if (ui->tabWidget->count() < 4)
        ui->tabWidget->addTab(ui->tabPlay, "Lejátszás");
    ui->tabPlay->setEnabled(true);
    ui->tabWidget->setCurrentWidget(ui->tabPlay);
}

void MainWindow::onFinished()
{
    ui->tabSettings->setEnabled(true);
    ui->tabWords->setEnabled(true);
    ui->tabList->setEnabled(true);
    // tabPlay remains enable, it has a playable list
}

