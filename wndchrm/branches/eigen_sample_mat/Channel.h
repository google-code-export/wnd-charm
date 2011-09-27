#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <string>

//=====================================================================
/*! Channels
 *
 */
class Channel {
	public:
		std::string name;
		Channel (std::string &s) { name = s;}
		Channel (const char *s) { name = s;}
		void print_info() const;
		
	};
#endif // __CHANNEL_H__
