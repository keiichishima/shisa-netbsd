/*	$NetBSD: pcibios.c,v 1.22 2005/02/03 21:35:44 perry Exp $	*/

/*-
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1999, by UCHIYAMA Yasushi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the developer may NOT be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 */

/*
 * Interface to the PCI BIOS and PCI Interrupt Routing table.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: pcibios.c,v 1.22 2005/02/03 21:35:44 perry Exp $");

#include "opt_pcibios.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/malloc.h>

#include <dev/isa/isareg.h>
#include <machine/isa_machdep.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

#include <i386/pci/pcibios.h>
#ifdef PCIBIOS_INTR_FIXUP
#include <i386/pci/pci_intr_fixup.h>
#endif
#ifdef PCIBIOS_BUS_FIXUP
#include <i386/pci/pci_bus_fixup.h>
#endif
#ifdef PCIBIOS_ADDR_FIXUP
#include <i386/pci/pci_addr_fixup.h>
#endif

#include <machine/bios32.h> 

#ifdef PCIBIOSVERBOSE
int	pcibiosverbose = 1;
#endif

int pcibios_present;

struct pcibios_pir_header pcibios_pir_header;
struct pcibios_intr_routing *pcibios_pir_table;
int pcibios_pir_table_nentries;
int pcibios_max_bus;

struct bios32_entry pcibios_entry;

void	pcibios_pir_init(void);

int	pcibios_get_status(u_int32_t *, u_int32_t *, u_int32_t *,
	    u_int32_t *, u_int32_t *, u_int32_t *, u_int32_t *);
int	pcibios_get_intr_routing(struct pcibios_intr_routing *,
	    int *, u_int16_t *);

int	pcibios_return_code(u_int16_t, const char *);

void	pcibios_print_exclirq(void);

#ifdef PCIBIOS_LIBRETTO_FIXUP
/* for Libretto L2/L3 hack */
static void	pcibios_fixup_pir_table(void);
static void	pcibios_fixup_pir_table_mask(struct pcibios_linkmap *);

struct pcibios_linkmap pir_mask[] = {
	{ 2,	0x0040 },
	{ 7,	0x0080 },
	{ 8,	0x0020 },
	{ 0,	0x0000 }
};
#endif

#ifdef PCIBIOS_SHARP_MM20_FIXUP
static void pcibios_mm20_fixup(void);
#endif

#ifdef PCIINTR_DEBUG
void	pcibios_print_pir_table(void);
#endif

#define	PCI_IRQ_TABLE_START	0xf0000
#define	PCI_IRQ_TABLE_END	0xfffff

static void pci_bridge_hook(pci_chipset_tag_t, pcitag_t, void *);
struct pci_bridge_hook_arg {
	void (*func)(pci_chipset_tag_t, pcitag_t, void *);
	void *arg;
};

