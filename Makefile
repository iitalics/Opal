CXX = clang++
CXXFLAGS = -std=c++0x -Wall -g -O1 -Isrc

LINK = $(CXX)
LFLAGS = -g

ifeq ($(OS), Windows_NT)
OUT = opal.exe
else
OUT = opal
endif


SRCDIRS = $(filter-out %.cpp %.h,$(wildcard src/*))
OBJDIRS = obj $(SRCDIRS:src/%=obj/%)
SRCS = $(wildcard src/*.cpp src/*/*.cpp)
OBJS = $(SRCS:src/%=obj/%.o)



all: $(OUT)


$(OUT): $(OBJDIRS) $(OBJS)
	$(LINK) -o $(OUT) $(LFLAGS) $(OBJS)

obj/%.o: src/%
	$(CXX) -c -o $@ $(CXXFLAGS) $<

obj:
	mkdir $@
obj/%:
	mkdir $@

clean:
	rm -rf obj $(OUT)
rebuild: clean $(OUT)