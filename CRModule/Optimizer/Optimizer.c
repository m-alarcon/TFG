/*****************************************************************************
 *
 *              Optimizer.c -- Optimizer Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Optimizer.c
 * Dependencies:    Optimizer.h
 * Processor:
 * BOARD:
 * Compiler:        C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         B105 -FAILURE IS NOT AN OPTION-
 *
 *****************************************************************************
 * File Description:
 *
 * El modulo Messenger encargado de gestionar las peticiones entre modulos.
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#include "CRModule/Optimizer/Optimizer.h"
#include "CRModule/Optimizer/ConfigOptimizer.h"

/*****************************************************************************/
/*********************************VARIABLES***********************************/
/*****************************************************************************/

//Para configurar cada cuanto tiempo realiza su rutina de ejecucion Optimizer,
//que recordemos que es la rutina de ejecucion periodica y principal del modulo
//cognitivo.
WORD Periodomseg;
WORD Cuentamseg; //Para la cuenta de los mseg que han pasado.

extern BOOL EnviandoMssgApp;/*Para  la preuba de que mientras se envían datos
                             /*de appliacion no se puede realizar la estrategia
                             de optimizacion. Me refiero a justo en el momento
                             en el que se tocan funciones de miapp que pueden
                             tocar registros conflictivos. Ver enviodatos.c*/
extern BOOL RecibiendoMssg;/*PArecido a lo de arriba.*/

//#if defined(TEST4)
BYTE BufferPrueba[RX_BUFFER_SIZE];
BYTE Direccion[MY_ADDRESS_LENGTH];
RECEIVED_MESSAGE BufferRecepcionPrueba;
VCC_MSSG_RCVD PeticionRecepcion; //La del VCC
OPTM_MSSG_RCVD PeticionTest2; //La del Optmizer
//#endif

#if defined TEST5
    MIWI_TICK t1G, t2G;
    DWORD_VAL tTG;
#endif

//#if defined TEST6
    OPTSTATMACH OptmEstado;
    WORD MilisDeTimeOut;
    BYTE PeticionesEnviadas;
    BYTE PeticionesAceptadas;
    WORD ProbabilidadDeCambio;
//#endif

extern radioInterface ri;

//Variables para las diferentes estrategias de optimizacion.
    //Optimzier de Elena.
    BYTE CosteTx;
    BYTE CosteRx;
    BYTE CosteSensing;

    BYTE NumMsj = numMsjXDefecto;
    BYTE MaxRTx;
    BYTE SumRTx;
    BYTE MssgRTx[numMsjXDefecto]; //TODO molaría crear el tamaño dinamicamente con
        //malloc y free en función de NumMsj, de momento lo ponemos fijo.
    BYTE MssgIndex = 0;

    BYTE CosteCh;
    BYTE CosteO;//Coste de tx en el actual que se supone un poco saturado.
    BYTE CosteN;//Coste de no cambiar de canal a pesar de que el compañero lo
                //haga.

    WORD ProbChngCompi;
    WORD ProbChngPropia;
    //Fin de optimizer de Elena.

    BYTE MSSG_PROC_OPTM = 0;
    BYTE n_msg = 2; /*Numero de mensajes necesarios para cambiar de canal. Cambiar con el número que sea realmente*/
    BYTE CosteCambio, CosteOcupado, CosteNoCambio;
    BYTE EstadoGT;

/*****************************************************************************/
/******************************FIN DE VARIABLES*******************************/
/*****************************************************************************/

/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De recepcion/interfaz con Messenger*/
BOOL CRM_Optm_Mssg_Rcvd(OPTM_MSSG_RCVD *Peticion)
{
    switch(Peticion->Action)
    {
            //#if !defined(TEST4) //CLEAN
        case(ActGameTh):
            CRM_Optm_GameTheory(Peticion);
            break;
//#endif
        case(ActProcRq):
            CRM_Optm_Processor(Peticion);
            break;
        case(ActCons):
            CRM_Optm_Cons(Peticion);
            break;
        default:
            break;
    }
    return TRUE;
}
/*Fin de funciones de recepcion/interfaz con Messenger*/

