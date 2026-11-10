#pragma once
#include <unordered_map>


#include "curl.h"


#pragma comment (lib, "libcurl_a.lib")
#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "Crypt32.lib" )
#pragma comment (lib, "advapi32.lib")

static size_t my_write(void* buffer, size_t size, size_t nmemb, void* param)
{
	std::string& text = *static_cast<std::string*>(param);
	size_t totalsize = size * nmemb;
	text.append(static_cast<char*>(buffer), totalsize);
	return totalsize;
}
class c_globals {
public:
	bool active = true;
	bool active2 = false;
	char user_name[255] = "";
	char pass_word[255] = "";
	char key[1000] = "";
	char redeem_key[1000] = "";
	std::string time_left;
	char user_hwid[1000];
	std::string user_hwid2;
	std::string subscribed;


};

inline c_globals globals;

namespace Settings
{
	static int Tab = 0;
}

namespace User
{
	static int role = -1;
	static int hwid = -1;
}

