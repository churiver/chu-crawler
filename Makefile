# Copyright (c) 2013-2014
# Author Li Yu
# Email churiver at gmail.com
# Create-Date 03/04/2014
# Description root Makefile


TARGET = chu-crawl

$(TARGET):
	cd ./src/lib && make
	cd ./src/modules && make
	cd ./src && make
	g++ -g -lpthread -std=c++0x ./src/lib/*.o ./src/modules/*.o ./src/*.o -o $(TARGET)

.PHONY: clean
clean:
	cd ./src && make clean
	cd ./src/modules && make clean
	cd ./src/lib && make clean
	rm $(TARGET)
