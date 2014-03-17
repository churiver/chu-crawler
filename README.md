What Is chu-crawler?
===================
chu-crawler is a simple web crawler. Currently it supports:  
1. Reap links from a given web page.  
2. Identify ineligible links (not html) and skip.  
3. Download the web page being crawled.  

Environment
==================
chu-crawler has been tested on:  
CrunchBang (Linux Kernel 3.2.0).  


How To Run It 
==================
Simply input the following command in root dir of chu-crawler.  

    $ make

An exetuable file `chu-crawler` will be generated in the same dir.  
The following command will start crawler to work on [url].

    $ ./chu-crawler [url]

You are all set!  


Future Features
===================
1. Customized configuration file.  DONE!
2. Multiple crawlers via multiple threads or epoll.  
3. Handle case of URL redirecting.  
4. Follow rules in robot.txt.  
5. And more...  

Contact
===================
Feel free to contact me for any question.  
Suggestions are welcome.  
e-mail: churiver86 at gmail.com  
