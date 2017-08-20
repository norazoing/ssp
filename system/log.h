/*
#define elog(a)		{ \
                        LogLock(); \
			            WriteError("%s %s(%d)", \
			            __FILE__,__FUNCTION__,__LINE__); \
			            WriteError a ; WriteError("\n"); \
			            LogUnlock(); \
			        }
*/
#define elog_main(a)	{ \
			            	LogMain("%s %s(%d)", __FILE__, __FUNCTION__, __LINE__); \
				            LogMain a ; LogMain("\n"); \
				        }

short InitLog(void);
void LogMain(const char * fmt, ...);
void LogGPS(const char * fmt, ...);
void LogTerm(const char * fmt, ...);
void LogDCS(const char * fmt, ...);
