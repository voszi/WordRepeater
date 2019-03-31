#ifndef PLAYFORM_H
#define PLAYFORM_H

#include <QWidget>
#include <QEventLoop>
#include <QTimer>
#include <QPropertyAnimation>
#include <QIcon>
#include "PlayListItem.h"
#include "MediaPlayer.h"
#include "Settings.h"

namespace Ui {
class PlayForm;
}

class PlayForm : public QWidget
{
    Q_OBJECT

public:
    explicit PlayForm(QWidget *parent = 0);
    ~PlayForm();

private:
    Ui::PlayForm *ui;
    PlayList list;
    MediaPlayer* player;
    QString srcWordDir;
    QString srcSentDir;
    QString trgtWordDir;
    QString trgtSentDir;
    QEventLoop* loop;
    Settings* settings;
    void checkDir(QString path);
    void setDirs(QString dbname);
    void playList();
    void playWords(PlayListIterator it);
    void playSentences(PlayListIterator it);
    QTimer timer;
    enum {
        statePlay,
        statePause,
        stateReplay,
        stateFinished,
        stateTerminated,
    } state;
    QIcon btnPlayIcons[3];
    QString btnPlayTooltips[3];
    void setBtnPlayState();

public slots:
    void onSettingsChanged(Settings* s);
    void onPlay(QString fname, bool deleteFile);
private slots:
    void on_btnPlay_clicked();
    void onPaylistIndexChanged(int pos);
    void on_btnExit_clicked();

    void on_sliderVolume_valueChanged(int value);

signals:
    void pause();
    void play();
    void resume();
    void finished();
    void setVolume(int);
};

#endif // PLAYFORM_H
