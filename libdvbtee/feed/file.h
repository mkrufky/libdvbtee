#ifndef FILE_H
#define FILE_H

#include "fdfeeder.h"

namespace dvbtee {

namespace feed {

class FileFeeder : public FdFeeder
{
public:
	FileFeeder();
	virtual ~FileFeeder();

	virtual int start();

	int openFile(char* new_file, int flags = 0) { setFilename(new_file); return doOpenFile(flags); }

private:
	void        *file_feed_thread();
#if USE_IOS_READ
	void    *ios_file_feed_thread();
#endif
	static void *file_feed_thread(void*);

	void setFilename(char* newFile);
	int doOpenFile(int flags);
};

}

}

#endif // FILE_H
