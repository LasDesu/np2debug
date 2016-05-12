// break.c - Emulation activity control
// -------
// Author: Nmlgc

#include "compiler.h"
#include "np2.h"
#include "mousemng.h"
#include "scrnmng.h"
#include "soundmng.h"
#include "sysmng.h"
#include "unasm.h"
#include "keystat.h"
#include "cpucore.h"
#include "break.h"
#include "unasmdef.tbl"
#include "viewer.h"
#ifdef WIN32
#include "viewmenu.h"
#include "viewstat.h"
#else
#define	viewmenu_all_debug_toggle(running)
#define	viewstat_all_breakpoint(type, addr)
#endif

#ifndef CPU_STAT_PM
#define CPU_STAT_PM 0
#endif

/// Globals
/// =======
UINT8	np2stopemulate = 0;
BOOL 	np2singlestep = 0;
np2break_t	np2breakflags[sizeof(mem)];
LISTARRAY	np2breakaddrs = NULL;
/// =======

void np2active_renewal(UINT8 breakflag) {										// ver0.30

	if (breakflag & (~NP2BREAK_MAIN)) {
		np2stopemulate = 2;
		soundmng_disable(SNDPROC_MASTER);
	}
	else if (breakflag & NP2BREAK_MAIN) {
		if (np2oscfg.background & 1) {
			np2stopemulate = 1;
		}
		else {
			np2stopemulate = 0;
		}
		if (np2oscfg.background) {
			soundmng_disable(SNDPROC_MASTER);
		}
		else {
			soundmng_enable(SNDPROC_MASTER);
		}
	}
	else {
		np2stopemulate = 0;
		soundmng_enable(SNDPROC_MASTER);
	}
	viewmenu_all_debug_toggle(!np2stopemulate);
	sysmng_updatecaption(0);
}

void np2active_set(int active)
{
	if (active) {
		scrnmng_update();
		keystat_allrelease();
		mousemng_enable(MOUSEPROC_BG);
	}
	else {
		mousemng_disable(MOUSEPROC_BG);
	}
	np2active_renewal(active ? NP2BREAK_RESUME : NP2BREAK_DEBUG);
}

void np2active_step()
{	
	if(!np2stopemulate)	{
		return;
	}
	np2active_set(1);
	np2singlestep = 1;
}

void np2active_step_over()	{

	UINT step;
	_UNASM una;

	if(!np2stopemulate)	{
		return;
	}
	step = unasm_next(&una);
	if(
		!strcmp(una.mnemonic, "call") ||
		!strcmp(una.mnemonic, "int")
	)	{
		np2break_toggle(CPU_CS, CPU_IP + step, NP2BP_EXECUTE | NP2BP_ONESHOT);
		np2active_set(1);
	} else {
		np2active_step();
	}
}

/// Breakpoints
/// -----------
void np2break_create()
{
	if(!np2breakaddrs) {
		np2breakaddrs = listarray_new(sizeof(UINT32), 16);
	}
}

static BOOL np2breakaddr_lookup_callback(void *vpItem, void *vpArg)
{
	return *((UINT32*)vpArg) == *((UINT32*)vpItem);
}

UINT32* np2breakaddr_lookup(UINT32 addr)
{
	return (UINT32*)listarray_enum(
		np2breakaddrs, np2breakaddr_lookup_callback, &addr
	);
}

np2break_t* np2break_lookup_real(UINT32 addr)
{
	if(addr > NELEMENTS(np2breakflags)) {
		// TODO: What do we even want to happen in this case?
		static np2break_t null_bp = NP2BP_NONE;
		return &null_bp;
	}
	return &np2breakflags[addr];
}

np2break_t* np2break_lookup(UINT32 *addr_if_hit, UINT16 seg, UINT16 off)
{
	UINT32 addr = (seg << 4) + off;
	np2break_t *ret = np2break_lookup_real(addr);
	if(*ret != NP2BP_NONE && addr_if_hit) {
		*addr_if_hit = addr;
	}
	return ret;
}

