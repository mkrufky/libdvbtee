#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>

#ifdef USE_JSONCPP
#include <jsoncpp/json/json.h>
#else
#include <QJsonDocument>
#endif

#include "curlhttpget.h"

// FIXME: this is just for the sleep call, which is a temporary hack:
#include <unistd.h>


#include <QStyledItemDelegate>

class ListViewDelegate : public QStyledItemDelegate
{
public:
	ListViewDelegate(QObject *parent = 0) {}
	virtual QString displayText(const QVariant &value, const QLocale &locale) const
	{
		Q_UNUSED(locale);
		QString chan_text(value.toString());
		return chan_text.remove(chan_text.indexOf("|"), chan_text.length());
	}
};


MainWindow::MainWindow(QWidget *parent, TunerProvider *provider, QString remoteServer, uint16_t remotePort) :
    QMainWindow(parent),
    dvbtee(provider),
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
    m_listBox(new QListView),
    remoteServerAddr(remoteServer),
    remoteServerPort(remotePort),
    ui(new Ui::MainWindow)
{
	setupMainWindow();
}

QString MainWindow::dvbteeServerAddr()
{
	QString host;
	char port[7];
	host = remoteServerAddr;
	snprintf(port, sizeof(port), ":%d", remoteServerPort);
	host += port;

	return host;
}

void MainWindow::setupMainWindow()
{
    ui->setupUi(this);
    this->setWindowTitle("TeeV");

    this->centralWidget()->setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

    channel_model = new QStringListModel(this);
#ifdef USE_PHONON
#if 0
    layout->addWidget(player, 0, 0);
#else
    layout->addWidget(videoWidget, 0, 0);

    // use videoWidget->setFullScreen(true); to toggle fullscreen mode
    //videoWidget->setFullScreen(true);

    Phonon::createPath(mediaObject, videoWidget);

    Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    Phonon::createPath(mediaObject, audioOutput);
#endif
#else
    player->setVideoOutput(videoWidget);

    layout->addWidget(videoWidget, 0, 0);

    videoWidget->show();
#endif
    //this->setStatusBar(0);

    //this->menuBar()->setVisible(false);
    this->removeToolBar(ui->mainToolBar);

    layout->addWidget(m_listBox, 0, 1);
    m_listBox->setMinimumWidth(120);

    m_listBox->setModel(channel_model);
    m_listBox->setItemDelegate(new ListViewDelegate(this));
    get_channels();
    if (!channel_model->rowCount()) {
	    if (!dvbtee) {
		    //
		    dvbtee = new TunerProvider;
		    //int tuner_number = dvbtee->add_hdhr_tuner();
		    //int tuner_number =
		    dvbtee->add_linuxtv_tuner();
		    //tune *thistuner = dvbtee->get_tuner(tuner_number);
	    }
	    remoteServerAddr = "127.0.0.1";
	    dvbtee->start_server(remoteServerPort);
	    get_channels();
    }

    connect(m_listBox, SIGNAL(clicked(QModelIndex)), SLOT(channel_clicked(QModelIndex)));
#ifdef USE_PHONON
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)), SLOT(playerSourceChanged(Phonon::MediaSource)));
#else
    connect(player, SIGNAL(currentMediaChanged(QMediaContent)), SLOT(playerMediaChanged(QMediaContent)));
#endif

    QPushButton *btn = new QPushButton("refresh");
    statusBar()->addPermanentWidget(btn);
    connect(btn, SIGNAL(clicked()), SLOT(refresh_clicked()));

#if 0
    tune("33~1"); // FIXME: LATER
#else
    if (channel_model->rowCount()) {
	    QString chan_text(channel_model->stringList().front());
	    //QString chan_text(channel_model->stringList().back());
	    tune(chan_text.remove(0,chan_text.indexOf("|")+1));
    }
#endif
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
    //delete player;
    delete videoWidget;
