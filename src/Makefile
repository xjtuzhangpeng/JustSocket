include Makefile.common.variables
include ffmpeg/Makefile.in
include sox/Makefile.in
include soxr/Makefile.in
#CFLAGS  = -O3 -Wall $(DEFS) $(INCLUDES)
.PHONY : clean all dep
%.o:%.cpp ; $(CC) -c $(CFLAGS) $< -o $@
%.o:%.c ;   $(CC) -c $(CFLAGS) $< -o $@

LINKING  = $(CC) $(CFLAGS) $(LIBS) -o $@ $^
INCLUDES = -I. -I../../include

DEFS     = 
LIBS     = -L../../lib -lffmpegsox -lavformat -lavcodec -lavutil -lsox -llog4cplus
CFLAGS   = $(CFLAGS1) $(DEFS) $(INCLUDES)

# here list all the source files
SOURCES  = $(FFMPEG_SOURCES) $(SOX_SOURCES) $(SOXR_SOURCES) \
           Audio2pcm.cpp Tit_Logger.cpp ServerMain.cpp

CSOURCES = 

OBJECT   = $(SOURCES:%.cpp=%.o)
OBJECT  += $(CSOURCES:%.c=%.o)
TARGET   = Server
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
