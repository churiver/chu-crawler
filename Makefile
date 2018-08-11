# Copyright (c) 2013-2014
# author Li Yu
# email churiver at gmail.com
# create-Date 03/04/2014
# description root Makefile


TARGET = chu-crawl

$(TARGET):
	cd ./src/lib && make
	cd ./src && make
	g++ -g -pthread -std=c++0x ./src/lib/*.o ./src/*.o -o $(TARGET)

.PHONY: clean
clean:
	cd ./src && make clean
	cd ./src/lib && make clean
	rm $(TARGET)
