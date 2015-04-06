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
//TODO crear timer para establecer el tiempo de aprendizaje
//TODO crear timer para contar el tiempo entre paquetes
int learningTimeMax = 2040000;
int learningTime = 0;
int t0 = 0;
BYTE *RSSI;
BYTE pruebaRSSI = 0;
double initRad = 0.3;
int tiempoMax;
BYTE potenciaMax;
int aprendizaje = 0;
int normalizado = 0;

int nClusters = 0;
cl cluster;

int paquetesRecibidos = 0;

//Pruebas de la funcion de calculo de distancia
coord p1;
coord p2;

void DataClustering(void){
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
        if(normalizado == 0) NormalizarCoordenadas();

        //Por cada paquete que se reciba, comprobar que pertenece a algun cluster
        //y si no marcar como atacante y mandar mensaje a los demas nodos.
        hayAtacante();
    }
}

void initTimers(){
    WORD T5_TICK = (CLOCK_FREQ/8/8/34000);
    OpenTimer5(T5_ON | T5_IDLE_CON | T5_GATE_OFF | T5_PS_1_8 | T5_SOURCE_INT, T5_TICK);
    ConfigIntTimer5(T5_INT_ON | T5_INT_PRIOR_7);
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
        tiempoPaquete = learningTime;
        t0 = learningTime; //Para el tiempo del proximo paquete
        tiempoMax = tiempoPaquete;
        potenciaMax = *RSSI;
        coordenadas.tiempo = tiempoPaquete;
        coordenadas.RSSI = *RSSI;
        return coordenadas;
    } else {
        *RSSI1 = *RSSI;
        if ( paquetesRecibidos > 0 && GetRSSI(ri,RSSI) == 0 && *RSSI1 != *RSSI){
            t1 = learningTime;
            tiempoPaquete = t1 - t0;
            if(t0 > tiempoMax) tiempoMax = t0;
            if(*RSSI > potenciaMax) potenciaMax = *RSSI;
            t0 = t1;

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
     if(nClusters == 0){
        int i;
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

/*******************************************************************************
 * Function:    IntTmp()
 * Input:       None.
 * Output:      None.
 * Overview:    Timer interruption routine.
 * Si ha terminado el tiempo de aprendizaje se para el timer.
 ******************************************************************************/
void __ISR(_TIMER_5_VECTOR, ipl7)IntTmp(void) {

    mT5ClearIntFlag();

    learningTime++;
    if (learningTime == 2040000){
        aprendizaje = 1;
        ConfigIntTimer5(T5_INT_OFF | T5_INT_PRIOR_7);
    }
}