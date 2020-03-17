ALL = animated menubar-add nativefilechooser-simple-app table-as-container
FLTKDIR=/mnt/ldata/program/fltk

FLTKCONFIG   = $(FLTKDIR)/bin/fltk-config
CXX          = $(shell $(FLTKCONFIG) --cxx)
CXXFLAGS     = $(shell $(FLTKCONFIG) --cxxflags) -Wall -I.
#LINKFLTK     = $(shell $(FLTKCONFIG) --ldstaticflags)
#LINKFLTK_GL  = $(shell $(FLTKCONFIG) --use-gl --ldstaticflags)
#LINKFLTK_IMG = $(shell $(FLTKCONFIG) --use-images --ldstaticflags)
#LINKFLTK_ALL = $(shell $(FLTKCONFIG) --use-images --use-gl --ldstaticflags)

CXXFLAGS = -I$(FLTKDIR)/include -I$(FLTKDIR)/include/FL/images -D_THREAD_SAFE -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

LINKLIBS = -lm -lX11 -lXext -lpthread -lXfixes -ldl
STATICLIBS = $(FLTKDIR)/lib/libfltk.a $(FLTKDIR)/lib/libfltk_images.a
RM    = rm -f

%.o: %.cxx
	@echo "--- compile $< ---"
	$(CXX) $(CXXFLAGS) -c $<

%: %.o
	@echo "--- link $@ ---"
	$(CXX) $(CXXFLAGS) -o $@ $< $(STATICLIBS) $(LINKLIBS)

# default target -- build everything
default all: $(ALL)

# clean everything
clean:
	$(RM) $(ALL)
	$(RM) *.o
	$(RM) core


