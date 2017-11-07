comma   := ,
CXX      = $(shell $(ROSE_INSTALLATION)/bin/rose-config cxx)
CPPFLAGS = $(shell $(ROSE_INSTALLATION)/bin/rose-config cppflags) -I.
CXXFLAGS = $(shell $(ROSE_INSTALLATION)/bin/rose-config cxxflags)
LIBDIRS  = $(shell $(ROSE_INSTALLATION)/bin/rose-config libdirs)
LDFLAGS  = $(shell $(ROSE_INSTALLATION)/bin/rose-config ldflags) -L. \
           $(addprefix -Wl$(comma)-rpath -Wl$(comma), $(subst :, , $(LIBDIRS)))

## Translator
TRANSLATOR=fullTranslator
TRANSLATOR_SOURCE=$(TRANSLATOR).cpp

## Input testcode for translator
TESTCODE=LinkedListNode.h


#-------------------------------------------------------------
# Makefile Targets #-------------------------------------------------------------

all: $(TRANSLATOR)

# compile the translator and generate an executable
# -g -> to be used by default to enable debugging code
$(TRANSLATOR): $(TRANSLATOR_SOURCE)
        $(CXX) -g $(TRANSLATOR_SOURCE) $(CPPFLAGS) $(LDFLAGS) -o $(TRANSLATOR)

# test the translator for atomic operations
check: $(TRANSLATOR)
        cp $(TESTCODE) RoseIntermediateCPPFile.cpp 
        ./$(TRANSLATOR) -c -std=c++11 -I. -I$(ROSE_INSTALLATION)/include RoseIntermediateCPPFile.cpp  
        cp rose_RoseIntermediateCPPFile.cpp rose_LinkedListNode.h 
        rm rose_RoseIntermediateCPPFile.cpp
        rm RoseIntermediateCPPFile.cpp       

rename: cp rose_LinkedListNode.cpp rose_LinkedListNode.h


clean:
        rm -rf $(TRANSLATOR) *.o rose_* *.dot

