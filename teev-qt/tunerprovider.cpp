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

int TunerProvider::start_server(unsigned int flags)
{
	if (server) return -1;

	server = new serve;

	for (map_tuners::const_iterator iter = tuners.begin(); iter != tuners.end(); ++iter) {
		server->add_tuner(iter->second);

		if (flags & 2)
			iter->second->feeder.parser.out.add_http_server(SERVE_DEFAULT_PORT+1+iter->first);
	}
	server->set_scan_flags(0, flags >> 2);

	return server->start();
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

int TunerProvider::add_hdhr_tuner()
{
#ifdef USE_HDHOMERUN
	int id = tuners.size();
	tuners[id] = new hdhr_tuner;
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
