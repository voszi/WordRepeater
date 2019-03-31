#ifndef LISTFORM_H
#define LISTFORM_H

#include <QWidget>
#include <QTreeWidgetItem>

namespace Ui {
class ListForm;
}

class ListForm : public QWidget
{
    Q_OBJECT

public:
    explicit ListForm(QWidget *parent = nullptr);
    ~ListForm();

public slots:
    void onPlay(QString fname);
    void onFinished();
private slots:
    void on_btnExit_clicked();
    void on_tree_itemSelectionChanged();
    void on_btnPlay_clicked();
    void on_btnDelete_clicked();

signals:
    void play(QString fname, bool deleteFile = false);

private:
    Ui::ListForm *ui;
    void loadList();
    void loadFile(QTreeWidgetItem* parent,  QString fname);
};

#endif // LISTFORM_H