void
pcibios_init(void)
{
	struct bios32_entry_info ei;
	u_int32_t rev_maj, rev_min, mech1, mech2, scmech1, scmech2;

	if (bios32_service(BIOS32_MAKESIG('$', 'P', 'C', 'I'),
	    &pcibios_entry, &ei) == 0) {
		/*
		 * No PCI BIOS found; will fall back on old
		 * mechanism.
		 */
		return;
	}

	/*
	 * We've located the PCI BIOS service; get some information
	 * about it.
	 */
	if (pcibios_get_status(&rev_maj, &rev_min, &mech1, &mech2,
	    &scmech1, &scmech2, &pcibios_max_bus) != PCIBIOS_SUCCESS) {
		/*
		 * We can't use the PCI BIOS; will fall back on old
		 * mechanism.
		 */
		return;
	}

	printf("PCI BIOS rev. %d.%d found at 0x%lx\n", rev_maj, rev_min >> 4,
	    ei.bei_entry);
#ifdef PCIBIOSVERBOSE
	printf("pcibios: config mechanism %s%s, special cycles %s%s, "
	    "last bus %d\n",
	    mech1 ? "[1]" : "[x]",
	    mech2 ? "[2]" : "[x]",
	    scmech1 ? "[1]" : "[x]",
	    scmech2 ? "[2]" : "[x]",
	    pcibios_max_bus);

#endif

	/*
	 * The PCI BIOS tells us the config mechanism; fill it in now
	 * so that pci_mode_detect() doesn't have to look for it.
	 */
	pci_mode = mech1 ? 1 : 2;

	pcibios_present = 1;

	/*
	 * Find the PCI IRQ Routing table.
	 */
	pcibios_pir_init();

#ifdef PCIBIOS_INTR_FIXUP
	if (pcibios_pir_table != NULL) {
		int rv;
		u_int16_t pciirq;

		/*
		 * Fixup interrupt routing.
		 */
		rv = pci_intr_fixup(NULL, X86_BUS_SPACE_IO, &pciirq);
		switch (rv) {
		case -1:
			/* Non-fatal error. */
			printf("Warning: unable to fix up PCI interrupt "
			    "routing\n");
			break;

		case 1:
			/* Fatal error. */
			panic("pcibios_init: interrupt fixup failed");
			break;
		}

		/*
		 * XXX Clear `pciirq' from the ISA interrupt allocation
		 * XXX mask.
		 */
	}
#endif

#ifdef PCIBIOS_BUS_FIXUP
	pcibios_max_bus = pci_bus_fixup(NULL, 0);
#ifdef PCIBIOSVERBOSE
	printf("PCI bus #%d is the last bus\n", pcibios_max_bus);
#endif
#endif

#ifdef PCIBIOS_ADDR_FIXUP
	pci_addr_fixup(NULL, pcibios_max_bus);
#endif
}

void
pcibios_pir_init(void)
{
	char *devinfo;
	paddr_t pa;
	caddr_t p;
	unsigned char cksum;
	u_int16_t tablesize;
	u_int8_t rev_maj, rev_min;
	int i;

	for (pa = PCI_IRQ_TABLE_START; pa < PCI_IRQ_TABLE_END; pa += 16) {
		p = (caddr_t)ISA_HOLE_VADDR(pa);
		if (*(int *)p != BIOS32_MAKESIG('$', 'P', 'I', 'R')) {
			/*
			 * XXX: Some laptops (Toshiba/Libretto L series)
			 * use _PIR instead of $PIR. So we try that too.
			 */
			if (*(int *)p != BIOS32_MAKESIG('_', 'P', 'I', 'R'))
				continue;
		}
		
		rev_min = *(p + 4);
		rev_maj = *(p + 5);
		tablesize = *(u_int16_t *)(p + 6);

		cksum = 0;
		for (i = 0; i < tablesize; i++)
			cksum += *(unsigned char *)(p + i);

		printf("PCI IRQ Routing Table rev. %d.%d found at 0x%lx, "
		    "size %d bytes (%d entries)\n", rev_maj, rev_min, pa,
		    tablesize, (tablesize - 32) / 16);

		if (cksum != 0) {
			printf("pcibios_pir_init: bad IRQ table checksum\n");
			continue;
		}

		if (tablesize < 32 || (tablesize % 16) != 0) {
			printf("pcibios_pir_init: bad IRQ table size\n");
			continue;
		}

		if (rev_maj != 1 || rev_min != 0) {
			printf("pcibios_pir_init: unsupported IRQ table "
			    "version\n");
			continue;
		}

		/*
		 * We can handle this table!  Make a copy of it.
		 */
		memcpy(&pcibios_pir_header, p, 32);
		pcibios_pir_table = malloc(tablesize - 32, M_DEVBUF,
		    M_NOWAIT);
		if (pcibios_pir_table == NULL) {
			printf("pcibios_pir_init: no memory for $PIR\n");
			return;
		}
		memcpy(pcibios_pir_table, p + 32, tablesize - 32);
		pcibios_pir_table_nentries = (tablesize - 32) / 16;

		printf("PCI Interrupt Router at %03d:%02d:%01d",
		    pcibios_pir_header.router_bus,
		    PIR_DEVFUNC_DEVICE(pcibios_pir_header.router_devfunc),
		    PIR_DEVFUNC_FUNCTION(pcibios_pir_header.router_devfunc));
		if (pcibios_pir_header.compat_router != 0) {
			devinfo = malloc(256, M_DEVBUF, M_NOWAIT);
			if (devinfo) {
				pci_devinfo(pcibios_pir_header.compat_router,
				    0, 0, devinfo, 256);
				printf(" (%s compatible)", devinfo);
				free(devinfo, M_DEVBUF);
			}
		}
		printf("\n");
		pcibios_print_exclirq();

#ifdef PCIBIOS_LIBRETTO_FIXUP
		/* for Libretto L2/L3 hack */
		pcibios_fixup_pir_table();
#endif
#ifdef PCIBIOS_SHARP_MM20_FIXUP
		pcibios_mm20_fixup();
#endif
#ifdef PCIINTR_DEBUG
		pcibios_print_pir_table();
#endif
		return;
	}

	/*
	 * If there was no PIR table found, try using the PCI BIOS
	 * Get Interrupt Routing call.
	 *
	 * XXX The interface to this call sucks; just allocate enough
	 * XXX room for 32 entries.
	 */
	pcibios_pir_table_nentries = 32;
	pcibios_pir_table = malloc(pcibios_pir_table_nentries *
	    sizeof(*pcibios_pir_table), M_DEVBUF, M_NOWAIT);
	if (pcibios_pir_table == NULL) {
		printf("pcibios_pir_init: no memory for $PIR\n");
		return;
	}
	if (pcibios_get_intr_routing(pcibios_pir_table,
	    &pcibios_pir_table_nentries,
	    &pcibios_pir_header.exclusive_irq) != PCIBIOS_SUCCESS) {
		printf("No PCI IRQ Routing information available.\n");
		free(pcibios_pir_table, M_DEVBUF);
		pcibios_pir_table = NULL;
		pcibios_pir_table_nentries = 0;
		return;
	}
	printf("PCI BIOS has %d Interrupt Routing table entries\n",
	    pcibios_pir_table_nentries);
	pcibios_print_exclirq();

#ifdef PCIBIOS_LIBRETTO_FIXUP
	/* for Libretto L2/L3 hack */
	pcibios_fixup_pir_table();
#endif
#ifdef PCIBIOS_SHARP_MM20_FIXUP
	pcibios_mm20_fixup();
#endif
#ifdef PCIINTR_DEBUG
	pcibios_print_pir_table();
#endif
}

