#ifndef NAS_BOARD
#define NAS_BOARD
#include "base.h"
enum{NR=4,NC=3};typedef struct{int r,c,totalR,totalC,nr,nc,okU,okS,okLive,nlive;W summary[640],live[420],user[NC][21],share[NR][61],path[NR][261];signed char mode[NR][NC];} Board;
void readboard(Board*,int,int);
#endif
