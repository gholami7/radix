/*
 * arch/i386/include/radix/cpu.h
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

#ifndef ARCH_I386_RADIX_CPU_H
#define ARCH_I386_RADIX_CPU_H

#define CPU_VENDOR_AMD          "AuthenticAMD"
#define CPU_VENDOR_CENTAUR      "CentaurHauls"
#define CPU_VENDOR_CYRIX        "CyrixInstead"
#define CPU_VENDOR_INTEL        "GenuineIntel"
#define CPU_VENDOR_TRANSMETA    "GenuineTMx86"
#define CPU_VENDOR_NEXGEN       "NexGenDriven"
#define CPU_VENDOR_RISE         "RiseRiseRise"
#define CPU_VENDOR_SIS          "SiS SiS SiS "
#define CPU_VENDOR_UMC          "UMC UMC UMC "
#define CPU_VENDOR_VIA          "VIA VIA VIA "
#define CPU_VENDOR_VORTEX       "Vortex86 SoC"

#define CPU_VENDOR_KVM          "KVMKVMKVM"
#define CPU_VENDOR_HYPERV       "Microsoft Hv"
#define CPU_VENDOR_VMWARE       "VMwareWMware"
#define CPU_VENDOR_PARALLELS    " lrpepyh vr"
#define CPU_VENDOR_XENHVM       "XenVMMXenVMM"

/* cpuid 0x1 EDX bits */
#define CPUID_FPU       (1 << 0)
#define CPUID_VME       (1 << 1)
#define CPUID_DE        (1 << 2)
#define CPUID_PSE       (1 << 3)
#define CPUID_TSC       (1 << 4)
#define CPUID_MSR       (1 << 5)
#define CPUID_PAE       (1 << 6)
#define CPUID_MCE       (1 << 7)
#define CPUID_CX8       (1 << 8)
#define CPUID_APIC      (1 << 9)
#define CPUID_SEP       (1 << 11)
#define CPUID_MTRR      (1 << 12)
#define CPUID_PGE       (1 << 13)
#define CPUID_MCA       (1 << 14)
#define CPUID_CMOV      (1 << 15)
#define CPUID_PAT       (1 << 16)
#define CPUID_PSE36     (1 << 17)
#define CPUID_PSN       (1 << 18)
#define CPUID_CLFSH     (1 << 19)
#define CPUID_DS        (1 << 21)
#define CPUID_ACPI      (1 << 22)
#define CPUID_MMX       (1 << 23)
#define CPUID_FXSR      (1 << 24)
#define CPUID_SSE       (1 << 25)
#define CPUID_SSE2      (1 << 26)
#define CPUID_SS        (1 << 27)
#define CPUID_HTT       (1 << 28)
#define CPUID_TM        (1 << 29)
#define CPUID_IA64      (1 << 30)
#define CPUID_PBE       (1 << 31)

/* EFLAGS register bits */
#define EFLAGS_CF       (1 << 0)
#define EFLAGS_PF       (1 << 2)
#define EFLAGS_AF       (1 << 4)
#define EFLAGS_ZF       (1 << 6)
#define EFLAGS_SF       (1 << 7)
#define EFLAGS_TF       (1 << 8)
#define EFLAGS_IF       (1 << 9)
#define EFLAGS_DF       (1 << 10)
#define EFLAGS_OF       (1 << 11)
#define EFLAGS_IOPL     ((1 << 12) | (1 << 13))
#define EFLAGS_NT       (1 << 14)
#define EFLAGS_RF       (1 << 16)
#define EFLAGS_VM       (1 << 17)
#define EFLAGS_AC       (1 << 18)
#define EFLAGS_VIF      (1 << 19)
#define EFLAGS_VIP      (1 << 20)
#define EFLAGS_ID       (1 << 21)

#include <radix/compiler.h>

static __always_inline unsigned long cpuid_supported(void)
{
	unsigned long res;

	asm volatile("pushf;"
	             "pop %%eax;"
	             "movl %%eax, %%ecx;"
	             "xorl $0x200000, %%eax;"
	             "push %%eax;"
	             "popf;"
	             "pushf;"
	             "pop %%eax;"
	             "xorl %%ecx, %%eax"
	             : "=a"(res)
	             :
	             : "%ecx");
	return res;
}

#define cpuid(eax, a, b, c, d) \
	asm volatile("xchg %%ebx, %1;" \
	             "cpuid;" \
	             "xchg %%ebx, %1" \
	             : "=a"(a), "=r"(b), "=c"(c), "=d"(d) \
	             : "0"(eax))

int cpu_has_apic(void);
unsigned long cpu_cache_line_size(void);

char *cpu_cache_str(void);

#endif /* ARCH_I386_RADIX_CPU_H */