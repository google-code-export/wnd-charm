#include <errno.h>
#include <iostream>
#include <sstream>
#include <stdarg.h>


#include "wndchrm_error.h"

using namespace std;

#define MAX_ERROR_MESSAGE 512


ostringstream error_messages;

/*
   Accumulates errors and warnings to be shown later
   N.B.: Variadic - use like printf
*/
void catError (const char *fmt, ...) {
va_list ap;
size_t err_lngth;
char error_buffer[MAX_ERROR_MESSAGE];
char newline=0;
size_t found;

	// process the printf-style parameters
	va_start (ap, fmt);
	err_lngth = vsnprintf (error_buffer,MAX_ERROR_MESSAGE, fmt, ap);
	va_end (ap);

	error_messages << error_buffer;


	if (errno != 0) {
	// Append any system error string

		strerror_r(errno, error_buffer, MAX_ERROR_MESSAGE);

		if (strlen(error_buffer)) {
			// clean trailing newlines
			found = error_messages.str().find_last_not_of("\n\r");
			if (found != std::string::npos) {
				if ( error_messages.str().find_first_of("\n\r",found) != std::string::npos ) newline = 1;
				error_messages.seekp(found+1);
			}
			error_messages << ": " << error_buffer;
			if (newline) error_messages << "\n";
		}

		errno = 0;
	}
	
	
}


/*
   displays an error message and optionally stops the program
   If stopping, shows the accumulated error_message generated by catError()
   N.B.: variadic.  First parameter is the stop flag, followed by parameters same as printf
*/
int showError(int stop, const char *fmt, ...) {
va_list ap;
size_t err_lngth;
char error_buffer[MAX_ERROR_MESSAGE];
size_t found;

// add the printf-style parameters to error_message
	if (fmt && *fmt) {
		va_start (ap, fmt);
		err_lngth = vsnprintf (error_buffer,MAX_ERROR_MESSAGE, fmt, ap);
		va_end (ap);
		if (err_lngth) error_messages << error_buffer;
	}


// Append any system error string
	if (errno != 0) {
		found = error_messages.str().find_last_not_of("\n\r");
		if (found != std::string::npos) {
			error_messages.seekp(found+1);
		}

		strerror_r(errno, error_buffer, MAX_ERROR_MESSAGE);
		error_messages << ": " << error_buffer << "\n";
	}

	if (error_messages.str().size()) {
		cerr << error_messages.str();
	// Make sure we print a newline
		found = error_messages.str().find_last_not_of("\n\r");
		if ( error_messages.str().find_first_of("\n\r",found) == std::string::npos ) cerr << "\n";
	}

	if (stop && !error_messages.str().size()) {
		cerr << "Fatal error - terminating.\n";
	} else if (stop) {
		exit(0);
	}

	return(0);
}

const std::string getErrorString () {
	return (error_messages.str());

}