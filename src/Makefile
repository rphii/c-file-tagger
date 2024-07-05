#### Start of system configuration section. ####

OPTIMIZE := #-march=native -O3 -fdevirtualize-at-ltrans -fipa-pta -fno-semantic-interposition -flto=auto -pipe

			#\
			-fmerge-all-constants -fgcse-lm -fgcse-sm -fgcse-las -fgcse-after-reload -flive-range-shrinkage -fsched-pressure \
		   -fsched-spec-load -fsched-spec-load-dangerous -fsched-stalled-insns=0 -fsched-stalled-insns-dep=0 -fsched2-use-superblocks -fsched-group-heuristic -fsched-critical-path-heuristic -fsched-spec-insn-heuristic -fsched-rank-heuristic -fsched-last-insn-heuristic -fsched-dep-count-heuristic -freschedule-modulo-scheduled-loops -fselective-scheduling -fselective-scheduling2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops \
		   -fno-semantic-interposition -fipa-pta -fisolate-erroneous-paths-attribute  \
		   -ftree-loop-linear -floop-strip-mine -floop-block -fgraphite-identity -floop-nest-optimize -floop-parallelize-all -fno-var-tracking-assignments -ftree-loop-if-convert -ftree-loop-distribution -floop-interchange -ftree-loop-im -floop-unroll-and-jam -ftree-loop-ivcanon -fivopts -ftree-parallelize-loops=16 -ftree-vectorize -fvect-cost-model=dynamic -fsimd-cost-model=dynamic -fsplit-paths -fpredictive-commoning -fprefetch-loop-arrays -freorder-blocks-algorithm=stc -fallow-store-data-races -fno-toplevel-reorder -fno-section-anchors -fweb -funroll-loops -fwhole-program  \
		   -flto -flto-partition=balanced -flto-compression-level=0 -fuse-linker-plugin -ffat-lto-objects -fprofile-correction \
		   -fvpt -ftracer -fpeel-loops -fsplit-loops -funswitch-loops -fversion-loops-for-strides -fstdarg-opt -ffunction-sections -fdata-sections -ftoplevel-reorder \
		   #-fprofile-partial-training -fauto-profile -fbranch-probabilities \


GIT_VERSION := "$(shell git describe --abbrev=4 --dirty --always --tags 2>/dev/null)"
CC      := gcc
CFLAGS  := -Wall -Wextra -Wimplicit -Wfatal-errors -std=gnu99 \
		   -Wno-unused-parameter \
		   -DPROC_COUNT=$(shell nproc --all) \
		   -O2 -march=native -DNDEBUG \
		   ##############\
		   -rdynamic -Og -ggdb3 \
		   ${OPTIMIZE} \
		   -O3 -march=native -DNDEBUG \
		   -pg -rdynamic \
		   -rdynamic -O3 -march=native -DNDEBUG \
		   -DCOLORPRINT_DISABLE \
		   -fsanitize=address -rdynamic \
		   -O3 -march=native \
		   -Wconversion \
		   -O0 -g \
		   -DCOLORPRINT_DISABLE \
		   -Werror \
		   -Warith-conversion \
		   -pg -O2 \
		   -DNDEBUG \
		   -march=native \
		   -Og -rdynamic \
		   -march=native \
		   -Wall \
		   ###############
LDFLAGS := \
		   -lm \
		   ##############\
		   -fsanitize=address -rdynamic \
		   ${OPTIMIZE} \
		   -pg -rdynamic \
		   -rdynamic -ggdb3 \
		   -rdynamic -Og -ggdb3 \
		   -fsanitize=address \
		   -pg \

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
	@echo compile : $<
	@$(CC) $(CFLAGS) -c -MMD -o $@ $<

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

