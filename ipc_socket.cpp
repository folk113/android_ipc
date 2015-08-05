#include "ipc_socket.h"
static const char* TAG = (char*)"IpcSocket";

#include "socket_local_server.c"

#define DEBUG_IPC_SOCKET_DATA

IpcSocket::IpcSocket(char* socketAddr)
{
	strcpy(mSocketAddr,socketAddr);
	logD(TAG,"%s","LocalServerSocket");

	#ifdef DEBUG_IPC_SOCKET_DATA
		mTraceFile = new TraceFile("/sdcard/stream.3gp");
	#endif

	mIsThreadRunning = true;

	if (pthread_create(&mThreadId,NULL,acceptClient,this)!=0)
	{
		logE(TAG,"%s","Create thread error!\n");
	}
}

int IpcSocket::readData(char*  data,int length)
{
	if(mClientFD)
	{
		int len = read(mClientFD, data, length);
		logD(TAG,"read len:%d,data:%c",len,data);

		#ifdef DEBUG_IPC_SOCKET_DATA
		if(len > 0)
		{
			bool ret = mTraceFile->write(data,length);
		}
		#endif

		return len;
	}
	else
	{
		logD(TAG,"clientFD:%d",mClientFD);
		return -1;
	}
}
int IpcSocket::writeData(char* data,int length)
{
	if(mClientFD)
	{
		int len = write(mClientFD, data, length);
		logD(TAG,"write len:%d,data:%c",len,data);
		return len;
	}
	else
	{
		logD(TAG,"clientFD:%d",mClientFD);
		return -1;
	}
}


void* IpcSocket::acceptClient(void* arg)
{
	IpcSocket* thiz = (IpcSocket*)arg;

	thiz->mServerFD = socket_local_server(thiz->mSocketAddr,ANDROID_SOCKET_NAMESPACE_FILESYSTEM,SOCK_STREAM);

	struct sockaddr_un client_addr;
	socklen_t len = sizeof(client_addr);
	logD(TAG,"%s","server waiting:");
	while(thiz->mIsThreadRunning&&(thiz->mClientFD = accept(thiz->mServerFD, (struct sockaddr *) &client_addr,
			&len) )>0)
	{
		logD(TAG,"get clientFD:%d",thiz->mClientFD);
	}
	logD(TAG,"%s","acceptClient exit");
	return 0;
}


IpcSocket::~IpcSocket()
{
	mIsThreadRunning = false;

	if(mClientFD)
	{
		close(mClientFD);
		mClientFD = -1;
	}
	if(mServerFD)
	{
		close(mServerFD);
		mServerFD = -1;
	}
	#ifdef DEBUG_IPC_SOCKET_DATA
		if(mTraceFile)
			delete mTraceFile;
	#endif
}

