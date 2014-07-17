#############################################################################
#
# Makefile for raspiBasdgerLogger on Raspberry Pi
#
# License: GPL (General Public License)
# Author:  TheBogueRat <bogue.rat.creations@gmail.com>
# Date:    2014/06/19 (version 0.1)
#
# Description:
# ------------
# use ‘make all’ and ‘make install’ to install the program
# You can change the install directory by editing the installDir line
#
installDir := /opt/raspiBadger

# The recommended compiler flags for the Raspberry Pi
CCFLAGS=-Wall -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

# The compiler flags for MySQL
MYSQLCFLAGS= -I/usr/include/mysql -DBIG_JOINS=1  -fno-strict-aliasing  -g
MYSQLLIBS= -L/usr/lib/arm-linux-gnueabihf -lmysqlclient -lpthread -lz -lm -lrt -ldl

# define the program 
PROGRAM = raspiBadgerLogger
SOURCES = ${PROGRAM:=.cpp}

all: ${PROGRAM}

${PROGRAM}: ${SOURCES}
	g++ ${CCFLAGS} -L../librf24/  -lrf24 $@.cpp -o $@ ${MYSQLCFLAGS} ${MYSQLLIBS}

clean:
	rm -rf $(PROGRAM)

install: all
	test -d $(prefix) || mkdir $(prefix)
	test -d $(prefix)/bin || mkdir $(prefix)/bin
	for prog in $(PROGRAM); do \
	  install -m 0755 $$prog $(prefix)/bin; \
	done

.PHONY: install
