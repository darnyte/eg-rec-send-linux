#EGReceiver (Linux).
#
EXECNAME=				EGReceiver
CC=						gcc

VERSION_MAJOR=			0
VERSION_MINOR=			5
VERSION_BUILD_BASE=		147

COMPILEFLAGS=			-pedantic -Wall -Wextra -O2 -D_GNU_SOURCE -DNDEBUG
COMPILEDEBUGFLAGS=		-pedantic -Wall -Wextra -O0 -D_GNU_SOURCE -DDEBUG -g

LINKFLAGS=				-pedantic -Wall -Wextra -O2 -D_GNU_SOURCE -lm -lconfig -DNDEBUG
LINKDEBUGFLAGS=			-pedantic -Wall -Wextra -O0 -D_GNU_SOURCE -lm -lconfig -DDEBUG -g

BASE-BIN-PATH=			./bin
BASE-OBJ-PATH=			./obj

DEBUG-OBJ-PATH=			./obj/Debug
OBJ-PATH=				./obj/Release

DEBUG-EXEC-PATH=		./bin/Debug
EXEC-PATH=				./bin/Release

DEFAULTBUILD=			release

############################################################################################################################################################

EXEC-FULL-NAME=			$(EXEC-PATH)/$(EXECNAME)
DEBUG-EXEC-FULL-NAME=	$(DEBUG-EXEC-PATH)/$(EXECNAME)

Server-DEPS=			./../SERVER/Server.c ./../SERVER/Server.h ./../SERVER/exiterror.h ./../SERVER/exiterror.h ./../SERVER/dropprivileges.h ./../SERVER/processclient.h ./../Common/globals.h ./../Common/network/network.h
Server-FILE=			./../SERVER/Server.c
Server-OBJ=				$(OBJ-PATH)/Server.o
Server-DBGOBJ=			$(DEBUG-OBJ-PATH)/Server-debug.o

exiterror-DEPS=			./../SERVER/exiterror.c ./../SERVER/exiterror.h ./../Common/globals.h
exiterror-FILE=			./../SERVER/exiterror.c
exiterror-OBJ=			$(OBJ-PATH)/exiterror.o
exiterror-DBGOBJ=		$(DEBUG-OBJ-PATH)/exiterror-debug.o

dropprivileges-DEPS=	./../SERVER/dropprivileges.c ./../SERVER/dropprivileges.h ./../SERVER/exiterror.h ./../Common/globals.h
dropprivileges-FILE=	./../SERVER/dropprivileges.c
dropprivileges-OBJ=		$(OBJ-PATH)/dropprivileges.o
dropprivileges-DBGOBJ=	$(DEBUG-OBJ-PATH)/debug-debug.o

network-DEPS=			./../Common/network/network.c ./../Common/network/network.h ./../SERVER/exiterror.h ./../Common/globals.h
network-FILE=			./../Common/network/network.c
network-OBJ=			$(OBJ-PATH)/network.o
network-DBGOBJ=			$(DEBUG-OBJ-PATH)/network-debug.o

md5-DEPS=				./../Common/md5/md5.c ./../Common/md5/md5.h
md5-FILE=				./../Common/md5/md5.c
md5-OBJ=				$(OBJ-PATH)/md5.o
md5-DBGOBJ=				$(DEBUG-OBJ-PATH)/md5-debug.o

processclient-DEPS=		./../SERVER/processclient.c ./../SERVER/processclient.h ./../SERVER/exiterror.h ./../SERVER/Server.h ./../Common/globals.h
processclient-FILE=		./../SERVER/processclient.c
processclient-OBJ=		$(OBJ-PATH)/processclient.o
processclient-DBGOBJ=	$(DEBUG-OBJ-PATH)/processclient-debug.o

globals-DEPS=			./../Common//globals.c ./../Common/globals.h ./../Common/md5/md5.h
globals-FILE=			./../Common/globals.c
globals-OBJ=			$(OBJ-PATH)/globals.o
globals-DBGOBJ=			$(DEBUG-OBJ-PATH)/globals-debug.o

getoptext-DEPS=			./../Common/getopt_ext/getoptext.c ./../Common/getopt_ext/getoptext.h ./../Common/globals.h
getoptext-FILE=			./../Common/getopt_ext/getoptext.c
getoptext-OBJ=			$(OBJ-PATH)/getoptext.o
getoptext-DBGOBJ=		$(DEBUG-OBJ-PATH)/getoptext-debug.o

