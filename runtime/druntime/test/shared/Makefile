# SHARED (from druntime/Makefile) is `1` for platforms supporting a shared druntime library, otherwise empty
LINK_SHARED:=$(SHARED)

include ../common.mak # affected by LINK_SHARED!

ifneq (,$(LINK_SHARED))
    # LDC: enable ~all tests on Windows too
    ifeq (,$(findstring ldmd2,$(DMD)))
        # TODO: enable tests on Windows
        ifeq (windows,$(OS))
            TESTS:=link linkD linkDR loadDR
        else
            TESTS:=link load linkD linkDR loadDR host finalize dynamiccast \
                   link_linkdep load_linkdep link_loaddep load_loaddep load_13414
        endif
    else
        TESTS:=link load linkD linkDR loadDR host finalize dynamiccast \
               link_linkdep link_loaddep load_loaddep load_13414
        # FIXME: `load_linkdep` needs a non-dummy `getDependencies()` in rt.sections_elf_shared,
        #        not implemented yet on Darwin and Windows
        ifneq (osx,$(OS))
            ifneq (windows,$(OS))
                TESTS+=load_linkdep
            endif
        endif
    endif
endif
# there are extra tests for Windows, not requiring a druntime DLL
ifeq (windows,$(OS))
    TESTS+=loadlibwin dllrefcount dllgc dynamiccast
endif

DOTIMPLIB:=$(if $(findstring $(OS),windows),.lib,$(DOTDLL))

.PHONY: all clean
all: $(addprefix $(ROOT)/,$(addsuffix .done,$(TESTS))) $(if $(findstring ldmd2,$(DMD)),$(if $(findstring $(OS),windows),dll_gc_proxy_teardown,),)

ifeq (windows,$(OS)) # extra tests on Windows

ifneq (,$(findstring ldmd2,$(DMD)))
    # LDC: required for executables to implicitly dllimport DLL data symbols
    DFLAGS+=-dllimport=all
else # DMD
ifeq ($(SHARED),1)
# dmd -shared does not (yet) imply -visibility=public
$(ROOT)/%$(DOTDLL): DFLAGS += -visibility=public

DFLAGS+=-version=SharedRuntime
PATH:=$(dir $(DRUNTIMESO));$(PATH)
endif
endif # end DMD

$(ROOT)/dllrefcount$(DOTEXE): $(SRC)/dllrefcount.d
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $<

$(ROOT)/loadlibwin$(DOTEXE): $(SRC)/loadlibwin.d
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $<

# LDC: this test is designed for .exe & .dll with separate druntimes
ifneq (,$(findstring ldmd2,$(DMD)))
$(ROOT)/dllgc$(DOTEXE): DFLAGS+=-link-defaultlib-shared=false -dllimport=none
endif

$(ROOT)/dllgc$(DOTEXE): $(SRC)/dllgc.d
	$(QUIET)$(DMD) $(DFLAGS) -version=DLL -shared -of$(ROOT)/dllgc$(DOTDLL) $<
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $<

# LDC addition: test teardown with separate druntimes, with the DLL using the .exe GC
ifneq (,$(findstring ldmd2,$(DMD)))
dll_gc_proxy_teardown: DFLAGS+=-link-defaultlib-shared=false -dllimport=none
dll_gc_proxy_teardown: $(SRC)/dll_gc_proxy_teardown.d
	$(QUIET)$(DMD) $(DFLAGS) -shared -L/EXPORT:gc_setProxy -L/EXPORT:gc_clrProxy -version=DLL -of$(ROOT)/dll_gc_proxy_teardown$(DOTDLL) $<
	$(QUIET)$(DMD) $(DFLAGS) -shared -L/EXPORT:gc_setProxy -L/EXPORT:gc_clrProxy -version=DLL -version=NoUnload -of$(ROOT)/dll_gc_proxy_teardown_nounload$(DOTDLL) $<
	$(QUIET)$(DMD) $(DFLAGS) -of$(ROOT)/load_dll_gc_proxy_teardown$(DOTEXE) $<
	$(QUIET)$(DMD) $(DFLAGS) -version=NoUnload -of$(ROOT)/load_dll_gc_proxy_teardown_nounload$(DOTEXE) $<
	$(QUIET)$(ROOT)/load_dll_gc_proxy_teardown$(DOTEXE)
	$(QUIET)$(ROOT)/load_dll_gc_proxy_teardown_nounload$(DOTEXE)
endif

endif # Windows

$(ROOT)/loadDR.done $(ROOT)/host.done: RUN_ARGS:=$(DRUNTIMESO:.lib=.dll)

$(ROOT)/%.done: $(ROOT)/%$(DOTEXE)
	@echo Testing $*
	$(QUIET)$(TIMELIMIT)$< $(RUN_ARGS)
	@touch $@

$(ROOT)/dynamiccast.done: $(ROOT)/%.done: $(ROOT)/%$(DOTEXE)
	@echo Testing $*
	$(QUIET)rm -f $(ROOT)/dynamiccast_end{bar,main}
	$(QUIET)$(TIMELIMIT)$< $(RUN_ARGS)
	$(QUIET)test -f $(ROOT)/dynamiccast_endbar
	$(QUIET)test -f $(ROOT)/dynamiccast_endmain
	@touch $@

