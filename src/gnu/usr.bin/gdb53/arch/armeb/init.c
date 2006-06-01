/* Do not modify this file.  */
/* It is created automatically by the Makefile.  */
#include "defs.h"
#include "call-cmds.h"
extern initialize_file_ftype _initialize_gdbtypes;
extern initialize_file_ftype _initialize_blockframe;
extern initialize_file_ftype _initialize_breakpoint;
extern initialize_file_ftype _initialize_regcache;
extern initialize_file_ftype _initialize_source;
extern initialize_file_ftype _initialize_values;
extern initialize_file_ftype _initialize_valops;
extern initialize_file_ftype _initialize_valarith;
extern initialize_file_ftype _initialize_valprint;
extern initialize_file_ftype _initialize_printcmd;
extern initialize_file_ftype _initialize_symtab;
extern initialize_file_ftype _initialize_symfile;
extern initialize_file_ftype _initialize_symmisc;
extern initialize_file_ftype _initialize_infcmd;
extern initialize_file_ftype _initialize_infrun;
extern initialize_file_ftype _initialize_stack;
extern initialize_file_ftype _initialize_thread;
extern initialize_file_ftype _initialize_macrocmd;
extern initialize_file_ftype _initialize_event_loop;
extern initialize_file_ftype _initialize_gdbarch;
extern initialize_file_ftype _initialize_gdbarch_utils;
extern initialize_file_ftype _initialize_gdb_osabi;
extern initialize_file_ftype _initialize_copying;
extern initialize_file_ftype _initialize_arm_tdep;
extern initialize_file_ftype _initialize_arm_netbsd_tdep;
extern initialize_file_ftype _initialize_corelow;
extern initialize_file_ftype _initialize_solib;
extern initialize_file_ftype _initialize_svr4_solib;
extern initialize_file_ftype _initialize_ser_hardwire;
extern initialize_file_ftype _initialize_ser_pipe;
extern initialize_file_ftype _initialize_ser_tcp;
extern initialize_file_ftype _initialize_kernel_u_addr;
extern initialize_file_ftype _initialize_infptrace;
extern initialize_file_ftype _initialize_inftarg;
extern initialize_file_ftype _initialize_arm_netbsd_nat;
extern initialize_file_ftype _initialize_remote;
extern initialize_file_ftype _initialize_dcache;
extern initialize_file_ftype _initialize_sr_support;
extern initialize_file_ftype _initialize_tracepoint;
extern initialize_file_ftype _initialize_ax_gdb;
extern initialize_file_ftype _initialize_mem;
extern initialize_file_ftype _initialize_parse;
extern initialize_file_ftype _initialize_language;
extern initialize_file_ftype _initialize_frame_reg;
extern initialize_file_ftype _initialize_signals;
extern initialize_file_ftype _initialize_kod;
extern initialize_file_ftype _initialize_gdb_events;
extern initialize_file_ftype _initialize_exec;
extern initialize_file_ftype _initialize_maint_cmds;
extern initialize_file_ftype _initialize_demangler;
extern initialize_file_ftype _initialize_dbxread;
extern initialize_file_ftype _initialize_coffread;
extern initialize_file_ftype _initialize_elfread;
extern initialize_file_ftype _initialize_mipsread;
extern initialize_file_ftype _initialize_stabsread;
extern initialize_file_ftype _initialize_core;
extern initialize_file_ftype _initialize_c_language;
extern initialize_file_ftype _initialize_f_language;
extern initialize_file_ftype _initialize_ui_out;
extern initialize_file_ftype _initialize_cli_out;
extern initialize_file_ftype _initialize_varobj;
extern initialize_file_ftype _initialize_java_language;
extern initialize_file_ftype _initialize_m2_language;
extern initialize_file_ftype _initialize_pascal_language;
extern initialize_file_ftype _initialize_pascal_valprint;
extern initialize_file_ftype _initialize_scheme_language;
extern initialize_file_ftype _initialize_complaints;
extern initialize_file_ftype _initialize_typeprint;
extern initialize_file_ftype _initialize_cp_valprint;
extern initialize_file_ftype _initialize_f_valprint;
extern initialize_file_ftype _initialize_nlmread;
extern initialize_file_ftype _initialize_serial;
extern initialize_file_ftype _initialize_mdebugread;
extern initialize_file_ftype _initialize_gnu_v2_abi;
extern initialize_file_ftype _initialize_gnu_v3_abi;
extern initialize_file_ftype _initialize_hpacc_abi;
extern initialize_file_ftype _initialize_annotate;
extern initialize_file_ftype _initialize_inflow;
extern initialize_file_ftype _initialize_cli_dump;
extern initialize_file_ftype _initialize_mi_out;
extern initialize_file_ftype _initialize_mi_cmds;
extern initialize_file_ftype _initialize_mi_main;
extern initialize_file_ftype _initialize_mi_parse;
extern initialize_file_ftype _initialize_nbsd_thread;
#ifdef USE_TUI
extern initialize_file_ftype _initialize_tui;
extern initialize_file_ftype _initialize_tuiLayout;
extern initialize_file_ftype _initialize_tuiRegs;
extern initialize_file_ftype _initialize_tuiStack;
extern initialize_file_ftype _initialize_tuiWin;
extern initialize_file_ftype _initialize_tui_out;
#endif

