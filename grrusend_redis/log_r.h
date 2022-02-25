#ifndef __LOG_R_H
#define __LOG_R_H

VOID PrintErrorLogR(PCSTR pszDebugStr, PCSTR pszFormatStr, ...);
VOID PrintDebugLogR(PCSTR pszDebugStr, PCSTR pszFormatStr, ...);
VOID PrintHexDebugLogR(PCSTR pszDebugStr, PCSTR pszPrintBuf, UINT nPrintLen);

#endif
