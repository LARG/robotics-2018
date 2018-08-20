#pragma once

#define SET_BIT(target,value,position) target ^= (-value ^ target) & (1 << position);
#define GET_BIT(target,position) target & (1 << position)
