/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 03/04/2014
* @description http response
*/

#include "httpResponse.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

#include "lib/Logger.h"

namespace http {


const char * CRLF = "\r\n";
const char * CRLFx2 = "\r\n\r\n";


Response::Response(const char * resp_msg)
{
    const char * status_end = strstr(resp_msg, CRLF);
    if ((setStatusCode(resp_msg, status_end - resp_msg) != 0) ||
            (*(status_end + 3) == '\0'))  {
        state = ERR_RESP_INVALID;
        return;
    }

    const char * header_start = status_end + strlen(CRLF);
    const char * header_end = strstr(header_start, CRLFx2);
    if (nullptr == header_end) {
        body = "";
        setHeaders(header_start, strlen(header_start));
    }
    else {
        body = std::string(header_end + strlen(CRLFx2));
        setHeaders(header_start, header_end - header_start);
    }
}


Response::~Response ( ) { }


int Response::setStatusCode (const char * status_line, int len )
{
    const char * code_begin = strchr(status_line, ' ');
    if ((nullptr == code_begin) ||
            (0 == (status_code = atoi(code_begin + 1)))) {
        return -1;
    }
    return 0; 
}


void Response::setHeaders (const char * header_start, int len )
{
    std::stringstream header_ss(std::string(header_start, len));
    std::string header_line;

    while (getline(header_ss, header_line, '\r')) {
        std::size_t colon_pos = header_line.find(':');
        if (std::string::npos == colon_pos) {
            continue;
        }
        // a '\n' is in each line except first one, skip it
        std::size_t line_start = (header_line[0] == '\n') ? 1 : 0;
        header_map[header_line.substr(line_start, colon_pos - line_start)] = 
            header_line.substr(colon_pos + 2); // skip ": "
    }
}


void Response::output (bool print_body )
{
    LOG(DEBUG3) << "Response\n\tStatus Code: " << status_code;
    //fprintf(stderr, "\t\n-- Output Response\n\tStatus Code: %d\n", status_code);
    std::map<std::string, std::string>::iterator it;
    for (it = header_map.begin(); it != header_map.end(); it++) {
        LOG(DEBUG3) << "\t" << it->first << ": " << it->second;
        //fprintf(stderr, "\t%s: %s\n", it->first.c_str(), it->second.c_str());
    }
    
    if (true == print_body) {
        LOG(DEBUG3) << "\t" << body;
        //fprintf(stderr, "\tBody: %s\n", body.c_str());
    }
}


int Response::download (const std::string & dirname, const std::string & filename)
{
    fprintf(stderr, "download string verstion\n");
    return download(dirname.c_str(), filename.c_str());
}

int Response::download (const char * dirname, const char * filename)
{
    static char * _dirname = nullptr, * _fullpath = nullptr;
    // TODO mod to read global dirname. not strcmp evertime
    if ((nullptr == _dirname) || (strcmp(_dirname, dirname) != 0)) {
        // create download dir if nonexist. TODO handle case of ./ ../
        char cwd[256] = {0};
        if ((nullptr == getcwd(cwd, 256)) && (dirname[0] != '/')) {
            return -1;
        }
        _dirname = strdup(dirname);
        _fullpath = strcat(strcat(cwd, "/"), _dirname);

        struct stat st = {0};
        if (stat(_fullpath, &st) == -1) {
            mkdir(_fullpath, 0700);
            LOG(INFO) << "creating download dir " << _fullpath;
            //fprintf(stdout, "dir %s does not exist, creating\n", _fullpath);
        }

        int len = strlen(_fullpath);
        if (_fullpath[len - 1] != '/') {
            _fullpath[len] = '/';
            _fullpath[len + 1] = '\0';
        }
        LOG(INFO) << "download dir is " << _fullpath;
        //fprintf(stderr, "download path is %s\n", _fullpath);
    }

    std::ofstream ofs_file;
    ofs_file.open(strcat(_fullpath, filename), std::ofstream::out);
    ofs_file << body;
    ofs_file.close();
    LOG(DEBUG3) << "downloaded " << filename;
    //fprintf(stderr, "downloaded %s to %s\n", filename, dirname);

    //delete [] filename;
    return 0;
}


//const char * Response::getFileName (const std::string & url )
//{
//    return getFileName(url.c_str());
//}


//const char * Response::getFileName (const char * url )
//{
//  regex throw regex_error
//    std::regex expr("[\\?/:*|<>\"']");
//    std::string result = std::regex_replace(url, expre, "_");
//    return result;
/*
    const char * url_ptr = strstr(url, "://");
    url_ptr = (nullptr == url_ptr) ? url : (url_ptr + 3);
    
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
                *url_ptr == '"' || *url_ptr == '\'') {
            *name_ptr = '_';
        }
        else {
            *name_ptr = *url_ptr;
        }
        name_ptr++;
        url_ptr++;
    }
    *name_ptr = '\0';

    return strcat(name, ".html");
}*/

};