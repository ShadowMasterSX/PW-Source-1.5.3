CPP := g++

DEFINES := -D_SKILL_SERVER -g -ggdb -O0
CPPFLAGS := -Wall -Werror

HEADERS := *.h
CPPFILES := playerwrapper.cpp skill.cpp skillfilter.cpp skillwrapper.cpp
OBJS := skill.o skillwrapper.o playerwrapper.o skillfilter.o
INCLUDES := -I. -I../header -I../header/include -I../common -I../share
STUBOBJS := $(patsubst %.cpp,%.o,$(wildcard ../skills/stubs*.cpp))
LIB := libskill.a

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJS) $(STUBOBJS)
	ar crs $(LIB) $(OBJS) $(STUBOBJS)

$(OBJS): $(HEADERS)

$(STUBOBJS):
	$(CPP) -c $(DEFINES) $(INCLUDES) $(CPPFLAGS) -w $(patsubst %.o,%.cpp,$@) -o $@

%.o: %.cpp
	$(CPP) -c $(DEFINES) $(INCLUDES) $(CPPFLAGS) $< -o $@

clean:
	rm -rf $(OBJS) $(STUBOBJS) $(LIB)