/*De funcionalidad propia del sub-modulo*/
//Es el procesador de mensajes y peticiones que proceden de otros nodos. Es
//decir, es el encargado de decidir si cambiar o no de canal cuando otro nodo se
// lo solicita. Tambien debe por ejemplo decidir que hacer si recibe un mensaje
//de otro nodo diciendole que aquel no va a cambiar de canal (respondiendo a una
// peticion que hayamos hecho.
BOOL CRM_Optm_Processor(OPTM_MSSG_RCVD *Peticion)
{
    switch(*((BYTE*)(Peticion->Param1)))
    {
        case 0x00:
            break;
        case ProcAsk4Chng: //Aqui procesamos la peticion de cambio de canal.
        {
//#if defined(TEST6)
            /***************************VARIABLES******************************/
            //Un par de tipos de mensaje que utilizaremos para responder.
            MSN_MSSG_RCVD PeticionNodoExt;
            VCC_MSSG_RCVD PeticionNodoExtVCC;
            //Y un tipo para el executor.
            EXEC_MSSG_RCVD PeticionCambio;
            /***************************FIN VARIABLES**************************/

            /*******************************CUERPO*****************************/
            if(OptmEstado == WaitinAnsw4ChngChn)
            {
                //TODO implementar estrategia para resolver el conflicto.
                BYTE i;
                for (i=0; i < MY_ADDRESS_LENGTH; i++)
                {
                    if(Peticion->EUINodo[MY_ADDRESS_LENGTH - i - 1] < GetMyLongAddress(MY_ADDRESS_LENGTH - i -1))
                    {
                        break;
                    }
                }
                OptmEstado = Idle;
                MilisDeTimeOut = 0;
            }
            /*Comprobamos en nuestras politicas si realizamos el cambio.*/
                /*********MESSAGE CREATION*******/
                POLI_MSSG_RCVD PeticionPolitica;
                BYTE PoliOutputIntrChng;
                PeticionPolitica.Action = ActRead;
                PeticionPolitica.DataType = TypeIntrChng;
                PeticionPolitica.Param1 = &PoliOutputIntrChng;/*Chequeamos por ejemplo
                                                               la propension al cambio
                                                               de canal.*/
                /*********FIN MESSAGE CREATION*******/
            CRM_Message(NMM, SubM_Poli, &PeticionPolitica);
            /*Fin de la comprobacion en las politicas.*/
            //Y ahora comprobamos el resultado.
            if(*((BYTE*)PeticionPolitica.Param1) < 10) /*Si es menor que 10 la predisposicion
                                                 al cambio de canal no lo hacemos.*/
NOACEPTA: //Si no queremos notifcar el no cambio comentariamos y dejaríamos solo el break.
            {
                /*Creamos la respuesta para el nodo externo y la enviamos.*/
                    /*********MESSAGE CREATION*******/
                    BYTE PeticionNodoExtBuff[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcChangAnsw, FALSE, ri};
                    PeticionNodoExtVCC.DirNodDest = Peticion->EUINodo;
                    PeticionNodoExtVCC.Action = ActSend;
                    PeticionNodoExtVCC.BufferVCC = PeticionNodoExtBuff;
                    PeticionNodoExtVCC.Transceiver = ri;//Peticion->Transceiver;
                    BYTE sizeOfBufferVCC = 6;
                    PeticionNodoExtVCC.Param1 = &sizeOfBufferVCC;
                    PeticionNodoExt.Peticion_Destino.PeticionVCC = &PeticionNodoExtVCC;
                    /*********FIN MESSAGE CREATION*******/
                CRM_Message(VCC, SubM_Ext, &PeticionNodoExt);
                /*Fin de la respuesta para el nodo externo.*/
                break;
            }
            //Si la comprobacion es mayor que 10 hacemos todo el resto.
            else if(!CRM_Optm_CostAlgorithm(CosteN, CosteO, CosteCh, 1)) /*Como el que nos lo ha pedido
                                                                     sabemos que cambia su prob es
                                                                     1.*/
            {
             goto NOACEPTA;
            }
            else
            {
                /*Creamos un Message para executor para cambiar de canal.*/
                    /*********MESSAGE CREATION*******/
                    PeticionCambio.Action = ActChnHop;
                    PeticionCambio.Param1 = *((BYTE*)(Peticion->Param2));
                    PeticionCambio.Transceiver = Peticion->Transceiver;
                    ri = Peticion->Transceiver;//XXX
                    /*********FIN MESSAGE CREATION*******/
                CRM_Message(NMM, SubM_Exec, &PeticionCambio);
                /*Fin del Message para executor.*/
                WORD riname;
                switch (ri) {
                    case MIWI_0434: riname = 434; break;
                    case MIWI_0868: riname = 868; break;
                    case MIWI_2400: riname = 2400; break;
                }
                char traza[80];
                sprintf(traza, "\r\n\r\nCambiado al canal %d de la interfaz %d.",
                        GetOpChannel(ri), riname);
                Printf(traza);
                /*Creamos la respuesta para el nodo externo y la enviamos.*/
                    /*********MESSAGE CREATION*******/
                    BYTE PeticionNodoExtBuff[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcChangAnsw, TRUE, ri};
                    PeticionNodoExtVCC.DirNodDest = Peticion->EUINodo;
                    PeticionNodoExtVCC.Action = ActSend;
                    PeticionNodoExtVCC.BufferVCC = PeticionNodoExtBuff;
                    PeticionNodoExtVCC.Transceiver = ri;//Peticion->Transceiver;
                    BYTE sizeOfBufferVCC = 6;
                    PeticionNodoExtVCC.Param1 = &sizeOfBufferVCC;
                    PeticionNodoExt.Peticion_Destino.PeticionVCC = &PeticionNodoExtVCC;
                    /*********FIN MESSAGE CREATION*******/
                    CRM_Message(VCC, SubM_Ext, &PeticionNodoExt);
                /*Fin de la respuesta para el nodo externo.*/

#if defined(TEST5)
                t2G = MiWi_TickGet();
                tTG.Val = MiWi_TickGetDiff(t2G, t1G);
#endif
                /******************************FIN CUERPO**************************/
                break;
            }
//#endif
        }
        case ProcChangAnsw: //Aqui procesamos la respuesta que recibimos aceptando o negando el cambio de canal.
        {
            //TODO debemos empaquetar la respuesta Podemos haberla puesto en el siguiente byte.
//#if defined(TEST6)
            /*Creamos la peticion para saber el numero de veces que le hemos preguntado y
             cuantas ha aceptado el nodo.*/
            /***************************VARIABLES******************************/
//                REPO_MSSG_RCVD PeticionProb;
            BYTE IndiceTabla;
            REPO_MSSG_RCVD PeticionActualProb;
            REPO_MSSG_RCVD PeticionProb;
            REPOSUBDATANETNODE TipoDeSubDato;
            BYTE PeticionesEnviadasProv;
            BYTE PeticionesAceptadasProv;

            /***************************FIN VARIABLES**************************/

            Printf("\r\nRespuesta recibida al cambio de canal ...");
                /*********MESSAGE CREATION*******/
                TipoDeSubDato = AdditionalNetNode;
                IndiceTabla = MiWi_Search4ShortAddress(Peticion->Transceiver, Peticion->EUINodo, Repo_Conn_Table); /*XXX-GuilJa REVISAR
                                                                                                   //porque la función busca
                                                                                                   //direcciones cortas.*/

                PeticionProb.Action = ActSndDta;
                PeticionProb.DataType = NetNode;
                PeticionProb.EUINodo = Peticion->EUINodo;
                PeticionProb.Param1 = &TipoDeSubDato;
                PeticionProb.Param2 = &IndiceTabla;//XXX
                PeticionProb.Param3 = &PeticionesEnviadas;
                PeticionProb.Param4 = &PeticionesAceptadas;
                PeticionProb.Transceiver = Peticion->Transceiver;//XXX
//                *((REPOSUBDATANETNODE*)(PeticionProb.Param2)) = AdditionalNetNode;
                /*********FIN MESSAGE CREATION*******/
            CRM_Message(NMM, SubM_Repo, &PeticionProb);

//            PeticionesEnviadas = *((BYTE*) (PeticionProb.Param3));
//            PeticionesAceptadas = *((BYTE*) (PeticionProb.Param4));

            if (PeticionesEnviadas == 255)
            {
                PeticionesEnviadas /= 10;
                PeticionesAceptadas = (PeticionesAceptadas / 10) == 0 ? 1 : (PeticionesAceptadas/10); /*XXX-GuilJa para que
                                                                                                       la probabilidad no se
                                                                                                       haga 0.*/
            }
            if(*(BYTE*)(Peticion->Param2) == TRUE)
            {
                Printf("AFIRMATIVO.\r\n");
                char Print[80];
                /*Realizamos el calculo con el param4 que es donde tenemos los datos necesarios.*/
                PeticionesEnviadas = PeticionesEnviadas + 1;
                sprintf(Print, "PeticionesEnviadas %d.", PeticionesEnviadas);
                Printf(Print);
                PeticionesAceptadas = PeticionesAceptadas + 1;
                ProbabilidadDeCambio = ((PeticionesAceptadas)*100)/(PeticionesEnviadas);

            }
            else
            {
                /*Realizamos el calculo con el param4 que es donde tenemos los datos necesarios.*/
                PeticionesEnviadas += 1;
                ProbabilidadDeCambio = (PeticionesAceptadas)*100/(PeticionesEnviadas);

                Printf("NEGATIVO.\r\n");
                //Pero RemoveConnection manda un mensaje y como ya hemos cambiado de canal y el otro no, no le
                //va a llegar, asi que esto habra que cambiarlo.
//                MiApp_RemoveConnection(0);
//                            ConnectionTable[i].status.bits.isValid = 0;
            }

                /*********MESSAGE CREATION*******/
                TipoDeSubDato = AdditionalNetNode;
                PeticionActualProb.Action = ActStr;
                PeticionActualProb.EUINodo = Peticion->EUINodo;
                PeticionActualProb.DataType = NetNode;
                PeticionActualProb.Transceiver = Peticion->Transceiver;//XXX
                PeticionActualProb.Param1 = &TipoDeSubDato;
                PeticionActualProb.Param2 = &IndiceTabla; //XXX
                PeticionActualProb.Param3 = &PeticionesEnviadas;
                PeticionActualProb.Param4 = &PeticionesAceptadas;
                /*********FIN MESSAGE CREATION*******/
            CRM_Message(NMM, SubM_Repo, &PeticionActualProb);

            //TODO actualizar el valor que se utiliza para la estrategia de optimizacion.

            //Ya no esperamos respuesta asi que..
            OptmEstado = Idle;
            MilisDeTimeOut = 0;
//#endif
            break;            
        }
//#endif
        default:
            break;
    }
    return FALSE;
}

