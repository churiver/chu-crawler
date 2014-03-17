Version 0.8.5 - March 16th, 2014
==============================
Update
- Load config (max download url number, seed urls, etc.) from file.
- Support multithread now, each worker running in a thread.
- Redirect debug output to log file.

Fix
- A segfault might be caused when HTTP response exceeds MAX_RESP_LEN (which is 1 Mb now)
- Untimely return from pasred_url/response/request constructor leaves pointers in such objects un-initialized, which leads to memory error.


Version 0.8.0 - March 4th, 2014
==============================
First runnable version for chu-crawler!
