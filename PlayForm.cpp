#include "PlayForm.h"
#include "ui_PlayForm.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QStringList>
#include <QRandomGenerator>
#include <QTextEdit>
#include <QFont>
#include "texts.h"

const QFont ftSrc("Arial", 10);
const QFont ftTrgt("Arial", 10);
const QFont ftSrcB("Arial", 10, 75);
const QFont ftTrgtB("Arial", 10, 75);
const char* strWords("/words/");
const char* strSentences("/sentences/");

PlayForm::PlayForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayForm)
{
    ui->setupUi(this);
    player = new MediaPlayer();
    loop = new QEventLoop(this);
    connect(this, &PlayForm::play, player, &MediaPlayer::playList);
    connect(this, &PlayForm::pause, player, &MediaPlayer::pause);
    connect(this, &PlayForm::resume, player, &MediaPlayer::resume);
    connect(this, &PlayForm::setVolume, player, &MediaPlayer::setVolume);
    connect(&timer, &QTimer::timeout, loop, &QEventLoop::quit);
    connect(player, &MediaPlayer::ready, loop, &QEventLoop::quit);
    connect(player, &MediaPlayer::playlistIndexChanged, this, &PlayForm::onPaylistIndexChanged);
    settings = nullptr;
    ui->btnPlay->setEnabled(false);
    ui->sliderVolume->setValue(80);
    timer.setSingleShot(true);

    btnPlayIcons[statePlay] = QIcon(":images/play-button.png");
    btnPlayIcons[statePause] = QIcon(":images/pause-button.png");
    btnPlayIcons[stateReplay] = QIcon(":images/replay-button.png");
}

PlayForm::~PlayForm()
{
    delete ui;
}

void PlayForm::checkDir(QString path)
{
    QDir dir(path);
    if (!dir.exists())
    {
        QMessageBox::critical(this, QString("Nem létező könyvtár"), QString("A %1 könyvtár nem található").arg(path));
    }
}

void PlayForm::setDirs(QString dbname)
{
    QFileInfo info(dbname);
    QDir root(info.absoluteDir());
    checkDir(root.absolutePath());
    root.cd("voices");
    checkDir(root.absolutePath());
    QStringList dirlist = root.entryList(QDir::AllDirs|QDir::NoDotAndDotDot);
    for (int i = 0; i < dirlist.size(); i++)
    {
        if (dirlist.at(i) == "HU")  // in this version the target is always Hungarian
        {
            trgtWordDir = root.absolutePath() + '/' + dirlist.at(i) + strWords;
            checkDir(trgtWordDir);
            trgtSentDir = root.absolutePath() + '/' + dirlist.at(i) + strSentences;
            checkDir(trgtSentDir);
        }
        else
        {
            srcWordDir = root.absolutePath() + '/' + dirlist.at(i) + strWords;
            checkDir(srcWordDir);
            srcSentDir = root.absolutePath() + '/' + dirlist.at(i) + strSentences;
            checkDir(srcSentDir);
        }
    }
}

void PlayForm::onSettingsChanged(Settings *s)
{
    settings = s;
}

void PlayForm::onPlay(QString fname, bool deleteFile)
{
    QFile file(fname);
    QString dbname;
    list.clear();
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
    {
        dbname = xml.attributes().value(strpath).toString();
    }
    else
    {
        QMessageBox::critical(this, strxmlerror, QString(strnodberror).arg(fname));
        file.close();
        return;
    }
    xml.skipCurrentElement();
    if (!xml.readNextStartElement() || xml.name() !=strlist)
    {
        QMessageBox::critical(this, strxmlerror, QString(strnolisterror).arg(fname).arg(xml.name()));
        file.close();
        return;
    }

    while (xml.readNextStartElement() && xml.name() == stritem)
    {
        PlayItem item;
        item.mp3 = xml.attributes().value(strmp3).toString();
        if (xml.readNextStartElement() && xml.name() == strsource)
        {
            item.wsrc = xml.attributes().value(strword).toString();
            item.ssrc = xml.attributes().value(strsentence).toString();
        }
        xml.skipCurrentElement();
        if (xml.readNextStartElement() && xml.name() == strtarget)
        {
            item.wtgt = xml.attributes().value(strword).toString();
            item.stgt = xml.attributes().value(strsentence).toString();
        }
        xml.skipCurrentElement();
        list.append(item);
        xml.skipCurrentElement();
    }
    file.close();
    if (deleteFile)
        file.remove();
    setDirs(dbname);
    ui->btnPlay->setEnabled(true);
    playList();
}


