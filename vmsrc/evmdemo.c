/*
 *  EmbedVM - Embedded Virtual Machine for uC Applications
 *
 *  Copyright (C) 2011  Clifford Wolf <clifford@clifford.at>
 *  
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "embedvm.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define UNUSED __attribute__((unused))

static bool stop;
static uint8_t memory[64*1024];

static int16_t mem_read(uint16_t addr, bool is16bit, void *ctx UNUSED)
{
	if (is16bit)
		return (memory[addr] << 8) | memory[addr+1];
	return memory[addr];
}

static void mem_write(uint16_t addr, int16_t value, bool is16bit, void *ctx UNUSED)
{
	if (is16bit) {
		memory[addr] = value >> 8;
		memory[addr+1] = value;
	} else
		memory[addr] = value;
}

static int16_t call_user(uint8_t funcid, uint8_t argc, int16_t *argv, void *ctx UNUSED)
{
	int16_t ret = 0;
	int i;

	if (funcid == 0) {
		stop = true;
		printf("Called user function 0 => stop.\n");
		return ret;
	}

	printf("Called user function %d with %d args:", funcid, argc);

	for (i = 0; i < argc; i++) {
		printf(" %d", argv[i]);
		ret += argv[i];
	}

	printf(" => %d\n", ret);
	return ret;
}

struct embedvm_s vm = {
	0, 0, 0, NULL,
	&mem_read, &mem_write, &call_user
};

int main(int argc, char **argv)
{
	FILE *f;
	int ch, addr;

	if (argc != 3) {
exit_with_helpmsg:
		fprintf(stderr, "Usage: %s [binfile] [hex-start-addr]\n", argv[0]);
		return 1;
	}

	memset(memory, 0, sizeof(memory));
	f = fopen(argv[1], "rb");
	if (!f)
		goto exit_with_helpmsg;
	for (addr = 0; (ch = fgetc(f)) != -1; addr++)
		memory[addr] = ch;
	fclose(f);

	vm.ip = strtol(argv[2], NULL, 16);

	stop = false;
	while (!stop) {
#if 1
		printf("IP: %04x (%02x %02x %02x %02x),  ", vm.ip,
				memory[vm.ip], memory[vm.ip+1], memory[vm.ip+2], memory[vm.ip+3]);
		printf("SP: %04x (%04x %04x %04x %04x)\n", vm.sp,
				*(uint16_t*)(memory + vm.sp + 0), *(uint16_t*)(memory + vm.sp + 1),
				*(uint16_t*)(memory + vm.sp + 2), *(uint16_t*)(memory + vm.sp + 3));
#endif
		embedvm_exec(&vm);
	}

	return 0;
}

