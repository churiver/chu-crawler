/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 03/04/2014
* @description http utilities
*/

#include "httpUtil.h"

#include <cstring>

#include <string>
#include <algorithm>

namespace http {


const std::string CONTENT_TYPE("Content-Type");
const std::string CONTENT_ENCODING("Content-Encoding");
const std::string CONTENT_TYPE_HTML("text/html");
const std::string ALLOWED_FORMAT(".html.htm.php.do.jsp.asp.apsx");

#define MAX_FILENAME_LEN 64


char * setRequest (const char * host, const char * path )
{
    char * req_msg = new char[512];
    sprintf(req_msg,
            "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n\r\n",
            path, host);
    return req_msg;
}


int validateUrl (const std::string & urlstr )
{
    // for urlstr like: javascript:toggleBibtex('brooks2012')
    std::size_t colon_pos= urlstr.find(':');
    if ((colon_pos != std::string::npos) &&
            (urlstr.substr(0, colon_pos).find("javascript") != std::string::npos)) {
        return -1;
    }

    std::size_t scheme_end = urlstr.find("://");
    scheme_end = (scheme_end == std::string::npos) ? 0 : scheme_end + 3;

    std::size_t last_slash_pos = urlstr.rfind('/');
    last_slash_pos = (last_slash_pos == std::string::npos) ? 0 : last_slash_pos;

    if (last_slash_pos < scheme_end) { // '/' is inside "://"
        return 0;
    }

    if (urlstr.back() == '/') {
        return 0;
    }

    std::size_t last_dot_pos = urlstr.rfind('.');
    if ((last_dot_pos == std::string::npos) ||
            (last_dot_pos < last_slash_pos)) { // www.abc.com/
        return 0;
    }
    
    if (urlstr.find('?', last_dot_pos + 1) != std::string::npos) {
        return 0; // urlstr with ? must be web page?
    }
    
    std::string format = urlstr.substr(last_dot_pos);
    std::transform(format.begin(), format.end(), format.begin(), ::tolower);

    if (ALLOWED_FORMAT.find(format) != std::string::npos) {
        return 0;
    }
    else {
        return -1;
    }

    // to cover cases like http://docs.spring.io/spring/docs/3.0.x,
    //  in the future may 1. request HEAD for urlstr of type 0,
    //  and check content-type in response, then decide GET or not.
}


int normalizeUrl (std::string & urlstr )
{
    if (urlstr.size() == 0) {
        return -1;
    }

    // 1. trim spaces " '
    std::size_t url_pos = urlstr.find_first_not_of(" \t\r\n\f");
    std::size_t url_end = urlstr.find_last_not_of(" \t\r\n\f");
    if (url_pos == std::string::npos) {
        return -1;
    }

    if (urlstr[url_pos] == '"' || urlstr[url_pos] == '\'') {
        if (urlstr[url_end] != urlstr[url_pos]) {
            return -1;
        }
        url_pos++;
        url_end--;
    }
    // "else if" is for case like:
    //  <script language="javascript">
    //      var bdis = "<a href=\"http://news.abc.com\">
    else if ((urlstr.find("\\'", url_pos) == url_pos) ||
             (urlstr.find("\\\"", url_pos) == url_pos)) {
        if ((urlstr[url_end - 1] != '\\') || 
            (urlstr[url_end] != urlstr[url_pos + 1])) {
            return -1;
        }
        url_pos += 2;
        url_end -= 2;
    }

    if (url_pos != 0 || url_end != (urlstr.size() - 1)) {
        urlstr = urlstr.substr(url_pos, url_end - url_pos + 1);
    }

    // 2. complete trailing '/' and lower case of host if existing
    if (strcasecmp(urlstr.c_str(), "http") != 0) {
        return 0; // here is relative urlstr, no need for tailing '/'
    }
    std::size_t host_end = urlstr.find('/', 8); // skip http(s)://
    std::string::iterator host_end_it;
    if (host_end == std::string::npos) {
        urlstr.push_back('/');
        host_end_it = urlstr.end();
    }
    else {
        host_end_it = urlstr.begin() + host_end;
    }

    std::transform(urlstr.begin(), host_end_it, urlstr.begin(), ::tolower);

    return 0; // absolute url 
}


int resolveUrl (std::string & urlstr, const std::string & host, const std::string & path )
{
   if ((urlstr.size() == 0) || (urlstr.find("http") == 0)) {
       return 0; // absolute url
   }
   else if (urlstr.find("http") != std::string::npos) {
       return -1; // bad url
   }

   if (urlstr[0] == '/') { // abs-relative url
       urlstr = host + urlstr;
   }
   else { // relative url
       urlstr = host + path + urlstr;
   }

   return 0;
}


const std::string urlToFileName (std::string urlstr )
{
    if (urlstr.size() > MAX_FILENAME_LEN) {
        urlstr = urlstr.substr(0, MAX_FILENAME_LEN);
    }

    std::size_t protocol = urlstr.find("://");
    std::size_t last_slash = urlstr.find_last_of('/');
    std::size_t last_dot = urlstr.find_last_of('.');
    
    if ((std::string::npos == last_slash) || // www.abc.com
        (protocol == (last_slash - 2)) ||    // http://www.abc.com
        (last_dot < last_slash)) {           // http://www.abc.com
        urlstr += ".html";
    }

    std::string::iterator it = urlstr.begin();
    for (it; it < urlstr.end(); it++) {
        if (*it == '/' || *it == '\\' || *it == '?' || 
            *it == ':' || *it == '*' || *it == '|' ||
            *it == '<' || *it == '>' || *it == '"' || 
                *it == '\'') {
            *it = '_';
        }        
    }
    return urlstr;
}

};
