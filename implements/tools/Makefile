TARGET = tools
SOURCES = crypto.cpp memcached.cpp client.cpp Markup.cpp http.cpp

CXX = g++
CXXFLAGES = -std=c++0x

OBJECTS = $(SOURCES:.cpp=.o)

INCLUDES = -I/usr/local/include/ClearSilver/

LIBDIRS = -L/usr/local/lib

LIBS = -ljson \
       -lcrypto \
       -lmemcached \
       -lneo_cgi -lneo_utl -lneo_cs \
       -lcurl

LDFLAGS = -Wl,-R /usr/local/lib

.PHONY : all clean

all:$(TARGET)

$(TARGET):$(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBDIRS) $(LIBS)

$(OBJECTS):%.o:%.cpp
	$(CXX) -c $(CXXFLAGES) $(INCLUDES) $< -o $@

clean:
	-$(RM) $(TARGET) $(OBJECTS)
