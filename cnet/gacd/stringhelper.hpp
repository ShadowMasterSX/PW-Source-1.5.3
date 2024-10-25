#ifndef __GNET_STRINGHELPER_HPP
#define __GNET_STRINGHELPER_HPP

#include <string>
#include <vector>
#include "octets.h"

namespace GNET
{

class StringHelper
{
public:
	typedef std::vector<std::pair<std::string, std::string> > string_pair_vec;
	static std::string trim(std::string const& source, char const* delims = " \t\r\n"); 
	static string_pair_vec parse_string_pair(const std::string &str);
	static std::string utf8_to_iso1(const std::string &str);
	static int ipstr_to_int(const std::string &str);
	static Octets hexstr_to_octets(std::string str);
};

};

#endif
