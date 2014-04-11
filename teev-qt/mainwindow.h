#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <stdint.h>

#include <QMainWindow>

#include <QListWidget>

#include <QGridLayout>

#include <QStringListModel>

#if (QT_VERSION < 0x050000)
#define USE_PHONON
#endif

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

#include "serverprovider.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, ServerProvider *provider = NULL, QString remoteServer = "127.0.0.1", uint16_t remotePort = 64080);
    ~MainWindow();

    void setupMainWindow();

public slots:
    void channel_clicked(QModelIndex);
#ifdef USE_PHONON
    void playerSourceChanged(Phonon::MediaSource);
#else
    void playerMediaChanged(QMediaContent);
#endif
    void refresh_clicked();

private:
    ServerProvider *dvbtee;
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
    QListView *m_listBox;
    std::string channels_buffer;
    std::string info_buffer;

    QStringListModel *channel_model;

    QString cur_chan_id;

    QString remoteServerAddr;
    uint16_t remoteServerPort;

    Ui::MainWindow *ui;

    void push(uint8_t *buffer, std::string &push_buffer, size_t size, size_t nmemb);

    void get_channels();
    void fill_channels_box();
    static void get_channels_callback(void *context, void *buffer, size_t size, size_t nmemb);

    void get_info();
    void fill_info_box();
    static void get_info_callback(void *context, void *buffer, size_t size, size_t nmemb);

    void tune(QString chan_id);

    QString dvbteeServerAddr();
};

#endif // MAINWINDOW_H
