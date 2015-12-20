#ifndef _IOOS_HTTP_LOG_
#define _IOOS_HTTP_LOG_

void closelog(void);
void openlog(void);
int log_write(char*string);

#define LOG_LEN_LEN 102400
#define LOGFILE "/tmp/ioos.log"
#define DEBUG(fmt,args...) do {char tmp[16384];snprintf(tmp,16384,"[%s :%d][%ld]" fmt"\n",__FUNCTION__, __LINE__,time(NULL), ##args);log_write(tmp);}while(0)
#define DEBUG_E(fmt,args...) do {char tmp[16384];snprintf(tmp,16384,"ERROR:[%s :%d][%ld]" fmt"\n",__FUNCTION__, __LINE__,time(NULL), ##args);log_write(tmp);}while(0)
#endif
