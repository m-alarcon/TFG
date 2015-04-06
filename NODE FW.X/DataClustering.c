/*****************************************************************************
 *
 *              DataClustering.c -- DataClustering v0.1
 *
 *****************************************************************************
 *
 * Author:          Manuel Alarcón Granero
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
//TODO crear contador para establecer el tiempo de aprendizaje
//TODO crear contador para contar el tiempo entre paquetes
long t0 = GetSystemClock();
long learningTime = 30000;
BYTE *RSSI;
BYTE pruebaRSSI = 0;
double initRad = 0.3;
int tiempoMax;
BYTE potenciaMax;
int normalizado = 0;

int nClusters = 0;
cl cluster;

int paquetesRecibidos = 0;

//Pruebas de la funcion de calculo de distancia
coord p1;
coord p2;

void DataClustering(void){
    while ( 1/*GetSystemClock() < learningTime*/ ){ //El getsystemclock no funciona con ese learningTime
        coord paquete = CalculoCoordenadas();
        if(paquete.RSSI != 0){          //Hay un paquete
            //Recogida de datos
            if (paquetesRecibidos == 0){
                //Guardar la coordenada en una lista
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
    }

    if(normalizado == 0) NormalizarCoordenadas();

}

//Calculo coordenadas ultimo paquete
coord CalculoCoordenadas(){

    BYTE prueba;
    BYTE *RSSI1;
    RSSI1 = &prueba;
    RSSI = &pruebaRSSI;
    int t1;
    int tiempoPaquete;
    coord coordenadas;
    extern radioInterface ri;
    if ( paquetesRecibidos == 0 && GetRSSI(ri,RSSI) == 0 ){  //Si es correcto el valor de RSSI y es el primer dato
        t1 = GetSystemClock();
        tiempoPaquete = t1-t0;      //Va a sobrar
        t0 = t1;                    //Va a sobrar
        tiempoMax = tiempoPaquete;
        potenciaMax = *RSSI;
        coordenadas.tiempo = tiempoPaquete;
        coordenadas.RSSI = *RSSI;
        return coordenadas;
    } else {
        *RSSI1 = *RSSI;
        if ( paquetesRecibidos > 0 && GetRSSI(ri,RSSI) == 0 && *RSSI1 != *RSSI){   //Ha habido un nuevo paquete
            t1 = GetSystemClock();      //Si hago yo el contador lo reinicio cada vez
                                        //que llegue un paquete por tanto el tiempo de paquete
                                        //es el tiempo que se ha calculado en el contador
            tiempoPaquete = t1-t0;      //Va a sobrar
            t0 = t1;                    //Va a sobrar
            if(t0 > tiempoMax) tiempoMax = t0;
            if(*RSSI > potenciaMax) potenciaMax = *RSSI;

            coordenadas.tiempo = tiempoPaquete;
            coordenadas.RSSI = *RSSI;

            return coordenadas;
        } else {
            coordenadas.RSSI = 0;
            return coordenadas;
        }
    }
}

void NormalizarCoordenadas(){
    /*Bucle que recorre todas las coordenadas calculadas y las normaliza con
     * respecto al maximo */
    if(nClusters == 0){
        int i;
        //Normalizar coordenadas
        for(i = 0; i <= paquetesRecibidos; i++){
            Lista_Paq_Rec_Aprendizaje[i].RSSI = Lista_Paq_Rec_Aprendizaje[i].RSSI/potenciaMax;
            Lista_Paq_Rec_Aprendizaje[i].tiempo = Lista_Paq_Rec_Aprendizaje[i].tiempo/tiempoMax;
        }
        normalizado = 1;
    }
}

void CalculoClusters(){

    int i,j;
    if(nClusters == 0){
        Lista_Clusters[0].centro = Lista_Paq_Rec_Aprendizaje[0];
        Lista_Clusters[0].radio = initRad;
        Lista_Clusters[0].nMuestras = 1;
        nClusters++;
    } else {
        for(i = 1; i <= paquetesRecibidos; i++){ //Puede salirse del array y ser solo i < paq...
            for(j = 0; j < nClusters; j++){
                double distancia = CalculoDistancia(Lista_Clusters[j].centro,Lista_Paq_Rec_Aprendizaje[i]);
                if( distancia <= Lista_Clusters[j].radio){
                    Lista_Clusters[j].radio += distancia;
                    Lista_Clusters[j].centro.RSSI = ((Lista_Clusters[j].centro.RSSI * Lista_Clusters[j].nMuestras) + Lista_Paq_Rec_Aprendizaje[i].RSSI)/(Lista_Clusters[j].nMuestras + 1);
                    Lista_Clusters[j].centro.tiempo = ((Lista_Clusters[j].centro.tiempo * Lista_Clusters[j].nMuestras) + Lista_Paq_Rec_Aprendizaje[i].tiempo)/(Lista_Clusters[j].nMuestras + 1);
                    Lista_Clusters[j].nMuestras++;
                } else {
                    Lista_Clusters[j+1].centro = Lista_Paq_Rec_Aprendizaje[i];
                    Lista_Clusters[j+1].radio = initRad;
                    Lista_Clusters[j+1].nMuestras = 1;
                    nClusters++;
                }
            }
        }
    }
}

double CalculoDistancia(coord pto1, coord pto2){
    double distancia;
    BYTE pto1Pr = pto1.RSSI;
    long pto1t = pto1.tiempo;
    BYTE pto2Pr = pto2.RSSI;
    long pto2t = pto2.tiempo;
    distancia = sqrt (pow(pto2Pr-pto1Pr, 2) + pow(pto2t-pto1t, 2));
    return distancia;
}

void hayAtacante(){

}