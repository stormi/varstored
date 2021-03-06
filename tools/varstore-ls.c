/*
 * Copyright (c) Citrix Systems, Inc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <backend.h>
#include <debug.h>
#include <depriv.h>
#include <serialize.h>

#include "tool-lib.h"

const struct backend *db = &xapidb_cmdline;
const enum log_level log_level = LOG_LVL_INFO;

static void
usage(const char *progname)
{
    printf("usage: %s [-h] [depriv options] <vm-uuid>\n\n", progname);
    printf("Lists the VM's EFI variables.\n");
    print_depriv_options();
}

static void
print_guid(const EFI_GUID *guid)
{
    printf("%02x", guid->data[3]);
    printf("%02x", guid->data[2]);
    printf("%02x", guid->data[1]);
    printf("%02x", guid->data[0]);
    printf("-");
    printf("%02x", guid->data[5]);
    printf("%02x", guid->data[4]);
    printf("-");
    printf("%02x", guid->data[7]);
    printf("%02x", guid->data[6]);
    printf("-");
    printf("%02x", guid->data[8]);
    printf("%02x", guid->data[9]);
    printf("-");
    printf("%02x", guid->data[10]);
    printf("%02x", guid->data[11]);
    printf("%02x", guid->data[12]);
    printf("%02x", guid->data[13]);
    printf("%02x", guid->data[14]);
    printf("%02x", guid->data[15]);
}

static bool
do_ls(void)
{
    uint8_t buf[SHMEM_SIZE];
    uint8_t name[NAME_LIMIT] = {0};
    uint8_t *ptr;
    EFI_GUID guid = {{0}};
    UINTN size = 0;
    int i;
    EFI_STATUS status;

    for (;;) {
        ptr = buf;
        serialize_uint32(&ptr, 1); /* version */
        serialize_uint32(&ptr, COMMAND_GET_NEXT_VARIABLE);
        serialize_uintn(&ptr, NAME_LIMIT);
        serialize_data(&ptr, name, size);
        serialize_guid(&ptr, &guid);
        *ptr = 0;

        dispatch_command(buf);

        ptr = buf;
        status = unserialize_uintn(&ptr);
        if (status == EFI_NOT_FOUND)
            break;
        if (status != EFI_SUCCESS) {
            print_efi_error(status);
            return false;
        }

        size = unserialize_uintn(&ptr);
        memcpy(name, ptr, size);
        ptr += size;
        unserialize_guid(&ptr, &guid);

        print_guid(&guid);
        printf(" ");
        /* Only supports printing a limited subset of UTF-16 for now. */
        for (i = 0; i < size; i += 2) {
            if (isprint(name[i]))
                printf("%c", (char)name[i]);
        }
        printf("\n");
    }

    return true;
}

int main(int argc, char **argv)
{
    DEPRIV_VARS

    for (;;) {
        int c = getopt(argc, argv, "h" DEPRIV_OPTS);

        if (c == -1)
            break;

        switch (c) {
        DEPRIV_CASES
        case 'h':
            usage(argv[0]);
            exit(0);
        default:
            usage(argv[0]);
            exit(1);
        }
    }

    if (argc - optind != 1) {
        usage(argv[0]);
        exit(1);
    }

    db->parse_arg("uuid", argv[optind]);

    if (opt_socket)
        db->parse_arg("socket", opt_socket);

    if (!drop_privileges(opt_chroot, opt_depriv, opt_gid, opt_uid))
        exit(1);

    if (!tool_init())
        exit(1);

    return !do_ls();
}
