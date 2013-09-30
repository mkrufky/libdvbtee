#include "tunerprovider.h"

#ifdef USE_HDHOMERUN
#include "hdhr_tuner.h"
#endif
#ifdef USE_LINUXTV
#include "linuxtv_tuner.h"
#endif
#include "atsctext.h"

void TunerProvider::stop_server()
{
	if (!server)
		return;

	if (server->is_running())
		server->stop();

	delete server;
	server = NULL;

	return;
}

int TunerProvider::start_server(unsigned int flags, uint16_t port_requested)
{
	if (server) return -1;

	server = new serve;

	for (map_tuners::const_iterator iter = tuners.begin(); iter != tuners.end(); ++iter) {
		server->add_tuner(iter->second);

		if (flags & 2)
			iter->second->feeder.parser.out.add_http_server(SERVE_DEFAULT_PORT+1+iter->first);
	}
	server->set_scan_flags(0, flags >> 2);

	return server->start(port_requested);
}

TunerProvider::TunerProvider()
	: server(NULL)
{
	ATSCMultipleStringsInit();
}

TunerProvider::~TunerProvider()
{
	stop_server();

	for (map_tuners::const_iterator iter = tuners.begin(); iter != tuners.end(); ++iter) {
#if 0
		iter->second->feeder.stop_without_wait();
		iter->second->feeder.close_file();
#else
		iter->second->stop_feed();
#endif
		iter->second->close_fe();
		iter->second->feeder.parser.cleanup();
	}
	tuners.clear();
	ATSCMultipleStringsDeInit();
}

int TunerProvider::add_hdhr_tuner(const char *device_str)
{
#ifdef USE_HDHOMERUN
	int id = tuners.size();
	hdhr_tuner* hdhr  = new hdhr_tuner;
	tuners[id] = hdhr;
	//FIXME: if we don't call set_hdhr_id we get undefined effects :-/
	hdhr->set_hdhr_id(device_str ? device_str : "");
#else
	int id = -1;
#endif
	return id;
}

int TunerProvider::add_hdhr_tuner(uint32_t device_id, uint32_t device_ip, unsigned int tuner)
{
#ifdef USE_HDHOMERUN
	int id = tuners.size();
	hdhr_tuner* hdhr  = new hdhr_tuner;
	tuners[id] = hdhr;
	hdhr->set_hdhr_id(device_id, device_ip, tuner);
#else
	int id = -1;
#endif
	return id;
}

int TunerProvider::add_linuxtv_tuner()
{
#ifdef USE_LINUXTV
	int id = tuners.size();
	tuners[id] = new linuxtv_tuner;
#else
	int id = -1;
#endif
	return id;
}

bool TunerProvider::add_linuxtv_tuner(int adap, int fe, int demux, int dvr)
{
#ifdef USE_LINUXTV
	int id = tuners.size();
	linuxtv_tuner *linuxtv = new linuxtv_tuner;
	tuners[id] = linuxtv;
	linuxtv->set_device_ids(adap, fe, demux, dvr);
#else
	int id = -1;
#endif
	return id;
}
