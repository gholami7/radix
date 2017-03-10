/*
 * arch/i386/cpu/apic.c
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

#include <acpi/acpi.h>
#include <acpi/tables/madt.h>

#include <radix/asm/msr.h>
#include <radix/error.h>
#include <radix/kernel.h>
#include <radix/mm.h>
#include <radix/page.h>

#include "apic.h"

#define APIC_BASE_MSR           0x1B
#define APIC_BASE_MSR_ENABLE    0x800

static struct acpi_madt *madt;

int apic_madt_check(void)
{
	madt = acpi_find_table(ACPI_MADT_SIGNATURE);
	if (!madt)
		return EINVAL;

	if (!acpi_valid_checksum((struct acpi_sdt_header *)madt)) {
		BOOT_FAIL_MSG("ACPI MADT checksum invalid\n");
		return EINVAL;
	}

	return 0;
}

static addr_t get_apic_phys_base(void)
{
	uint32_t eax, edx;

	rdmsr(APIC_BASE_MSR, &eax, &edx);
	return eax & PAGE_MASK;
}

void apic_init(void)
{
	addr_t phys;

	phys = get_apic_phys_base();
	map_page(__ARCH_APIC_VIRT_PAGE, phys);
}