BOOL CRM_Optm_GameTheory(OPTM_MSSG_RCVD *Peticion)
{
    BOOL chngChnn = FALSE;
    //TODO la teoria de juegos.
    switch (Peticion->Action)
    {
        case(SubActChngCost):
            /*Realiza el calculo de la estrategia de optimizacion.*/
            chngChnn = CRM_Optm_CostAlgorithm(CosteN, CosteO, CosteCh, ProbChngCompi);
            if(chngChnn)
            {
                /*Primero calculamos el canal optimo para el cambio.*/
                    //Creamos unos parametros para un mensaje para Discovery.
                    //Para cada una de las interfaces:
                    BYTE CanalOptimo, CanalOptimo434, CanalOptimo868, CanalOptimo2400;
                    DWORD TodosCanales = 0xFFFFFFFF;
                    BYTE TiempoScan = 5;
                    BYTE TipoScan = NOISE_DETECT_ENERGY;
                    //Para cada una de las interfaces:
                    BYTE RSSIResultado434 = 255;
                    BYTE RSSIResultado868 = 255;
                    BYTE RSSIResultado2400 = 255;
                    //Creacion de un mensaje de discovery.
                    DISC_MSSG_RCVD PeticionDiscTest2;
                    PeticionDiscTest2.OrgModule = SubM_Opt;
                    PeticionDiscTest2.Action = ActSignDetc;
                    PeticionDiscTest2.Param1 = &TodosCanales;
                    PeticionDiscTest2.Param2 = &TiempoScan;
                    PeticionDiscTest2.Param3 = &TipoScan; //NOISE DETECT ENERGY.

                    //Ahora calculo el canal óptimo de cada interfaz y el canal más óptimo de todos:
                    PeticionDiscTest2.Transceiver = MIWI_0434;
                    PeticionDiscTest2.Param4 = &RSSIResultado434;
                    CanalOptimo434 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);
                    PeticionDiscTest2.Transceiver = MIWI_0868;
                    PeticionDiscTest2.Param4 = &RSSIResultado868;
                    CanalOptimo868 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);
                    PeticionDiscTest2.Transceiver = MIWI_2400;
                    PeticionDiscTest2.Param4 = &RSSIResultado2400;
                    CanalOptimo2400 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);
                    if ((RSSIResultado434 <= RSSIResultado868) && (RSSIResultado434 <= RSSIResultado2400)) {
                        CanalOptimo = CanalOptimo434;
                        ri = MIWI_0434;
                    } else if ((RSSIResultado868 < RSSIResultado434) && (RSSIResultado868 <= RSSIResultado2400)) {
                        CanalOptimo = CanalOptimo868;
                        ri = MIWI_0868;
                    } else {
                        CanalOptimo = CanalOptimo2400;
                        ri = MIWI_2400;
                    }
                    //Jose//CanalOptimo = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);//envio del messge.
