#ifdef __DEBUG
	#define DEBUG(format,...) printf("FILE: "__FILE__", LINE: %d: "format"/n", __LINE__, ##__VA_ARGS__)
#else
	#define DEBUG(info)
    #endif
