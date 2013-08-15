#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
#ifdef USE_PHONON
#if 0
    player(new Phonon::VideoPlayer(Phonon::VideoCategory, this)),
#else
    mediaObject(new Phonon::MediaObject(this)),
    videoWidget(new Phonon::VideoWidget(this)),
#endif
#else
    player(new QMediaPlayer(this)),
    videoWidget(new QVideoWidget),
#endif
    layout(new QGridLayout),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("TeeV");

    QUrl url("http://127.0.0.1:64080/tune=44+3/stream/");

    this->centralWidget()->setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

#ifdef USE_PHONON
#if 0
    layout->addWidget(player, 0, 0);

    player->play(url);
#else
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
    //QMediaPlaylist *playlist = new QMediaPlaylist(player);
    //playlist->addMedia(url);
    //playlist->setCurrentIndex(0);

    player->setMedia(url);

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
#ifdef USE_PHONON
#if 0
    player->stop();
    delete player;
#else
    mediaObject->stop();
    delete mediaObject;
    delete videoWidget;
#endif
#else
    player->stop();
    delete player;
    delete videoWidget;
#endif
    delete layout;
    delete ui;
}
