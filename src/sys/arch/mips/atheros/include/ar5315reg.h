/* $Id: ar5315reg.h,v 1.2 2006/09/26 17:09:32 gdamore Exp $ */
/*
 * Copyright (c) 2006 Urbana-Champaign Independent Media Center.
 * Copyright (c) 2006 Garrett D'Amore.
 * All rights reserved.
 *
 * This code was written by Garrett D'Amore for the Champaign-Urbana
 * Community Wireless Network Project.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgements:
 *      This product includes software developed by the Urbana-Champaign
 *      Independent Media Center.
 *	This product includes software developed by Garrett D'Amore.
 * 4. Urbana-Champaign Independent Media Center's name and Garrett
 *    D'Amore's name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE URBANA-CHAMPAIGN INDEPENDENT
 * MEDIA CENTER AND GARRETT D'AMORE ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE URBANA-CHAMPAIGN INDEPENDENT
 * MEDIA CENTER OR GARRETT D'AMORE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	_MIPS_ATHEROS_AR5315REG_H_
#define	_MIPS_ATHEROS_AR5315REG_H_

#define	AR5315_MEM0_BASE		0x00000000	/* sdram */
#define	AR5315_MEM1_BASE		0x08000000	/* spi flash */
#define	AR5315_WLAN_BASE		0x10000000
#define	AR5315_PCI_BASE			0x10100000
#define	AR5315_SDRAMCTL_BASE		0x10300000
#define	AR5315_LOCAL_BASE		0x10400000	/* local bus */
#define	AR5315_ENET_BASE		0x10500000
#define	AR5315_SYSREG_BASE		0x11000000
#define	AR5315_UART_BASE		0x11100000
#define	AR5315_SPI_BASE			0x11300000	/* spi flash */
#define	AR5315_BOOTROM_BASE		0x1FC00000	/* boot rom */
#define	AR5315_CONFIG_BASE		0x087D0000	/* flash start */
#define	AR5315_CONFIG_END		0x087FF000	/* flash end */
#define	AR5315_RADIO_END		0x1FFFF000	/* radio end */

#if 0
#define	AR5315_PCIEXT_BASE		0x80000000	/* pci external */
#define	AR5315_RAM2_BASE		0xc0000000
#define	AR5315_RAM3_BASE		0xe0000000
#endif

/*
 * SYSREG registers  -- offset relative to AR531X_SYSREG_BASE
 */
#define	AR5315_SYSREG_COLDRESET		0x0000
#define	AR5315_SYSREG_RESETCTL		0x0004
#define	AR5315_SYSREG_AHB_ARB_CTL	0x0008
#define	AR5315_SYSREG_ENDIAN		0x000c
#define	AR5315_SYSREG_SREV		0x0014
#define	AR5315_SYSREG_ISR		0x0020
#define	AR5315_SYSREG_IMR		0x0024
#define	AR5315_SYSREG_GISR		0x0028
#define	AR5315_SYSREG_WDOG_TIMER	0x0038
#define	AR5315_SYSREG_WDOG_CTL		0x003c
#define	AR5315_SYSREG_AHB_ERR0		0x0050
#define	AR5315_SYSREG_AHB_ERR1		0x0054
#define	AR5315_SYSREG_AHB_ERR2		0x0058
#define	AR5315_SYSREG_AHB_ERR3		0x005c
#define	AR5315_SYSREG_AHB_ERR4		0x0060
#define	AR5315_SYSREG_PLLC_CTL		0x0064
#define	AR5315_SYSREG_PLLV_CTL		0x0068
#define	AR5315_SYSREG_CPUCLK		0x006c
#define	AR5315_SYSREG_AMBACLK		0x0070

/* Cold resets (AR5315_SYSREG_COLDRESET) */
#define	AR5315_COLD_AHB				0x00000001
#define	AR5315_COLD_APB				0x00000002
#define	AR5315_COLD_CPU				0x00000004
#define	AR5315_COLD_CPU_WARM			0x00000008

/* Resets (AR5315_SYSREG_RESETCTL) */
#define	AR5315_RESET_WARM_WLAN0_MAC		0x00000001
#define	AR5315_RESET_WARM_WLAN0_BB		0x00000002
#define	AR5315_RESET_MPEGTS			0x00000004	/* MPEG-TS */
#define	AR5315_RESET_PCIDMA			0x00000008	/* PCI dma */
#define	AR5315_RESET_MEMCTL			0x00000010
#define	AR5315_RESET_LOCAL			0x00000020	/* local bus */
#define	AR5315_RESET_I2C			0x00000040	/* i2c */
#define	AR5315_RESET_SPI			0x00000080	/* SPI */
#define	AR5315_RESET_UART			0x00000100
#define	AR5315_RESET_IR				0x00000200	/* infrared */
#define	AR5315_RESET_PHY0			0x00000400	/* enet phy */
#define	AR5315_RESET_ENET0			0x00000800

/* Watchdog control (AR5315_SYSREG_WDOG_CTL) */
#define	AR5315_WDOG_CTL_IGNORE			0x0000
#define	AR5315_WDOG_CTL_NMI			0x0001
#define	AR5315_WDOG_CTL_RESET			0x0002

