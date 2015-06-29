/*****************************************************************************
 *
 *              Repository.h -- Repository Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Repository.h
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
 * Este archivo desarrolla las funciones y datos del modulo repository
 * (CR Module).
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/

#ifndef  _REPOSITORY_H_
#define  _REPOSITORY_H_

#include "GenericTypeDefs.h"
#include "../CRModule.h"
//#include "WirelessProtocols/MCHP_API.h"
#include "../../CWSN LSI Node/Include/NodeHAL.h"
#include "math.h"

#include "ConfigRepository.h"

/*****************************************************************************/
/***************************DEFINICION DE TIPOS*******************************/

/*****************************************************************************/

typedef enum _REPACTION
{
    ActStr = 0x01, ActSndDta
} REPACTION;

typedef enum _REPODATATYPE
{
    OwnNode = 0x01, NetNode, EnvNode, Enviro, EnvPotencias, IncluirPotencia, AddMsg, EnvRTx, EnvNMsg, EnvRSSI, SaveRSSI, RstRSSI, RstRTx, AddCoord, NormCoord, InclClusters, DetAttNodoPropio, DetAtt, AllInfo = 0x10
} REPODATATYPE;

/*Los sub-tipos de datos para cada tipo de dato*/
//Sub-OwnNode
typedef enum _REPOSUBDATAOWNNODE
{
    ASOwnNode = 0x01, AdditionalOwnNode, PermOwnNode, PolicOwnNode, AllOwnNode = 0x10
} REPOSUBDATAOWNNODE;

//Sub-NetNode
typedef enum _REPOSUBDATANETNODE
{
    AdditionalNetNode = 0x01, AllNetNode = 0x10, RSSINetNode
} REPOSUBDATANETNODE;

//Sub-NetNode
typedef enum _REPOSUBDATAENVNODE
{
    AllEnvNode = 0x10
} REPOSUBDATAENVNODE;

//Sub-NetNode
typedef enum _REPOSUBDATAENVIRO
{
    AllEnviro = 0x10
} REPOSUBDATAENVIRO;


/*La estructura de los parametros del Mensaje con destino este modulo.*/
typedef struct _REPO_MSSG_RCVD
{
    INPUT BYTE OrgModule;
    REPACTION Action;
    INPUT BYTE *EUINodo;
    INPUT REPODATATYPE DataType;
    INPUT radioInterface Transceiver;
    IOPUT void *Param1;
    IOPUT void *Param2;
    IOPUT void *Param3;
    IOPUT void *Param4;
}REPO_MSSG_RCVD;

#include "CRModule/Access/Access.h" //Para incluir los tipos de Access.

/*Estructura de los datos que va a almacenar el repositorio*/
//REVISAR deberia ir en el .c o nos interesa que pueda utilizarlas otro
//sub-modulo??.


typedef struct coordenadas {
    double RSSI;
    double tiempo;
} coord;

typedef struct cluster {
    coord centro;
    double radio;
    int nMuestras;
} cl;

typedef struct atacantes {
    BYTE direccionAtacante[MY_ADDRESS_LENGTH];
    BYTE direccionDetector[MY_ADDRESS_LENGTH];
    BYTE esAtacante;
} at;


extern radioInterface ri;

coord Lista_Paq_Rec_Aprendizaje[MAX_PAQ_APRENDIZAJE];
cl Lista_Clusters[MAX_CLUSTERS];
at Tabla_Atacantes[(CONNECTION_SIZE+1)*(CONNECTION_SIZE+1)];

BYTE MIWI434_rtx[MIWI0434NumChannels];
BYTE MIWI868_rtx[MIWI0868NumChannels];
BYTE MIWI2400_rtx[MIWI2400NumChannels];

BYTE MIWI434_RSSI_values[MIWI0434NumChannels];
BYTE MIWI868_RSSI_values[MIWI0868NumChannels];
BYTE MIWI2400_RSSI_values[MIWI2400NumChannels];

BYTE MIWI434_RSSI_optimo_ext[CONNECTION_SIZE];
BYTE MIWI434_canal_optimo_ext[CONNECTION_SIZE];
BYTE MIWI868_RSSI_optimo_ext[CONNECTION_SIZE];
BYTE MIWI868_canal_optimo_ext[CONNECTION_SIZE];
BYTE MIWI2400_RSSI_optimo_ext[CONNECTION_SIZE];
BYTE MIWI2400_canal_optimo_ext[CONNECTION_SIZE];

BYTE CanalOptimo;
radioInterface riCanalOptimo;

BYTE CanalOptimoExt[CONNECTION_SIZE];
radioInterface riCanalOptimoExt[CONNECTION_SIZE];
UINT16 NumMssgIntercambiados[CONNECTION_SIZE];

