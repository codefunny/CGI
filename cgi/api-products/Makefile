TARGET = api-products
INSTALL_PATH = /huishoubao/cgi

include ../../implements/log/Makefile.log
include ../../implements/tools/Makefile.tools
include ../../implements/db/Makefile.db
include ../../implements/configure/Makefile.configure
include ../../implements/protocol/Makefile.protocol
include ../../implements/user/Makefile.user
include ../../implements/logic/Makefile.logic

vpath %.cpp ../../implements/log
vpath %.cpp ../../implements/tools
vpath %.cpp ../../implements/db
vpath %.cpp ../../implements/configure
vpath %.cpp ../../implements/protocol
vpath %.cpp ../../implements/user
vpath %.cpp ../../implements/logic

INCLUDES = -I/usr/local/include/log4cplus \
           -I/usr/local/include/ClearSilver \
           -I/usr/include/mysql \
           -I../../implements/log \
           -I../../implements/tools \
           -I../../implements/db \
           -I../../implements/configure \
           -I../../implements/protocol \
           -I../../implements/user \
           -I../../implements/logic

LIBDIRS = -L/usr/local/lib \
          -L/usr/lib64/mysql

LIBS += -lfcgi \
        -ljson \
        -lmysqlclient \
        -llog4cplus -lpthread \
        -lcrypto \
        -lmemcached \
        -lneo_cgi -lneo_utl -lneo_cs \
        -lcurl

CC = gcc
CXX = g++

CXXFLAGES += -Wall -O2
LDFLAGS = -Wl,-R /usr/local/lib

RM = rm -f
CP = cp -f

SOURCES += api-products.cpp 

OBJECTS = $(SOURCES:.cpp=.o)

.PHONY : all clean install mem

all:$(TARGET)

mem:
	$(CXX) -c -std=c++0x ../../implements/tools/memcached.cpp -o memcached.o

$(TARGET):$(OBJECTS)
	$(CXX) -o $(TARGET) memcached.o $(OBJECTS) $(LDFLAGS) $(LIBDIRS) $(LIBS)

$(OBJECTS):%.o:%.cpp
	$(CXX) -c $(CXXFLAGES) $(INCLUDES) $< -o $@

clean:
	-$(RM) $(TARGET) $(OBJECTS) memcached.o

install:
	-$(CP) $(TARGET) $(DESTDIR)$(INSTALL_PATH)/$(TARGET)
	-$(CP) ../../implements/log/log4cplus.properties $(DESTDIR)/data/log/
