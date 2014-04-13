# Copyright (c) 2013-2014
# Author Li Yu
# Email churiver at gmail.com
# Create-Date 03/04/2014
# Description modules Makefile


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
