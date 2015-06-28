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

#include "DataClustering.h"
/*


void DataClustering(void){
    BYTE i;
    if (aprendizaje == 0){
        coord paquete = CalculoCoordenadas();
        if(paquete.RSSI != 0){
            if (paquetesRecibidos == 0){
                Lista_Paq_Rec_Aprendizaje[paquetesRecibidos] = paquete;
                paquetesRecibidos++;
            } else {
                if(paquetesRecibidos < MAX_PAQ_APRENDIZAJE){
                    Lista_Paq_Rec_Aprendizaje[paquetesRecibidos] = paquete;
                    paquetesRecibidos++;
                } else {
                    //Se ha excedido el número máximo de paquetes permitidos
                }
            }
        } else {
            //No hace nada
        }
    } else {
        if(normalizado == 0) {
            NormalizarCoordenadas();
        } else if (normalizado == 1 && clustersDone == 0) {
            CalculoClusters();
        } else {
            if (learningTime == 0xEFFFFFFF){
                learningTime = 0;
            }
            pAttackerAddr = hayAtacante();
            if (pAttackerAddr != NULL) {
                //Guardar en una tabla quien es el atacante y mandar mensajes de quien es el atacante.
                BYTE dirAtt[MY_ADDRESS_LENGTH], dirDet[MY_ADDRESS_LENGTH], a;
                for (i = 0; i < MY_ADDRESS_LENGTH; i++){
                    dirAtt[i] = *(pAttackerAddr + i);
                    dirDet[i] = GetMyLongAddress(i);
                }
                for (i = 0; i < CONNECTION_SIZE*CONNECTION_SIZE; i++){
                    if (Tabla_Atacantes[i].direccionAtacante == dirAtt && Tabla_Atacantes[i].esAtacante == 1){
                        nDetectado++;
                    }
                }
                if (nDetectado >= 2){
                    //Se tiene que pasar de los paquetes de este
                }
                //Guardo en la tabla quién es el atacante
                Tabla_Atacantes[a].esAtacante = 1;
                //Envio_Datos_Atacantes(Tabla_Atacantes[a]);
            }
            //Recibir mensajes del resto de nodos con los atacantes y decidir qué hacer
            Recibir_info();

        }
    }
}

coord CalculoCoordenadas(radioInterface ri){

    pRSSI = &RSSI;
    double t1;
    double tiempoPaquete;
    coord coordenadas;
    if ( paquetesRecibidos == 0 && GetRSSI(ri,pRSSI) == 0x00 ){  //Si es correcto el valor de RSSI y es el primer dato de un nodo
        Printf("\r\nSe ha recibido el primer paquete\r\n");
        inicializarTablaAtacantes();
        tiempoPaquete = learningTime;
        t0 = learningTime; //Para el tiempo del proximo paquete
        tiempoMax = tiempoPaquete;
        potenciaMax = *pRSSI;
        coordenadas.tiempo = tiempoPaquete;
        coordenadas.RSSI = *pRSSI;
        limpiaBufferRX();
        return coordenadas;
    } else {
        if ( paquetesRecibidos > 0 && GetRSSI(ri,pRSSI) == 0x00){
            Printf("\r\nSe han recibido mas paquetes\r\n");
            t1 = learningTime;
            tiempoPaquete = t1 - t0;
            if(tiempoPaquete > tiempoMax) tiempoMax = tiempoPaquete;
            if(*pRSSI > potenciaMax) potenciaMax = *pRSSI;
            t0 = t1;

            coordenadas.tiempo = tiempoPaquete;
            coordenadas.RSSI = (double) *pRSSI;
            limpiaBufferRX();
            return coordenadas;
        } else {
            coordenadas.RSSI = 0;
            return coordenadas;
        }
    }
}

void NormalizarCoordenadas(){
     if(nClusters == 0){
        int i;
        for(i = 0; i < paquetesRecibidos; i++){
            Lista_Paq_Rec_Aprendizaje[i].RSSI = Lista_Paq_Rec_Aprendizaje[i].RSSI/potenciaMax;
            Lista_Paq_Rec_Aprendizaje[i].tiempo = Lista_Paq_Rec_Aprendizaje[i].tiempo/tiempoMax;
        }
        normalizado = 1;
    }
}



void inicializarTablaAtacantes(){
    BYTE i,j,k;
    BYTE TablaDirecciones[CONNECTION_SIZE+1][MY_ADDRESS_LENGTH];

    for (i = 0; i < CONNECTION_SIZE+1; i++){
        if (i < CONNECTION_SIZE && CONNECTION_SIZE != 0){
            for (k = 0; k < MY_ADDRESS_LENGTH; k++){
                TablaDirecciones[i][k] = ConnectionTable[i].Address[k];
            }
        } else {
            for (k = 0; k < MY_ADDRESS_LENGTH; k++){
                TablaDirecciones[i][k] = GetMyLongAddress(k);
            }
        }

    }
    
    for (i = 0; i < CONNECTION_SIZE+1; i++){
        for (j = i*(CONNECTION_SIZE+1); j < (i*(CONNECTION_SIZE+1))+CONNECTION_SIZE+1; j++){
            for (k = 0; k < MY_ADDRESS_LENGTH; k++){
                Tabla_Atacantes[j].direccionAtacante[k] = TablaDirecciones[i][k];
            }
            Tabla_Atacantes[j].esAtacante = 0;
        }
    }

    j = 0;
    for (i = 0; i < (CONNECTION_SIZE+1)*(CONNECTION_SIZE+1); i++){
        if (j < CONNECTION_SIZE+1){
            for (k = 0; k < MY_ADDRESS_LENGTH; k++){
                Tabla_Atacantes[i].direccionDetector[k] = TablaDirecciones[j][k];
            }
            j++;
        } else if (j == CONNECTION_SIZE+1){
            j = 0;
            for (k = 0; k < MY_ADDRESS_LENGTH; k++){
                Tabla_Atacantes[i].direccionDetector[k] = TablaDirecciones[j][k];
            }
            j++;
        }
    }
}

void CalculoClusters(){

    int i,j;
    for(i = 0; i < paquetesRecibidos; i++){
        if(nClusters == 0){
            Lista_Clusters[0].centro = Lista_Paq_Rec_Aprendizaje[0];
            Lista_Clusters[0].radio = initRad;
            Lista_Clusters[0].nMuestras = 1;
            nClusters++;
        } else {
            int clusterActualizado = 0;
            for(j = 0; j < nClusters; j++){
                double distancia = CalculoDistancia(Lista_Clusters[j].centro,Lista_Paq_Rec_Aprendizaje[i]);
                if (distancia <= Lista_Clusters[j].radio){
                    Lista_Clusters[j].radio += distancia;
                    Lista_Clusters[j].centro.RSSI = ((Lista_Clusters[j].centro.RSSI * Lista_Clusters[j].nMuestras) + Lista_Paq_Rec_Aprendizaje[i].RSSI)/(Lista_Clusters[j].nMuestras + 1);
                    Lista_Clusters[j].centro.tiempo = ((Lista_Clusters[j].centro.tiempo * Lista_Clusters[j].nMuestras) + Lista_Paq_Rec_Aprendizaje[i].tiempo)/(Lista_Clusters[j].nMuestras + 1);
                    Lista_Clusters[j].nMuestras++;
                    clusterActualizado = 1;
                }
            }
            if (clusterActualizado == 0) {
                Lista_Clusters[nClusters].centro = Lista_Paq_Rec_Aprendizaje[i];
                Lista_Clusters[nClusters].radio = initRad;
                Lista_Clusters[nClusters].nMuestras = 1;
                nClusters++;
            }
        }
    }
    clustersDone = 1;
}

double CalculoDistancia(coord pto1, coord pto2){
    double distancia;
    double pto1Pr = pto1.RSSI;
    double pto1t = pto1.tiempo;
    double pto2Pr = pto2.RSSI;
    double pto2t = pto2.tiempo;
    distancia = sqrt (pow(pto2Pr-pto1Pr, 2) + pow(pto2t-pto1t, 2));
    return distancia;
}

BYTE* hayAtacante(){
    pRSSI = &RSSI;
    BYTE *attacker;
    BYTE txAddr = 0;
    attacker = &txAddr;
    radioInterface ri = MIWI_2400;
    double t1;
    double tiempoPaquete;
    coord paquete;
    int i, inclCluster;
    inclCluster = 0;
    if ( GetRSSI(ri,pRSSI) == 0x00 ){  //Se ha recibido un paquete
        Printf("\r\nSe ha recibido un paquete\r\n");
        t1 = learningTime;
        tiempoPaquete = t1 - t0;
        t0 = t1;

        if (tiempoPaquete > 0){ //Se van a descartar los paquetes que se reciban cuando desborde el contador.
            paquete.tiempo = tiempoPaquete/tiempoMax;
            paquete.RSSI = (double) *pRSSI/potenciaMax;

            //Recorrer el array de clusters y comprobar si el paquete pertenece a algun cluster
            for(i = 0; i < nClusters; i++){
                double distancia = CalculoDistancia(Lista_Clusters[i].centro,paquete);
                if (distancia < Lista_Clusters[i].radio && inclCluster == 0){
                    inclCluster = 1;
                }
            }

            if (inclCluster == 0) {
                GetRXSourceAddr(ri, attacker);
                //limpiaBufferRX();
                return attacker;
            } else {
                //limpiaBufferRX();
                return NULL;
            }
        }
    }
    return NULL;
}

/*
void Envio_Datos_Atacantes(at data)
{

    radioInterface ri = MIWI_2400;

    TestAddress[0] = 0x00;
    TestAddress[1] = 0x11;
    TestAddress[2] = 0x22;
    TestAddress[3] = 0x33;
    TestAddress[4] = 0x44;
    TestAddress[5] = 0x55;
    TestAddress[6] = 0x66;

    #if defined NODE_1
    TestAddress[7] = 0x22;//Dirección del nodo 2
    #elif defined NODE_2
    TestAddress[7] = 0x11;//Dirección del nodo 1
    #endif

    BYTE i, j;
    i = 0;
    EnviandoMssgApp = TRUE;
    j = PutTXData(ri, 0x11);
    if (j) {
        Printf("\r\nFallo al escribir en el buffer. Codigo de error: ");
        PrintChar(j);
    }
    while(i < sizeof(data.direccionAtacante)) {
        j = PutTXData(ri, data.direccionAtacante[i]);
        if (j) {
            Printf("\r\nFallo al escribir en el buffer. Codigo de error: ");
            PrintChar(j);
        } else {
            i++;
        }
    }
    j = PutTXData(ri, data.esAtacante);
    if (j) {
        Printf("\r\nFallo al escribir en el buffer. Codigo de error: ");
        PrintChar(j);
    }
    i = SendPckt(ri, BROADCAST_ADDRMODE, NULL);
    Printf("\r\n\r\nMensaje de atacante enviado: ");
    if (i == 0) {
        Printf(" => OK");
    } else {
        Printf(" => FALLO: ");
        PrintChar(i);
    }
    EnviandoMssgApp = FALSE;
    HayDatosApp = FALSE;//Revisar según lo que quiera hacer
    //SWDelay(500);

    //CRM_Message(VCC, SubM_AccCtrl, &PeticionCrearEntrada);
}
*/

