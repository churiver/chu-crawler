# Copyright (c) 2013-2014
# author Li Yu
# email churiver at gmail.com
# create-Date 04/12/2014
# description lib Makefile


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
