/*
This file is part of INDDGO.

Copyright (C) 2012, Oak Ridge National Laboratory 

This product includes software produced by UT-Battelle, LLC under Contract No. 
DE-AC05-00OR22725 with the Department of Energy. 

This program is free software; you can redistribute it and/or modify
it under the terms of the New BSD 3-clause software license (LICENSE). 

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
LICENSE for more details.

For more information please contact the INDDGO developers at: 
inddgo-info@googlegroups.com

*/

#define MODULUS 2147483647
#define MULT 16807
#include "GraphUtil.h"
#include "RndNumGen.h"
#include <math.h>

// A collection of random seeds from Mathematica for the LCG -

static long long zrng[] =
{ 1585898122, 977263903, 1405617513, 992686252, 18121536, 2063068841, 754752318,
41984681, 105083035, 139298838, 185623219, 949931193, 844017127,
220735746, 2086518219, 1159920248, 223790403, 521639770, 383329278,
1351669096, 546508335, 241074248, 1250430659, 1360027002, 2133008156,
801417619, 234137012, 820590974, 208037679, 762045751, 1878813713,
238884113, 1183679960, 1043965154, 976700693, 2008340726, 677399582,
1285359843, 1138040391, 468728879, 1173244317, 235237876, 1232346133,
471001204, 1863659670, 1165359604, 1863730245, 1039575367, 33196368,
447728821, 803148007, 732593271, 1682165068, 1423782112, 2088550953,
936772830, 1669565296, 1410105093, 1883854503, 157181746, 237269108,
530564635, 448848243, 286675281, 363322466, 861763630, 825225764,
1406888526, 1616456635, 916354297, 1567694919, 1200774035, 81218620,
54228387, 964954038, 392387524, 1451979814, 2122793904, 1202721819,
628095876, 1098922114, 95833991, 1140244378, 2013076662, 1482101325,
815732936, 154659849, 1575364612, 1969394111, 2057580375, 2015360901,
1435017398, 1311586731, 314500068, 162190131, 1309049839, 1760010706,
827285638, 554810512, 942750612, 1888673353, 1658410347, 1535796884,
610141401, 1095089179, 473990272, 1591707931, 1650890892, 2129059111,
409806116, 739034658, 1074994661, 1150159741, 269214387, 1516429880,
1369667127, 2122951567, 1536355824, 1947835842, 823114858, 2001677049,
243498667, 438835589, 2084577438, 1778854242, 1589215500, 993105500,
1399779082, 680250157, 987051556, 1597580442, 71887978, 974657481,
665567351, 689482979, 1947413384, 1450689852, 1817370782, 1249183224,
94259407, 1576903163, 194915665, 1059712692, 453727562, 2124585319,
1602930620, 1142904759, 965673653, 246108792, 1654947015, 1878828492,
567267685, 878355741, 881707113, 580077084, 901920333, 518339521,
1010914174, 1742574627, 1097862854, 1000362984, 1022589700, 821089113,
2040587093, 350389248, 322891321, 1159453516, 614435100, 485423061,
580074455, 1038514233, 582690813, 1394060287, 1193771124, 2030353361,
1331961065, 1091003721, 480683205, 172421798, 1314856444, 1020643186,
161954324, 1198718865, 1108252489, 696760670, 1050697945, 871635855,
485876747, 179770165, 2110038934, 1877118219, 1944112804, 889651228,
1709232582, 1021461548, 1062416269, 1775284287, 1548647752, 1468622131,
1501559878, 1641100862, 2043157460, 9225604, 516609908, 1918652536,
890178664, 2139455918, 723888229, 380523573, 998128782, 2122099165,
193945359, 1163177060, 496833754, 1752971070, 842393719, 651771348,
357600681, 448174576, 1539044660, 1204659744, 526302849, 1683442778,
2055374782, 2096711886, 1847065567, 2103775300, 1867545195, 180757560,
756099417, 2122354186, 911394104, 377306975, 1060803589, 1558009924,
1147998809, 30263572, 545721691, 1043063797, 982875671, 170326373,
1003183201, 6932742, 567081165, 1339773006, 668884182, 469588686,
1338125418, 1959613029, 1576328955, 1873529200, 501967489, 85306291,
1735183136, 2001471533, 960275626, 130336187, 1334655399, 1331594739,
345982965, 590996008, 1535438073, 1316368256, 401559792, 122512210,
1415929389, 1011799313, 1869058634, 1064463722, 1447264924, 474372764,
134983996, 1166979876, 2140794736, 1078919141, 564009419, 1273245050,
212505753, 650918775, 1716276860, 1008717539, 1491859263, 2098344229,
1700342627, 2098195790, 702806311, 430088402, 1841775273, 873208521,
2015656192, 1659123069, 935557457, 1250857761, 206689908, 1035486424,
410559828, 883746510, 1317520915, 1194730483, 121224379 };

#if TD_ADD_ENTROPY
static int initialized=0;
#endif

namespace Graph
{
	double lcgrand(int stream)
	{
		///
		/// A run of the mill LCG. Returns a uniformly distributed double in (0,1).
		///

		long long zi;
#if TD_ADD_ENTROPY
		// Add a time component to the RNG to get different
		// results on different runs.

		if(!initialized)
		{
			time_t now;
			time(&now);
			srand((unsigned int)now);
			zrng[stream]+=rand();

			initialized=1;

		}
#endif

		zi = zrng[stream];

		zrng[stream] = (MULT * zi) % MODULUS;
		//Update the state of the RNG

		return ((double) zrng[stream] / MODULUS);
	}

	void random_permutation(int *perm, int n)
	{
		///
		/// Randomly permutes the perm array of size n.  Assumes
		/// that perm[] is already filled with the elements to be
		/// permuted.
		///

		int r, len, temp;

		len = n - 1;

		while (len >= 0)
		{
			r = rand_int(0, n - 1); //between 0 and n-1 inclusive
			if (r < 0)
				r = 0;
			if (r > n - 1)
				r = n - 1;
			temp = perm[len];
			perm[len] = perm[r];
			perm[r] = temp;
			len--;
		}

		return;
	}


	/**
	* Function to generate a random integer in {min,min+1,...,max-1,max}
	*/
	int rand_int(int min, int max)
	{
		return min + (int) floor((double) (max - min + 1) * lcgrand(0));
	}

}
