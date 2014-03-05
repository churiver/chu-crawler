# Copyright (c) 2013-2014
# Version 0.8.0
# Author Li Yu
# Email churiver at gmail.com
# Date 03/04/2014
# Description root Makefile


TARGET = chu-crawl

$(TARGET):
	cd ./src/modules && make
	cd ./src && make
	g++ -g -std=c++0x ./src/modules/*.o ./src/*.o -o $(TARGET)

.PHONY: clean
clean:
	cd ./src && make clean
	cd ./src/modules && make clean
	rm $(TARGET)
