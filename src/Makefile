#### Start of system configuration section. ####

OPTIMIZE := -DNDEBUG -march=native -O3 -fdevirtualize-at-ltrans -fipa-pta -fno-semantic-interposition -flto=auto -pipe

JEMALLOC_USE := 1
JEMALLOC_LIB := jemalloc.so


ifeq ($(JEMALLOC_USE),0)
JEMALLOC :=
else
JEMALLOC := -I`jemalloc-config --includedir` \
			-L`jemalloc-config --libdir` -Wl,-rpath,`jemalloc-config --libdir` \
			-ljemalloc `jemalloc-config --libs`
endif


GIT_VERSION := "$(shell git describe --abbrev=6 --always --tags 2>/dev/null)"
CC      := gcc
CFLAGS  := -Wall -Wextra -Wimplicit -Wfatal-errors -std=gnu2x \
		   -Wno-unused-parameter \
		   -DPROC_COUNT=$(shell nproc --all) \
		   ${OPTIMIZE} $(JEMALLOC) \
		   ##############\
		   -Og -fsanitize=address -rdynamic -ggdb3 \
		   -pg -O2 -rdynamic \
		   -DCOLORPRINT_DISABLE \
		   ###############
LDFLAGS := \
		   -lm \
		   ${OPTIMIZE} $(JEMALLOC) \
		   ##############\
		   -Og -fsanitize=address -rdynamic -ggdb3 \
		   -pg -rdynamic \
		   ##############
CSUFFIX	:= .c
HSUFFIX	:= .h

#### End of system configuration section. ####

APPNAME = cft

# Path for important files
# .c and .h files
SRC_DIR = .
# .o files
OBJ_DIR = obj
# target directory
TRG_DIR = .

.PHONY: all clean


# Files to compile
TARGET  := $(addprefix $(TRG_DIR)/,$(APPNAME))
C_FILES := $(wildcard $(SRC_DIR)/*$(CSUFFIX))
O_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(C_FILES:$(CSUFFIX)=.o)))
D_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(C_FILES:$(CSUFFIX)=.d)))

VERSION_FILE = $(addprefix $(TRG_DIR)/,$(addprefix $(APPNAME),.version))

all: $(TARGET)

install: $(TARGET)
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp -p $(TARGET) $(DESTDIR)$(PREFIX)/bin/${APPNAME}

uninstall:
	@rm -rf $(DESTDIR)$(PREFIX)/bin/${APPNAME}


# link all .o files
$(TARGET): $(O_FILES) | $(TRG_DIR)
	@echo link    : $@ #$^
	@$(CC) $(LDFLAGS) -o $@ $^

# depend include files
-include $(D_FILES)

$(VERSION_FILE): FORCE Makefile | $(OBJ_DIR)
ifneq ($(GIT_VERSION),"")
	@echo Version: [$(GIT_VERSION)]
	@echo "\"$(GIT_VERSION)\"" > $(VERSION_FILE)
endif

# compile all .c Files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%$(CSUFFIX) Makefile | $(OBJ_DIR) $(VERSION_FILE)
#@echo compile : $< $(CFLAGS) -c -MMD -
	$(CC) $(CFLAGS) -c -MMD -o $@ $<

# create directories if they don't exist
# .o dir
$(OBJ_DIR):
	@mkdir $@
# target dir
$(TRG_DIR):
	@mkdir $@

FORCE: #leave empty

#### CLEANING ####
ifeq ($(OS),Windows_NT)
# Cleaning rules for Windows OS (no clue if this works)
clean:
	@del /q $(OBJ_DIR), $(TRG_DIR)
	@rmdir $(OBJ_DIR)
	@rmdir $(TRG_DIR)
else
# Cleaning rules for Unix-based OS
clean:
	@rm -rf $(OBJ_DIR) $(TRG_DIR) $(TARGET)
endif

