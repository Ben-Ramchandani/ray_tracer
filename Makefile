CPP = g++
CPPFLAGS = -Wall -pedantic -O3
TESTFLAGS = -Wall -pedantic -g
SOURCES = pnmout.cpp trace.cpp world.cpp
DEP = ray.h Makefile
OBJECTS=$(SOURCES:.cpp=.o)
TESTEXES=$(SOURCES:.cpp=)
EXE=ray
IMAGE=out.png

all: $(EXE)

$(EXE): $(OBJECTS) $(DEP)
	$(CPP) $(CPPFLAGS) $(OBJECTS) -o $@

%.o: %.cpp $(DEP)
	$(CPP) $(CPPFLAGS) -c $< -o $@

%: %.cpp $(DEP)
	$(CPP) $(TESTFLAGS) $< -o $@

clean:
	-rm -f $(TESTEXES) *.o $(EXE)

run: all
	./$(EXE)

$(IMAGE): all
	./$(EXE) | pnmtopng > $(IMAGE)

disp: $(IMAGE)
	eog $(IMAGE)

runtrace: trace
	./trace
