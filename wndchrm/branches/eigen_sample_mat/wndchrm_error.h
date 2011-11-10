#ifndef __WNDCHRM_ERROR_H__
#define __WNDCHRM_ERROR_H__

enum WNDCHRM_ERROR {
	WC_UNINITIALIZED,
	WC_NO_ERROR,
	WC_FAIL_NULL_POINTER,
	WC_FAIL_RECURSIVE_CALL,
	WC_TRANSFORM_FAIL,
	WC_EMPTY,
	WC_TRANSFORM_NOT_IN_PHONEBOOK,
	WC_NOT_IMPLEMENTED
};


void catError (const char *fmt, ...);
int showError(int stop, const char *fmt, ...);
const std::string getErrorString ();

#endif // __WNDCHRM_ERROR_H__