int
pcibios_get_status(u_int32_t *rev_maj, u_int32_t *rev_min,
    u_int32_t *mech1, u_int32_t *mech2, u_int32_t *scmech1, u_int32_t *scmech2,
    u_int32_t *maxbus)
{
	u_int16_t ax, bx, cx;
	u_int32_t edx;
	int rv;

	__asm __volatile("lcall *(%%edi)				; \
			jc 1f						; \
			xor %%ah, %%ah					; \
		1:"
		: "=a" (ax), "=b" (bx), "=c" (cx), "=d" (edx)
		: "0" (0xb101), "D" (&pcibios_entry));

	rv = pcibios_return_code(ax, "pcibios_get_status");
	if (rv != PCIBIOS_SUCCESS)
		return (rv);

	if (edx != BIOS32_MAKESIG('P', 'C', 'I', ' '))
		return (PCIBIOS_SERVICE_NOT_PRESENT);	/* XXX */

	/*
	 * Fill in the various pieces if info we're looking for.
	 */
	*mech1 = ax & 1;
	*mech2 = ax & (1 << 1);
	*scmech1 = ax & (1 << 4);
	*scmech2 = ax & (1 << 5);
	*rev_maj = (bx >> 8) & 0xff;
	*rev_min = bx & 0xff;
	*maxbus = cx & 0xff;

	return (PCIBIOS_SUCCESS);
}

int
pcibios_get_intr_routing(struct pcibios_intr_routing *table,
    int *nentries, u_int16_t *exclirq)
{
	u_int16_t ax, bx;
	int rv;
	struct {
		u_int16_t size;
		caddr_t offset;
		u_int16_t segment;
	} __attribute__((__packed__)) args;

	args.size = *nentries * sizeof(*table);
	args.offset = (caddr_t)table;
	args.segment = GSEL(GDATA_SEL, SEL_KPL);

	memset(table, 0, args.size);

	__asm __volatile("lcall *(%%esi)				; \
			jc 1f						; \
			xor %%ah, %%ah					; \
		1:	movw %w2, %%ds					; \
			movw %w2, %%es"
		: "=a" (ax), "=b" (bx)
		: "r" GSEL(GDATA_SEL, SEL_KPL), "0" (0xb10e), "1" (0),
		  "D" (&args), "S" (&pcibios_entry));

	rv = pcibios_return_code(ax, "pcibios_get_intr_routing");
	if (rv != PCIBIOS_SUCCESS)
		return (rv);

	*nentries = args.size / sizeof(*table);
	*exclirq = bx;

	return (PCIBIOS_SUCCESS);
}

