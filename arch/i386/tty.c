/*
 * arch/i386/tty.c
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

#include <ctype.h>
#include <string.h>
#include <untitled/tty.h>

#include "vga.h"

#define TTY_BUFSIZE     (VGA_HEIGHT * VGA_WIDTH)

#define ASCII_ESC       0x1B

static size_t vga_row;
static size_t vga_col;

static uint8_t vga_fg;
static uint8_t vga_bg;
static uint8_t vga_color;

static uint16_t *vga_buf;

static char tty_buf[TTY_BUFSIZE];
static char *tty_pos;

/* tty_init: initialize tty variables and populate vga buffer */
void tty_init(void)
{
	size_t x, y, ind;

	vga_row = 0;
	vga_col = 0;
	vga_fg = VGA_COLOR_WHITE;
	vga_bg = VGA_COLOR_BLACK;
	vga_color = vga_entry_color(vga_fg, vga_bg);
	vga_buf = (uint16_t *)VGA_TEXT_BUFFER_ADDR;
	tty_pos = tty_buf;

	for (y = 0; y < VGA_HEIGHT; ++y) {
		for (x = 0; x < VGA_WIDTH; ++x) {
			ind = y * VGA_WIDTH + x;
			vga_buf[ind] = vga_entry(' ', vga_color);
		}
	}
}

static void tty_nextrow(void);
static void tty_put(int c, uint8_t color, size_t x, size_t y);

/* tty_putchar: write character c at current tty position, and increment pos */
void tty_putchar(int c)
{
	if (tty_pos - tty_buf == TTY_BUFSIZE)
		tty_flush();

	*tty_pos++ = c;
	if (c == '\n')
		tty_flush();
}

/* tty_write: write size bytes of string data to the tty */
void tty_write(const char *data, size_t size)
{
	while (size--)
		tty_putchar(*data++);
}

static int get_ansi_command(char *s)
{
	while (isdigit(*s) || *s == ';')
		++s;
	return *s;
}

/* set_mode: set VGA buffer colors from ANSI graphics mode */
static size_t set_mode(char *s)
{
	size_t i, n;
	int intensity;

	n = 0;
	intensity = VGA_NORMAL;

	while (s[n] != 'm') {
		i = 0;
		while (isdigit(s[n])) {
			i = 10 * i + (s[n] - '0');
			++n;
		}

		if (i == 0)
			intensity = VGA_NORMAL;
		else if (i == 1)
			intensity = VGA_BOLD;
		else if (i >= 30 && i <= 37)
			vga_fg = (i - 30) | intensity;
		else if (i >= 40 && i <= 47)
			vga_bg = (i - 40) | intensity;
		else
			return 0;

		vga_color = vga_entry_color(vga_fg, vga_bg);

		if (s[n] == ';')
			++n;
	}

	return n;
}

/*
 * process_ansi_esc:
 * Process an ANSI escape sequence in string s and modify VGA buffer settings
 * accordingly. Return number of characters skipped.
 */
static size_t process_ansi_esc(char *s)
{
	size_t n;
	int cmd;

	if (s[1] != '[')
		return 0;

	n = 2;
	s += n;
	cmd = get_ansi_command(s);

	switch (cmd) {
	case 'm':
		n += set_mode(s);
		if (n == 2)
			return 0;
		break;
	}

	return n;
}

/* tty_flush: write tty buffer to vga text buffer */
void tty_flush(void)
{
	char *s;
	size_t n;

	for (s = tty_buf; s < tty_pos; ++s) {
		switch (*s) {
		case '\n':
			tty_nextrow();
			continue;
		case '\t':
			while ((++vga_col) % TTY_TAB_STOP != 0) {
				if (vga_col == VGA_WIDTH)
					tty_nextrow();
			}
			continue;
		case ASCII_ESC:
			n = process_ansi_esc(s);
			if (!n) {
				tty_put(*s, vga_color, vga_col, vga_row);
				break;
			} else {
				s += n;
				continue;
			}
		default:
			tty_put(*s, vga_color, vga_col, vga_row);
			break;
		}
		if (++vga_col == VGA_WIDTH)
			tty_nextrow();
	}
	tty_pos = tty_buf;
}

/* tty_nextrow: advance to the next row, "scrolling" if necessary */
static void tty_nextrow(void)
{
	size_t x, dst;

	vga_col = 0;
	if (vga_row == VGA_HEIGHT - 1) {
		/* move each row up by one, discarding the first */
		memmove(vga_buf, vga_buf + VGA_WIDTH,
		        vga_row * VGA_WIDTH * sizeof(uint16_t));
		/* clear the final row */
		for (x = 0; x < VGA_WIDTH; ++x) {
			dst = vga_row * VGA_WIDTH + x;
			vga_buf[dst] = vga_entry(' ', vga_color);
		}
	} else {
		++vga_row;
	}
}

/* tty_put: write c to position x, y */
static void tty_put(int c, uint8_t color, size_t x, size_t y)
{
	size_t ind;

	ind = y * VGA_WIDTH + x;
	vga_buf[ind] = vga_entry(c, color);
}
