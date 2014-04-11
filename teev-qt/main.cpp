#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("teev");

    bool b_read_dvr = false;
    bool b_hdhr     = false;

    unsigned int scan_flags = 0;

    int num_tuners = -1;

    int opt, channel = 0;

    /* LinuxDVB context: */
    int dvb_adap = 0; /* ID X, /dev/dvb/adapterX/ */
    int fe_id    = 0; /* ID Y, /dev/dvb/adapterX/frontendY */
    int demux_id = 0; /* ID Y, /dev/dvb/adapterX/demuxY */
    int dvr_id   = 0; /* ID Y, /dev/dvb/adapterX/dvrY */

    QString hdhrname;
    QString server("127.0.0.1");
    uint16_t port = 64080;

    while ((opt = getopt(argc, argv, "a:A:c:f:T:d::H::s:p:")) != -1) {
	    switch (opt) {
	    case 'a': /* adapter */
		    dvb_adap = strtoul(optarg, NULL, 0);
		    b_read_dvr = true;
		    break;
	    case 'A': /* ATSC / QAM */
		    scan_flags = strtoul(optarg, NULL, 0);
		    b_read_dvr = true;
		    break;
	    case 'c': /* channel list | channel / scan min */
		    channel = strtoul(optarg, NULL, 0);
		    b_read_dvr = true;
		    break;
	    case 'f': /* frontend */
		    fe_id = strtoul(optarg, NULL, 0);
		    b_read_dvr = true;
		    break;
	    case 'T': /* number of tuners (dvb adapters) allowed to use, 0 for all */
		    num_tuners = strtoul(optarg, NULL, 0);
		    b_read_dvr = true;
		    break;
	    case 'd':
		    if (optarg)
			    libdvbtee_set_debug_level(strtoul(optarg, NULL, 0));
		    else
			    libdvbtee_set_debug_level(255);
		    break;
	    case 'H':
		    hdhrname = optarg;
		    b_hdhr = true;
		    break;
	    case 's': /* remote server address */
		    server = optarg;
		    break;
	    case 'p': /* server port */
		    port = strtoul(optarg, NULL, 0);
		    break;
	    default:
		    return -1;
	    }
    }

    ServerProvider *provider = (b_read_dvr || b_hdhr) ? new ServerProvider() : NULL;

    if (b_hdhr)
	    provider->add_hdhr_tuner(hdhrname.toStdString().c_str());

    if (b_read_dvr)
	    provider->add_linuxtv_tuner(dvb_adap, fe_id, demux_id, dvr_id);


    MainWindow w(0, provider, server, port);
    w.show();

    return a.exec();
}
