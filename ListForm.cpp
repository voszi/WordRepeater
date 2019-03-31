#include "ListForm.h"
#include "ui_ListForm.h"
#include <QDir>
#include <QList>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QColor>
#include <QBrush>
#include <QDebug>
#include "texts.h"

static const QColor bckgrcol0(0, 102, 153);
static const QColor bckgrcol1(51, 204, 204);
static const QColor bckgrhdr0(0, 51, 102);
static const QColor bckgrhdr1(0, 102, 102);

ListForm::ListForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ListForm)
{
    ui->setupUi(this);
    QTreeWidgetItem* hdr;
    hdr = new QTreeWidgetItem;
    hdr->setText(0, "Listanév");
    hdr->setText(1, "Tartalom");
    ui->tree->setHeaderItem(hdr);
    ui->tree->setHeaderHidden(false);

    ui->btnDelete->setEnabled(false);
    ui->btnPlay->setEnabled(false);
    loadList();
}

ListForm::~ListForm()
{
    delete ui;
}

void ListForm::loadList()
{
    QDir dir;
    dir.setPath(strPlaylistPath);
    QStringList filter;
    filter << tr("*.%1").arg(strPlaylistExt);
    QStringList entries = dir.entryList(filter, QDir::Files, QDir::Name);
    QTreeWidgetItem* root = new QTreeWidgetItem;
    root->setText(0, "Lejátszási listák");
    root->setBackground(0, QBrush(bckgrhdr1));
    root->setTextColor(0, Qt::white);
    foreach (QString fname, entries)
    {
        QString label = fname.left(fname.lastIndexOf(tr(".%1").arg(strPlaylistExt)));
        fname.prepend(tr("%1/").arg(strPlaylistPath));
        QTreeWidgetItem* item = new QTreeWidgetItem(root);
        item->setText(0, label);
        item->setBackgroundColor(0, bckgrcol0);
        item->setTextColor(0, Qt::white);
        item->setData(0, Qt::UserRole, label);
        loadFile(item, fname);
    }
    ui->tree->addTopLevelItem(root);
    ui->tree->adjustSize();
}

void ListForm::loadFile(QTreeWidgetItem *parent, QString fname)
{
    QList<QTreeWidgetItem*> list;
    QTreeWidgetItem* child;
    QFile file(fname);
    if (!file.open((QIODevice::ReadOnly)))
    {
        QMessageBox::critical(this, strfileerror, QString(stropenrror).arg(fname));
        return;
    }
    QXmlStreamReader xml;
    xml.setDevice(&file);
    if (xml.readNextStartElement()) {
        if (xml.name() != strplaylistdocname || xml.attributes().value(strversion) != strplaylistversion)
        {
            QMessageBox::critical(this, strxmlerror, QString(strxmldocerror).arg(fname));
            file.close();
            return;
        }
    }

    if (xml.readNextStartElement() && xml.name() == strdatabase)
        xml.skipCurrentElement();
    if (!xml.readNextStartElement() || xml.name() != strlist)
    {
        QMessageBox::critical(this, strxmlerror, QString(strnolisterror).arg(fname).arg(xml.name()));
        file.close();
        return;
    }

    while (xml.readNextStartElement() && xml.name() == stritem)
    {
        child = new QTreeWidgetItem(parent);
        child->setBackgroundColor(0, bckgrcol1);
        QTreeWidgetItem* grandchild1 = new QTreeWidgetItem;
        QTreeWidgetItem* grandchild2 = new QTreeWidgetItem;
        QTreeWidgetItem* grandchild3 = new QTreeWidgetItem;
        if (xml.readNextStartElement() && xml.name() == strsource)
        {
            child->setText(0, xml.attributes().value(strword).toString());
            grandchild2->setText(0, "példamondat:");
            grandchild2->setText(1, xml.attributes().value(strsentence).toString());
        }
        xml.skipCurrentElement();
        if (xml.readNextStartElement() && xml.name() == strtarget)
        {
            grandchild1->setText(0, "jelentés:");
            grandchild1->setText(1, xml.attributes().value(strword).toString());
            grandchild3->setText(0, "fordítás:");
            grandchild3->setText(1, xml.attributes().value(strsentence).toString());
        }
        child->addChild(grandchild1);
        child->addChild(grandchild2);
        child->addChild(grandchild3);
        xml.skipCurrentElement();
        list.append(child);
        xml.skipCurrentElement();
    }
    file.close();
    parent->addChildren(list);
}

void ListForm::onPlay(QString fname)
{
    QString label = fname.mid(strlen(strPlaylistPath)+1);
    label = label.left(label.lastIndexOf(strPlaylistExt)-1);
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, label);
    item->setData(0, Qt::UserRole, label);
    item->setBackgroundColor(0, bckgrcol0);
    item->setTextColor(0, Qt::white);
    loadFile(item, fname);

    QTreeWidgetItem* root = ui->tree->topLevelItem(0);
    for (int i = 0; i < root->childCount(); i++)
    {
        QTreeWidgetItem* child = root->child(i);
        int c = QString::compare(label, child->text(0), Qt::CaseInsensitive);
        if (c <= 0)
        {
            root->insertChild(i, item); // first insert, i is fine
            if (c == 0) // same listname, remove
                root->removeChild(child);
            return;
        }
    }
    root->addChild(item);
}

void ListForm::onFinished()
{
    ui->tree->setEnabled(true);
    on_tree_itemSelectionChanged(); // for restroring the proper state of buttons
}

void ListForm::on_btnExit_clicked()
{
    QApplication::exit();
}

void ListForm::on_tree_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> list = ui->tree->selectedItems();
    QTreeWidgetItem* item = 0;
    if (list.size())
    {
        item = ui->tree->selectedItems().at(0);
        while (item && item->data(0, Qt::UserRole).toString().isEmpty())
            item = item->parent();
    }
    ui->edtListName->setText(item ? item->text(0) : "");
    ui->btnPlay->setEnabled(item != 0);
    ui->btnDelete->setEnabled(item != 0);
}

void ListForm::on_btnPlay_clicked()
{
    if (!ui->edtListName->text().isEmpty())
    {
        ui->tree->setEnabled(false);
        ui->btnDelete->setEnabled(false);
        ui->btnPlay->setEnabled(false);
        emit play(tr("%1/%2.%3").arg(strPlaylistPath).arg(ui->edtListName->text()).arg(strPlaylistExt));
    }
}

void ListForm::on_btnDelete_clicked()
{
    QString fname = ui->edtListName->text();
    if (!fname.isEmpty() &&
            QMessageBox::question(this, strdeleteplaylist, QString(strdeleteplaylistquestion).arg(fname),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        fname.prepend(QString("%1/").arg(strPlaylistPath)).append(QString(".%1").arg(strPlaylistExt));
        QFile::remove(fname);
        QTreeWidgetItem* item = ui->tree->selectedItems().at(0);
        while (item && item->data(0, Qt::UserRole).toString().isEmpty())
        {
            item = item->parent();
        }
        if (item && item->parent())
        {
            item->parent()->removeChild(item);
            delete item;
        }
    }
}
