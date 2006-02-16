/* $Id$ */
#include <rtems.h>
#include <libcpu/spr.h>
#include <libcpu/cpuIdent.h>
#include <rtems/bspIo.h>

/* Till Straumann <straumanatslacdotstanforddotedu>, 2005 */

/* Simple memory probing routine
 * 
 *  - call from MMU-disabled section to avoid having to 
 *	  set up mappings.
 *    NOTE: this implies WIMG = 0011
 *  - call AFTER image is at its destination and PRIOR
 *    to setting up the heap or using any memory beyond
 *    __rtems_end, i.e., the probing algorithm may safely
 *    tamper with memory > __rtems_end.
 *  - MUST lock caches since we're gonna hit space with
 *    no memory attached.
 *
 * ASSUMPTIONS:
 *		o image occupies addresses between 0..__rtems_end
 *		o memory size is a multiple of 1<<LD_MEM_PROBE_STEP
 *
 * CAVEATS:
 *      o all caches must be disabled or locked (some
 *        boards really don't like it if you try to
 *        cache physical addresses with nothing attached)
 *        and this is highly CPU dependent :-(...
 *
 *	- RETURNS size of memory detected in bytes or 0 on
 *    error.
 */

/* declare as an array so the compiler doesn't generate
 * a reloc to .sdata & friends
 */
extern uint32_t __rtems_end[];

#ifndef LD_MEM_PROBE_STEP
#define LD_MEM_PROBE_STEP	(24) /* 16MB */
#endif

#define TAG 0xfeedcafe

#define __DO_ALIGN(a, s)	(((uint32_t)(a) + (s)-1) & ~((s)-1))
#define __ALIGN(a)	__DO_ALIGN(a, (1<<LD_MEM_PROBE_STEP))

void __here_s_the_real_end();

#define SWITCH_MSR(msr)		\
	asm volatile(			\
		"	mtsrr1	%0	\n"	\
		"   bl 1f		\n"	\
		"1:	mflr    %0	\n"	\
		"	addi %0, %0, 1f-1b	\n"\
		"	mtsrr0  %0	\n"	\
		"	sync		\n"	\
		"	rfi			\n"	\
		"1:				\n"	\
		:					\
		:"b"(msr)			\
		:"lr"				\
	)

SPR_RW(L2CR)
SPR_RW(L3CR)
SPR_RO(PVR)
SPR_RW(HID0)


/* Shouldn't matter if the caches are enabled or not... */

/* FIXME: This should go into libcpu, really... */
int
CPU_lockUnlockCaches(register int doLock)
{
register uint32_t v, x;
	if ( _read_MSR() & MSR_VE ) {
#define DSSALL  0x7e00066c  /* dssall opcode */
        asm volatile("  .long %0"::"i"(DSSALL));
#undef  DSSALL
	}
	asm volatile("sync");
	switch ( _read_PVR()>>16 ) {
		default:		printk(__FILE__" CPU_lockUnlockCaches(): unknown CPU (PVR = 0x%08x)\n",_read_PVR());
						return -1;
		case PPC_750:	printk("CPU_lockUnlockCaches(): Can't lock L2 on a mpc750, sorry :-(\n");
						return -2;	/* cannot lock L2 :-( */
		case PPC_7455:	
		case PPC_7457:	
						v = _read_L3CR();
						x = 1<<(31-9);
						v = doLock ? v | x : v & ~x;
						_write_L3CR(v);

						v = _read_L2CR();
						x = 1<<(31-11);
						v = doLock ? v | x : v & ~x;
						_write_L2CR(v);
				break;

		case PPC_7400:
						v = _read_L2CR();
						x = 1<<(31-21);
						v = doLock ? v | x : v & ~x;
						_write_L2CR(v);
				break;
		case PPC_603:
		case PPC_604:
		case PPC_604e:
				break;
	}
	
	v = _read_HID0();
	x = 1<<(31-19);
	v = doLock ? v | x : v & ~x;
	_write_HID0(v);
	asm volatile("sync");
	return 0;
}

uint32_t
probeMemoryEnd()
{
register volatile uint32_t *probe;
register          uint32_t scratch;
register          uint32_t tag = TAG;
register          uint32_t flags;

	probe = (volatile uint32_t *)__ALIGN(__rtems_end);

	/* Start with some checks. We avoid using any services
	 * such as 'printk' so we can run at a very early stage.
	 * Also, we *try* to avoid to really rely on the memory
	 * being unused by restoring the probed locations and
	 * keeping everything in registers. Hence we could
	 * even probe our own stack :-)
	 */

	/* are we really at the highest address ? */
	if ( probe < (volatile uint32_t *)__here_s_the_real_end ) {
		probe = (volatile uint32_t*)__ALIGN(__here_s_the_real_end);
	}

	if ( CPU_lockUnlockCaches(1) )
		return 0;

	asm volatile("mfmsr %0":"=r"(flags));

	SWITCH_MSR( flags & ~(MSR_EE|MSR_DR|MSR_IR) );

	for ( ; (uint32_t)probe ;  probe += (1<<LD_MEM_PROBE_STEP)/sizeof(*probe) ) {

		/* see if by chance our tag is already there */
		if ( tag == (scratch = *probe) ) {
			/* try another tag */
			tag = ~tag;
		}
		*probe = tag;

		/* make sure it's written out */
		asm volatile ("sync");

		/* try to read back */
		if ( tag != *probe ) {
				break;
		}
		/* restore */
		*probe = scratch;
		/* make sure the icache is not contaminated */
		asm volatile ("sync; icbi 0, %0"::"r"(probe));
	}

	SWITCH_MSR(flags);

	CPU_lockUnlockCaches(0);

	return (uint32_t) probe;
}

void __here_s_the_real_end() {}