//                  CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);//envio del messge.
//                    CanalOptimo = *((BYTE*)(PeticionDiscTest2.Param4));
                /*Fin del calculo del canal optimo.*/
#if defined(TEST4)
                /*Creamos la peticion para el nodo externo y la enviamos.*/
                    MSN_MSSG_RCVD PeticionNodoExt;
                    VCC_MSSG_RCVD PeticionNodoExtVCC;

                    //BYTE EUI_Dest[] = {0x01, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, EUI_7};
                    BYTE EUI_Dest[] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, 0x22};
                    BYTE PeticionNodoExtBuff[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcAsk4Chng, CanalOptimo, ri};
                    PeticionNodoExtVCC.DirNodDest = EUI_Dest;
                    PeticionNodoExtVCC.Action = ActSend;
                    PeticionNodoExtVCC.BufferVCC = PeticionNodoExtBuff;
                    PeticionNodoExtVCC.Transceiver = Peticion->Transceiver;//XXX
                    BYTE sizeOfBufferVCC = 6;
                    PeticionNodoExtVCC.Param1 = &sizeOfBufferVCC;

                    PeticionNodoExt.Peticion_Destino.PeticionVCC = &PeticionNodoExtVCC;

                    CRM_Message(VCC, SubM_Ext, &PeticionNodoExt);
                /*Fin de enviar la peticion al nodo externo.*/
#endif
#if defined TEST6
                //Iniciar el contador de espera de respuesta
                MilisDeTimeOut = 0;
                OptmEstado = WaitinAnsw4ChngChn;
#endif
                /*Creamos y enviamos un Message a Executor para cambiar el canal.*/
                    EXEC_MSSG_RCVD PeticionExecTest2;
                    PeticionExecTest2.OrgModule = SubM_Opt;
                    PeticionExecTest2.Action = ActChnHop;
                    PeticionExecTest2.Param1 = CanalOptimo;
                    PeticionExecTest2.Transceiver = ri;//Peticion->Transceiver;

                    CRM_Message(NMM, SubM_Exec, &PeticionExecTest2);//envio del messg.
                /*Fin del Message para Executor.*/
                WORD riname;
                switch (ri) {
                    case MIWI_0434: riname = 434; break;
                    case MIWI_0868: riname = 868; break;
                    case MIWI_2400: riname = 2400; break;
                }
                char traza[80];
                sprintf(traza, "\r\nCambiado al canal %d de la interfaz %d y enviada "
                        "la peticion al otro nodo.\r\n", GetOpChannel(Peticion->Transceiver), riname);
                Printf(traza);

                    //TODO. Actualizar la probabilidad de cambio.
#if defined(TEST5)
                //Para medir los tiempos de ejecucion.
            t2G = MiWi_TickGet();
            tTG.Val = MiWi_TickGetDiff(t2G, t1G);
#endif
            }
            //TODO HECHO llamar a la funcion del coste de cambio con los parametros que toquen.
            break;
        default:
            break;
    }
    return TRUE;
}
/*Fin de funciones de funcionalidad propia del sub-modulo*/

