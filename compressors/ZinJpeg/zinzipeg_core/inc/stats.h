#ifndef _STATS_H_
#define _STATS_H_


void OverheadData(unsigned length);
void NoOverheadData(unsigned length);
unsigned GetOverheadStats();
float GetNoOverheadStats();
void ResetStats();

#endif
