LIBS=
OBJS = uinput.o
CXXFLAGS += -std=c++11 -O0 -g

uinputd: $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJS) uinputd
        
