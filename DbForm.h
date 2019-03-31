#ifndef DBFORM_H
#define DBFORM_H

#include <QTableWidget>
#include <QSqlDatabase>
#include <QMutex>
#include "PlayListItem.h"
#include "Settings.h"

namespace Ui {
class DbForm;
}

class DbForm : public QWidget
{
    Q_OBJECT

public:
    explicit DbForm(QWidget *parent = 0);
    ~DbForm();

private:
    Ui::DbForm *ui;
    QSqlDatabase db;
    Settings* settings;
    PlayList list;
    PlayMap map;
    QMutex mutex;
    bool addToPlayList(int row);
    void removeFromPlayList(int row);

public slots:
    void onItemClicked(QTableWidgetItem* clickedItem);
    void onSettingsChanged(Settings* s);
signals:
    void dbchanged(QString dbName); // !!!!
    void selected(int id, PlayItem item);
    void deselected(int id);
    void play(QString fname, bool deleteFile = false);

private slots:
    void on_btnExit_clicked();
    void on_btnAll_clicked();
    void on_btnPlay_clicked();
    void on_btnRandom_clicked();
};

#endif // DBFORM_H
