/*****************************************************************************
 *
 *              DataClustering.c -- DataClustering v0.1
 *
 *****************************************************************************
 *
 * Author:          malarcon
 * FileName:        DataClustering.c
 * Dependencies:    DataClustering.h
 * Processor:
 * BOARD:
 * Compiler:        C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         B105 -FAILURE IS NOT AN OPTION-
 *
 *****************************************************************************
 * File Description: Archivo donde se implementa la estrategia de data
 * clustering.
 *
 *
 * Change History:
 *  Rev   Date(m/d/y)   	Description
 *****************************************************************************/

#include "CWSN LSI Node/Include/NodeHAL.h"
#include "Consumo.h"
#include "CRModule/Optimizer/ConfigOptimizer.h"
#include "DataClustering.h"

BYTE *pVectorPotencias = &vectorPotencias[0];

msgChng datosPrueba;
BOOL cambioCanal = FALSE;
BYTE canal = 0;
BOOL recPaqCambioCanal = FALSE;
/*
void Consumo(void){

    Recibir_info();
    if(IncluirPotencia(pVectorPotencias) == TRUE && CalcularMedia(pVectorPotencias) < UMBRAL_POTENCIA){
        cambioCanal = CalcularCostes(); //Calcular los costes y poner lo que devuelva el metodo en cambioCanal.
    }

    if(cambioCanal == FALSE && recPaqCambioCanal == FALSE){
        //Si decidimos no cambiar
    } else if(cambioCanal == FALSE && recPaqCambioCanal == TRUE){
        //No se ha decidido cambiar el canal todavia y otro nodo ha decidido cambiar
    } else if(cambioCanal == TRUE && recPaqCambioCanal == FALSE){
        //Se decide cambiar de canal y es el primero o se está esperando a que todos los nodos transmitan su respuesta
    } else {
        //Se ha decidido cambiar de canal y uno de los otros nodos ha transmitido su respuesta
    }

    //Prueba CanalOptimo
    msgChng *datos = &datosPrueba;
    CanalOptimo(MIWI_2400,datos);

}

BOOL IncluirPotencia(BYTE *pVector){

    radioInterface ri = MIWI_2400;
    BYTE RSSI,i;
    BYTE *pRSSI = &RSSI;
    if(GetRSSI(ri,pRSSI) == 0){
        for(i = MAX_VECTOR_POTENCIA-1; i >= 0; i--){
            if(i > 0){
                *(pVector+i) = *(pVector+i-1);
            } else {
                *(pVector) = *(pRSSI);
            }
        }
        return TRUE;
    }
    return FALSE;
}




BOOL CanalOptimo(radioInterface ri, msgChng *datos){

    BYTE i;
    BYTE RSSI_ch_opt;
    BYTE *pRSSI_ch_opt = &RSSI_ch_opt;
    switch(ri){
        case MIWI_0434:
            (*datos).chOpt = DoChannelScanning(ri, pRSSI_ch_opt);
            for (i = 0; i < MIWI0434NumChannels; i++){
                GetScanResult(ri,i,&(*datos).potCanales[i]);
            }
            break;
        case MIWI_0868:
            (*datos).chOpt = DoChannelScanning(ri, pRSSI_ch_opt);
            for (i = 0; i < MIWI0868NumChannels; i++){
                GetScanResult(ri,i,&(*datos).potCanales[i]);
            }
            break;
        case MIWI_2400:
            (*datos).chOpt = DoChannelScanning(ri, pRSSI_ch_opt);
            for (i = 0; i < MIWI2400NumChannels; i++){
                GetScanResult(ri,i,&(*datos).potCanales[i]);
            }
            break;
        default:
            break;
    }
    return TRUE;

}
*/
/*
BOOL TransmitirDecision(msgChng *datos){

    radioInterface ri = MIWI_2400;

    BYTE i, j;
    i = 0;
    EnviandoMssgApp = TRUE;
    j = PutTXData(ri, 0x12);
    if (j) {
        Printf("\r\nFallo al escribir en el buffer. Codigo de error: ");
        PrintChar(j);
    }
    j = PutTXData(ri, (*datos).chOpt);
    if (j) {
        Printf("\r\nFallo al escribir en el buffer. Codigo de error: ");
        PrintChar(j);
    }
    while(i < sizeof((*datos).potCanales)) {
        j = PutTXData(ri, (*datos).potCanales[i]);
        if (j) {
            Printf("\r\nFallo al escribir en el buffer. Codigo de error: ");
            PrintChar(j);
        } else {
            i++;
        }
    }
    i = SendPckt(ri, BROADCAST_ADDRMODE, NULL);
    Printf("\r\nDecisión enviada al resto de nodos");
    if (i == 0) {
        Printf(" => OK");
    } else {
        Printf(" => FALLO: ");
        PrintChar(i);
    }
    EnviandoMssgApp = FALSE;

}*/