#endif
    if (dvbtee) {
	dvbtee->stop_server();
	delete dvbtee;
	dvbtee = NULL;
    }
    delete layout;
    delete ui;
}

void MainWindow::channel_clicked(QModelIndex index)
{
	QString chan_text(index.data().toString());
	tune(chan_text.remove(0,chan_text.indexOf("|")+1));
}

void MainWindow::tune(QString chan_id)
{
    QUrl url("http://"+dvbteeServerAddr()+"/tune="+ chan_id +"/stream/");
    cur_chan_id = chan_id;
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
	QString channels_url("http://"+dvbteeServerAddr()+"/json/channels");
	curlhttpget Curl(channels_url.toStdString().c_str(), get_channels_callback, this);
	fill_channels_box();
	channels_buffer.clear();
}

void MainWindow::get_info()
{
	info_buffer.clear();
	QString info_url("http://"+dvbteeServerAddr()+"/json/info="+ cur_chan_id);
	curlhttpget Curl(info_url.toStdString().c_str(), get_info_callback, this);
	fill_info_box();
	info_buffer.clear();
}

void MainWindow::fill_channels_box()
{
    QStringList ChannelList;
#ifdef USE_JSONCPP
    std::string json_str(channels_buffer);

    Json::Value root;
    Json::Reader reader;

    if ( (!json_str.empty()) && reader.parse(json_str, root) ) {
      for ( Json::ArrayIndex idx = 0; idx < root.size(); idx++ ) {
	const Json::Value thisEntry = root[idx];
	if (!thisEntry.isObject()) continue;
	QString str_id(thisEntry["Id"].asString().c_str());
	QString str_name(thisEntry["DisplayName"].asString().c_str());
	QString str_major(thisEntry["MajorChannelNo"].asString().c_str());
	QString str_minor(thisEntry["MinorChannelNo"].asString().c_str());
	QString this_item = str_major + "." + str_minor + ": " + str_name + " |" + str_id;
	if (str_id.length()) ChannelList << this_item;
      }
    }
#else
    if (!channels_buffer.length())
	return;

    QJsonDocument d = QJsonDocument::fromJson(QString(channels_buffer.c_str()).toUtf8());
    QJsonArray a = d.array();

    foreach (const QJsonValue & v, a)
    {
	QJsonObject thisEntry = v.toObject();

	QString str_id(thisEntry["Id"].toString());
	QString str_name(thisEntry["DisplayName"].toString());
	QString str_major(thisEntry["MajorChannelNo"].toString());
	QString str_minor(thisEntry["MinorChannelNo"].toString());
	QString this_item = str_major + "." + str_minor + ": " + str_name + " |" + str_id;

	if (str_id.length()) ChannelList << this_item;
    }
#endif
    channel_model->setStringList(ChannelList);
}

void MainWindow::fill_info_box()
{
    QString chan;

    statusBar()->clearMessage();
#ifdef USE_JSONCPP
    std::string json_str(info_buffer);
    Json::Value root;
    Json::Reader reader;

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
#else
    if (!info_buffer.length())
	return;

    QJsonDocument d = QJsonDocument::fromJson(QString(info_buffer.c_str()).toUtf8());
    QJsonArray a = d.array();

    foreach (const QJsonValue & v, a)
    {
	QJsonObject thisEntry = v.toObject();

	QString str_id(thisEntry["Id"].toString());
	QString str_name(thisEntry["DisplayName"].toString());
	QString str_major(thisEntry["MajorChannelNo"].toString());
	QString str_minor(thisEntry["MinorChannelNo"].toString());
	QString this_item = str_major + "." + str_minor + ": " + str_name;// + " |" + str_id;
	if (str_id.length()) {
	    chan = this_item;
	    continue;
	}

	QString str_title(thisEntry["Title"].toString());
	QString text(chan+" | "+str_title);
	statusBar()->showMessage(tr(text.toStdString().c_str()));
	// force exit after first (now) entry
	return;
    }
#endif
}
