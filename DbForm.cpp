#include "DbForm.h"
#include "ui_DbForm.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QDir>
#include <QFile>
#include <QXmlStreamWriter>
#include <QMutexLocker>
#include <QRandomGenerator>
#include <QDebug>
#include "texts.h"

DbForm::DbForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DbForm)
{
    ui->setupUi(this);
    Q_UNUSED(parent)
    db = QSqlDatabase::addDatabase("QSQLITE");
    connect(ui->table, &QTableWidget::itemClicked, this, &DbForm::onItemClicked);
    ui->btnPlay->setEnabled(false);
    ui->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);

    QDir dir;
    dir.setPath(strPlaylistPath);
    QStringList filter;
    filter << tr("*.%1").arg(strPlaylistExt);
    QStringList entries = dir.entryList(filter, QDir::Files, QDir::Name);
    ui->cmbListName->addItem(strNoList);
    foreach (QString fname, entries)
    {
        fname = fname.left(fname.indexOf(tr(".%1").arg(strPlaylistExt)));
        ui->cmbListName->addItem(fname);
    }
}

DbForm::~DbForm()
{
    delete ui;
}

bool DbForm::addToPlayList(int row)
{
    QMutexLocker locker(&mutex);
    int id = ui->table->item(row, 0)->data(Qt::UserRole).toInt();
    if (!map.contains(id))
    {
        PlayItem playItem;
        playItem.wsrc = ui->table->item(row, 0)->text();
        playItem.wtgt = ui->table->item(row, 1)->text();
        playItem.ssrc = ui->table->item(row, 2)->text();
        playItem.stgt = ui->table->item(row, 3)->text();
        playItem.mp3 = QString::number(id) + ".mp3";
        list.append(playItem);
        map.insert(id, list.size()-1);
        ui->btnPlay->setEnabled(ui->table->selectedItems().size() > 0);
        ui->btnAll->setText(ui->table->selectedItems().size() > 0 ? strDeselectAll : strSelectAll);
        ui->edtCount->setText(tr("%1/%2").arg(list.size()).arg(ui->table->rowCount()));
        return true;
    }
    return false;
}

void DbForm::removeFromPlayList(int row)
{
    QMutexLocker locker(&mutex);
    int id = ui->table->item(row, 0)->data(Qt::UserRole).toInt();
    PlayMapIterator it = map.find(id);
    if (it != map.end())
    {
        list.removeAt(it.value());
        map.remove(id);
        ui->btnPlay->setEnabled(ui->table->selectedItems().size() > 0);
        ui->btnAll->setText(ui->table->selectedItems().size() > 0 ? strDeselectAll : strSelectAll);
        ui->edtCount->setText(tr("%1/%2").arg(list.size()).arg(ui->table->rowCount()));
    }
}

void DbForm::onItemClicked(QTableWidgetItem *clickedItem)
{
    int row = clickedItem->row();
    if (clickedItem->isSelected())
    {
        addToPlayList(row);
    }
    else
    {
        removeFromPlayList(row);
    }
}

void DbForm::onSettingsChanged(Settings *s)
{
    db.close();
    QString dbname = s->dbname;
    db.setDatabaseName(dbname);
    if (!db.open())
    {
        QMessageBox::critical(this, tr("Adatbázis hiba"), tr("A(z) %1 adatbázist nem sikerült megnyitni!").arg(dbname));
        return;
    }

    QString sql = "SELECT WORDS.MP3ID, WORDS.WORD, TRANSLATIONS.WORD, SENTENCES.SENTENCE, TRANSLATIONS.SENTENCE "
            "FROM WORDS "
            "INNER JOIN TRANSLATIONS ON WORDS.ID = TRANSLATIONS.WID "
            "INNER JOIN SENTENCES ON WORDS.ID = SENTENCES.WID";
    QSqlQuery query(sql);

    ui->table->setRowCount(0);
    int row = 0;
    while (query.next())
    {
        ui->table->setRowCount(row+1);
        QString mp3id = query.value(0).toString();
        QTableWidgetItem* item = new QTableWidgetItem(query.value(1).toString());
        item->setData(Qt::UserRole, mp3id);
        ui->table->setItem(row, 0, item);
        item = new QTableWidgetItem(query.value(2).toString());
        ui->table->setItem(row, 1, item);
        item = new QTableWidgetItem(query.value(3).toString());
        ui->table->setItem(row, 2, item);
        item = new QTableWidgetItem(query.value(4).toString());
        ui->table->setItem(row, 3, item);
        row++;
    }
    settings = s;
    ui->edtCount->setText(tr("%1/%2").arg(list.size()).arg(ui->table->rowCount()));
}

