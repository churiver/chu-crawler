/**
* Coopyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @date 03/04/2014
* @description HTTP utilities
*/

#include <cstring>

#include <string>
#include <algorithm>

namespace http {


int checkUrlType (const std::string & url )
{
    // for url like: javascript:toggleBibtex('brooks2012')
    std::size_t colon_pos= url.find(':');
    if ((colon_pos != std::string::npos) &&
            (url.substr(0, colon_pos).find("javascript") != std::string::npos))
        return 1;

    std::size_t scheme_end = url.find("://");
    scheme_end = (scheme_end == std::string::npos) ? 0 : scheme_end + 3;

    std::size_t last_slash_pos = url.rfind('/');
    last_slash_pos = (last_slash_pos == std::string::npos) ? 0 : last_slash_pos;

    if (last_slash_pos < scheme_end) // '/' is inside "://"
        return 0;

    if (url.back() == '/')
        return 0;

    std::size_t last_dot_pos = url.rfind('.');
    if ((last_dot_pos == std::string::npos) ||
            (last_dot_pos < last_slash_pos)) // www.abc.com/
        return 0;
    
    if (url.find('?', last_dot_pos + 1) != std::string::npos)
        return 0; // url with ? must be web page?
    
    std::string extension = url.substr(last_dot_pos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if ((extension.compare(0, 3, "htm") == 0) ||
            (extension.compare(0, 3, "php") == 0) ||
            (extension.compare(0, 3, "asp") == 0) ||
            (extension.compare(0, 3, "jsp") == 0) ||
            (extension.compare(0, 2, "do") == 0))
        return 0;
    else
        return 1;

    // to cover cases like http://docs.spring.io/spring/docs/3.0.x,
    //  in the future may 1. request HEAD for url of type 0,
    //  and check content-type in response, then decide GET or not.
    //  2. put common extension names in set and compare with them.
}


int normalizeUrl (std::string & url )
{
    if (url.size() == 0)
        return 1;
    // 1. trim spaces " '
    std::size_t url_pos = url.find_first_not_of(" \t\r\n\f");
    std::size_t url_end = url.find_last_not_of(" \t\r\n\f");
    if (url_pos == std::string::npos)
        return 2;
    if (url[url_pos] == '"' || url[url_pos] == '\'') {
        if (url[url_end] != url[url_pos])
            return 3;
        url_pos++;
        url_end--;
    }
    // else if is for case as:
    //  <script language="javascript">
    //      var bdis = "<a href=\"http://news.rpi.edu\">
    else if (url.find("\\'", url_pos) == url_pos ||
        url.find("\\\"", url_pos) == url_pos)
    {
        if (url[url_end - 1] != '\\' || url[url_end] != url[url_pos + 1])
            return 1;
        url_pos += 2;
        url_end -= 2;
    }

    if (url_pos != 0 || url_end != (url.size() - 1))
        url = url.substr(url_pos, url_end - url_pos + 1);

    // 2. complete trailing / and lower case of host if existing
    if (strcasecmp(url.c_str(), "http") != 0)
        return 0; // relative url
    std::size_t host_end = url.find('/', 8); // skip http(s)://
    if (host_end == std::string::npos)
        url.push_back('/');
    std::transform(url.begin(), url.end(), url.begin(), ::tolower);

    return 0; // abs url    
}


int resolveUrl (std::string & url, const std::string & base_host, const std::string & path )
{
   if (url.size() == 0 || url.find("http") == 0)
       return 0; // abs url
   else if (url.find("http") != std::string::npos)
       return 2; // bad url

   if (url[0] == '/') // abs-relative url
       url = base_host + url;
   else // relative url
       url = base_host + path + url;

   return 1;
}

};
