/*****************************************************************************
 *
 *              DataClustering.h -- DataClustering v0.1
 *
 *****************************************************************************
 *
 * Author:          malarcon
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
 *  0.1   03/11/2015                  Primera versión
 *****************************************************************************/

#include "CWSN LSI Node/Include/NodeHAL.h"
#include "CWSN LSI Node/Include/WirelessProtocols/Console.h"
#include "CWSN LSI Node/Include/WirelessProtocols/ConfigApp.h"

#include "CRModule/CRModule.h"
#include "CRModule/VCC/VCC.h"
#include <math.h>

#ifndef DATACLUSTERING_H
#define	DATACLUSTERING_H

//Cambiar según necesidades


void DataClustering(void);
coord CalculoCoordenadas();
void NormalizarCoordenadas();
void CalculoClusters();
void inicializarTablaAtacantes();
double CalculoDistancia(coord pto1, coord pto2);
BYTE* hayAtacante();
void Envio_Datos_Atacantes(at data);
void Recibir_info(void);

#endif	/* DATACLUSTERING_H */

