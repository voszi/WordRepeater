#include "MediaPlayer.h"
#include "PlayForm.h"

MediaPlayer::MediaPlayer()
{
    player = new QMediaPlayer;
    list = new QMediaPlaylist;
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MediaPlayer::onStateChanged);
    connect(list, &QMediaPlaylist::currentIndexChanged, this, &MediaPlayer::onIndexChanged);
}

MediaPlayer::~MediaPlayer()
{
    delete player;
    delete list;
}

void MediaPlayer::clearList()
{
    list->clear();
}

void MediaPlayer::append(QUrl url)
{
    list->addMedia(url);
}

void MediaPlayer::playList()
{
    if (!list->isEmpty())
    {
        player->setPlaylist(list);
        player->play();
    }
}

void MediaPlayer::setVolume(int vol)
{
    player->setVolume(vol);
}

void MediaPlayer::pause()
{
    player->pause();
}

void MediaPlayer::resume()
{
    player->play();
}


void MediaPlayer::onStateChanged(QMediaPlayer::MediaStatus state)
{
    if (state == QMediaPlayer::NoMedia)
    {
        emit ready();
    }
}

void MediaPlayer::onIndexChanged(int pos)
{
    emit playlistIndexChanged(pos);
}