void
initialize_all_files (void)
{
  _initialize_gdbtypes ();
  _initialize_blockframe ();
  _initialize_breakpoint ();
  _initialize_regcache ();
  _initialize_source ();
  _initialize_values ();
  _initialize_valops ();
  _initialize_valarith ();
  _initialize_valprint ();
  _initialize_printcmd ();
  _initialize_symtab ();
  _initialize_symfile ();
  _initialize_symmisc ();
  _initialize_infcmd ();
  _initialize_infrun ();
  _initialize_stack ();
  _initialize_thread ();
  _initialize_macrocmd ();
  _initialize_event_loop ();
  _initialize_gdbarch ();
  _initialize_gdbarch_utils ();
  _initialize_gdb_osabi ();
  _initialize_copying ();
  _initialize_arm_tdep ();
  _initialize_arm_netbsd_tdep ();
  _initialize_corelow ();
  _initialize_solib ();
  _initialize_svr4_solib ();
  _initialize_ser_hardwire ();
  _initialize_ser_pipe ();
  _initialize_ser_tcp ();
  _initialize_kernel_u_addr ();
  _initialize_infptrace ();
  _initialize_inftarg ();
  _initialize_arm_netbsd_nat ();
  _initialize_remote ();
  _initialize_dcache ();
  _initialize_sr_support ();
  _initialize_tracepoint ();
  _initialize_ax_gdb ();
  _initialize_mem ();
  _initialize_parse ();
  _initialize_language ();
  _initialize_frame_reg ();
  _initialize_signals ();
  _initialize_kod ();
  _initialize_gdb_events ();
  _initialize_exec ();
  _initialize_maint_cmds ();
  _initialize_demangler ();
  _initialize_dbxread ();
  _initialize_coffread ();
  _initialize_elfread ();
  _initialize_mipsread ();
  _initialize_stabsread ();
  _initialize_core ();
  _initialize_c_language ();
  _initialize_f_language ();
  _initialize_ui_out ();
  _initialize_cli_out ();
  _initialize_varobj ();
  _initialize_java_language ();
  _initialize_m2_language ();
  _initialize_pascal_language ();
  _initialize_pascal_valprint ();
  _initialize_scheme_language ();
  _initialize_complaints ();
  _initialize_typeprint ();
  _initialize_cp_valprint ();
  _initialize_f_valprint ();
  _initialize_nlmread ();
  _initialize_serial ();
  _initialize_mdebugread ();
  _initialize_nbsd_thread ();
  _initialize_gnu_v2_abi ();
  _initialize_gnu_v3_abi ();
  _initialize_hpacc_abi ();
  _initialize_annotate ();
  _initialize_inflow ();
  _initialize_cli_dump ();
  _initialize_mi_out ();
  _initialize_mi_cmds ();
  _initialize_mi_main ();
  _initialize_mi_parse ();
#ifdef USE_TUI
  _initialize_tui ();
  _initialize_tuiLayout ();
  _initialize_tuiRegs ();
  _initialize_tuiStack ();
  _initialize_tuiWin ();
  _initialize_tui_out ();
#endif
}
