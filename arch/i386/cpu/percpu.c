/*
 * arch/i386/cpu/percpu.c
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

#include <radix/percpu.h>

#include "gdt.h"

#define BOOT_PERCPU_OFFSET 0

DEFINE_PER_CPU(unsigned long, __this_cpu_offset);

void arch_percpu_init_early(void)
{
	gdt_set_initial_fsbase(BOOT_PERCPU_OFFSET);
	this_cpu_write(__this_cpu_offset, BOOT_PERCPU_OFFSET);
}
