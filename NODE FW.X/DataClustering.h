/*****************************************************************************
 *
 *              DataClustering.h -- DataClustering v0.1
 *
 *****************************************************************************
 *
 * Author:          Manuel Alarcón Granero
 * FileName:        DataClustering.h
 * Dependencies:
 * Processor:
 * BOARD:
 * Compiler:        C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         B105 -FAILURE IS NOT AN OPTION-
 *
 *****************************************************************************
 * File Description:
 *
 *
 *
 * Change History:
 *  Rev   Date(m/d/y)   	      Description
 *  0.1   03/11/2015                  Primer intento
 *****************************************************************************/

#include "CWSN LSI Node/Include/NodeHAL.h"
#include "CWSN LSI Node/Include/WirelessProtocols/Console.h"
#include <math.h>

#ifndef DATACLUSTERING_H
#define	DATACLUSTERING_H

#define MAX_PAQ_APRENDIZAJE 100
#define MAX_CLUSTERS        10

typedef struct coordenadas {
    BYTE RSSI;
    long tiempo;
} coord;

typedef struct cluster {
    coord centro;
    double radio;
    int nMuestras;
} cl;

coord Lista_Paq_Rec_Aprendizaje[MAX_PAQ_APRENDIZAJE];
cl Lista_Clusters[MAX_CLUSTERS];

void DataClustering(void);
coord CalculoCoordenadas();
void NormalizarCoordenadas();
void CalculoClusters();
double CalculoDistancia(coord pto1, coord pto2);

#endif	/* DATACLUSTERING_H */

