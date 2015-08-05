#define ANDROID_SOCKET_NAMESPACE_ABSTRACT 0
//这个系统保留，不能使用
#define ANDROID_SOCKET_NAMESPACE_RESERVED 1
#define ANDROID_SOCKET_NAMESPACE_FILESYSTEM 2
#define HAVE_LINUX_LOCAL_SOCKET_NAMESPACE
#define LISTEN_BACKLOG 4
#define FILESYSTEM_SOCKET_PREFIX "/tmp/"
///下面这个系统保留
#define ANDROID_RESERVED_SOCKET_PREFIX "/dev/socket/"

inline int socket_make_sockaddr_un(const char *name, int namespaceId,
		struct sockaddr_un *p_addr, socklen_t *socklen) {
	size_t namelen;

	memset(p_addr,0, sizeof(*p_addr));
#ifdef HAVE_LINUX_LOCAL_SOCKET_NAMESPACE

	namelen = strlen(name);

	// Test with length +1 for the *initial* '\0'.
	if ((namelen + 1) > sizeof(p_addr->sun_path)) {
		return -1;
//		return LINUX_MAKE_ADDRUN_ERROR;
	}
	p_addr->sun_path[0] = 0;
	memcpy(p_addr->sun_path + 1, name, namelen);

#else

	namelen = strlen(name) + strlen(FILESYSTEM_SOCKET_PREFIX);

	/* unix_path_max appears to be missing on linux */
	if (namelen > (sizeof(*p_addr) - offsetof(struct sockaddr_un, sun_path) - 1))
	{
		return -1;
//		return NO_LINUX_MAKE_ADDRUN_ERROR;
	}

	strcpy(p_addr->sun_path, FILESYSTEM_SOCKET_PREFIX);
	strcat(p_addr->sun_path, name);

#endif

	p_addr->sun_family = AF_LOCAL;
	*socklen = namelen + offsetof(struct sockaddr_un, sun_path) + 1;

	return 0;
}

inline int socket_local_server_bind(int s, const char *name, int namespaceId) {
	struct sockaddr_un addr;
	socklen_t alen;
	int n;
	int err;

	err = socket_make_sockaddr_un(name, namespaceId, &addr, &alen);

	if (err < 0) {
		return -1;
	}

	/* basically: if this is a filesystem path, unlink first */
#ifndef HAVE_LINUX_LOCAL_SOCKET_NAMESPACE
	if (1) {
#else
	if (namespaceId == ANDROID_SOCKET_NAMESPACE_RESERVED
			|| namespaceId == ANDROID_SOCKET_NAMESPACE_FILESYSTEM) {
#endif
		/*ignore ENOENT*/
		unlink(addr.sun_path);
	}

	n = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));

	if (bind(s, (struct sockaddr *) &addr, alen) < 0) {
		return -1;
	}

	return s;
}

inline  int socket_local_server(const char *name, int ns, int type) {
	logD(TAG, "%s", "socket_local_server");
	int err;
	int s;

	s = socket(AF_LOCAL, type, 0);
	if (s < 0)
	{
		logE(TAG, "socket create error :%d", s);
		return -1;
	}

	err = socket_local_server_bind(s, name, ns);

	if (err < 0) {
		logE(TAG, "socket_local_server_bind error:%d", err);
		close(s);
		return -1;
	}

	if (type == SOCK_STREAM) {
		int ret;

		ret = listen(s, LISTEN_BACKLOG);

		if (ret < 0) {
			logE(TAG, "socket listen error ret:%d", ret);
			close(s);
			return -1;
		}
	}

	return s;
}
;