BOOL np2break_toggle_real(UINT32 addr, np2break_t flag)
{
	// LISTARRAY doesn't support element deletion.
	// We work around that here by zeroing out the elements that should be deleted.
	// Once a new element should be appended, we check for these zeroed entries
	// in order to keep the list from becoming larger and larger over time.
	UINT32* addr_slot = np2breakaddr_lookup(addr);
	np2break_t *flag_slot = np2break_lookup_real(addr);
	if(addr_slot && *flag_slot != NP2BP_NONE) {
		// Disable
		memset(addr_slot, 0, sizeof(*addr_slot));
		*flag_slot = NP2BP_NONE;
		return(TRUE);
	}
	
	// Look for a zeroed element
	addr_slot = np2breakaddr_lookup(0);
	if(!addr_slot) {
		// Nothing found, append a new one
		addr_slot = (UINT32*)listarray_append(np2breakaddrs, NULL);
		if(!addr_slot) {
			return(FALSE);
		}
	}
	// Set this element
	*addr_slot = addr;
	*flag_slot |= flag;
	return(TRUE);
}

BOOL np2break_toggle(UINT16 seg, UINT16 off, np2break_t flag)
{
	return np2break_toggle_real((seg << 4) + off, flag);
}

static UINT8 is_mem_type(const UNASM_MEMINFO *mi)	{

	if(!mi->off)	{
		return FALSE;
	}
	switch(mi->type)	{
		case OP_MEM:
		case OP_EA:
		case OP_PEA:
		case OP1_STR:
			return TRUE;
	}
	return FALSE;
}

// This is kept in here to catch edge cases not yet covered by unasm
static UINT32 np2break_memory_write_naive()	{

	static UINT bp_num = 0;
	static UINT8* probe = NULL;
	UINT bp_num_new = listarray_getitems(np2breakaddrs);
	UINT i;
	if(bp_num_new > bp_num)	{
		probe = (UINT8*)realloc(probe, bp_num_new);
		bp_num = bp_num_new;
	}
	for(i = 0; i < bp_num; i++)	{
		UINT32* bp = (UINT32*)listarray_getitem(np2breakaddrs, i);
		if(!bp)	{
			bp_num = i;
			return 0;
		}
		if(*bp && *np2break_lookup_real(*bp) & NP2BP_WRITE) {
			UINT8 cur_val = mem[*bp];
			if(probe[i] != cur_val)	{
				probe[i] = cur_val;
				return *bp;
			}
		}
	}
	return 0;
}

UINT32 np2break_is_next()	{

	_UNASM una;
	UINT32 addr = 0;
	np2break_t type = NP2BP_NONE;

#ifdef DEBUG
	addr = np2break_memory_write_naive();
	if(addr) {
		viewstat_all_breakpoint(NP2BP_WRITE, addr);
		return addr;
	}
#endif
	if(!listarray_getitems(np2breakaddrs)) {
		return 0;
	}

	unasm_next(&una);
	if(strcmp(una.mnemonic, "lea") && strcmp(una.mnemonic, "les"))	{
		if(is_mem_type(&una.meminf[MI_READ]))	{
			type |= *np2break_lookup(
				&addr, una.meminf[MI_READ].seg, una.meminf[MI_READ].off
			);
#ifndef DEBUG
		} else if(is_mem_type(&una.meminf[MI_WRITE]))	{
			type |= *np2break_lookup(
				&addr, una.meminf[MI_WRITE].seg, una.meminf[MI_WRITE].off
			);
#endif
		}
	}
	if(type == NP2BP_NONE)	{
		type |= *np2break_lookup(&addr, CPU_CS, CPU_EIP);
	}
	if(type != NP2BP_NONE)	{
		if(type & NP2BP_ONESHOT)	{
			np2break_toggle_real(addr, 0);
		}
		viewstat_all_breakpoint(type, addr);
	}
	return addr;
}

void np2break_reset()
{
	memset(np2breakflags, 0, sizeof(np2breakflags));
	listarray_clr(np2breakaddrs);
}

void np2break_destroy()
{
	listarray_destroy(np2breakaddrs);
}
/// -----------

/// Helper
/// ------
UINT unasm_next(UNASM una)	{

	UINT32 addr;
	UINT8 ins[16];

	addr = (CPU_STAT_PM ? CS_BASE : (CPU_CS<<4))+CPU_IP;
	memp_reads(addr, ins, 16);
	return unasm(una, ins, 16, FALSE, addr);
}
/// ------
