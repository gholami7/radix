/*
 * arch/i386/acpi.c
 * Copyright (C) 2017 Alexei Frolov
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

#include <radix/asm/acpi.h>
#include <radix/asm/apic.h>
#include <radix/klog.h>

#define ACPI "ACPI: "

static void __madt_lapic(struct acpi_madt_local_apic *s)
{
	klog(KLOG_INFO, ACPI "LAPIC id %d %sactive",
	     s->apic_id, s->flags & 1 ? "" : "in");
}

static void __madt_ioapic(struct acpi_madt_io_apic *s)
{
	ioapic_add(s->id, s->address, s->global_irq_base);
	klog(KLOG_INFO, ACPI "I/O APIC id %d base %p irq_base %d",
	     s->id, s->address, s->global_irq_base);
}

static void __madt_override(struct acpi_madt_interrupt_override *s)
{
	struct ioapic *ioapic;
	int pin, polarity, trigger;

	ioapic = ioapic_from_vector(s->irq_source);
	if (!ioapic)
		klog(KLOG_ERROR, ACPI
		     "ignoring ISA IRQ override for invalid vector %d",
		     s->irq_source);

	pin = s->global_irq - ioapic->irq_base;
	polarity = s->flags & ACPI_MADT_INTI_POLARITY_MASK;
	trigger = s->flags & ACPI_MADT_INTI_TRIGGER_MODE_MASK;

	ioapic_set_vector(ioapic, pin, s->irq_source);
	ioapic_set_polarity(ioapic, pin, polarity);
	ioapic_set_trigger_mode(ioapic, pin, trigger);

	klog(KLOG_INFO, ACPI "IRQ override bus %d int %d ioapic %d pin %d",
	     s->bus_source, s->irq_source, ioapic->id, pin);
}

/*
 * madt_walk:
 * Walk the ACPI MADT table, calling `entry_handler` on each entry.
 */
static void madt_walk(struct acpi_madt *madt,
                      void (*entry_handler)(struct acpi_subtable_header *))
{
	struct acpi_subtable_header *header;
	unsigned char *p, *end;

	p = (unsigned char *)(madt + 1);
	end = (unsigned char *)madt + madt->header.length;

	while (p < end) {
		header = (struct acpi_subtable_header *)p;
		entry_handler(header);
		p += header->length;
	}
}

static void madt_parse_ioapics(struct acpi_subtable_header *header)
{
	switch (header->type) {
	case ACPI_MADT_IO_APIC:
		__madt_ioapic((struct acpi_madt_io_apic *)header);
		break;
	}
}

static void madt_parse_all(struct acpi_subtable_header *header)
{
	switch (header->type) {
	/* TODO: add other MADT entries */
	case ACPI_MADT_LOCAL_APIC:
		__madt_lapic((struct acpi_madt_local_apic *)header);
		break;
	case ACPI_MADT_INTERRUPT_OVERRIDE:
		__madt_override((struct acpi_madt_interrupt_override *) header);
		break;
	}
}

/*
 * acpi_parse_madt:
 * Parse the ACPI MADT table and extract APIC information.
 */
int acpi_parse_madt(void)
{
	struct acpi_madt *madt;

	madt = acpi_find_table(ACPI_MADT_SIGNATURE);
	if (!madt)
		return 1;

	lapic_phys_base = madt->address;
	klog(KLOG_INFO, ACPI "local APIC %p", lapic_phys_base);

	madt_walk(madt, madt_parse_ioapics);
	madt_walk(madt, madt_parse_all);

	return 0;
}