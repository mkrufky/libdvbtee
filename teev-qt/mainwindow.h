#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QGridLayout>

#ifdef USE_PHONON
#include <phonon/MediaObject>
#include <phonon/VideoPlayer>

#include <phonon/VideoWidget>
#include <phonon/AudioOutput>
#else
#include <QtMultimedia>
#include <QMediaPlayer>
#include <QVideoWidget>
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
#ifdef USE_PHONON
#if 0
    Phonon::VideoPlayer *player;
#else
    Phonon::MediaObject *mediaObject;

    Phonon::VideoWidget *videoWidget;
#endif
#else
    QMediaPlayer *player;
    QVideoWidget *videoWidget;
#endif
    QGridLayout *layout;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