/*Del Propio nodo*/
    //La direccion larga.
    extern BYTE myLongAddress[MY_ADDRESS_LENGTH]; //esta en MiWi.c REVISAR que
                            //esto es correcto hacerlo.
    typedef union //REVISAR esto esta en MiWi.c pero duplicamos aqui para que
                  //se definir el siguiente extern.
    {
        BYTE        Val;
        struct
        {
            BYTE    Sleep           :1;
            BYTE    Role            :2;
            BYTE    Security        :1;
            BYTE    ConnMode        :2;
            BYTE    CoordCap        :1;
        }bits;
    } MIWI_CAPACITY_INFO;
    MIWI_CAPACITY_INFO MiWiCapacityInfo;//A este extern me refiero en el
                    //en el typedef que he escrito antes. XXX: quito el extern
    //Info variada.
    ACTIVE_SCAN_RESULT InfoPropioNodo; //Cogemos la misma estructura que el
        //active-scan pero esta tenemos que rellenarla a mano con ayuda de
        //CoonfigHAL.h.
    extern BYTE AdditionalNodeID[ADDITIONAL_NODE_ID_SIZE];
    ENTRADA_PERM_REG Repo_Tabla_Permo_Acc[MAX_NUMERO_PERM_NODOS];
    POLI_WEIGHTS Repo_Poli_Weights;
    extern POLI_WEIGHTS Reparto_Pesos;
    rtccTime RepoNodPropioTime;
    rtccDate RepoNodPropioDate;
        //TODO numero total de bytes enviados.
        //TODO numero total de bytes recibidos.
        //Informacion relativa a la optimizacion.
        //Informacion relativa a la bateria.

    BYTE vectorPotencias[MAX_VECTOR_POTENCIA];

/*Del resto de nodos de la red*/
CONNECTION_ENTRY Repo_Conn_Table[CONNECTION_SIZE]; //Con quien esta 
    //conectado y el estado de esa conexion.
rtccTime RepoNodRedTime;
rtccDate RepoNodRedDate;


/*Del resto de nodos del entorno (que no pertenecen a la red*/
ACTIVE_SCAN_RESULT Repo_Activ_Scan[ACTIVE_SCAN_RESULT_SIZE]; //Info
rtccTime RepoNodEnvTime;
rtccDate RepoNodEnvDate;

/*Del entorno*///REVISAR. Provisional
extern BYTE ActiveScanResultIndex;
typedef struct _REPO_EVAL_CANAL
{
    BYTE CanalEval;
    BYTE EvalRSSI;
    rtccTime RepoEnvTime; //REVISAR. Quiero momento de sensado para cada canal.
    rtccDate RepoEnvDate;
}REPO_EVAL_CANAL;
REPO_EVAL_CANAL Repo_EvalCanl;

/*Fin de las estructuras de datos del repositorio.*/

/*****************************************************************************/
/*************************FIN DEFINICION DE TIPOS*****************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De recepcion/interfaz con Messenger*/
BOOL CRM_Repo_Mssg_Rcvd(REPO_MSSG_RCVD *Peticion);
/*Fin de funciones de recepcion/interfaz con Messenger*/

/*Funciones propias del sub-modulo de I/O de datos*/
BOOL CRM_Repo_Store(REPO_MSSG_RCVD *Peticion);
BOOL CRM_Repo_SendDat(REPO_MSSG_RCVD *Peticion);
void CRM_Repo_NodoPropio(REPO_MSSG_RCVD *Peticion);
//void CRM_Repo_NodosRed(void);
BOOL CRM_Repo_NodosRed(REPO_MSSG_RCVD *Peticion);
void CRM_Repo_NodosEnv(void);
void CRM_Repo_Env(BYTE canal, BYTE InfoRSSI);
void CRM_Repo_NRTx(BYTE n_rtx, BYTE canal, radioInterface ri);
void CRM_Repo_Mensajes_Intercambiados(BYTE *Address);
void CRM_Repo_Str_RSSI(radioInterface ri);
BOOL CRM_Repo_Reiniciar_Potencias(void);
BOOL CRM_Repo_Reiniciar_RTx(void);
BOOL CRM_Repo_Calculo_Coordenadas();
void CRM_Repo_Normalizar_Coordenadas();
void CRM_Repo_Calculo_Clusters();
double CRM_Repo_Calculo_Distancia(coord pto1, coord pto2);
BOOL CRM_Optm_Detectar_Atacante();
BOOL CRM_Repo_Proc_Mens_Att(REPO_MSSG_RCVD *Peticion);
/*Fin de funciones de I/O de datos*/

/*Funciones de inicializacion*/
BOOL CRM_Repo_Init(void);
/*Fin de las funciones de inicializacion*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/


#endif