popenext-DEPS=			./../Common/popen_ext/popenext.c ./../Common/popen_ext/popenext.h
popenext-FILE=			./../Common/popen_ext/popenext.c
popenext-OBJ=			$(OBJ-PATH)/popenext.o
popenext-DBGOBJ=		$(DEBUG-OBJ-PATH)/popenext-debug.o
############################################################################################################################################################

OBJS=					$(Server-OBJ) $(exiterror-OBJ) $(network-OBJ) $(dropprivileges-OBJ) $(md5-OBJ) $(processclient-OBJ)
OBJS+=					$(globals-OBJ) $(getoptext-OBJ) $(popenext-OBJ)
DEBUGOBJS=				$(Server-DBGOBJ) $(exiterror-DBGOBJ) $(network-DBGOBJ) $(dropprivileges-DBGOBJ) $(md5-DBGOBJ)
DEBUGOBJS+=				$(processclient-DBGOBJ) $(globals-DBGOBJ) $(getoptext-DBGOBJ) $(popenext-DBGOBJ)
ANYFILE=				$(Server-DEPS) $(exiterror-DEPS) $(dropprivileges-DEPS) $(network-DEPS) $(md5-DEPS) $(processclient-DEPS)
ANYFILE+=				$(globals-DEPS) $(getoptext-DEPS) $(popenext-DEPS)
############################################################################################################################################################


DEBUG-FILE=				$(DEBUG-EXEC-PATH)/.debug
RELEASE-FILE=			$(EXEC-PATH)/.release


DEBUG_BUILD_NUM_FILE=	$(DEBUG-EXEC-PATH)/.buildnumber
BUILD_NUM_FILE=			$(EXEC-PATH)/.buildnumber


VERSION_DEFINE_DFLAGS=	-DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR)

DEBUG_BUILD_NUM_DFLAGS=	-DBUILD_NUMBER="$$(cat $(DEBUG_BUILD_NUM_FILE))"
BUILD_NUM_DFLAGS=		-DBUILD_NUMBER="$$(cat $(BUILD_NUM_FILE))"

UNAME_DEFINES_DFLAGS=	-DUNAME_PLATFORM=1 -DKERNEL_UNAME="\"$$(uname -s)\"" -DNODE_UNAME="\"$$(uname -n)\""
UNAME_DEFINES_DFLAGS+=	-DKERNEL_RELEASE_UNAME="\"$$(uname -r)\"" -DKERNEL_VERSION_UNAME="\"$$(uname -v)\""
UNAME_DEFINES_DFLAGS+=	-DMACHINE_UNAME="\"$$(uname -m)\"" -DPROCESSOR_UNAME="\"$$(uname -p)\""
UNAME_DEFINES_DFLAGS+=	-DHARDWARE_UNAME="\"$$(uname -i)\"" -DOS_UNAME="\"$$(uname -o)\""

BUILD_TIME_DFLAGS=		-DBUILD_DATE_TIME="\"$$(date)\""

PROG_NAME_DFLAGS=		-DSTR_PROGRAM_NAME="\"$(EXECNAME)\""



COMPILEFLAGS+=			$(PROG_NAME_DFLAGS) $(VERSION_DEFINE_DFLAGS) $(UNAME_DEFINES_DFLAGS)
COMPILEFLAGS+=			$(BUILD_TIME_DFLAGS) $(BUILD_NUM_DFLAGS)

COMPILEDEBUGFLAGS+=		$(PROG_NAME_DFLAGS) $(VERSION_DEFINE_DFLAGS) $(UNAME_DEFINES_DFLAGS)
COMPILEDEBUGFLAGS+=		$(BUILD_TIME_DFLAGS) $(DEBUG_BUILD_NUM_DFLAGS)

############################################################################################################################################################


$(EXECNAME): $(DEFAULTBUILD)

#***

$(DEBUG-FILE):
	@echo
	@echo "Marking (debug) executable as built."
	@touch $(DEBUG-FILE)
#@rm -f $(RELEASE-FILE)
	@echo "Done."
	@echo

$(DEBUG-EXEC-FULL-NAME): $(DEBUG-OBJ-PATH) $(DEBUG-EXEC-PATH) $(DEBUGOBJS) $(DEBUG-FILE)
	@echo
	@echo -n "Building (debug) executable: "
	@basename $(DEBUG-EXEC-FULL-NAME)
	@$(CC) -o $(DEBUG-EXEC-FULL-NAME) $(DEBUGOBJS) $(LINKDEBUGFLAGS)
	@echo "Done."
	@echo