/*Sub-metodos de funciones*/
//El de coste de cambio de canal de Elena.
BOOL CRM_Optm_CostAlgorithm(INPUT BYTE CosteNotChanging , INPUT BYTE CosteOcup, INPUT BYTE CosteChngChann, INPUT WORD ProbCambioY)
{
    BOOL change;
    //Función de optimización//XXX-GuilJa.Los dos últimos terminos no sobrarian
    // dado que son constantes que se le suman a los dos terminos y por tanto no
    //variará el resultado de la comparacion.
    //XXX-GuilJa lo simplifico más abajo  WORD ganancia0 = (2*CosteNotChanging-CosteOcup)*0*ProbCambioY + (CosteOcup-CosteChngChann-CosteNotChanging)*0 + (CosteOcup-CosteNotChanging)*ProbCambioY - CosteOcup;
    //XXX-GuilJa lo simplifico más abajo.  WORD ganancia1 = (2*CosteNotChanging-CosteOcup)*1*ProbCambioY + (CosteOcup-CosteChngChann-CosteNotChanging)*1 + (CosteOcup-CosteNotChanging)*ProbCambioY - CosteOcup;

    //  WORD ganancia0 = 0;//XXX-GuilJa. Como queda 0 lo quitamos y cambiamos la proxima comprobacion.
    WORD ganancia1 = (2*CosteNotChanging-CosteOcup)*1*ProbCambioY + (CosteOcup-CosteChngChann-CosteNotChanging)*1;

    //XXX-GuilJa por la simplificacion que hemos hecho más arriba.  if(ganancia0>ganancia1)
    if(0>=ganancia1)
    {
        change = FALSE; //no cambio canal
    }
    else
    {
        change = TRUE; //cambio canal
    }
    return change;
    //TODO REVISAR no actualiza la probabilidad de cambio de el nodo propio no?
}
/*Fin de sub-metodos de funciones*/

/*Funciones de inicializacion*/
BOOL CRM_Optm_Init(void)
{
    BYTE i;
    Cuentamseg = 0;//Ponemos el contador a 0.
    Periodomseg = PeriodoXDefecto;
    //TODO la inicializacion.
    //Ini de las estrategias de optimizacion.
    //La de coste de cambio de canal.
        CosteTx = CosteTxXDefecto;
        CosteRx = CosteRxXDefecto;
        CosteSensing = CosteSensingXDefecto;
       // NumMsj = numMsjXDefecto; //Este es mi "espacio muestral" de los mensajes
            //que tengo en cuenta para realizar los calculos.
        MaxRTx = maxRTxXDefecto;
        //TODO lo relacionado con las retransmisiones hay que corregirlos.
        //TODO el lastTRX
        #if defined(MRF24J40)
            //TODO el MssgRTx//Hay rtx a nivel mac, buscar "retry" o "retries".
        #else
            //TODO el MssgRTx
        #endif

        //definicion de los costes
        CosteCh = CosteSensing + CosteTx + CosteRx;
        CosteO = CosteTx;//XXX-GuilJa. Lo inicializamos al coste de tx.
        CosteN = CosteTx * MaxRTx;


        ProbChngCompi = probChngCompiXDefecto;
    //Fin de la de coste de cambio

#if defined(TEST4)
    PeticionRecepcion.Action = ActRecv;
    BufferRecepcionPrueba.SourceAddress = Direccion;
    BufferRecepcionPrueba.Payload = BufferPrueba;
    PeticionRecepcion.BufferVCC = &BufferRecepcionPrueba;
    BYTE TamanoBuffer = RX_BUFFER_SIZE;
    PeticionRecepcion.Param1 = &TamanoBuffer;

    PeticionTest2.Action = SubActChngCost;

#endif
#if defined TEST6
//TODO inicializar lo que haga falta como por ejemplo el timeout.
//    MilisDeTimeOut = TIEMPODEESPERARESPUESTA;
    MilisDeTimeOut = 0;
    OptmEstado = Idle;
#endif
    return TRUE;
}
/*Fin de las funciones de inicializacion*/

/*Función de configuracion*/
BOOL CRM_Optm_Config(INPUT OPTACTION SubAccion, INPUT BYTE ConfigCosteTx, INPUT BYTE ConfigCosteRx,
        INPUT BYTE ConfigCosteSensing,INPUT BYTE ConfigmaxRTx, INPUT BYTE ConfignumMsj, INPUT WORD ConfigProbY)
{
    if(ConfigCosteTx != 0)
    {
        CosteTx = ConfigCosteTx;
        CosteCh = CosteSensing + CosteTx + CosteRx;
        CosteO = CosteTx * (SumRTx / MssgIndex);
        CosteN = CosteTx * MaxRTx;
    }
    if(ConfigCosteRx != 0)
    {
        CosteRx = ConfigCosteRx;
        CosteCh = CosteSensing + CosteTx + CosteRx;
    }
    if(ConfigCosteSensing != 0)
    {
        CosteSensing = ConfigCosteSensing;
        CosteCh = CosteSensing + CosteTx + CosteRx;
    }
    if(ConfigmaxRTx != 0)
    {
        MaxRTx = ConfigmaxRTx;
//        SumRTx = MaxRTx - MssgRTx;//Para nuestro sumRtx no influye por el tema
            //de que contamos directamente el num de rtx y no como castalia que
            //cuenta las que faltan para llegar al maximo.
        CosteN = CosteTx * MaxRTx;
//        CosteO = CosteTx * (SumRTx / NumMsj);//XXX-GuilJa. Puesto que ya no
        //influye directamente en SumRTx
    }
    if(ConfignumMsj != 0) //TODO //REVISAR
    {
        NumMsj = ConfignumMsj;
//        CosteO = CosteTx * (SumRTx / NumMsj);//XXX-GuilJa
    }
    if(ConfigProbY != 0)
    {
        ProbChngCompi = ConfigProbY;
    }
}
/*Fin de la funcion de configuracion*/

