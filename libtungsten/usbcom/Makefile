## Settings

CXX=g++
CXXFLAGS=-std=c++11 -Wall -g
LFLAGS=-lusb-1.0
OBJS=USBCom.o


## RULES

.PHONY: clean

all: usbcom_dump usbcom_write

usbcom_dump: usbcom_dump.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LFLAGS)

usbcom_write: usbcom_write.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LFLAGS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LFLAGS)

clean: 
	rm -f usbcom_dump usbcom_write *.o
