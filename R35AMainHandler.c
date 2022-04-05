#include "R35Aclient.h"
#include "R35AMainHandler.h"

int posChannel[2][12] = {
	{
	2,	//0-PA0		POS_UC1		POS_UA0
	3,	//1-PA1		POS_UA2		POS_UB0
	0,	//2-PA2		POS_UA1		POS_IA1
	5,	//3-PA3		POS_UB1		POS_IC1
	4,	//4-PA6		POS_IB1		POS_IC2
	13,	//5-PA7		POS_IA1		POS_IA2
	1,	//6-PB0		POS_IC2		POS_IC4
	10,	//7-PB1		POS_IB2		POS_IA4
	11,	//10-PC2	POS_UB2		POS_UC0
	12,	//11-PC3	POS_UC2		POS_3U0
	7,	//12-PC4	POS_IA2		POS_IC3
	6	//13-PC5	POS_IC1		POS_IA3
	},
	{
	0,//0-PA0	POS_UC1		POS_UA0
	1,	//1-PA1		POS_UA2		POS_UB0
	10,	//10-PC2	POS_UA1		POS_IA1
	11,	//11-PC3	POS_UB1		POS_IC1
	2,	//2-PA2		POS_IB1		POS_IC2
	3,	//3-PA3		POS_IA1		POS_IA2
	5,	//5-PA7		POS_IC2		POS_IC4
	4,	//4-PA6		POS_IB2		POS_IA4
	13,	//13-PC5	POS_UB2		POS_UC0
	12,	//12-PC4	POS_UC2		POS_3U0
	7,	//7-PB1		POS_IA2		POS_IC3
	6,	//6-PB0		POS_IC1		POS_IA3
	}
};

