#pragma once

#define READ_REG(addr) (*(uint32_t*)(addr))
#define WRITE_REG(addr, val) (*(uint32_t*)(addr)) = (val)
#define SET_REG(addr, val) (*(uint32_t*)(addr)) |= (val)
#define RESET_REG(addr, val) (*(uint32_t*)(addr)) &= (~val)