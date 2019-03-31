#include "ConfigForm.h"
#include "ui_ConfigForm.h"
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QDebug>
#include "texts.h"

ConfigForm::ConfigForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigForm)
{
    settings = new Settings;
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setText("Alkalmaz");
    ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setText("Alapbeállítás");
    ui->buttonBox->button(QDialogButtonBox::Close)->setText("Kilépés");
    QString  dir = QDir::currentPath() + "/vocabularies";
    if (!QFile::exists(dir)){
        QMessageBox::critical(this, "Vocabularies", dir);
        return;
    }
    QStringList list = QDir(dir).entryList(QDir::Dirs|QDir::NoDotAndDotDot);
    ui->cmbVocabularies->addItems(list);
}

ConfigForm::~ConfigForm()
{
    delete ui;
    if (settings)
        delete settings;
}

void ConfigForm::on_buttonBox_clicked(QAbstractButton *button)
{
    switch (ui->buttonBox->buttonRole(button))
    {
    case QDialogButtonBox::ApplyRole:
        settings->dbname = QDir::currentPath() + "/vocabularies/" + ui->cmbVocabularies->currentText() + "/vocabulary.sqlite";
        settings->wordRepaetCount = ui->spinWordRepeat->value();
        settings->noSentence = ui->checkNoSentence->isChecked();
        settings->sentenceOnce = ui->checkSentenceRepeat->isChecked();
        settings->listLength = ui->spinListLenght->value();
        settings->listRepeatCount = ui->checkEndless->isChecked() ?
                    0 : ui->spinListRepeat->value();
        settings->random = ui->checkRandom->isChecked();
        save();
        emit settingsChanged(settings);
        break;
    case QDialogButtonBox::ResetRole:
        setDefaults();
        break;
    case QDialogButtonBox::RejectRole:
        QCoreApplication::exit();
        break;
    default:
        break;
    }
}

void ConfigForm::on_checkNoSentence_stateChanged(int arg1)
{
    ui->checkSentenceRepeat->setEnabled(arg1 != Qt::Checked);
}

void ConfigForm::on_checkEndless_stateChanged(int arg1)
{
    ui->spinListRepeat->setEnabled(arg1 == Qt::Unchecked);
    ui->checkRandom->setEnabled(arg1 == Qt::Checked || ui->spinListRepeat->value() > 1);
}

void ConfigForm::on_spinListRepeat_valueChanged(int arg1)
{
    ui->checkRandom->setEnabled(arg1 == Qt::Checked || ui->spinListRepeat->value() > 1);
}

void ConfigForm::setDefaults()
{
    ui->spinListLenght->setValue(20);
    ui->spinWordRepeat->setValue(3);
    ui->spinListRepeat->setValue(1);
    ui->checkSentenceRepeat->setCheckState(Qt::Checked);
    ui->checkNoSentence->setCheckState(Qt::Unchecked);
    ui->checkRandom->setCheckState(Qt::Unchecked);
    ui->checkRandom->setEnabled(false);
}

void ConfigForm::load()
{
    QFile file(strfilename);
    if (!file.exists())
    {
        setDefaults();
        return;
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, strfileerror, QString(stropenrror).arg(strfilename));
        setDefaults();
        return;
    }
    QXmlStreamReader xml;
    xml.setDevice(&file);
    if (xml.readNextStartElement()) {
        if (xml.name() != strsettingsdocname || xml.attributes().value(strversion) != strsettingsversion)
        {
            QMessageBox::critical(this, strxmlerror, QString(strxmldocerror).arg(strfilename));
            file.close();
            setDefaults();
            return;
        }
    }

    // since here we trust in the same sequence as written in save
    xml.readNextStartElement();
    settings->dbname = xml.attributes().value(strpath).toString();
    xml.skipCurrentElement();
    if (!QFile::exists(settings->dbname)) {
        QMessageBox::critical(this, strxmlerror, QString(strnodberror2).arg(strfilename, settings->dbname));
        file.close();
        setDefaults();
        return;
    }
    QStringList list = settings->dbname.split('/');
    int i = ui->cmbVocabularies->findText(list.at(list.size()-2));
    if (i > -1)
    {
        ui->cmbVocabularies->setCurrentIndex(i);
    }
    else
    {
        QMessageBox::critical(this, strxmlerror, QString(strnodberror2).arg(strfilename, settings->dbname));
    }


    xml.readNextStartElement();
    settings->wordRepaetCount = xml.readElementText().toInt();

    xml.readNextStartElement();
    settings->noSentence = xml.readElementText().toInt() == 1;

    xml.readNextStartElement();
    settings->sentenceOnce = xml.readElementText().toInt() == 1;

    xml.readNextStartElement();
    settings->listLength = xml.readElementText().toInt();

    xml.readNextStartElement();
    settings->listRepeatCount = xml.readElementText().toInt();

    xml.readNextStartElement();
    settings->random = xml.readElementText().toInt() == 1;

    file.close();

    // update controls
    ui->spinWordRepeat->setValue(settings->wordRepaetCount);
    ui->checkNoSentence->setChecked(settings->noSentence);
    ui->checkSentenceRepeat->setChecked(settings->sentenceOnce);
    ui->checkSentenceRepeat->setEnabled(settings->noSentence != true);
    ui->spinListLenght->setValue(settings->listLength);
    ui->spinListRepeat->setValue(settings->listRepeatCount);
    ui->checkEndless->setChecked(settings->listRepeatCount == 0);
    ui->checkRandom->setChecked(settings->random == 1);

    emit settingsChanged(settings);
}

void ConfigForm::save()
{
    QFile file(strfilename);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        QMessageBox::critical(this, strfileerror, QString(stropenrror).arg(strfilename));
        return;
    }

    QXmlStreamWriter xml;
    xml.setDevice(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeDTD(QString(strdoctype).arg(strsettingsdocname));
    xml.writeStartElement(strsettingsdocname);
    xml.writeAttribute(strversion, strsettingsversion);

    xml.writeStartElement(strdatabase);
    xml.writeAttribute(strpath, settings->dbname);
    xml.writeEndElement();

    xml.writeTextElement(strwordrepaetcount, QString::number(settings->wordRepaetCount));
    xml.writeTextElement(strnosentence, settings->noSentence ? "1" : "0");
    xml.writeTextElement(strsentenceonce, settings->sentenceOnce ? "1" : "0");
    xml.writeTextElement(strlistlength, QString::number(settings->listLength));
    xml.writeTextElement(strlistrepeatcount, QString::number(settings->listRepeatCount));
    xml.writeTextElement(strrandom, settings->random ? "1" : "0");

    xml.writeEndDocument();
    file.close();
}


void ConfigForm::on_cmbVocabularies_currentIndexChanged(const QString &arg1)
{
    settings->dbname = QDir::currentPath() + "/vocabularies/" + arg1 + "/vocabulary.sqlite";
}
