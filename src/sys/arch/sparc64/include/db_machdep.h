/*	$NetBSD: db_machdep.h,v 1.16 2004/01/06 20:41:23 petrov Exp $ */

/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#ifndef	_SPARC64_DB_MACHDEP_H_
#define	_SPARC64_DB_MACHDEP_H_

/*
 * Machine-dependent defines for new kernel debugger.
 */

#include <uvm/uvm_extern.h>

#include <machine/frame.h>
#include <machine/psl.h>
#include <machine/trap.h>
#include <machine/reg.h>


typedef	vaddr_t		db_addr_t;	/* address - unsigned */
typedef	long		db_expr_t;	/* expression - signed */

struct trapstate {
	int64_t tstate;
	int64_t tpc;
	int64_t	tnpc;
	int64_t	tt;
};

typedef struct {
	struct trapframe64	ddb_tf;
	struct frame64		ddb_fr;
	struct trapstate	ddb_ts[5];
	int			ddb_tl;
	struct fpstate64	ddb_fpstate;
} db_regs_t;

extern db_regs_t		ddb_regs;
#define	DDB_REGS	(&ddb_regs)
#define	DDB_TF		(&ddb_regs.ddb_tf)
#define	DDB_FR		(&ddb_regs.ddb_fr)
#define	DDB_FP		(&ddb_regs.ddb_fpstate)

#if defined(lint)
#define	PC_REGS(regs)	((regs)->ddb_tf.tf_pc)
#else
#define	PC_REGS(regs)	((db_addr_t)(regs)->ddb_tf.tf_pc)
#endif
#define	PC_ADVANCE(regs) do {				\
	vaddr_t n = (regs)->ddb_tf.tf_npc;		\
	(regs)->ddb_tf.tf_pc = n;			\
	(regs)->ddb_tf.tf_npc = n + 4;			\
} while(0)

#define	BKPT_ADDR(addr)	(addr)		/* breakpoint address */
#define	BKPT_INST	0x91d02001	/* breakpoint instruction */
#define	BKPT_SIZE	(4)		/* size of breakpoint inst */
#define	BKPT_SET(inst)	(BKPT_INST)

#define	IS_BREAKPOINT_TRAP(type, code)	\
	((type) == T_BREAKPOINT || (type) == T_KGDB_EXEC)
#define IS_WATCHPOINT_TRAP(type, code)	\
	((type) ==T_PA_WATCHPT || (type) == T_VA_WATCHPT)

/*
 * Sparc cpus have no hardware single-step.
 */
#define SOFTWARE_SSTEP

boolean_t	db_inst_trap_return __P((int inst));
boolean_t	db_inst_return __P((int inst));
boolean_t	db_inst_call __P((int inst));
boolean_t	db_inst_branch __P((int inst));
int		db_inst_load __P((int inst));
int		db_inst_store __P((int inst));
boolean_t	db_inst_unconditional_flow_transfer __P((int inst));
db_addr_t	db_branch_taken __P((int inst, db_addr_t pc, db_regs_t *regs));

#define inst_trap_return(ins)	db_inst_trap_return(ins)
#define inst_return(ins)	db_inst_return(ins)
#define inst_call(ins)		db_inst_call(ins)
#define inst_branch(ins)	db_inst_branch(ins)
#define inst_load(ins)		db_inst_load(ins)
#define inst_store(ins)		db_inst_store(ins)
#define	inst_unconditional_flow_transfer(ins) \
				db_inst_unconditional_flow_transfer(ins)
#define branch_taken(ins, pc, regs) \
				db_branch_taken((ins), (pc), (regs))

/* see note in db_interface.c about reversed breakpoint addrs */
#define next_instr_address(pc, bd) \
	((bd) ? (pc) : ddb_regs.ddb_tf.tf_npc)

#define DB_MACHINE_COMMANDS

int kdb_trap __P((int, struct trapframe64 *));

/*
 * We will use elf symbols in DDB when they work.
 */
#if 1
#define	DB_ELF_SYMBOLS
#ifdef __arch64__
#define DB_ELFSIZE	64
#else
#define DB_ELFSIZE	32
#endif
#else
#define DB_AOUT_SYMBOLS
#endif
/*
 * KGDB definitions
 */
typedef u_long		kgdb_reg_t;
#define KGDB_NUMREGS	125
#define KGDB_BUFLEN	2048

#endif	/* _SPARC64_DB_MACHDEP_H_ */
