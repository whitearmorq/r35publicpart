/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __R35_MH0_H
#define __R35_MH0_H

/* Includes ------------------------------------------------------------------*/


double dUAB1[PERIOD_COUNT];

double d3U01noF[PERIOD_COUNT];
double d3U01[PERIOD_COUNT];
double dU21noF[PERIOD_COUNT];
double dU21[PERIOD_COUNT];
double dUL1[PERIOD_COUNT];

double dHalfRMS_UAB1[2];
double dRMS_UAB1;

double dHalfRMS_3U01[2];
double dRMS_3U01;

double dHalfRMS_U21[2];
double dRMS_U21;

double dHalfRMS_UL1[2];
double dRMS_UL1;

double dHalfRMS_IA1[2]; // Для Ver.1 -> U30
double dRMS_IA1;

double dHalfRMS_IB1[2]; // Для Ver.1 -> IA1
double dRMS_IB1;

double dHalfRMS_IC1[2]; // Для Ver.1 -> IC1
double dRMS_IC1;
///////////////////////////////////////////////////////////
double dUAB2[PERIOD_COUNT];

double d3U02noF[PERIOD_COUNT];
double d3U02[PERIOD_COUNT];
double dU22noF[PERIOD_COUNT];
double dU22[PERIOD_COUNT];
double dUL2[PERIOD_COUNT];

double dHalfRMS_UAB2[2]; // Для Ver.1 -> IA2
double dRMS_UAB2;

double dHalfRMS_3U02[2]; // Для Ver.1 -> IC2
double dRMS_3U02;

double dHalfRMS_U22[2]; // Для Ver.1 -> IA3
double dRMS_U22;

double dHalfRMS_UL2[2]; // Для Ver.1 -> IC3
double dRMS_UL2;

double dHalfRMS_IA2[2]; // Для Ver.1 -> IA4
double dRMS_IA2;

double dHalfRMS_IB2[2]; // Для Ver.1 -> IC4
double dRMS_IB2;

double dHalfRMS_IC2[2];
double dRMS_IC2;


#endif /* __R35_MH0_H */