int
pcibios_return_code(u_int16_t ax, const char *func)
{
	const char *errstr;
	int rv = ax >> 8;

	switch (rv) {
	case PCIBIOS_SUCCESS:
		return (PCIBIOS_SUCCESS);

	case PCIBIOS_SERVICE_NOT_PRESENT:
		errstr = "service not present";
		break;

	case PCIBIOS_FUNCTION_NOT_SUPPORTED:
		errstr = "function not supported";
		break;

	case PCIBIOS_BAD_VENDOR_ID:
		errstr = "bad vendor ID";
		break;

	case PCIBIOS_DEVICE_NOT_FOUND:
		errstr = "device not found";
		break;

	case PCIBIOS_BAD_REGISTER_NUMBER:
		errstr = "bad register number";
		break;

	case PCIBIOS_SET_FAILED:
		errstr = "set failed";
		break;

	case PCIBIOS_BUFFER_TOO_SMALL:
		errstr = "buffer too small";
		break;

	default:
		printf("%s: unknown return code 0x%x\n", func, rv);
		return (rv);
	}

	printf("%s: %s\n", func, errstr);
	return (rv);
}

void
pcibios_print_exclirq(void)
{
	int i;

	if (pcibios_pir_header.exclusive_irq) {
		printf("PCI Exclusive IRQs:");
		for (i = 0; i < 16; i++) {
			if (pcibios_pir_header.exclusive_irq & (1 << i))
				printf(" %d", i);
		}
		printf("\n");
	}
}

#ifdef PCIBIOS_LIBRETTO_FIXUP
/* for Libretto L2/L3 hack */
static void 
pcibios_fixup_pir_table(void)
{
	struct pcibios_linkmap *m;

	for (m = pir_mask; m->link != 0; m++)
		pcibios_fixup_pir_table_mask(m);
}

void 
pcibios_fixup_pir_table_mask(struct pcibios_linkmap *mask)
{
	int i, j;

	for (i = 0; i < pcibios_pir_table_nentries; i++) {
		for (j = 0; j < 4; j++) {
			if (pcibios_pir_table[i].linkmap[j].link == mask->link) {
				pcibios_pir_table[i].linkmap[j].bitmap
				    &= mask->bitmap; 
			}
		}
	}
}
#endif

#ifdef PCIINTR_DEBUG
void
pcibios_print_pir_table(void)
{
	int i, j;

	for (i = 0; i < pcibios_pir_table_nentries; i++) {
		printf("PIR Entry %d:\n", i);
		printf("\tBus: %d  Device: %d\n",
		    pcibios_pir_table[i].bus,
		    PIR_DEVFUNC_DEVICE(pcibios_pir_table[i].device));
		for (j = 0; j < 4; j++) {
			printf("\t\tINT%c: link 0x%02x bitmap 0x%04x\n",
			    'A' + j,
			    pcibios_pir_table[i].linkmap[j].link,
			    pcibios_pir_table[i].linkmap[j].bitmap);
		}
	}
}
#endif

void 
pci_device_foreach(pci_chipset_tag_t pc, int maxbus,
    void (*func)(pci_chipset_tag_t, pcitag_t, void *), void *context)
{
	pci_device_foreach_min(pc, 0, maxbus, func, context);
}