/*Rutina de tareas generales del sub-modulo*/
BOOL CRM_Optm_Task(TASKPARAM Opcion)
{
    switch(Opcion)
    {
        case TaskParamTx:
            /*Con esta opcion deberiamos llamar cada vez que
                   hayamos enviado un mensaje y de esta forma
                   actualizar el numero de retransmisiones que se
                   hayan producido para dicho envio.*/
            MssgIndex += 1;
            #if defined(MRF24J40)
                //TODO el MssgRTx//Hay rtx a nivel mac, buscar retries".
                //TODO creo que lo mejor sería que llamaramos a esta funcion desde la
                //interrupcion del MRF24J40 que es donde estamos seguro que ha enviado
                //un paquete.
            MssgRTx[MssgIndex] = 2;//XXX-GuilJa para prueba.
            #else
                //TODO el MssgRTx
                //Se llama reTry y esta en el TxPacket del MRF49XA.
            #endif
            //lo que puedo hacer aqui es emular unos cambios en el MssgRtx.
            if(MssgIndex < NumMsj)
            {
                SumRTx += MssgRTx[MssgIndex];//Esto es lo que en Castalia está como una resta
                    //porque ellos tienen el numero de rtx que les queda para llegar
                    //al maximo en vez del numero de rtx tal cual del ultimo
                    //mensaje.
            }
            CosteO = CosteTx * (SumRTx / MssgIndex);
            break;
        default:
            break;
    }
}
/*Fin de rutina de tareas generales del sub-modulo*/

BOOL CRM_Optm_Incluir_Potencia(BYTE *pVector){
    
    BYTE RSSI,i;
    BYTE *pRSSI = &RSSI;
    if (MSSG_PROC_OPTM == 0){
        if(GetRSSI(ri,pRSSI) == 0){
            for(i = MAX_VECTOR_POTENCIA-1; i >= 0; i--){
                if(i > 0){
                    *(pVector+i) = *(pVector+i-1);
                } else {
                    *(pVector) = *(pRSSI);
                }
            }
        }
        MSSG_PROC_OPTM = 1;

        //Creo el mensaje para repository y se lo mando
        REPO_MSSG_RCVD PeticionRepoPotencias;
        BYTE potencias[MAX_VECTOR_POTENCIA];
        REPODATATYPE PetPotencia = IncluirPotencia;
        PeticionRepoPotencias.Action = ActStr;
        PeticionRepoPotencias.DataType = PetPotencia;
        PeticionRepoPotencias.Param1 = &PetPotencia;
        PeticionRepoPotencias.Param2 = pRSSI;

        CRM_Message(NMM, SubM_Repo, &PeticionRepoPotencias);

        return TRUE;
    } else {

    }
    return FALSE;
}

BOOL CRM_Optm_Inicio_GT(BYTE *pVector){

    BOOL inicio;
    BYTE i,s,n,media;
    n = 0;
    s = 0;
    for(i = 0; i < MAX_VECTOR_POTENCIA; i++){
        s += *pVector;
        n++;
    }
    media = s/n;
    if(media < UMBRAL_POTENCIA){
        inicio = TRUE;
    } else {
        inicio = FALSE;
    }
    return inicio;

}

/*Devuelve si se decide cambiar de canal o quedarse en el que está*/
BOOL CRM_Optm_Calcular_Costes(BYTE n_rtx){
    //El CosteNoCambio siempre va a ser mayor o igual que el CosteOcupado. Lo calculo pero al decidir
    //si cambio o no, no lo tengo en cuenta.
    BOOL cambio;
    CosteCambio = CosteSensing + ((CosteTx + CosteRx) * n_msg);
    CosteOcupado = CosteTx * n_rtx;
    CosteNoCambio = CosteTx * MaxRTx;
    if (CosteCambio < CosteOcupado){
        cambio = TRUE;
    } else {
        cambio = FALSE;
    }
    return cambio;

}

