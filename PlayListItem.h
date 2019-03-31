#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H
#include <QList>
#include <QMap>

struct PlayItem
{
    QString wsrc;
    QString wtgt;
    QString ssrc;
    QString stgt;
    QString mp3;
};

typedef QList<PlayItem> PlayList;
typedef QList<PlayItem>::iterator PlayListIterator;
typedef QMap<int, int> PlayMap;
typedef PlayMap::iterator PlayMapIterator;

#endif // PLAYLISTITEM_H
