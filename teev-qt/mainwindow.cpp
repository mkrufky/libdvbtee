#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifndef USE_PHONON
#include <QtMultimedia>
#include <QMediaPlayer>
#include <QVideoWidget>
#else
#include <phonon/MediaObject>
#include <phonon/VideoPlayer>

#include <phonon/VideoWidget>
#include <phonon/AudioOutput>
#endif

#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("TeeV");

    QUrl url("http://127.0.0.1:64080/tune=44+3/stream/");

    QGridLayout *layout = new QGridLayout;
    this->centralWidget()->setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

#ifdef USE_PHONON
#if 0
    Phonon::VideoPlayer *player = new Phonon::VideoPlayer(Phonon::VideoCategory, this);

    layout->addWidget(player, 0, 0);

    player->play(url);
#else
    Phonon::MediaObject *mediaObject = new Phonon::MediaObject(this);

    Phonon::VideoWidget *videoWidget = new Phonon::VideoWidget(this);

    layout->addWidget(videoWidget, 0, 0, 3, 3);

    // use videoWidget->setFullScreen(true); to toggle fullscreen mode
    //videoWidget->setFullScreen(true);

    Phonon::createPath(mediaObject, videoWidget);

    Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    Phonon::createPath(mediaObject, audioOutput);

    Phonon::MediaSource *mediaSource = new Phonon::MediaSource(url);
    mediaObject->setCurrentSource(*mediaSource);

    mediaObject->play();
#endif
#else
    QMediaPlayer *player = new QMediaPlayer(this);

    //QMediaPlaylist *playlist = new QMediaPlaylist(player);
    //playlist->addMedia(url);
    //playlist->setCurrentIndex(0);

    player->setMedia(url);

    QVideoWidget *videoWidget = new QVideoWidget;
    player->setVideoOutput(videoWidget);

    layout->addWidget(videoWidget, 0, 0, 3, 3);

    videoWidget->show();

    player->play();
#endif
    this->setStatusBar(0);
    //this->menuBar()->setVisible(false);
    this->removeToolBar(ui->mainToolBar);
}

MainWindow::~MainWindow()
{
    delete ui;
}