BOOL CRM_Optm_Cons(OPTM_MSSG_RCVD *Peticion){

    BOOL inicioCambio = FALSE;
    BOOL cambioCanal = FALSE;
    BYTE n_rtx, i;
    REPO_MSSG_RCVD PeticionRepoPotencias;
    BYTE potencias[MAX_VECTOR_POTENCIA];
    REPO_MSSG_RCVD PeticionRepoRTx;
    BYTE num_RTx;
    BYTE *pPotencias = &potencias[0];
    BYTE canal = GetOpChannel(ri);

    switch (Peticion->Action)
    {
        case(SubActCambio):

            //Pedir a repo el vector de potencias y el numero de retransmisiones en el canal actual
            PeticionRepoPotencias.Action = ActSndDta;
            PeticionRepoPotencias.DataType = EnvPotencias;
            PeticionRepoPotencias.Param1 = pPotencias;

            CRM_Message(NMM, SubM_Repo, &PeticionRepoPotencias);

            PeticionRepoRTx.Action = ActSndDta;
            PeticionRepoRTx.DataType = EnvRTx;
            PeticionRepoRTx.Param1 = &canal;

            CRM_Message(NMM, SubM_Repo, &PeticionRepoRTx);

            /*Realiza el calculo de la estrategia de optimizacion.*/
            inicioCambio = CRM_Optm_Inicio_GT(pPotencias);
            if(inicioCambio){
                cambioCanal = CRM_Optm_Calcular_Costes(num_RTx);
            }

            if(cambioCanal)
            {
                /*Primero calculamos el canal optimo para el cambio.*/
                    //Creamos unos parametros para un mensaje para Discovery.
                    //Para cada una de las interfaces:
                    BYTE CanalOptimo, CanalOptimo434, CanalOptimo868, CanalOptimo2400;
                    DWORD TodosCanales = 0xFFFFFFFF;
                    BYTE TiempoScan = 5;
                    BYTE TipoScan = NOISE_DETECT_ENERGY;
                    //Para cada una de las interfaces:
                    BYTE RSSIResultado434 = 255;
                    BYTE RSSIResultado868 = 255;
                    BYTE RSSIResultado2400 = 255;
                    //Creacion de un mensaje de discovery.
                    DISC_MSSG_RCVD PeticionDiscCOpt;
                    PeticionDiscCOpt.OrgModule = SubM_Opt;
                    PeticionDiscCOpt.Action = ActSignDetc;
                    PeticionDiscCOpt.Param1 = &TodosCanales;
                    PeticionDiscCOpt.Param2 = &TiempoScan;
                    PeticionDiscCOpt.Param3 = &TipoScan; //NOISE DETECT ENERGY.

                    //Ahora calculo el canal óptimo de cada interfaz y el canal más óptimo de todos:
                    #ifdef MIWI_0434_RI
                    PeticionDiscCOpt.Transceiver = MIWI_0434;
                    PeticionDiscCOpt.Param4 = &RSSIResultado434;
                    CanalOptimo434 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscCOpt);
                    #endif
                    #ifdef MIWI_0868_RI
                    PeticionDiscCOpt.Transceiver = MIWI_0868;
                    PeticionDiscCOpt.Param4 = &RSSIResultado868;
                    CanalOptimo868 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscCOpt);
                    #endif
                    #ifdef MIWI_2400_RI
                    PeticionDiscCOpt.Transceiver = MIWI_2400;
                    PeticionDiscCOpt.Param4 = &RSSIResultado2400;
                    CanalOptimo2400 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscCOpt);
                    #endif
                    if ((RSSIResultado434 <= RSSIResultado868) && (RSSIResultado434 <= RSSIResultado2400)) {
                        CanalOptimo = CanalOptimo434;
                        ri = MIWI_0434;
                    } else if ((RSSIResultado868 < RSSIResultado434) && (RSSIResultado868 <= RSSIResultado2400)) {
                        CanalOptimo = CanalOptimo868;
                        ri = MIWI_0868;
                    } else {
                        CanalOptimo = CanalOptimo2400;
                        ri = MIWI_2400;
                    }

                    //Mandar la información del cambio de canal al resto de nodos.
                    BYTE MensajeVCC[] = {RSSIResultado434, RSSIResultado868, RSSIResultado2400, CanalOptimo, ri};
                    #if defined NODE_1
                        BYTE DireccionNodoDestino[] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6 , 0x22};
                    #elif defined NODE_2
                        BYTE DireccionNodoDestino[] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6 , 0x11};
                    #endif
                    MSN_MSSG_RCVD PeticionCambioCanal;
                    VCC_MSSG_RCVD PeticionVCCCambioCanal;

                    PeticionVCCCambioCanal.Action = ActSend;
                    PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                    PeticionVCCCambioCanal.DirNodDest = DireccionNodoDestino;
                    PeticionVCCCambioCanal.Transceiver = Peticion->Transceiver;
                    BYTE sizeBufferVCC = 27;
                    PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                    PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;

                    CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);

                    EstadoGT = EsperandoDecisionRestoNodos;
            }
        case(SubActProcInfo):
            //Comprobar el EstadoGT y serían mensajes recibidos de otros nodos
            break;

    }
}






















/*La rutina propia de ejecución en forma de una interrupción para que ocurra
 de forma periodica (sujeto a que otras interrupciones puedan retrasarla en
 funcion de la prioridad que tengan).*/
