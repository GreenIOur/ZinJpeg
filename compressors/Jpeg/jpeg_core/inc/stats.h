#ifndef _STATS_H_
#define _STATS_H_

#ifdef __cplusplus
extern "C" {
#endif

void OverheadData(unsigned length);
void NoOverheadData(unsigned length);
unsigned GetOverheadStats();
unsigned GetNoOverheadStats();
void ResetStats();

#ifdef __cplusplus
}
#endif

#endif
