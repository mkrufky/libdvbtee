#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <stdint.h>

#include <QMainWindow>

#include <QListWidget>

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

public slots:
    void channel_clicked(QListWidgetItem*);
//    void playerMediaChanged(QMediaContent);

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
    QListWidget *m_listBox;
    std::string channels_buffer;
    std::string info_buffer;

    Ui::MainWindow *ui;

    void push(uint8_t *buffer, std::string &push_buffer, size_t size, size_t nmemb);

    void get_channels();
    void fill_channels_box();
    static void get_channels_callback(void *context, void *buffer, size_t size, size_t nmemb);

    void get_info(QString chan_id);
    void fill_info_box();
    static void get_info_callback(void *context, void *buffer, size_t size, size_t nmemb);
};

#endif // MAINWINDOW_H
