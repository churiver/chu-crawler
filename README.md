What Is chu-crawler?
===================
chu-crawler is a simple web crawler. Currently it supports:  
1. Reap links from given web page.  
2. Identify ineligible links (not html) and skip.  
3. Download the web page being crawled into local disk.  
4. Customized configuration file.  
5. Multiple crawlers via multiple threads or epoll.  

Environment
==================
chu-crawler has been tested on:  
CrunchBang (Linux Kernel 3.2.0).  


How To Run It 
==================
Simply input the following command in root dir of chu-crawler.  

    $ make

An exetuable file `chu-crawler` will be generated in the same dir.  
Edit the file `crawl.conf` and set arguments (seed url, download target-number, etc.) properly.  
The following command will start crawler based on info from `crawl.conf`.  

    $ ./chu-crawler

You are all set!  


Future Features
===================
1. Handle case of URL redirecting.  
2. Follow rules in robot.txt.  
3. And more...  

Contact
===================
Feel free to contact me for any question or report on bug.  
Suggestions are welcome.  
E-mail: churiver86 at gmail.com  