/* AR5315 AHB arbitration control (AR5315_SYSREG_AHB_ARB_CTL) */
#define	AR5315_ARB_CPU				0x00001
#define	AR5315_ARB_WLAN				0x00002
#define	AR5315_ARB_MPEGTS			0x00004
#define	AR5315_ARB_LOCAL			0x00008
#define	AR5315_ARB_PCI				0x00010
#define	AR5315_ARB_ENET				0x00020
#define	AR5315_ARB_RETRY			0x00100

/* AR5315 endianness control (AR5315_SYSREG_ENDIAN) */
#define	AR5315_ENDIAN_AHB			0x00001
#define	AR5315_ENDIAN_WLAN			0x00002
#define	AR5315_ENDIAN_MPEGTS			0x00004
#define	AR5315_ENDIAN_PCI			0x00008
#define	AR5315_ENDIAN_MEMCTL			0x00010
#define	AR5315_ENDIAN_LOCAL			0x00020
#define	AR5315_ENDIAN_ENET			0x00040
#define	AR5315_ENDIAN_MERGE			0x00200
#define	AR5315_ENDIAN_CPU			0x00400
#define	AR5315_ENDIAN_PCIAHB			0x00800
#define	AR5315_ENDIAN_PCIAHB_BRIDGE		0x01000
#define	AR5315_ENDIAN_SPI			0x08000
#define	AR5315_ENDIAN_CPU_DRAM			0x10000
#define	AR5315_ENDIAN_CPU_PCI			0x20000
#define	AR5315_ENDIAN_CPU_MMR			0x40000

/* AR5315 AHB error bits */
#define	AR5315_AHB_ERROR_DET			1	/* error detected */
#define	AR5315_AHB_ERROR_OVR			2	/* AHB overflow */
#define	AR5315_AHB_ERROR_WDT			4	/* wdt (not hresp) */

/* AR5315 clocks */
#define	AR5315_PLLC_REF_DIV(reg)		((reg) & 0x3)
#define	AR5315_PLLC_FB_DIV(reg)			(((reg) & 0x7c) >> 2)
#define	AR5315_PLLC_DIV_2(reg)			(((reg) & 0x80) >> 7)
#define	AR5315_PLLC_CLKC(reg)			(((reg) & 0x1c000) >> 14)
#define	AR5315_PLLC_CLKM(reg)			(((reg) & 0x700000) >> 20)

#define	AR5315_CLOCKCTL_SELECT(reg)		((reg) & 0x3)
#define	AR5315_CLOCKCTL_DIV(reg)		(((reg) & 0xc) >> 2)

/*
 * SDRAMCTL registers  -- offset relative to SDRAMCTL
 */
#define	AR5315_SDRAMCTL_MEM_CFG			0x0000
#define	AR5315_MEM_CFG_DATA_WIDTH_MASK		0x00000600
#define	AR5315_MEM_CFG_DATA_WIDTH_SHIFT		13
#define	AR5315_MEM_CFG_COL_WIDTH_MASK		0x00001E00
#define	AR5315_MEM_CFG_COL_WIDTH_SHIFT		9
#define	AR5315_MEM_CFG_ROW_WIDTH_MASK		0x000001E0
#define	AR5315_MEM_CFG_ROW_WIDTH_SHIFT		5

/* memory config 1 bits */
#define	AR531X_MEM_CFG1_BANK0_MASK		0x00000700
#define	AR531X_MEM_CFG1_BANK0_SHIFT		8
#define	AR531X_MEM_CFG1_BANK1_MASK		0x00007000
#define	AR531X_MEM_CFG1_BANK1_SHIFT		12

/*
 * PCI configuration stuff.  I don't pretend to fully understand these
 * registers, they seem to be magic numbers in the Linux code.
 */
#define	AR5315_PCI_MAC_RC			0x4000
#define	AR5315_PCI_MAC_SCR			0x4004
#define	AR5315_PCI_MAC_INTPEND			0x4008
#define	AR5315_PCI_MAC_SFR			0x400c
#define	AR5315_PCI_MAC_PCICFG			0x4010
#define	AR5315_PCI_MAC_SREV			0x4020

#define	PCI_MAC_RC_MAC				0x1
#define	PCI_MAC_RC_BB				0x2

#define	PCI_MAC_SCR_SLM_MASK			0x00030000
#define	PCI_MAC_SCR_SLM_FWAKE			0x00000000
#define	PCI_MAC_SCR_SLM_FSLEEP			0x00010000
#define	PCI_MAC_SCR_SLM_NORMAL			0x00020000

#define PCI_MAC_PCICFG_SPWR_DN			  0x00010000

/* IRQS */
#define	AR5315_CPU_IRQ_MISC			0
#define	AR5315_CPU_IRQ_WLAN			1
#define	AR5315_CPU_IRQ_ENET			2

#define	AR5315_MISC_IRQ_UART			0
#define	AR5315_MISC_IRQ_I2C			1
#define	AR5315_MISC_IRQ_SPI			2
#define	AR5315_MISC_IRQ_AHBE			3
#define	AR5315_MISC_IRQ_AHPE			4
#define	AR5315_MISC_IRQ_TIMER			5
#define	AR5315_MISC_IRQ_GPIO			6
#define	AR5315_MISC_IRQ_WDOG			7
#define	AR5315_MISC_IRQ_IR			8

#endif	/* _MIPS_ATHEROS_AR531XREG_H_ */