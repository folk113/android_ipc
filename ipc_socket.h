#ifndef __IPC_SOCKET__
#define __IPC_SOCKET__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "log.h"
#include <pthread.h>
#include "trace_file.h"

class IpcSocket
{
	public:
		IpcSocket(char* socketAddr);
		~IpcSocket();
		int readData(char*  data,int length);
		int writeData(char* data,int length);
	private:
		static void* acceptClient(void* arg);
		pthread_t mThreadId;
		bool mIsPasedHead;
		int mServerFD;
		int mClientFD;
		char mSocketAddr[100];
		volatile bool mIsThreadRunning;
		TraceFile* mTraceFile;
};

#endif
