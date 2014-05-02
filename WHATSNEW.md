Version 0.9.6 - May 1st, 2014
==============================
Update
- Provide a new macro LOGnPRINT to print log on console while writing to log file.

Fix
- Initially there is only one thread pool running both tasks of handleUrl/handleResponse. In case all threads are in handleResponse and adding handleUrl task to pool, the task queue in pool will be full and all threads blocking in adding task since no thread is running handleUrl which will take task from queue.
  Now put the tasks of handling url and response in separate thread pools to avoid block.
- Logger was set to throw when the Level is ERROR, which leads to unhandling exit of crawl and threadpool object.
  Now log of ERROR level will not cause process exit.


Version 0.9.5 - April 29th, 2014
==============================
Update
- Refactor code with epoll and half-sync/half-async multi-threads pattern.
- Provide logger as lib.


Version 0.9.0 - April 16th, 2014
==============================
Update
- Provide blocking queue and thread pool as lib.


Version 0.8.5 - March 16th, 2014
==============================
Update
- Load config (max download url number, seed urls, etc.) from file.
- Support multithread now, each worker running in a thread.
- Redirect debug output to log file.

Fix
- A segfault might be caused when HTTP response exceeds MAX_RESP_LEN (which is 1 Mb now).
- Untimely return from pasred_url/response/request constructor leaves pointers in such objects un-initialized, which leads to memory error.


Version 0.8.0 - March 4th, 2014
==============================
First runnable version of chu-crawler!
