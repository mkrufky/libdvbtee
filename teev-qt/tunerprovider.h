#ifndef TUNERPROVIDER_H
#define TUNERPROVIDER_H

#include "serve.h"
#include "tune.h"

typedef std::map<uint8_t, tune*> map_tuners;

class TunerProvider
{
public:
	TunerProvider();
	~TunerProvider();

	//the following will return the new tuner id:
	int add_hdhr_tuner(uint32_t device_id, uint32_t device_ip, unsigned int tuner = 0);
	int add_hdhr_tuner(unsigned int tuner) { return add_hdhr_tuner(0, 0, tuner); }
	int add_hdhr_tuner(const char *device_str = NULL);

	int add_linuxtv_tuner();
	bool add_linuxtv_tuner(int adap, int fe, int demux, int dvr);

	tune *get_tuner(int id) { return tuners.count(id) ? tuners[id] : NULL; }

	int start_server(uint16_t port_requested, unsigned int flags = 0);
	void stop_server();
private:
	map_tuners tuners;
	serve *server;
};

#endif // TUNERPROVIDER_H