void DbForm::on_btnExit_clicked()
{
    QCoreApplication::exit();
}

void DbForm::on_btnAll_clicked()
{
    list.clear();
    map.clear();
    if (ui->table->selectedItems().size() > 0)
    {
        ui->table->clearSelection();
        ui->btnAll->setText(strSelectAll);
    }
    else
    {
        ui->table->selectAll();
        ui->btnAll->setText(strDeselectAll);
        for (int row = 0; row < ui->table->rowCount(); row++)
            addToPlayList(row);
    }
    ui->btnPlay->setEnabled(ui->table->selectedItems().size() > 0);
    ui->edtCount->setText(tr("%1/%2").arg(list.size()).arg(ui->table->rowCount()));
}

void DbForm::on_btnPlay_clicked()
{
    QDir dir;
    if (!dir.exists(strPlaylistPath))
        dir.mkdir(strPlaylistPath);
    QString fname = tr("%1/%2.%3").arg(strPlaylistPath, ui->cmbListName->currentText(), strPlaylistExt);
    QFile file(fname);
    if (!file.open((QIODevice::WriteOnly|QIODevice::Truncate)))
    {
        QMessageBox::critical(this, tr("Fájl hiba"), tr("A(z) %1 fájlt nem sikerült megnyitni!").arg(fname));
        return;
    }

    QXmlStreamWriter xml;
    xml.setDevice(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE playlist>");
    xml.writeStartElement("playlist");
    xml.writeAttribute("version", "1.0");

    xml.writeStartElement("database");
    xml.writeAttribute("path", settings->dbname);
    xml.writeEndElement();

    xml.writeStartElement("list");
    for (int i = 0; i < list.size(); i++)
    {
        PlayItem item = list.at(i);
        xml.writeStartElement(stritem);
        // <mp3
        xml.writeAttribute(strmp3, item.mp3);
        // <source
        xml.writeStartElement(strsource);
        xml.writeAttribute(strword, item.wsrc);
        xml.writeAttribute(strsentence, item.ssrc);
        xml.writeEndElement();
        // <target
        xml.writeStartElement(strtarget);
        xml.writeAttribute(strword, item.wtgt);
        xml.writeAttribute(strsentence, item.stgt);
        xml.writeEndElement();
        xml.writeEndElement();
    }
    xml.writeEndElement(); // </list>
    xml.writeEndElement(); // </playlist>
    xml.writeEndDocument();
    file.close();

    // töröljük minden lejátszás előtt
    list.clear();
    map.clear();
    ui->table->clearSelection();
    ui->edtCount->setText(tr("%1/%2").arg(list.size()).arg(ui->table->rowCount()));
    // listanév felvétele a comboba
    if (ui->cmbListName->findText(ui->cmbListName->lineEdit()->text()) < 0)
    {
        ui->cmbListName->insertItem(0, ui->cmbListName->lineEdit()->text());
        ui->cmbListName->model()->sort(0);
    }
    ui->btnPlay->setEnabled(false);
    ui->btnAll->setText(strSelectAll);
    emit play(fname, ui->cmbListName->currentText() == strNoList);
}

void DbForm::on_btnRandom_clicked()
{
    list.clear();
    map.clear();
    ui->table->clearSelection();
    QRandomGenerator gen(*QRandomGenerator::system());
    int max = (settings->listLength < ui->table->rowCount() ? settings->listLength : ui->table->rowCount());
    while (list.size() < max) // not correct but I trust it ends
    {
        quint32 row = gen.bounded(ui->table->rowCount());
        if (addToPlayList(row))
            ui->table->selectRow(row);
    }
}
