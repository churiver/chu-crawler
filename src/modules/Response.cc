/**
* Coopyright (c) 2013-2014
* @version 0.8.0
* @author Li Yu
* @email churiver86 at gmail.com
* @date 03/04/2014
* @description HTTP Response
*/

#include "Response.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

namespace http {


const std::string CRLF = "\r\n";
const std::string Response::CONTENT_TYPE = "Content-Type";
const std::string Response::CONTENT_ENCODING = "Content-Encoding";

const std::string Response::CONTENT_TYPE_HTML = "text/html";


Response::Response(char * recv_msg)
{
    // printf("-- Response constructor called\n");

    char * status_end = strstr(recv_msg, "\r\n");
    if ((setStatusCode(recv_msg, status_end - recv_msg) < 1) ||
        (*(status_end + 3) == '\0'))  {
        state = ERR_RESP_INVALID;
        return;
    }

    char * header_start = status_end + strlen("\r\n");
    char * header_end = strstr(header_start, "\r\n\r\n");
    if (header_end == NULL) {
        body = "";
        setHeaders(header_start, strlen(header_start));
    }
    else {
        body = std::string(header_end + strlen("\r\n\r\n"));
        setHeaders(header_start, header_end - header_start);
    }
}


 Response::~Response ( )
{
    // printf("-- Response destructor called\n");
}


int Response::setStatusCode (char * status_line, int len )
{
    char * code_begin = strchr(status_line, ' ');
    if ((code_begin == NULL) ||
        (status_code = atoi(code_begin + 1)) == 0)
        return -1;
   return status_code; 
}


void Response::setHeaders (char * header_start, int len )
{
    std::stringstream header_ss(std::string(header_start, len));
    std::string header_line;

    while (getline(header_ss, header_line, '\r')) {
        std::size_t colon_pos = header_line.find(':');
        if (colon_pos == std::string::npos)
            continue;
        // a '\n' is in each line except first one, skip it
        std::size_t line_start = (header_line[0] == '\n') ? 1 : 0;
        header_map[header_line.substr(line_start, colon_pos - line_start)] = 
            header_line.substr(colon_pos + 2); // skip ": "
    }
}


void Response::output (bool print_body )
{
    printf("\t\n-- Output Response\n\tStatus Code: %d\n", status_code);
    std::map<std::string, std::string>::iterator it;
    for (it = header_map.begin(); it != header_map.end(); it++)
        printf("\t%s: %s\n", it->first.c_str(), it->second.c_str());
    
    if (print_body)
        printf("\tBody: %s\n", body.c_str());
}


int Response::downloadFile (const std::string & url, const std::string & path)
{
    // prepare download dir. TODO handle case of ./ ../
    char cwd[256] = {0};
    if ((path[0] != '/') && (getcwd(cwd, 256) == NULL))
        return 1;
    char * dir_path = strcat(strcat(cwd, "/"), path.c_str());
    printf("downloading file to %s\n", dir_path);

    struct stat st = {0};
    if (stat(dir_path, &st) == -1) {
        mkdir(dir_path, 0700);
        printf("dir %s does not exist, creating\n", dir_path);
    }

    int len = strlen(dir_path);
    if (dir_path[len - 1] != '/') {
        dir_path[len] = '/';
        dir_path[len + 1] = '\0';
    }

    const char * file_name = getNameByUrl(url);

    std::ofstream ofs_file;
    ofs_file.open(strcat(dir_path, file_name), std::ofstream::out);
    ofs_file << body;
    ofs_file.close();
    printf("downloaded file for url %s\n", url.c_str());

    delete [] file_name;
    return 0;
}


const char * Response::getNameByUrl (const std::string & url )
{
    return getNameByUrl(url.c_str());
}


const char * Response::getNameByUrl (const char * url )
{
//  regex throw regex_error
//    std::regex expr("[\\?/:*|<>\"']");
//    std::string result = std::regex_replace(url, expre, "_");
//    return result;

    const char * url_ptr = strstr(url, "://");
    url_ptr = (url_ptr == NULL) ? url : (url_ptr + 3);
    
    size_t name_len = strlen(url_ptr);
    // TODO max name_len 64 load from conf
    name_len = (name_len < 64) ? name_len : 64;
    char * name = new char[name_len + 6]();
    char * name_ptr = name;
    
    for (int i = 0; i < name_len; i++) {
        if (*url_ptr == '/' || *url_ptr == '\\' ||
                *url_ptr == '?' || *url_ptr == ':' ||
                *url_ptr == '*' || *url_ptr == '|' ||
                *url_ptr == '<' || *url_ptr == '>' ||
                *url_ptr == '"' || *url_ptr == '\'')
            *name_ptr = '_';
        else
            *name_ptr = *url_ptr;
        name_ptr++;
        url_ptr++;
    }
    *name_ptr = '\0';

    return strcat(name, ".html");
}

};
