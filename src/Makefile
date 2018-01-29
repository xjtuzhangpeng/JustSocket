include Makefile.common.variables
include ffmpeg/Makefile.in
include sox/Makefile.in
include soxr/Makefile.in

#CFLAGS  = -O3 -Wall $(DEFS) $(INCLUDES)
INCLUDES = -I. -I../include -Iffmpeg -Isox -Isoxr
DEFS     = 
LIBS     = -L../lib  -llog4cplus -lffmpegsox -lavformat -lavcodec -lavutil -lsox -lsoxr-lsr -lsoxr
CFLAGS   = $(CFLAGS1) $(DEFS) $(INCLUDES)
LINKING  = $(CC) $(CFLAGS) $(LIBS) -o $@ $^

# here list all the source files
SOURCES  = $(FFMPEG_SOURCES) $(SOX_SOURCES) $(SOXR_SOURCES) \
           Tit_Logger.cpp ServerMain.cpp

CSOURCES = $(FFMPEG_CSOURCES) $(SOX_CSOURCES) $(SOXR_CSOURCES)

OBJECT   = $(SOURCES:%.cpp=%.o)
OBJECT  += $(CSOURCES:%.c=%.o)
TARGET   = Server

.PHONY : clean all dep
%.o:%.cpp %.h ; $(CC) -c $(CFLAGS) $< -o $@
%.o:%.c %.h   ; $(cc) -c $(CFLAGS) $< -o $@

# here list all the target binary files
all:$(TARGET)
	@cp -f $^ ../bin/

# here list all the linking dependencies
$(TARGET): $(OBJECT)
	$(LINKING)
# don't modify these lines
# dep: $(SOURCES) ; $(CC) -MM $(CFLAGS) $^ | $(SED) > Makefile.dep
clean: ; rm -f $(OBJECT) $(TARGET)
-include Makefile.dep