BOOL CRM_Optm_Int(void)
{

    //Cada vez que entramos incluimos la potencia del paquete que hemos recibido
    REPO_MSSG_RCVD PeticionRepoPotencias;
    BYTE potencias[MAX_VECTOR_POTENCIA];
    BYTE *pPotencias = &potencias[0];
    PeticionRepoPotencias.Action = ActSndDta;
    PeticionRepoPotencias.DataType = EnvPotencias;
    PeticionRepoPotencias.Param1 = pPotencias;

    CRM_Message(NMM, SubM_Repo, &PeticionRepoPotencias);

    BOOL i = CRM_Optm_Incluir_Potencia(pPotencias);
    if (i == FALSE){
        Printf("\r\nHa habido un error al incluir la potencia del último paquete");
    }

#if defined(TEST5)
    t1G = MiWi_TickGet(); //Cada vez que entra se guarda en t1 el tiempo.
#endif
//    if(!RecibiendoMssg && !EnviandoMssgApp)
        CRM_VCC_Mssg_Rcvd(&PeticionRecepcion); //Esto va a enviar o recibir los mensajes de VCC

#if defined TEST6
#if defined (NODE_1)//NodoEmisor)
    if(OptmEstado == WaitinAnsw4ChngChn)
    {
        if(MilisDeTimeOut < TIEMPODEESPERARESPUESTA)
            MilisDeTimeOut += 1;
        else if(MilisDeTimeOut == TIEMPODEESPERARESPUESTA)
        {
            //TODO actualizar que no ha aceptado la peticion y eliminar de la tabla de conexiones.
            OptmEstado = Idle; //Ya no esperamos respuesta.
            MilisDeTimeOut = 0;//Ponemos a 0.
//            MiApp_RemoveConnection(0); //Eliminamos de la tabla de conexiones.
        }
    }
#endif
#endif

    if(Cuentamseg<Periodomseg)
    {
        Cuentamseg++;
        return FALSE;
    }
    else if(Cuentamseg==Periodomseg)
    {
        if(!EnviandoMssgApp && !RecibiendoMssg)
        {
            Cuentamseg = 0;
#if defined TEST4 || defined TEST5
#if defined(NODE_2)//NodoReceptor)
            Printf("\r\n\r\nSe ejecuta la parte reactiva de la estrategia cognitiva.");
            //        t2 = MiWi_TickGet(); //XXX-GuilJa lo llevamos al procesado del test
            //        tT = MiWi_TickGetDiff(t2,t1); //más arriba.
            //        Printf("\r\n");
            //        Printf("El numero de Ticks para el nodo receptor (rutina de recepcion y cambio de canal) :");
            //        Printf("\r\n");
            //        PrintDec(tT.byte.MB);
            //        PrintDec(tT.byte.UB);
            //        PrintDec(tT.byte.HB);
            //        PrintDec(tT.byte.LB);
            //        Printf("\r\n");
            //        Printf("Nota: Todo 5s en caso de error.");
            //        Printf("\r\n");
            //        tT.Val = 0xFFFFFFFF; //Por si a la siguiente no hemos entrado en el procesador.
//                    DumpConnection(0xFF); //Para comprobar que hemos cambiado de canal
            return TRUE;
#elif defined(NODE_1)//NodoEmisor
            Printf("\r\n\r\nSe ejecuta la parte activa de la estrategia cognitiva.");
            Test4();
            //        Printf("\r\n");
            //        Printf("El numero de Ticks para el nodo emisor (rutina de optimizacion, envio de mensaje y cambio de canal) :");
            //        Printf("\r\n");
            //        PrintDec(tT.byte.MB);
            //        PrintDec(tT.byte.UB);
            //        PrintDec(tT.byte.HB);
            //        PrintDec(tT.byte.LB);
            //        Printf("\r\n");
            //        Printf("Nota: Todo 5s en caso de error.");
            //        Printf("\r\n");
            //        tT.Val = 0xFFFFFFFF; //Por si a la siguiente no hemos entrado en el procesador.
//                    DumpConnection(0xFF); //Para comprobar que hemos cambiado de canal
            return TRUE;
#endif
#endif
#if defined TEST3
            Test2();
            Cuentamseg = 0;
            return TRUE;
#endif
            //TODO aqui deben ir las llamadas a las funciones que deba ejecutar el
            //optimizer.

    //Aquí llamo a la función de la teoría de juegos que se encarga de ver si hay
    //que cambiar de canal o no y de hacer todo el proceso del algoritmo.





















        }

    }
    return FALSE;
}

#if defined TEST3
//TEST2-BORRABLE
/*Solo util para realizar el test2*/
//Estamos intentando simular una situacion en la que optimizer solicita una 
//accion a discovery y en consecuencia ordena a executor llevar a cabo otra
//determinada accion. En concreto mide niveles de señal en discovery y a 
//continuacion cambia el canal al 15.
BYTE Test2(void)
{
    //Hacemos una invocacion primero a Discovery.
    DWORD TodosCanales = 0xFFFFFFFF;
    BYTE TiempoScan = 10;
    BYTE TipoScan = NOISE_DETECT_ENERGY;
    //Creacion de un mensaje de discovery.
    DISC_MSSG_RCVD PeticionDiscTest2;
    PeticionDiscTest2.OrgModule = SubM_Opt;
    PeticionDiscTest2.Action = ActSignDetc;
    PeticionDiscTest2.Param1 = &TodosCanales;
    PeticionDiscTest2.Param2 = &TiempoScan;
    PeticionDiscTest2.Param3 = &TipoScan; //NOISE DETECT ENERGY.
    CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);//envio del messge.
    //Creacion de un mensaje para executor.
    EXEC_MSSG_RCVD PeticionExecTest2;
    PeticionExecTest2.OrgModule = SubM_Opt;
    PeticionExecTest2.Action = ActChnHop;
    PeticionExecTest2.Param1 = 15;
    CRM_Message(NMM, SubM_Exec, &PeticionExecTest2);//envio del messg.

    DumpConnection(0xFF);//Para comprobar que hemos cambiado de canal
}
#endif
/*Fin de la funcion para el test2*/

#if defined(TEST4)
BYTE Test4(void)
{
    PeticionTest2.Transceiver = ri;
    CRM_Optm_GameTheory(&PeticionTest2);
    return 0;
}
#endif

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/