void
pci_device_foreach_min(pci_chipset_tag_t pc, int minbus, int maxbus,
    void (*func)(pci_chipset_tag_t, pcitag_t, void *), void *context)
{
	const struct pci_quirkdata *qd;
	int bus, device, function, maxdevs, nfuncs;
	pcireg_t id, bhlcr;
	pcitag_t tag;

	for (bus = minbus; bus <= maxbus; bus++) {
		maxdevs = pci_bus_maxdevs(pc, bus);
		for (device = 0; device < maxdevs; device++) {
			tag = pci_make_tag(pc, bus, device, 0);
			id = pci_conf_read(pc, tag, PCI_ID_REG);

			/* Invalid vendor ID value? */
			if (PCI_VENDOR(id) == PCI_VENDOR_INVALID)
				continue;
			/* XXX Not invalid, but we've done this ~forever. */
			if (PCI_VENDOR(id) == 0)
				continue;

			qd = pci_lookup_quirkdata(PCI_VENDOR(id),
			    PCI_PRODUCT(id));

			bhlcr = pci_conf_read(pc, tag, PCI_BHLC_REG);
			if (PCI_HDRTYPE_MULTIFN(bhlcr) ||
			    (qd != NULL &&
			     (qd->quirks & PCI_QUIRK_MULTIFUNCTION) != 0))
				nfuncs = 8;
			else
				nfuncs = 1;

			for (function = 0; function < nfuncs; function++) {
				tag = pci_make_tag(pc, bus, device, function);
				id = pci_conf_read(pc, tag, PCI_ID_REG);

				/* Invalid vendor ID value? */
				if (PCI_VENDOR(id) == PCI_VENDOR_INVALID)
					continue;
				/*
				 * XXX Not invalid, but we've done this
				 * ~forever.
				 */
				if (PCI_VENDOR(id) == 0)
					continue;
				(*func)(pc, tag, context);
			}
		}
	}
}

void
pci_bridge_foreach(pci_chipset_tag_t pc, int minbus, int maxbus,
    void (*func)(pci_chipset_tag_t, pcitag_t, void *), void *ctx)
{
	struct pci_bridge_hook_arg bridge_hook;

	bridge_hook.func = func;
	bridge_hook.arg = ctx;
	
	pci_device_foreach_min(pc, minbus, maxbus, pci_bridge_hook,
	    &bridge_hook);
}

void
pci_bridge_hook(pci_chipset_tag_t pc, pcitag_t tag, void *ctx)
{
	struct pci_bridge_hook_arg *bridge_hook = (void *)ctx;
	pcireg_t reg;

	reg = pci_conf_read(pc, tag, PCI_CLASS_REG);
	if (PCI_CLASS(reg) == PCI_CLASS_BRIDGE &&
	    (PCI_SUBCLASS(reg) == PCI_SUBCLASS_BRIDGE_PCI ||
		PCI_SUBCLASS(reg) == PCI_SUBCLASS_BRIDGE_CARDBUS)) {
		(*bridge_hook->func)(pc, tag, bridge_hook->arg);
	}
}

#ifdef PCIBIOS_SHARP_MM20_FIXUP
/*
 * This is a gross hack to get the interrupt from the EHCI controller
 * working on a Sharp MM20.  The BIOS is just incredibly buggy.
 *
 * The story thus far:
 * The modern way to route the interrupt is to use ACPI.  But using
 * ACPI fails with an error message about an uninitialized local
 * variable in the AML code.  (It works in Windows, but fails in NetBSD
 * and Linux.)
 *
 * The second attempt is to use PCI Interrupt Routing table.  But this
 * fails because the table does not contain any information about the
 * interrupt from the EHCI controller.  This is probably due to the fact
 * that the table is compatible with ALi M1543, but the MM20 has an ALi M1563.
 * The M1563 has additional interrupt lines.  The ali1543.c code also
 * cannot handle the M1653's extended interrupts.  And fixing this is
 * difficult since getting a data sheet from ALi requires signing an NDA.
 *
 * The third attempt is to use a BIOS call to route the interrupt
 * (as FreeBSD does) with manually generated information.  But the BIOS call
 * fails because the BIOS code is not quite position independent.  It makes
 * some assumption about where the code segment register points.
 * 
 * So the solution is to use the third attempt, but with a patched version
 * of the BIOS.
 *    -- lennart@augustsson.net
 */

#define	BIOS32_START	0xe0000
#define	BIOS32_SIZE	0x20000

static char pcibios_shadow[BIOS32_SIZE];
static struct bios32_entry pcibios_entry_shadow;