void MainHandler0(void)
{
	int i, j;
	enum posValues
	{
		VPOS_UA1 = 0,
		VPOS_UB1, VPOS_UC1, VPOS_IA1, VPOS_IB1, VPOS_IC1,
		VPOS_UA2, VPOS_UB2, VPOS_UC2, VPOS_IA2, VPOS_IB2, VPOS_IC2,
	};

	memcpy(dUAB1, dUAB1 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(d3U01noF, d3U01noF + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(d3U01, d3U01 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(dU21noF, dU21noF + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(dU21, dU21 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(dUL1, dUL1 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));

	memcpy(dUAB2, dUAB2 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(d3U02noF, d3U02noF + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(d3U02, d3U02 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(dU22noF, dU22noF + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(dU22, dU22 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(dUL2, dUL2 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));


	dHalfRMS_UAB1[0] = dHalfRMS_UAB1[1];
	dHalfRMS_3U01[0] = dHalfRMS_3U01[1];
	dHalfRMS_U21[0] = dHalfRMS_U21[1];
	dHalfRMS_UL1[0] = dHalfRMS_UL1[1];
	dHalfRMS_IA1[0] = dHalfRMS_IA1[1];
	dHalfRMS_IB1[0] = dHalfRMS_IB1[1];
	dHalfRMS_IC1[0] = dHalfRMS_IC1[1];

	dHalfRMS_UAB2[0] = dHalfRMS_UAB2[1];
	dHalfRMS_3U02[0] = dHalfRMS_3U02[1];
	dHalfRMS_U22[0] = dHalfRMS_U22[1];
	dHalfRMS_UL2[0] = dHalfRMS_UL2[1];
	dHalfRMS_IA2[0] = dHalfRMS_IA2[1];
	dHalfRMS_IB2[0] = dHalfRMS_IB2[1];
	dHalfRMS_IC2[0] = dHalfRMS_IC2[1];


	
	dHalfRMS_UAB1[1] = dHalfRMS_3U01[1] = dHalfRMS_U21[1] = dHalfRMS_UL1[1] = 0.0;
	dHalfRMS_IA1[1] = dHalfRMS_IB1[1] = dHalfRMS_IC1[1] = 0.0;
	dHalfRMS_UAB2[1] = dHalfRMS_3U02[1] = dHalfRMS_U22[1] = dHalfRMS_UL2[1] = 0.0;
	dHalfRMS_IA2[1] = dHalfRMS_IB2[1] = dHalfRMS_IC2[1] = 0.0;


	for(i=HALF_PERIOD_COUNT,j=HALF_PERIOD_COUNT*(START_SKIP_COUNT-1);i<PERIOD_COUNT;i++,j++)
	{	
		dUAB1[i] = dAC[VPOS_UA1][j] - dAC[VPOS_UB1][l];
		d3U01noF[i] = dAC[VPOS_UA1][j] + dAC[VPOS_UB1][j] + dAC[VPOS_UC1][j];
		dU21noF[i] = dUAB1[i] + dAC[VPOS_UB1][j-6]-dAC[VPOS_UC1][j-6];

		dUAB2[i] = dAC[VPOS_UA2][j] - dAC[VPOS_UB2][j];
		d3U02noF[i] = dAC[VPOS_UA2][j] + dAC[VPOS_UB2][j] + dAC[VPOS_UC2][j];
		dU22noF[i] = dUAB2[i] + dAC[VPOS_UB2][j-6] - dAC[VPOS_UC2][j-6];

		//Режекторный фильтр
		//d3U0
		d3U01[i] =	d3U01noF[i];/*0.7575 * d3U01noF[i]
					-0.289137 * d3U01noF[i-1] 
					-0.262539 * d3U01noF[i-2]
					+0.7477776 * d3U01noF[i-3]
					-0.262539 * d3U01noF[i-4]
					-0.289137 * d3U01noF[i-5]
					+0.7575 * d3U01noF[i-6];*/

		d3U02[i] =	d3U02noF[i];/*0.7575 * d3U02noF[i]
					-0.289137 * d3U02noF[i-1] 
					-0.262539 * d3U02noF[i-2]
					+0.7477776 * d3U02noF[i-3]
					-0.262539 * d3U02noF[i-4]
					-0.289137 * d3U02noF[i-5]
					+0.7575 * d3U02noF[i-6];*/
		//dU2
		dU21[i] =	dU21noF[i];/*0.7575 * dU21noF[i]
					-0.289137 * dU21noF[i-1] 
					-0.262539 * dU21noF[i-2]
					+0.7477776 * dU21noF[i-3]
					-0.262539 * dU21noF[i-4]
					-0.289137 * dU21noF[i-5]
					+0.7575 * dU21noF[i-6];*/

		dU22[i] =	dU22noF[i];/*0.7575 * dU22noF[i]
					-0.289137 * dU22noF[i-1] 
					-0.262539 * dU22noF[i-2]
					+0.7477776 * dU22noF[i-3]
					-0.262539 * dU22noF[i-4]
					-0.289137 * dU22noF[i-5]
					+0.7575 * dU22noF[i-6];*/
					
		//Полосовой фильтр
		dUL1[i] = 0.093188882174 * dUAB1[i]
					-0.2036787007 * dUAB1[i-1]
					-0.087916542295 * dUAB1[i-2]
					+0.394469459646 * dUAB1[i-3]
					-0.087916542295 * dUAB1[i-4]
					-0.2036787007 * dUAB1[i-5]
					+0.093188882174 * dUAB1[i-6];
					
		dUL2[i] = 0.093188882174 * dUAB2[i]
					-0.2036787007 * dUAB2[i-1]
					-0.087916542295 * dUAB2[i-2]
					+0.394469459646 * dUAB2[i-3]
					-0.087916542295 * dUAB2[i-4]
					-0.2036787007 * dUAB2[i-5]
					+0.093188882174 * dUAB2[i-6];

		dHalfRMS_UAB1[1] += dUAB1[i] * dUAB1[i];
		dHalfRMS_3U01[1] += d3U01[i] * d3U01[i];
		dHalfRMS_U21[1] += dU21[i] * dU21[i];
		dHalfRMS_UL1[1] += dUL1[i] * dUL1[i] ;
		dHalfRMS_IA1[1] += dAC[VPOS_IA1][j] * dAC[VPOS_IA1][j];
		dHalfRMS_IB1[1] += dAC[VPOS_IB1][j] * dAC[VPOS_IB1][j];
		dHalfRMS_IC1[1] += dAC[VPOS_IC1][j] * dAC[VPOS_IC1][j];

		dHalfRMS_UAB2[1] += dUAB2[i] * dUAB2[i];
		dHalfRMS_3U02[1] += d3U02[i] * d3U02[i];
		dHalfRMS_U22[1] += dU22[i] * dU22[i];
		dHalfRMS_UL2[1] += dUL2[i] * dUL2[i] ;
		dHalfRMS_IA2[1] += dAC[VPOS_IA2][j] * dAC[VPOS_IA2][j];
		dHalfRMS_IB2[1] += dAC[VPOS_IB2][j] * dAC[VPOS_IB2][j];
		dHalfRMS_IC2[1] += dAC[VPOS_IC2][j] * dAC[VPOS_IC2][j];
	}

	dRMS_UAB1 = sqrt(dHalfRMS_UAB1[0] + dHalfRMS_UAB1[1]) / 6.0;
	dRMS_3U01 = sqrt(dHalfRMS_3U01[0] + dHalfRMS_3U01[1]) / 6.0;
	dRMS_3U01 /= 1.732;
	dRMS_U21 = sqrt(dHalfRMS_U21[0] + dHalfRMS_U21[1]) / 6.0;
	dRMS_U21 /= 1.732; 
	dRMS_UL1 = sqrt(dHalfRMS_UL1[0] + dHalfRMS_UL1[1]) / 6.0;
	dRMS_IA1 = sqrt(dHalfRMS_IA1[0] + dHalfRMS_IA1[1]) / 6.0;
	dRMS_IB1 = sqrt(dHalfRMS_IB1[0] + dHalfRMS_IB1[1]) / 6.0;
	dRMS_IC1 = sqrt(dHalfRMS_IC1[0] + dHalfRMS_IC1[1]) / 6.0;

	dRMS_UAB2 = sqrt(dHalfRMS_UAB2[0] + dHalfRMS_UAB2[1]) / 6.0;
	dRMS_3U02 = sqrt(dHalfRMS_3U02[0] + dHalfRMS_3U02[1]) / 6.0;
	dRMS_3U02 /= 1.732;
	dRMS_U22 = sqrt(dHalfRMS_U22[0] + dHalfRMS_U22[1]) / 6.0;
	dRMS_U22 /= 1.732; 
	dRMS_UL2 = sqrt(dHalfRMS_UL2[0] + dHalfRMS_UL2[1]) / 6.0;
	dRMS_IA2 = sqrt(dHalfRMS_IA2[0] + dHalfRMS_IA2[1]) / 6.0;
	dRMS_IB2 = sqrt(dHalfRMS_IB2[0] + dHalfRMS_IB2[1]) / 6.0;
	dRMS_IC2 = sqrt(dHalfRMS_IC2[0] + dHalfRMS_IC2[1]) / 6.0;
}

void MainHandler1(void)
{
	int i, j;
	enum posValues
	{
		VPOS_UA0 = 0,
		VPOS_UB0, VPOS_UC0, VPOS_U30, VPOS_IA1, VPOS_IC1,
		VPOS_IA2, VPOS_IC2, VPOS_IA3, VPOS_IC3, VPOS_IA4, VPOS_IC4,
	};

	memcpy(dUAB1, dUAB1 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(d3U01noF, d3U01noF + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(d3U01, d3U01 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(dU21noF, dU21noF + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(dU21, dU21 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));
	memcpy(dUL1, dUL1 + HALF_PERIOD_COUNT, HALF_PERIOD_COUNT * sizeof(double));

	dHalfRMS_UAB1[0] = dHalfRMS_UAB1[1];
	dHalfRMS_3U01[0] = dHalfRMS_3U01[1];
	dHalfRMS_U21[0] = dHalfRMS_U21[1];
	dHalfRMS_UL1[0] = dHalfRMS_UL1[1];
	dHalfRMS_IA1[0] = dHalfRMS_IA1[1];// Для Ver.1 -> U30
	dHalfRMS_IB1[0] = dHalfRMS_IB1[1];// Для Ver.1 -> IA1
	dHalfRMS_IC1[0] = dHalfRMS_IC1[1];// Для Ver.1 -> IC1

	dHalfRMS_UAB2[0] = dHalfRMS_UAB2[1];// Для Ver.1 -> IA2
	dHalfRMS_3U02[0] = dHalfRMS_3U02[1];// Для Ver.1 -> IC2
	dHalfRMS_U22[0] = dHalfRMS_U22[1];// Для Ver.1 -> IA3
	dHalfRMS_UL2[0] = dHalfRMS_UL2[1];// Для Ver.1 -> IC3
	dHalfRMS_IA2[0] = dHalfRMS_IA2[1];// Для Ver.1 -> IA4
	dHalfRMS_IB2[0] = dHalfRMS_IB2[1];// Для Ver.1 -> IC4


	dHalfRMS_UAB1[1] = dHalfRMS_3U01[1] = dHalfRMS_U21[1] = dHalfRMS_UL1[1] = 0.0;
	dHalfRMS_IA1[1] = dHalfRMS_IB1[1] = dHalfRMS_IC1[1] = 0.0;
	dHalfRMS_UAB2[1] = dHalfRMS_3U02[1] = dHalfRMS_U22[1] = dHalfRMS_UL2[1] = 0.0;
	dHalfRMS_IA2[1] = dHalfRMS_IB2[1] = 0.0;

	for(i=HALF_PERIOD_COUNT,j=HALF_PERIOD_COUNT*(START_SKIP_COUNT-1);i<PERIOD_COUNT;i++,j++)
	{	
		dUAB1[i] = dAC[VPOS_UA0][j] - dAC[VPOS_UB0][j];
		d3U01noF[i] = dAC[VPOS_UA0][j] + dAC[VPOS_UB0][j] + dAC[VPOS_UC0][j];
		dU21noF[i] = dUAB1[i] + dAC[VPOS_UB0][j-6]-dAC[VPOS_UC0][j-6];

		//Режекторный фильтр
		//d3U0
		d3U01[i] =	d3U01noF[i];/*0.7575 * d3U01noF[i]
					-0.289137 * d3U01noF[i-1] 
					-0.262539 * d3U01noF[i-2]
					+0.7477776 * d3U01noF[i-3]
					-0.262539 * d3U01noF[i-4]
					-0.289137 * d3U01noF[i-5]
					+0.7575 * d3U01noF[i-6];*/

		//dU2
		dU21[i] =	dU21noF[i];/*0.7575 * dU21noF[i]
					-0.289137 * dU21noF[i-1] 
					-0.262539 * dU21noF[i-2]
					+0.7477776 * dU21noF[i-3]
					-0.262539 * dU21noF[i-4]
					-0.289137 * dU21noF[i-5]
					+0.7575 * dU21noF[i-6];*/

					
		//Полосовой фильтр
		dUL1[i] = 0.093188882174 * dUAB1[i]
					-0.2036787007 * dUAB1[i-1]
					-0.087916542295 * dUAB1[i-2]
					+0.394469459646 * dUAB1[i-3]
					-0.087916542295 * dUAB1[i-4]
					-0.2036787007 * dUAB1[i-5]
					+0.093188882174 * dUAB1[i-6];
					

		dHalfRMS_UAB1[1] += dUAB1[i] * dUAB1[i];
		dHalfRMS_3U01[1] += d3U01[i] * d3U01[i];
		dHalfRMS_U21[1] += dU21[i] * dU21[i];
		dHalfRMS_UL1[1] += dUL1[i] * dUL1[i] ;
		dHalfRMS_IA1[1] += dAC[VPOS_U30][j] * dAC[VPOS_U30][j];
		dHalfRMS_IB1[1] += dAC[VPOS_IA1][j] * dAC[VPOS_IA1][j];
		dHalfRMS_IC1[1] += dAC[VPOS_IC1][j] * dAC[VPOS_IC1][j];

		dHalfRMS_UAB2[1] += dAC[VPOS_IA2][j] * dAC[VPOS_IA2][j];
		dHalfRMS_3U02[1] += dAC[VPOS_IC2][j] * dAC[VPOS_IC2][j];
		dHalfRMS_U22[1] += dAC[VPOS_IA3][j] * dAC[VPOS_IA3][j];
		dHalfRMS_UL2[1] += dAC[VPOS_IC3][j] * dAC[VPOS_IC3][j];
		dHalfRMS_IA2[1] += dAC[VPOS_IA4][j] * dAC[VPOS_IA4][j];
		dHalfRMS_IB2[1] += dAC[VPOS_IC4][j] * dAC[VPOS_IC4][j];
	}

	dRMS_UAB1 = sqrt(dHalfRMS_UAB1[0] + dHalfRMS_UAB1[1]) / 6.0;
	dRMS_3U01 = sqrt(dHalfRMS_3U01[0] + dHalfRMS_3U01[1]) / 6.0;
	dRMS_3U01 /= 1.732;
	dRMS_U21 = sqrt(dHalfRMS_U21[0] + dHalfRMS_U21[1]) / 6.0;
	dRMS_U21 /= 1.732; 
	dRMS_UL1 = sqrt(dHalfRMS_UL1[0] + dHalfRMS_UL1[1]) / 6.0;
	dRMS_IA1 = sqrt(dHalfRMS_IA1[0] + dHalfRMS_IA1[1]) / 6.0;
	dRMS_IB1 = sqrt(dHalfRMS_IB1[0] + dHalfRMS_IB1[1]) / 6.0;
	dRMS_IC1 = sqrt(dHalfRMS_IC1[0] + dHalfRMS_IC1[1]) / 6.0;

	dRMS_UAB2 = sqrt(dHalfRMS_UAB2[0] + dHalfRMS_UAB2[1]) / 6.0;
	dRMS_3U02 = sqrt(dHalfRMS_3U02[0] + dHalfRMS_3U02[1]) / 6.0;
	dRMS_U22 = sqrt(dHalfRMS_U22[0] + dHalfRMS_U22[1]) / 6.0;
	dRMS_UL2 = sqrt(dHalfRMS_UL2[0] + dHalfRMS_UL2[1]) / 6.0;
	dRMS_IA2 = sqrt(dHalfRMS_IA2[0] + dHalfRMS_IA2[1]) / 6.0;
	dRMS_IB2 = sqrt(dHalfRMS_IB2[0] + dHalfRMS_IB2[1]) / 6.0;
}