$(RELEASE-FILE):
	@echo
	@echo "Marking executable as built."
	@touch $(RELEASE-FILE)
#@rm -f $(DEBUG-FILE)
	@echo "Done."
	@echo

$(EXEC-FULL-NAME): $(OBJ-PATH) $(EXEC-PATH) $(OBJS) $(RELEASE-FILE)
	@echo
	@echo -n "Building executable: "
	@basename $(EXEC-FULL-NAME)
	@$(CC) -o $(EXEC-FULL-NAME) $(OBJS) $(LINKFLAGS)
	@echo "Done."
	@echo

$(OBJ-PATH):
	@echo
	@echo -n "Creating directory: "
	@echo $@
	@mkdir -p $@
	@echo "Done."
	@echo

$(DEBUG-OBJ-PATH):
	@echo
	@echo -n "Creating directory: "
	@echo $@
	@mkdir -p $@
	@echo "Done."
	@echo

$(DEBUG-EXEC-PATH):
	@echo
	@echo -n "Creating directory: "
	@echo $@
	@mkdir -p $@
	@echo "Done."
	@echo

$(EXEC-PATH):
	@echo
	@echo -n "Creating directory: "
	@echo $@
	@mkdir -p $@
	@echo "Done."
	@echo

$(DEBUG_BUILD_NUM_FILE): $(ANYFILE)
	@echo
	@echo "Updating build number file (debug)."
	@if ! test -f $(DEBUG_BUILD_NUM_FILE); then echo $(VERSION_BUILD_BASE) > $(DEBUG_BUILD_NUM_FILE); fi
	@echo $$(($$(cat $(DEBUG_BUILD_NUM_FILE)) + 1)) > $(DEBUG_BUILD_NUM_FILE)
	@echo "Done."
	@echo

$(BUILD_NUM_FILE): $(ANYFILE)
	@echo
	@echo "Updating build number file."
	@if ! test -f $(BUILD_NUM_FILE); then echo $(VERSION_BUILD_BASE) > $(BUILD_NUM_FILE); fi
	@echo $$(($$(cat $(BUILD_NUM_FILE)) + 1)) > $(BUILD_NUM_FILE)
	@echo "Done."
	@echo

#****
.PHONY : debug

debug:	$(DEBUG-EXEC-FULL-NAME)

.PHONY: Debug

Debug: debug

.PHONY: release

release: $(EXEC-FULL-NAME)

.PHONY: Release

Release: release

.PHONY: compile

compile: $(DEFAULTBUILD)

.PHONY: Compile

Compile: $(DEFAULTBUILD)

build: clean $(DEFAULTBUILD)
#****

###
$(Server-OBJ): $(Server-DEPS) $(BUILD_NUM_FILE)
	@echo
	@echo -n "Compiling: "
	@basename $<
	@$(CC) $(COMPILEFLAGS) -c $(Server-FILE) -o $@
	@echo "Done."
	@echo

$(Server-DBGOBJ): $(Server-DEPS) $(DEBUG_BUILD_NUM_FILE)
	@echo
	@echo -n "Compiling (debug): "
	@basename $<
	@$(CC) $(COMPILEDEBUGFLAGS) -c $(Server-FILE) -o $@
	@echo "Done."
	@echo
###

###
$(exiterror-OBJ): $(exiterror-DEPS)
	@echo
	@echo -n "Compiling: "
	@basename $<
	@$(CC) $(COMPILEFLAGS) -c $(exiterror-FILE) -o $@
	@echo "Done."
	@echo

$(exiterror-DBGOBJ): $(exiterror-DEPS)
	@echo
	@echo -n "Compiling (debug): "
	@basename $<
	@$(CC) $(COMPILEDEBUGFLAGS) -c $(exiterror-FILE) -o $@
	@echo "Done."
	@echo
###

###
$(network-OBJ): $(network-DEPS)
	@echo
	@echo -n "Compiling: "
	@basename $<
	@$(CC) $(COMPILEFLAGS) -c $(network-FILE) -o $@
	@echo "Done."
	@echo


$(network-DBGOBJ): $(network-DEPS)
	@echo
	@echo -n "Compiling (debug): "
	@basename $<
	@$(CC) $(COMPILEDEBUGFLAGS) -c $(network-FILE) -o $@
	@echo "Done."
	@echo
###

