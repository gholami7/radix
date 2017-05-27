/*
 * arch/i386/mm/cache.c
 * Copyright (C) 2016-2017 Alexei Frolov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <radix/mm.h>

static inline void invlpg(addr_t addr)
{
	asm volatile("invlpg (%0)" ::"r"(addr) :"memory");
}

static inline void __tlb_flush_nonglobal(void)
{
	asm volatile("movl %%cr3, %%eax;"
	             "movl %%eax, %%cr3"
	             :::"%eax");
}

/*
 * i386_tlb_flush_nonglobal:
 * Flush all non-global pages from each processor's TLB.
 */
void i386_tlb_flush_nonglobal(int sync)
{
	__tlb_flush_nonglobal();
	(void)sync;
}

/*
 * i386_tlb_flush_page:
 * Flush a single page from the all processors' TLBs.
 */
void i386_tlb_flush_page(addr_t addr, int sync)
{
	invlpg(addr);
	(void)sync;
}

/*
 * i386_tlb_flush_nonglobal_lazy:
 * Flush all non-global pages from the current processor's TLB.
 */
void i386_tlb_flush_nonglobal_lazy(void)
{
	__tlb_flush_nonglobal();
}

/*
 * i386_tlb_flush_page_lazy:
 * Flush a single page from the current processor's TLB.
 */
void i386_tlb_flush_page_lazy(addr_t addr)
{
	invlpg(addr);
}