$(ROOT)/link$(DOTEXE): $(SRC)/link.d $(ROOT)/lib$(DOTDLL) $(DRUNTIMESO)
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $< -L$(ROOT)/lib$(DOTIMPLIB)

$(ROOT)/link_linkdep$(DOTEXE): $(SRC)/link_linkdep.d $(ROOT)/lib$(DOTDLL) $(ROOT)/liblinkdep$(DOTDLL) $(DRUNTIMESO)
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $< $(LINKFLAGS) -L$(ROOT)/liblinkdep$(DOTIMPLIB) -L$(ROOT)/lib$(DOTIMPLIB)

$(ROOT)/load_linkdep$(DOTEXE): $(SRC)/load_linkdep.d $(ROOT)/lib$(DOTDLL) $(ROOT)/liblinkdep$(DOTDLL) $(DRUNTIMESO)
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $< $(LINKFLAGS) $(LINKDL)

$(ROOT)/link_loaddep$(DOTEXE): $(SRC)/link_loaddep.d $(ROOT)/lib$(DOTDLL) $(ROOT)/libloaddep$(DOTDLL) $(DRUNTIMESO)
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $< $(LINKFLAGS) -L$(ROOT)/libloaddep$(DOTIMPLIB)

$(ROOT)/load_loaddep$(DOTEXE): $(SRC)/load_loaddep.d $(ROOT)/lib$(DOTDLL) $(ROOT)/libloaddep$(DOTDLL) $(DRUNTIMESO)
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $< $(LINKFLAGS) $(LINKDL)

$(ROOT)/load$(DOTEXE) $(ROOT)/finalize$(DOTEXE): $(ROOT)/%$(DOTEXE): $(SRC)/%.d $(ROOT)/lib$(DOTDLL) $(DRUNTIMESO)
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $< $(LINKDL)

$(ROOT)/load_13414$(DOTEXE): $(ROOT)/%$(DOTEXE): $(SRC)/%.d $(ROOT)/lib_13414$(DOTDLL) $(DRUNTIMESO)
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $< $(LINKDL)

$(ROOT)/dynamiccast$(DOTEXE): $(SRC)/dynamiccast.d $(ROOT)/dynamiccast$(DOTDLL) $(if $(LINK_SHARED),$(DRUNTIMESO),$(DRUNTIME))
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $(SRC)/dynamiccast.d $(LINKDL)

$(ROOT)/dynamiccast$(DOTDLL): $(SRC)/dynamiccast.d  $(if $(LINK_SHARED),$(DRUNTIMESO),$(DRUNTIME))
	$(QUIET)$(DMD) $(DFLAGS) -of$@ $< -version=DLL -shared $(LINKDL)

ifeq (windows,$(OS))
    CC:=cl
    CC_OUTFLAG:=/Fe
    # additionally specify the .obj output directory to prevent collisions
    CC_EXTRAS:=/Fo$(ROOT)/
else
    CC_OUTFLAG:=-o
    CC_EXTRAS:=$(LDL) -pthread
endif

$(ROOT)/linkD$(DOTEXE): $(SRC)/linkD.c $(ROOT)/lib$(DOTDLL) $(DRUNTIMESO)
	$(QUIET)$(CC) $(CFLAGS) $(CC_OUTFLAG)$@ $< $(ROOT)/lib$(DOTIMPLIB) $(CC_EXTRAS)

$(ROOT)/linkDR$(DOTEXE): $(SRC)/linkDR.c $(ROOT)/lib$(DOTDLL) $(DRUNTIMESO)
	$(QUIET)$(CC) $(CFLAGS) $(CC_OUTFLAG)$@ $< $(DRUNTIME_IMPLIB) $(CC_EXTRAS)

$(ROOT)/loadDR$(DOTEXE): $(SRC)/loadDR.c $(ROOT)/lib$(DOTDLL) $(DRUNTIMESO)
	$(QUIET)$(CC) $(CFLAGS) $(CC_OUTFLAG)$@ $< $(CC_EXTRAS)

$(ROOT)/host$(DOTEXE): $(SRC)/host.c $(ROOT)/plugin1$(DOTDLL) $(ROOT)/plugin2$(DOTDLL)
	$(QUIET)$(CC) $(CFLAGS) $(CC_OUTFLAG)$@ $< $(CC_EXTRAS)

$(ROOT)/liblinkdep$(DOTDLL): $(ROOT)/lib$(DOTDLL)
$(ROOT)/liblinkdep$(DOTDLL): DFLAGS+=-L$(ROOT)/lib$(DOTIMPLIB)

$(ROOT)/plugin1$(DOTDLL) $(ROOT)/plugin2$(DOTDLL): $(SRC)/plugin.d $(DRUNTIMESO)
	$(QUIET)$(DMD) -shared $(DFLAGS) -of$@ $<

$(ROOT)/%$(DOTDLL): $(SRC)/%.d $(DRUNTIMESO)
	$(QUIET)$(DMD) -shared $(DFLAGS) -of$@ $< $(LINKDL)

clean:
	rm -rf $(ROOT)
