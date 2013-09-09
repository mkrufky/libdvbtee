#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>

#include <jsoncpp/json/json.h>

#include "curlhttpget.h"

// FIXME: this is just for the sleep call, which is a temporary hack:
#include <unistd.h>

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
    m_listBox(new QListWidget),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("TeeV");

    cur_chan_id = "33+1";
    QUrl url("http://127.0.0.1:64080/tune="+cur_chan_id+"/stream/");

    this->centralWidget()->setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

#ifdef USE_PHONON
#if 0
    layout->addWidget(player, 0, 0);

    player->play(url);
#else
    layout->addWidget(videoWidget, 0, 0);

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

    layout->addWidget(videoWidget, 0, 0);

    videoWidget->show();

    player->play();
#endif
    //this->setStatusBar(0);

    //this->menuBar()->setVisible(false);
    this->removeToolBar(ui->mainToolBar);

    layout->addWidget(m_listBox, 0, 1);
    m_listBox->setMinimumWidth(120);

    get_channels();

    connect(m_listBox, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(channel_clicked(QListWidgetItem*)));
#ifdef USE_PHONON
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)), SLOT(playerSourceChanged(Phonon::MediaSource)));
#else
    connect(player, SIGNAL(currentMediaChanged(QMediaContent)), SLOT(playerMediaChanged(QMediaContent)));
#endif

    QPushButton *btn = new QPushButton("refresh");
    statusBar()->addPermanentWidget(btn);
    connect(btn, SIGNAL(clicked()), SLOT(refresh_clicked()));

    sleep(3);
    get_info();
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

void MainWindow::channel_clicked(QListWidgetItem *item)
{
    cur_chan_id = item->text().remove(0,item->text().indexOf("|")+1);
    QUrl url("http://127.0.0.1:64080/tune="+ cur_chan_id +"/stream/");
#ifdef USE_PHONON
#if 0
    player->stop();
    player->play(url);
#else
    mediaObject->stop();
    Phonon::MediaSource *mediaSource = new Phonon::MediaSource(url);
    mediaObject->setCurrentSource(*mediaSource);

    mediaObject->play();
#endif
#else
    player->stop();
    player->setMedia(url);
    player->play();
#endif
}

#ifdef USE_PHONON
void MainWindow::playerSourceChanged(Phonon::MediaSource)
{
	sleep(2);
#else
void MainWindow::playerMediaChanged(QMediaContent c)
{
	sleep(3);
#endif
	get_info();
}

void MainWindow::refresh_clicked()
{
	get_info();
	get_channels();
}

void MainWindow::push(uint8_t *buffer, std::string &push_buffer, size_t size, size_t nmemb)
{
    uint8_t buf[size * nmemb + 1];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, buffer, size * nmemb);
    push_buffer.append((const char*)buf);
}

void MainWindow::get_channels_callback(void *context, void *buffer, size_t size, size_t nmemb)
{
    return static_cast<MainWindow*>(context)->push((uint8_t*)buffer, static_cast<MainWindow*>(context)->channels_buffer, size, nmemb);
}

void MainWindow::get_info_callback(void *context, void *buffer, size_t size, size_t nmemb)
{
	return static_cast<MainWindow*>(context)->push((uint8_t*)buffer, static_cast<MainWindow*>(context)->info_buffer, size, nmemb);
}

void MainWindow::get_channels()
{
	channels_buffer.clear();
	curlhttpget Curl("http://127.0.0.1:64080/json/channels", get_channels_callback, this);
	fill_channels_box();
	channels_buffer.clear();
}

void MainWindow::get_info()
{
	info_buffer.clear();
	QString info_url("http://127.0.0.1:64080/json/info="+ cur_chan_id);
	curlhttpget Curl(info_url.toStdString().c_str(), get_info_callback, this);
	fill_info_box();
	info_buffer.clear();
}

void MainWindow::fill_channels_box()
{
    std::string json_str(channels_buffer);

    Json::Value root;
    Json::Reader reader;

    m_listBox->clear();

    if ( (!json_str.empty()) && reader.parse(json_str, root) ) {
      for ( Json::ArrayIndex idx = 0; idx < root.size(); idx++ ) {
	const Json::Value thisEntry = root[idx];
	if (!thisEntry.isObject()) continue;
	QString str_id(thisEntry["Id"].asString().c_str());
	QString str_name(thisEntry["DisplayName"].asString().c_str());
	QString str_major(thisEntry["MajorChannelNo"].asString().c_str());
	QString str_minor(thisEntry["MinorChannelNo"].asString().c_str());
	QString this_item = str_major + "." + str_minor + ": " + str_name + " |" + str_id;
	if (str_id.length()) m_listBox->addItem(this_item);
      }
    }
}

void MainWindow::fill_info_box()
{
    std::string json_str(info_buffer);
    Json::Value root;
    Json::Reader reader;

    QString chan;

    statusBar()->clearMessage();

    if ( (!json_str.empty()) && reader.parse(json_str, root) ) {
      for ( Json::ArrayIndex idx = 0; idx < 2/*root.size()*/; idx++ ) {
	const Json::Value thisEntry = root[idx];
	if (!thisEntry.isObject()) continue;
	QString str_id(thisEntry["Id"].asString().c_str());
	QString str_name(thisEntry["DisplayName"].asString().c_str());
	QString str_major(thisEntry["MajorChannelNo"].asString().c_str());
	QString str_minor(thisEntry["MinorChannelNo"].asString().c_str());
	QString this_item = str_major + "." + str_minor + ": " + str_name;// + " |" + str_id;
	if (str_id.length()) {
	  chan = this_item;
	  continue;
	}
	QString str_title(thisEntry["Title"].asString().c_str());
	QString text(chan+" | "+str_title);
	statusBar()->showMessage(tr(text.toStdString().c_str()));
      }
    }
}
