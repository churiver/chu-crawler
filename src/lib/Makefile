# Copyright (c) 2013-2014
# Author Li Yu
# Email churiver at gmail.com
# Create-Date 04/12/2014
# Description lib Makefile


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
