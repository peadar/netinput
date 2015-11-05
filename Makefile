LIBS=
OBJS = uinput.o
CXXFLAGS += -std=c++11 -O0 -g
TARGETS = server client uinputd

all: $(TARGETS)

uinputd: uinput.o util.o
	$(CXX) -o $@ $^ $(LIBS)

server: server.o
	$(CXX) -o $@ $^ $(LIBS)

client: client.o
	$(CXX) -o $@ $^ $(LIBS)


clean:
	rm -f *.o $(TARGETS)
        
