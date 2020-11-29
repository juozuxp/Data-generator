#pragma once
#define ERROR_PRINTF(Error, ...) {printf(Error, __VA_ARGS__); system("pause"); exit(0);}