###
$(dropprivileges-OBJ): $(dropprivileges-DEPS)
	@echo
	@echo -n "Compiling: "
	@basename $<
	@$(CC) $(COMPILEFLAGS) -c $(dropprivileges-FILE) -o $@
	@echo "Done."
	@echo

$(dropprivileges-DBGOBJ): $(dropprivileges-DEPS)
	@echo
	@echo -n "Compiling (debug): "
	@basename $<
	@$(CC) $(COMPILEDEBUGFLAGS) -c $(dropprivileges-FILE) -o $@
	@echo "Done."
	@echo
###

###
$(md5-OBJ): $(md5-DEPS)
	@echo
	@echo -n "Compiling: "
	@basename $<
	@$(CC) $(COMPILEFLAGS) -c $(md5-FILE) -o $@
	@echo "Done."
	@echo

$(md5-DBGOBJ): $(md5-DEPS)
	@echo
	@echo -n "Compiling (debug): "
	@basename $<
	@$(CC) $(COMPILEDEBUGFLAGS) -c $(md5-FILE) -o $@
	@echo "Done."
	@echo
###

###
$(processclient-OBJ): $(processclient-DEPS)
	@echo
	@echo -n "Compiling: "
	@basename $<
	@$(CC) $(COMPILEFLAGS) -c $(processclient-FILE) -o $@
	@echo "Done."
	@echo

$(processclient-DBGOBJ): $(processclient-DEPS)
	@echo
	@echo -n "Compiling (debug): "
	@basename $<
	@$(CC) $(COMPILEDEBUGFLAGS) -c $(processclient-FILE) -o $@
	@echo "Done."
	@echo
###

###
$(globals-OBJ): $(globals-DEPS)
	@echo
	@echo -n "Compiling: "
	@basename $<
	@$(CC) $(COMPILEFLAGS) -c $(globals-FILE) -o $@
	@echo "Done."
	@echo

$(globals-DBGOBJ): $(globals-DEPS)
	@echo
	@echo -n "Compiling (debug): "
	@basename $<
	@$(CC) $(COMPILEDEBUGFLAGS) -c $(globals-FILE) -o $@
	@echo "Done."
	@echo
###

###
$(getoptext-OBJ): $(getoptext-DEPS)
	@echo
	@echo -n "Compiling: "
	@basename $<
	@$(CC) $(COMPILEFLAGS) -c $(getoptext-FILE) -o $@
	@echo "Done."
	@echo

$(getoptext-DBGOBJ): $(getoptext-DEPS)
	@echo
	@echo -n "Compiling (debug): "
	@basename $<
	@$(CC) $(COMPILEDEBUGFLAGS) -c $(getoptext-FILE) -o $@
	@echo "Done."
	@echo
###

###
$(popenext-OBJ): $(popenext-DEPS)
	@echo
	@echo -n "Compiling: "
	@basename $<
	@$(CC) $(COMPILEFLAGS) -c $(popenext-FILE) -o $@
	@echo "Done."
	@echo

$(popenext-DBGOBJ): $(popenext-DEPS)
	@echo
	@echo -n "Compiling (debug): "
	@basename $<
	@$(CC) $(COMPILEDEBUGFLAGS) -c $(popenext-FILE) -o $@
	@echo "Done."
	@echo
###


.PHONY: cleanDebug

cleanDebug:
	@echo
	@echo "Cleaning - Debug."
	@rm -f *~ $(DEBUG-OBJ-PATH)/*.o $(DEBUG-EXEC-FULL-NAME) $(DEBUG-FILE)
	@echo "Done."
	@echo

.PHONY: cleanRelease

cleanRelease:
	@echo
	@echo "Cleaning - Release."
	@rm -f *~ $(OBJ-PATH)/*.o $(EXEC-FULL-NAME) $(RELEASE-FILE)
	@echo "Done."
	@echo

clean: cleanRelease cleanDebug

Clean: clean

.PHONY : cleanall

cleanall: clean
	@rm -f -r $(BASE-BIN-PATH) $(BASE-OBJ-PATH) $(DEBUG-EXEC-PATH) $(EXEC-PATH) $(DEBUG_BUILD_NUM_FILE) $(BUILD_NUM_FILE)
	@clear

.PHONY : install

install: $(EXEC-FULL-NAME)
	@echo
	@echo "Not implemented"
	@echo

.PHONY : uninstall

uninstall: /usr/bin/$(EXECNAME)
	@echo
	@echo "Not implemented"
	@echo
