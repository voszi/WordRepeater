#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QMediaPlaylist>

class MediaPlayer : public QObject
{
    Q_OBJECT

public:
    MediaPlayer();
    virtual ~MediaPlayer();

private:
    QMediaPlayer* player;
    QMediaPlaylist* list;

public:
    void clearList();
    void append(QUrl url);
public slots:
    void playList();
    void setVolume(int vol);
    void pause();
    void resume();
protected slots:
    void onStateChanged(QMediaPlayer::MediaStatus state);
    void onIndexChanged(int pos);
signals:
    void ready();
    void playlistIndexChanged(int pos);
};

#endif // MEDIAPLAYER_H
