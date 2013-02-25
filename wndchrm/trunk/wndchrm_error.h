#ifndef __WNDCHRM_ERROR_H__
#define __WNDCHRM_ERROR_H__

#include <string>
void catError (const char *fmt, ...);
int showError(int stop, const char *fmt, ...);
const std::string getErrorString ();

#endif // __WNDCHRM_ERROR_H__