/* 
 * Copy BIOS and zap offending instruction.
 * The bad instruction is
 *    mov    %cs:0x63c(%ebx),%ah
 * NetBSD does not have the code segment set up for this to work.
 * Using the value 0xff for the table entry seems to work.
 * The replacement is
 *    mov $0xff,%ah; nop; nop; nop; nop; nop
 */
static void
pcibios_copy_bios(void)
{
	u_int8_t *bad_instr;

	memcpy(pcibios_shadow, ISA_HOLE_VADDR(BIOS32_START), BIOS32_SIZE);
	pcibios_entry_shadow = pcibios_entry;
	pcibios_entry_shadow.offset =
	    (void*)((u_long)pcibios_shadow +
		    (u_long)pcibios_entry.offset -
		    (u_long)ISA_HOLE_VADDR(BIOS32_START));
	
	bad_instr = (u_int8_t *)pcibios_entry_shadow.offset + 0x499;
	if (*bad_instr != 0x2e)
		panic("bad bios");
	bad_instr[0] = 0xb4; bad_instr[1] = 0xff; /* mov $0xff,%ah */
	bad_instr[2] = 0x90;		/* nop */
	bad_instr[3] = 0x90;		/* nop */
	bad_instr[4] = 0x90;		/* nop */
	bad_instr[5] = 0x90;		/* nop */
	bad_instr[6] = 0x90;		/* nop */
}

/*
 * Call BIOS to route an interrupt.
 * The PCI device is identified by bus,device,func.
 * The interrupt is on pin PIN (A-D) and interrupt IRQ.
 * BIOS knows the magic for the interrupt controller.
 */
static int
pcibios_biosroute(int bus, int device, int func, int pin, int irq)
{
	u_int16_t ax, bx, cx;
	int rv;

	printf("pcibios_biosroute: b,d,f=%d,%d,%d pin=%x irq=%d\n",
	       bus, device, func, pin+0xa, irq);

	bx = (bus << 8) | (device << 3) | func;
	cx = (irq << 8) | (0xa + pin);

	__asm __volatile("lcall *(%%esi)				; \
			jc 1f						; \
			xor %%ah, %%ah					; \
		1:	movw %w1, %%ds					; \
			movw %w1, %%es"
			 : "=a" (ax)
			 : "r" GSEL(GDATA_SEL, SEL_KPL), "0" (0xb10f),
			   "b" (bx), "c" (cx),
		           "S" (&pcibios_entry_shadow));

	rv = pcibios_return_code(ax, "pcibios_biosroute");

	return rv;
}

#define MM20_PCI_BUS 0
#define MM20_PCI_EHCI_DEV 15
#define MM20_PCI_EHCI_FUNC 3
#define MM20_PCI_EHCI_PIN 3
#define MM20_PCI_EHCI_INTR 11
#define MM20_PCI_ISA_DEV 3
#define MM20_PCI_ISA_FUNC 0

static void
pcibios_mm20_fixup(void)
{
	pci_chipset_tag_t pc;
	pcitag_t tag;

	/* Copy BIOS */
	pcibios_copy_bios();
	/* Route the interrupt for the EHCI controller. */
	(void)pcibios_biosroute(MM20_PCI_BUS,
				MM20_PCI_EHCI_DEV,
				MM20_PCI_EHCI_FUNC,
				MM20_PCI_EHCI_PIN,
				MM20_PCI_EHCI_INTR);

	/* Fake some tags. */
	pc = NULL;
	tag = pci_make_tag(pc, MM20_PCI_BUS, MM20_PCI_EHCI_DEV,
			   MM20_PCI_EHCI_FUNC);
	/* Set interrupt register in EHCI controller */
	pci_conf_write(pc, tag, 0x3c, 0x50000400 + MM20_PCI_EHCI_INTR);
	tag = pci_make_tag(pc, MM20_PCI_BUS, MM20_PCI_ISA_DEV,
			   MM20_PCI_ISA_FUNC);
	/* Set some unknown registers in the ISA bridge. */
	pci_conf_write(pc, tag, 0x58, 0xd87f5300);
	pci_conf_write(pc, tag, 0x74, 0x00000009);
}

#endif /* PCIBIOS_SHARP_MM20_FIXUP */
