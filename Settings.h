#ifndef SETTINGS_H
#define SETTINGS_H
#include <QMetaType>
#include <QString>

struct Settings {
    QString dbname;
    int wordRepaetCount;
    bool noSentence;
    bool sentenceOnce;
    int listLength;
    int listRepeatCount;   // if 0 endless
    bool random;
};

Q_DECLARE_METATYPE(Settings)

#endif // SETTINGS_H