void PlayForm::onPaylistIndexChanged(int pos)
{
    switch (pos)
    {
    case 0:
        ui->textSrc->setFont(ftSrcB);
        break;
    case 1:
        ui->textSrc->setFont(ftSrc);
        ui->textTrgt->setFont(ftTrgtB);
        break;
    case -1: // end of list
        ui->textTrgt->setFont(ftTrgt);
        break;
    default:
        break;
    }
}

void PlayForm::playList()
{
    state = statePlay;
    setBtnPlayState();
    ui->progList->setVisible(settings->listRepeatCount != 0);
    int proglistval = 0;
    int proglistmax = list.size();
    if (!settings->noSentence)
    {
        if (settings->sentenceOnce)
            proglistmax *= (settings->wordRepaetCount+1);
        else
            proglistmax *= 2*settings->wordRepaetCount;
    }
    if (settings->listRepeatCount)
    {
            proglistmax *= settings->listRepeatCount;
    }
    ui->progList->setMinimum(proglistval);
    ui->progList->setMaximum(proglistmax);
    for (int l = 0; l < settings->listRepeatCount || settings->listRepeatCount == 0; l++)
    {
        if (state == stateTerminated) return;
        ui->progWords->setMinimum(0);
        ui->progWords->setMaximum(2*list.size());   // half steps
        int wcount = 0;
        for (PlayListIterator it = list.begin(); it != list.end(); it++)
        {
            ui->progWords->setValue(++wcount);
            for (int w = 0; w < settings->wordRepaetCount; w++)
            {
                if (state == stateTerminated) return;
                if (settings->listRepeatCount)
                    ui->progList->setValue(++proglistval);
                playWords(it);
                if (!settings->noSentence && !settings->sentenceOnce)
                {
                    ui->progList->setValue(++proglistval);
                    playSentences(it);
                }
            }
            if (!settings->noSentence && settings->sentenceOnce)
            {
                ui->progList->setValue(++proglistval);
                playSentences(it);
            }
            ui->progWords->setValue(++wcount);
        }
        if (settings->random)
        {
            // we swap some items, not so sofisticated but folks like it
            QRandomGenerator gen(*QRandomGenerator::system());
            for (int r = 0; r < list.size(); r++)
            {
                quint32 i1 = gen.bounded(list.size());
                quint32 i2 = gen.bounded(list.size());
                if (i1 != i2)
                {
                    PlayItem temp = list.at(i1);
                    list.replace(i1, list.at(i2));
                    list.replace(i2, temp);
                }
            }
        }
    }
    ui->textSrc->setText(tr("A lejátszás befejeződött."));
    ui->textTrgt->setText("");
    state = stateFinished;
    setBtnPlayState();
    emit finished();
}

void PlayForm::playWords(PlayListIterator it)
{
    QUrl url;
    player->clearList();
    ui->textSrc->setText(it->wsrc);
    ui->textSrc->repaint();
    ui->textTrgt->setText(it->wtgt);
    ui->textTrgt->repaint();
    url.setUrl(srcWordDir + it->mp3);
    player->append(url);
    url.setUrl(trgtWordDir + it->mp3);
    player->append(url);
    emit play();
    loop->exec();
}

void PlayForm::playSentences(PlayListIterator it)
{
    QUrl url;
    player->clearList();
    ui->textSrc->setText(it->ssrc);
    ui->textSrc->repaint();
    ui->textTrgt->setText(it->stgt);
    ui->textTrgt->repaint();
    url.setUrl(srcSentDir + it->mp3);
    player->append(url);
    url.setUrl(trgtSentDir + it->mp3);
    player->append(url);
    emit play();
    loop->exec();
}

void PlayForm::setBtnPlayState()
{
    switch (state) {
    case statePlay:
        ui->btnPlay->setIcon(btnPlayIcons[statePause]);
        ui->btnPlay->setToolTip(tr("Stop"));
        break;
    case statePause:
        ui->btnPlay->setIcon(btnPlayIcons[statePlay]);
        ui->btnPlay->setToolTip(tr("Lejátszás"));
        break;
    case stateFinished:
        ui->btnPlay->setIcon(btnPlayIcons[stateReplay]);
        ui->btnPlay->setToolTip(tr("Újra"));
        break;
    default:
        break;
    }
}

void PlayForm::on_btnExit_clicked()
{
    emit pause();
    loop->quit();
    state = stateTerminated;
    QApplication::exit();
}

void PlayForm::on_btnPlay_clicked()
{
    switch (state) {
    case statePlay:
        state = statePause;
        emit pause();
        break;
    case statePause:
        state = statePlay;
        emit resume();
        break;
    case stateFinished:
        state = statePlay;
        playList();
        break;
    default:
        break;
    }
    setBtnPlayState();
}


void PlayForm::on_sliderVolume_valueChanged(int value)
{
    emit setVolume(value);
}
