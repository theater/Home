#ifndef CGI_H
#define CGI_H
#define INFO os_printf

#include "httpd.h"

int cgiLed(HttpdConnData *connData);
void tplLed(HttpdConnData *connData, char *token, void **arg);
int cgiGPIO(HttpdConnData *connData);
void tplGPIO(HttpdConnData *connData, char *token, void **arg);
int cgiReadFlash(HttpdConnData *connData);
void tplCounter(HttpdConnData *connData, char *token, void **arg);

#endif
