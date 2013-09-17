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
	int add_hdhr_tuner();
	int add_linuxtv_tuner();

	tune *get_tuner(int id) { return tuners.count(id) ? tuners[id] : NULL; }

	int start_server(unsigned int flags = 0);
	void stop_server();
private:
	map_tuners tuners;
	serve *server;
};

#endif // TUNERPROVIDER_H
