/** @file
  Universal Script File protocol for MicroPython on UEFI.

Copyright (c) 2018-2021, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
// x64 specific stuff

#include "py/mpconfig.h"

#if MICROPY_EMIT_X64

// This is defined so that the assembler exports generic assembler API macros
#define GENERIC_ASM_API (1)
#include "asmx64.h"

// Word indices of REG_LOCAL_x in nlr_buf_t
#define NLR_BUF_IDX_LOCAL_1 (5) // rbx
#define NLR_BUF_IDX_LOCAL_2 (6) // r12
#define NLR_BUF_IDX_LOCAL_3 (7) // r13

#define N_X64 (1)
#define EXPORT_FUN(name) emit_native_x64_##name
#include "py/emitnative.c"

#endif
