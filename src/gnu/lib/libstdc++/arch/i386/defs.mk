# This file is automatically generated.  DO NOT EDIT!
# Generated from: 	NetBSD: toolchain2netbsd,v 1.14 2002/01/22 13:13:00 mrg Exp 
#
G_CINCLUDES=-I. -I${DIST}/libio
G_LIBIOSTREAM_USE=filedoalloc.o floatconv.o genops.o fileops.o  iovfprintf.o  iovfscanf.o ioignore.o iopadn.o  iofgetpos.o iofread.o iofscanf.o  iofsetpos.o iogetdelim.o iogetline.o  ioprintf.o ioseekoff.o ioseekpos.o  outfloat.o strops.o iofclose.o iopopen.o ioungetc.o peekc.o iogetc.o  ioputc.o iofeof.o ioferror.o builtinbuf.o filebuf.o fstream.o  indstream.o ioassign.o ioextend.o iomanip.o iostream.o  isgetline.o isgetsb.o isscan.o  osform.o procbuf.o sbform.o sbgetline.o sbscan.o  stdiostream.o stdstrbufs.o stdstreams.o stream.o streambuf.o strstream.o  PlotFile.o SFile.o parsestream.o pfstream.o editbuf.o ioprims.o iostrerror.o cleanup.o
G_MT_CFLAGS=
G_XCFLAGS=-g
G_XCXXFLAGS=-g -O2 -fno-implicit-templates
G_LIBIBERTY_OBJS=strncmp.o strerror.o
G_COMFUNCS=MAIN ADDCC ADDCF ADDFC SUBCC SUBCF SUBFC MULCC MULCF MULFC DIVCC  DIVCF DIVFC PLUS MINUS EQCC EQCF EQFC NECC NECF NEFC ABS ARG POLAR  CONJ NORM COS COSH EXP LOG POWCC POWCF POWCI POWFC SIN SINH SQRT
G_COMIO=EXTRACT INSERT
G_CXXINCLUDES=-I${DIST}/libstdc++ -I${DIST}/libstdc++/stl -I../libio -I${DIST}/libstdc++/../libio -nostdinc++
G_HEADERS=cassert cctype cerrno cfloat ciso646 climits clocale cmath complex  csetjmp csignal cstdarg cstddef cstdio cstdlib cstring ctime  cwchar cwctype string stdexcept  algorithm deque functional hash_map hash_set iterator list map  memory numeric pthread_alloc queue rope set slist stack utility  vector fstream iomanip iostream strstream iosfwd bitset valarray  sstream
G_OBJS=cstringi.o stdexcepti.o cstdlibi.o cmathi.o stlinst.o valarray.o
G_STRFUNCS=REP MAIN TRAITS ADDSS ADDPS ADDCS ADDSP ADDSC  EQSS EQPS EQSP NESS NEPS NESP LTSS LTPS LTSP GTSS GTPS GTSP  LESS LEPS LESP GESS GEPS GESP
G_STRIO=EXTRACT INSERT GETLINE
STD_HEADERS= complex.h stl.h std/bastring.h std/complext.h std/dcomplex.h std/fcomplex.h std/gslice.h std/gslice_array.h std/indirect_array.h std/ldcomplex.h std/mask_array.h std/slice.h std/slice_array.h std/std_valarray.h std/straits.h std/valarray_array.h std/valarray_meta.h std/bastring.cc std/complext.cc std/valarray_array.tcc
STL_HEADERS= algo.h algobase.h alloc.h bvector.h defalloc.h deque.h function.h hash_map.h hash_set.h hashtable.h heap.h iterator.h list.h map.h multimap.h multiset.h pair.h pthread_alloc.h rope.h ropeimpl.h set.h slist.h stack.h stl_algo.h stl_algobase.h stl_alloc.h stl_bvector.h stl_config.h stl_construct.h stl_deque.h stl_function.h stl_hash_fun.h stl_hash_map.h stl_hash_set.h stl_hashtable.h stl_heap.h stl_iterator.h stl_list.h stl_map.h stl_multimap.h stl_multiset.h stl_numeric.h stl_pair.h stl_queue.h stl_raw_storage_iter.h stl_relops.h stl_rope.h stl_set.h stl_slist.h stl_stack.h stl_tempbuf.h stl_tree.h stl_uninitialized.h stl_vector.h tempbuf.h tree.h type_traits.h vector.h
LIBIO_HEADERS= PlotFile.h SFile.h builtinbuf.h editbuf.h floatio.h fstream.h indstream.h iolibio.h iomanip.h iostdio.h iostream.h istream.h libio.h ostream.h parsestream.h pfstream.h procbuf.h stdiostream.h stream.h streambuf.h strfile.h strstream.h
