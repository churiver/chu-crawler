# Copyright (c) 2013-2014
# Version 0.8.0
# Author Li Yu
# Email churiver at gmail.com
# Date 03/04/2014
# Description src Makefile


#compiler information
CC = g++
CFLAGS = -std=c++0x

SRCS = $(wildcard *.cc)
OBJS = $(patsubst %.cc, %.o, $(SRCS))

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -g -c $^

.PHONY: clean
clean:
	rm -f $(OBJS)
