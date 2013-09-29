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
    int dvb_adap = -1; /* ID X, /dev/dvb/adapterX/ */
    int fe_id    = -1; /* ID Y, /dev/dvb/adapterX/frontendY */
    int demux_id = 0; /* ID Y, /dev/dvb/adapterX/demuxY */
    int dvr_id   = 0; /* ID Y, /dev/dvb/adapterX/dvrY */

    char hdhrname[256];
    memset(&hdhrname, 0, sizeof(hdhrname));

    while ((opt = getopt(argc, argv, "a:A:c:f:T:d::H::")) != -1) {
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
		    if (optarg)
			    strcpy(hdhrname, optarg);
		    b_hdhr = true;
		    break;
	    default:
		    return -1;
	    }
    }

    TunerProvider *provider = (b_read_dvr || b_hdhr) ? new TunerProvider() : NULL;

    if (b_hdhr)
	    provider->add_hdhr_tuner(); // FIXME: specify

    if (b_read_dvr)
	    provider->add_linuxtv_tuner(); // FIXME: specify


    MainWindow w(0, provider);
    w.show();

    return a.exec();
}
