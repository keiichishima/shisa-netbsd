# This file is automatically generated.  DO NOT EDIT!
# Generated from: 	NetBSD: mknative-gcc,v 1.12 2004/02/10 09:37:58 skrll Exp 
#
G_ALL_CFLAGS=   -g -DIN_GCC  -W -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wtraditional -pedantic -Wno-long-long   -DHAVE_CONFIG_H
G_ALL_CPPFLAGS=  
G_C_AND_OBJC_OBJS=attribs.o c-errors.o c-lex.o c-pragma.o c-decl.o c-typeck.o  c-convert.o c-aux-info.o c-common.o c-opts.o c-format.o c-semantics.o  c-objc-common.o c-dump.o libcpp.a 
G_C_OBJS=c-parse.o c-lang.o c-pretty-print.o attribs.o c-errors.o c-lex.o c-pragma.o c-decl.o c-typeck.o  c-convert.o c-aux-info.o c-common.o c-opts.o c-format.o c-semantics.o  c-objc-common.o c-dump.o libcpp.a 
G_CCCP_OBJS=
G_GCOV_OBJS=gcov.o intl.o version.o
G_PROTO_OBJS=intl.o version.o cppdefault.o
G_HOST_PRINT=print-rtl1.o
G_HOST_RTL=build-rtl.o read-rtl.o build-bitmap.o  build-ggc-none.o
G_HOST_RTLANAL=
G_HOST_SUPPORT=gensupport.o insn-conditions.o
G_HOST_EARLY_SUPPORT=gensupport.o dummy-conditions.o
G_HOST_ERRORS=build-errors.o
G_HOST_VARRAY=build-varray.o
G_INCLUDES=-I. -I. -I${GNUHOSTDIST}/gcc -I${GNUHOSTDIST}/gcc/.  -I${GNUHOSTDIST}/gcc/config -I${GNUHOSTDIST}/gcc/../include
G_md_file=${GNUHOSTDIST}/gcc/config/i386/i386.md
G_OBJC_OBJS=objc-lang.o objc-parse.o objc-act.o attribs.o c-errors.o c-lex.o c-pragma.o c-decl.o c-typeck.o  c-convert.o c-aux-info.o c-common.o c-opts.o c-format.o c-semantics.o  c-objc-common.o c-dump.o libcpp.a 
G_OBJS=alias.o bb-reorder.o bitmap.o builtins.o caller-save.o calls.o	    cfg.o cfganal.o cfgbuild.o cfgcleanup.o cfglayout.o cfgloop.o		    cfgrtl.o combine.o conflict.o convert.o cse.o cselib.o dbxout.o	    debug.o df.o diagnostic.o doloop.o dominance.o		                    dwarf2asm.o dwarf2out.o dwarfout.o emit-rtl.o except.o explow.o	    expmed.o expr.o final.o flow.o fold-const.o function.o gcse.o		    genrtl.o ggc-common.o global.o graph.o gtype-desc.o			    haifa-sched.o hashtable.o hooks.o ifcvt.o insn-attrtab.o insn-emit.o	    insn-extract.o insn-opinit.o insn-output.o insn-peep.o insn-recog.o	    integrate.o intl.o jump.o  langhooks.o lcm.o lists.o local-alloc.o	    loop.o mbchar.o optabs.o params.o predict.o print-rtl.o print-tree.o	    profile.o ra.o ra-build.o ra-colorize.o ra-debug.o ra-rewrite.o	    real.o recog.o reg-stack.o regclass.o regmove.o regrename.o		    reload.o reload1.o reorg.o resource.o rtl.o rtlanal.o rtl-error.o	    sbitmap.o sched-deps.o sched-ebb.o sched-rgn.o sched-vis.o sdbout.o	    sibcall.o simplify-rtx.o ssa.o ssa-ccp.o ssa-dce.o stmt.o		    stor-layout.o stringpool.o timevar.o toplev.o tracer.o tree.o tree-dump.o  tree-inline.o unroll.o varasm.o varray.o version.o vmsdbgout.o xcoffout.o  et-forest.o ggc-page.o i386.o 
G_out_file=${GNUHOSTDIST}/gcc/config/i386/i386.c
G_version=3.3.3
G_BUILD_PREFIX=build-
G_RTL_H=rtl.h rtl.def machmode.h machmode.def ${GNUHOSTDIST}/gcc/config/i386/i386-modes.def genrtl.h
G_TREE_H=tree.h tree.def machmode.h machmode.def ${GNUHOSTDIST}/gcc/config/i386/i386-modes.def tree-check.h version.h builtins.def  location.h
G_HCONFIG_H=hconfig.h auto-build.h ${GNUHOSTDIST}/gcc/../include/ansidecl.h ${GNUHOSTDIST}/gcc/config/i386/i386.h ${GNUHOSTDIST}/gcc/config/i386/unix.h ${GNUHOSTDIST}/gcc/config/i386/att.h ${GNUHOSTDIST}/gcc/config/dbxelf.h ${GNUHOSTDIST}/gcc/config/elfos.h ${GNUHOSTDIST}/gcc/config/netbsd.h ${GNUHOSTDIST}/gcc/config/netbsd-elf.h ${GNUHOSTDIST}/gcc/config/i386/netbsd-elf.h
G_BASIC_BLOCK_H=basic-block.h bitmap.h sbitmap.h varray.h ${GNUHOSTDIST}/gcc/../include/partition.h  hard-reg-set.h
G_GCC_H=gcc.h version.h
G_GTFILES_SRCDIR=${GNUHOSTDIST}/gcc
G_GTFILES_FILES_FILES=${GNUHOSTDIST}/gcc/cp/mangle.c  ${GNUHOSTDIST}/gcc/cp/cp-tree.h  ${GNUHOSTDIST}/gcc/cp/decl.h  ${GNUHOSTDIST}/gcc/cp/lex.h  ${GNUHOSTDIST}/gcc/cp/call.c  ${GNUHOSTDIST}/gcc/cp/decl.c  ${GNUHOSTDIST}/gcc/cp/decl2.c  ${GNUHOSTDIST}/gcc/cp/parse.y  ${GNUHOSTDIST}/gcc/cp/pt.c  ${GNUHOSTDIST}/gcc/cp/repo.c  ${GNUHOSTDIST}/gcc/cp/spew.c  ${GNUHOSTDIST}/gcc/cp/tree.c  ${GNUHOSTDIST}/gcc/c-common.c  ${GNUHOSTDIST}/gcc/c-common.h  ${GNUHOSTDIST}/gcc/c-pragma.c  ${GNUHOSTDIST}/gcc/f/com.c  ${GNUHOSTDIST}/gcc/f/com.h  ${GNUHOSTDIST}/gcc/f/ste.c  ${GNUHOSTDIST}/gcc/f/where.h  ${GNUHOSTDIST}/gcc/f/where.c  ${GNUHOSTDIST}/gcc/f/lex.c  ${GNUHOSTDIST}/gcc/objc/objc-act.h  ${GNUHOSTDIST}/gcc/c-parse.in  ${GNUHOSTDIST}/gcc/c-tree.h  ${GNUHOSTDIST}/gcc/c-decl.c  ${GNUHOSTDIST}/gcc/c-objc-common.c  ${GNUHOSTDIST}/gcc/c-common.c  ${GNUHOSTDIST}/gcc/c-common.h  ${GNUHOSTDIST}/gcc/c-pragma.c  ${GNUHOSTDIST}/gcc/c-parse.in  ${GNUHOSTDIST}/gcc/c-lang.c  ${GNUHOSTDIST}/gcc/c-parse.in  ${GNUHOSTDIST}/gcc/c-tree.h  ${GNUHOSTDIST}/gcc/c-decl.c  ${GNUHOSTDIST}/gcc/c-common.c  ${GNUHOSTDIST}/gcc/c-common.h  ${GNUHOSTDIST}/gcc/c-pragma.c  ${GNUHOSTDIST}/gcc/c-objc-common.c
G_GTFILES_FILES_LANGS=cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  f  f  f  f  f  f  objc  objc  objc  objc  objc  objc  objc  objc  objc  c  c  c  c  c  c  c  c
G_GTFILES=config.h auto-host.h ${GNUHOSTDIST}/gcc/../include/ansidecl.h ${GNUHOSTDIST}/gcc/config/i386/i386.h ${GNUHOSTDIST}/gcc/config/i386/unix.h ${GNUHOSTDIST}/gcc/config/i386/att.h ${GNUHOSTDIST}/gcc/config/dbxelf.h ${GNUHOSTDIST}/gcc/config/elfos.h ${GNUHOSTDIST}/gcc/config/netbsd.h ${GNUHOSTDIST}/gcc/config/netbsd-elf.h ${GNUHOSTDIST}/gcc/config/i386/netbsd-elf.h ${GNUHOSTDIST}/gcc/defaults.h ${GNUHOSTDIST}/gcc/defaults.h ${GNUHOSTDIST}/gcc/location.h  ${GNUHOSTDIST}/gcc/../include/hashtab.h  ${GNUHOSTDIST}/gcc/bitmap.h ${GNUHOSTDIST}/gcc/function.h  ${GNUHOSTDIST}/gcc/rtl.h ${GNUHOSTDIST}/gcc/optabs.h  ${GNUHOSTDIST}/gcc/tree.h ${GNUHOSTDIST}/gcc/libfuncs.h ${GNUHOSTDIST}/gcc/hashtable.h ${GNUHOSTDIST}/gcc/real.h  ${GNUHOSTDIST}/gcc/varray.h ${GNUHOSTDIST}/gcc/ssa.h ${GNUHOSTDIST}/gcc/insn-addr.h ${GNUHOSTDIST}/gcc/cselib.h  ${GNUHOSTDIST}/gcc/c-common.h ${GNUHOSTDIST}/gcc/c-tree.h  ${GNUHOSTDIST}/gcc/basic-block.h  ${GNUHOSTDIST}/gcc/alias.c ${GNUHOSTDIST}/gcc/bitmap.c ${GNUHOSTDIST}/gcc/cselib.c  ${GNUHOSTDIST}/gcc/dwarf2out.c ${GNUHOSTDIST}/gcc/emit-rtl.c  ${GNUHOSTDIST}/gcc/except.c ${GNUHOSTDIST}/gcc/explow.c ${GNUHOSTDIST}/gcc/expr.c  ${GNUHOSTDIST}/gcc/fold-const.c ${GNUHOSTDIST}/gcc/function.c  ${GNUHOSTDIST}/gcc/gcse.c ${GNUHOSTDIST}/gcc/integrate.c ${GNUHOSTDIST}/gcc/lists.c ${GNUHOSTDIST}/gcc/optabs.c  ${GNUHOSTDIST}/gcc/profile.c ${GNUHOSTDIST}/gcc/ra-build.c ${GNUHOSTDIST}/gcc/regclass.c  ${GNUHOSTDIST}/gcc/reg-stack.c  ${GNUHOSTDIST}/gcc/sdbout.c ${GNUHOSTDIST}/gcc/stmt.c ${GNUHOSTDIST}/gcc/stor-layout.c  ${GNUHOSTDIST}/gcc/tree.c ${GNUHOSTDIST}/gcc/varasm.c  ${GNUHOSTDIST}/gcc/config/i386/i386.c  ${GNUHOSTDIST}/gcc/cp/mangle.c ${GNUHOSTDIST}/gcc/cp/cp-tree.h ${GNUHOSTDIST}/gcc/cp/decl.h ${GNUHOSTDIST}/gcc/cp/lex.h ${GNUHOSTDIST}/gcc/cp/call.c ${GNUHOSTDIST}/gcc/cp/decl.c ${GNUHOSTDIST}/gcc/cp/decl2.c ${GNUHOSTDIST}/gcc/cp/parse.y ${GNUHOSTDIST}/gcc/cp/pt.c ${GNUHOSTDIST}/gcc/cp/repo.c ${GNUHOSTDIST}/gcc/cp/spew.c ${GNUHOSTDIST}/gcc/cp/tree.c ${GNUHOSTDIST}/gcc/c-common.c ${GNUHOSTDIST}/gcc/c-common.h ${GNUHOSTDIST}/gcc/c-pragma.c ${GNUHOSTDIST}/gcc/f/com.c ${GNUHOSTDIST}/gcc/f/com.h ${GNUHOSTDIST}/gcc/f/ste.c ${GNUHOSTDIST}/gcc/f/where.h ${GNUHOSTDIST}/gcc/f/where.c ${GNUHOSTDIST}/gcc/f/lex.c ${GNUHOSTDIST}/gcc/objc/objc-act.h ${GNUHOSTDIST}/gcc/c-parse.in ${GNUHOSTDIST}/gcc/c-tree.h ${GNUHOSTDIST}/gcc/c-decl.c ${GNUHOSTDIST}/gcc/c-objc-common.c ${GNUHOSTDIST}/gcc/c-common.c ${GNUHOSTDIST}/gcc/c-common.h ${GNUHOSTDIST}/gcc/c-pragma.c ${GNUHOSTDIST}/gcc/c-parse.in ${GNUHOSTDIST}/gcc/c-lang.c ${GNUHOSTDIST}/gcc/c-parse.in ${GNUHOSTDIST}/gcc/c-tree.h ${GNUHOSTDIST}/gcc/c-decl.c ${GNUHOSTDIST}/gcc/c-common.c ${GNUHOSTDIST}/gcc/c-common.h ${GNUHOSTDIST}/gcc/c-pragma.c ${GNUHOSTDIST}/gcc/c-objc-common.c
G_GTFILES_LANG_DIR_NAMES=cp f objc
G_tm_defines=NETBSD_ENABLE_PTHREADS
G_host_xm_file=auto-host.h ansidecl.h  i386/i386.h i386/unix.h i386/att.h dbxelf.h elfos.h netbsd.h netbsd-elf.h i386/netbsd-elf.h defaults.h
G_host_xm_defines=POSIX
G_tm_p_file=i386/i386-protos.h
G_target_cpu_default=
G_LIBCPP_OBJS=cpplib.o cpplex.o cppmacro.o cppexp.o cppfiles.o cpptrad.o  cpphash.o cpperror.o cppinit.o cppdefault.o cppmain.o  hashtable.o line-map.o mkdeps.o prefix.o mbchar.o
G_LIBCPP_H=
G_lang_specs_files=${GNUHOSTDIST}/gcc/cp/lang-specs.h ${GNUHOSTDIST}/gcc/f/lang-specs.h ${GNUHOSTDIST}/gcc/objc/lang-specs.h
G_LIB2ADDEHDEP= unwind-dw2-fde.h
G_CXX_OBJS=call.o decl.o expr.o pt.o typeck2.o  class.o decl2.o error.o lex.o parse.o ptree.o rtti.o  spew.o typeck.o cvt.o except.o friend.o init.o method.o  search.o semantics.o tree.o repo.o dump.o  optimize.o mangle.o cp-lang.o
G_CXX_C_OBJS=attribs.o c-common.o c-format.o c-pragma.o c-semantics.o c-lex.o  c-dump.o  c-pretty-print.o c-opts.o
G_F77_OBJS=bad.o bit.o bld.o com.o data.o equiv.o expr.o  global.o implic.o info.o intrin.o lab.o lex.o malloc.o  name.o parse.o src.o st.o sta.o stb.o stc.o  std.o ste.o storag.o stp.o str.o sts.o stt.o stu.o  stv.o stw.o symbol.o target.o top.o type.o where.o
G_ENABLE_SHARED=yes
G_SHLIB_LINK= ./xgcc -B./ -B/usr/local/i386--netbsdelf/bin/ -isystem /usr/local/i386--netbsdelf/include -isystem /usr/local/i386--netbsdelf/sys-include -O2  -DIN_GCC   -W -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -isystem ./include   -g -DHAVE_GTHR_DEFAULT -DIN_LIBGCC2 -D__GCC_FLOAT_NOT_NEEDED -shared -nodefaultlibs  -Wl,--soname=@shlib_so_name@.so.1  -Wl,--version-script=@shlib_map_file@  -o @shlib_dir@@shlib_so_name@.so.1 @multilib_flags@ @shlib_objs@ -lc &&  rm -f @shlib_base_name@.so &&  ln -s @shlib_dir@@shlib_so_name@.so.1 @shlib_base_name@.so
G_SHLIB_MULTILIB=
