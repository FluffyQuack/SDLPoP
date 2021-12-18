#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void CreateConsole();
void DeleteConsole();
void PrintToConsole(const char *text, ...);

#ifdef __cplusplus
}
#endif