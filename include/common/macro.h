#pragma once

#define READ_REG(addr) (*(uint32_t*)(addr))
#define WRITE_REG(addr, val) (*(uint32_t*)(addr)) = (val)
#define S_WRITE_REG(addr, val) (*(uint32_t*)(addr)) |= (val)
#define R_WRITE_REG(addr, val) (*(uint32_t*)(addr)) &= (~val)