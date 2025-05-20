# Angelscript makefile for linux (based on MingW makefile)
# Type 'make' then 'make install' to complete the installation of the library

# For 'make install' to work, set LOCAL according to your system configuration
LOCAL = /usr/local

LIB = libangelscript.a
INC = pixelscript.h

SRCDIR = ../../Source
LIBDIR = ../../lib
INCDIR = ../../Include
OBJDIR = obj

CXX = gcc
CXXFLAGS = -g -Wall
DELETER = rm -f
COPIER = cp

SRCNAMES = \
  Builder.cpp \
  ByteCode.cpp \
  Callfunc_x86.cpp  \
  Compiler.cpp \
  Context.cpp \
  DataType.cpp \
  Module.cpp \
  ObjectType.cpp \
  Parser.cpp \
  Restore.cpp \
  ScriptCode.cpp \
  ScriptEngine.cpp \
  ScriptFunction.cpp \
  ScriptNode.cpp \
  String.cpp \
  ThreadManager.cpp \
  Tokenizer.cpp \
  VariableScope.cpp \
  pixelscript.cpp \
  stdstring.cpp \

 
OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.cpp=.o)))
BIN = $(LIBDIR)/$(LIB)

all: $(BIN)

$(BIN): $(OBJ)
	ar r $(BIN) $(OBJ)
	ranlib $(BIN)
	@echo -------------------------------------------------------------------
	@echo Done. As root, type 'make install' to install the library.

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<


clean:
	$(DELETER) $(OBJ) $(BIN)

install: $(BIN)
	@echo Installing to: $(LOCAL)/lib and $(LOCAL)/include...
	@echo -------------------------------------------------------------------
	$(COPIER) $(BIN) $(LOCAL)/lib
	$(COPIER) $(INCDIR)/$(INC) $(LOCAL)/include
	@echo -------------------------------------------------------------------
	@echo Angelscript library installed. Enjoy!

uninstall:
	$(DELETER) $(LOCAL)/include/$(INC) $(LOCAL)/lib/$(LIB)
	@echo -------------------------------------------------------------------
	@echo Angelscript library uninstalled.

.PHONY: all clean install uninstall
