#ifndef CONFIGFORM_H
#define CONFIGFORM_H

#include <QWidget>
#include <QAbstractButton>
#include "Settings.h"

namespace Ui {
class ConfigForm;
}

class ConfigForm : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigForm(QWidget *parent = 0);
    void load();
protected:
    ~ConfigForm();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_checkNoSentence_stateChanged(int arg1);
    void on_checkEndless_stateChanged(int arg1);
    void on_spinListRepeat_valueChanged(int arg1);

    void on_cmbVocabularies_currentIndexChanged(const QString &arg1);

private:
    Ui::ConfigForm *ui;
    QString dbName;
    Settings* settings;
    void setDefaults();
    void save();
signals:
    void settingsChanged(Settings* );
};

#endif // CONFIGFORM_H
