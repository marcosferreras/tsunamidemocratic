#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
//Tamanio de la cola de solicitudes
#define tamCola 15
#define true 1
#define false 0
//Defino las funciones
void *accionesAtendedor(void *ptr);
void *accionesSolicitud(void *ptr);
void manejadoraSolicitud(int signal);
void manejadoraFinalizar(int signal);
void writeLogMessage (char *id , char *msg);
int sacarNumero(char *id);
void *usuarioEnActividad(void *id);
void *accionesCoordinadorSocial();
pthread_mutex_t mutexLog;
pthread_mutex_t mutexColaSolicitudes;
pthread_mutex_t mutexColaSocial;
pthread_cond_t condActividades;
FILE *logFile;
typedef struct solicitud {
	char id[50];
	//0->Sin atender 1->En proceso de atención 2->Atendido 3->Atendido con Antecedentes
	int atendido;
	//Tipo 0->Invitación 1->QR
	int tipo;
	pthread_t hilo;
} Solicitud;
void inicializarSolicitud(Solicitud* solicitud);
//Cola de solicitudes
Solicitud colaSolicitudes[tamCola];
int idUsuariosActividad[4];
//Para la asignacion de ID a las solicitudes
int contadorSolicitudes;
//Para saber cuantos hay en cola
int contadorSolicitudesCola;
int contadorActividadesCola;
//Para saber 
int listaCerrada;

/*
 *	Main
 */

int main(){
	//Imprime el pid para poder utilizar el mandador
	printf("Pid: %d\n",getpid());
	//Tipo de atendedor 1->Invitacion 2->QR 3->PRO
	int tipoAtendedor[3]={1,2,3};
	int i;
	//Tratamiento de señales
	struct sigaction sSolicitud={0};
	struct sigaction sFinalizar={0};
	sSolicitud.sa_handler=manejadoraSolicitud;
	sigaction(SIGUSR1,&sSolicitud,NULL);
	sigaction(SIGUSR2,&sSolicitud,NULL);
	sFinalizar.sa_handler=manejadoraFinalizar;
	sigaction(SIGINT,&sFinalizar,NULL);
	//Inicializar recursos	
	srand(time(NULL));
	contadorSolicitudes=0;
	contadorSolicitudesCola=0;
	contadorActividadesCola=0;
	listaCerrada=false;
	logFile=NULL;
	for(i=0;i<tamCola;i++){
		inicializarSolicitud(&colaSolicitudes[i]);
	}
	for(i=0;i<4;i++){
		idUsuariosActividad[i]=0;
	}
	pthread_mutex_init(&mutexColaSocial,NULL);
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexColaSolicitudes, NULL);
	pthread_cond_init(&condActividades, NULL);
	//Encargados de las solicitudes de:
	pthread_t atendedor_1, atendedor_2, atendedor_3, coordinador;
	//Creamos los hilos de los "usuarios destacados"
	printf("Iniciando tsunami\n");
	writeLogMessage("1","Iniciando tsunami");
	pthread_create(&atendedor_1, NULL, accionesAtendedor, (void *) &tipoAtendedor[0]);
	pthread_create(&atendedor_2, NULL, accionesAtendedor, (void *) &tipoAtendedor[1]);
	pthread_create(&atendedor_3, NULL, accionesAtendedor, (void *) &tipoAtendedor[2]);
	//Creamos el hilo del coordinador
	pthread_create(&coordinador, NULL, accionesCoordinadorSocial, NULL);

	while(true){
		pause();
	}
}

void nuevaSolicitud(int signal){
	printf("Nueva solicitud\n");
//Variable para imprimir los mensajes /*sprintf(buffer, "mensaje");*/ /*writeLogMessage (char *id , char *msg)*/
	char buffer[500];
	//Declaro la solicitud
	Solicitud *solicitud;
//Seccion Critica
	pthread_mutex_lock(&mutexColaSolicitudes);
	//Contador del bucle
	int i = 0;
	//Condicion de salida (Solicitud guardada)
	int noGuardado = false;
	//Variable para el numero de solicitud
	int valorId = 0;
	//Compruebo el espacio en la lista de solicitudes
	while((i < tamCola) && (noGuardado == false)){
		//Si el hueco de la solicitud esta libre, entro
		if(strcmp(colaSolicitudes[i].id,"0")==0){
			solicitud = &colaSolicitudes[i];
			//Introduzco el identificador con su valor y lo incremento
			contadorSolicitudes++;
			//Guardo el valor en un char para introducirlo en id
			valorId = contadorSolicitudes;
			//Introduzco el numero en el identificador de la solicitud
			sprintf(solicitud->id,"solicitud_%d",valorId);
			printf("%s añadida a la lista de solicitudes\n",solicitud->id);
			sprintf(buffer, "%s añadida a la lista de solicitudes\n",solicitud->id);
			printf("%s lista para ser atendida\n",solicitud->id);
			sprintf(buffer, "%s lista para ser atendida\n",solicitud->id);
			solicitud->atendido = 0;
			//Indico el tipo de solicitud segun la senial
			//Invitacion
			if(signal == SIGUSR1){
				printf("%s de invitacion\n",solicitud->id);
				sprintf(buffer, "%s de invitacion\n",solicitud->id);
				solicitud->tipo = 0;
			//QR
			}else{
				printf("%s por codigo QR\n",solicitud->id);
				sprintf(buffer,"%s por codigo QR\n",solicitud->id);
				solicitud->tipo = 1;
			}
			//Solicitud guardada, salgo del bucle
			noGuardado = true;
			//Mando la solicitud para ser procesada
			pthread_create(&solicitud->hilo, NULL, accionesSolicitud, (void *) solicitud);
		//Posicion ya ocupada
		}	
	i++;
	}
//Fin de la seccion critica
	pthread_mutex_unlock(&mutexColaSolicitudes);
	//Cola llena
	if(i == tamCola){
		printf("Cola de solicitudes llena, Solicitud ignorada\n");
		sprintf(buffer,"Cola de solicitudes llena, Solicitud ignorada\n");
	}
//Envio el mensaje guardado en el buffer a la funcion writeLogMessage
	writeLogMessage ( solicitud->id , buffer);
}
/**
 * Función que se encarga de las acciones de la solicitud
 * @autor Marcos Ferreras
*/
void *accionesSolicitud(void *structSolicitud){
	//char idSolicitud[50], cadenaLog[100];
	Solicitud* solicitud = (Solicitud *)structSolicitud;
	//0->Sin atender 1->En proceso de atención 2->Atendido 3->Atendido con Antecedentes
	int enAtencion=0;
	int aleatorio;
	int participo;
	char id[50];
	
	//strcpy(solicitud->id,"1");
	//if(strcmp("0\0",solicitud->id)==0)	
	//	printf("Hola\n");
	if(solicitud->tipo==0){
		printf("ID %s : La solicitud por invitacion se ha registrado\n", solicitud->id);
		writeLogMessage(solicitud->id,"La solicitud por invitacion se ha registrado");
	} else {
		printf("ID %s : La solicitud por QR se ha registrado\n", solicitud->id);
		writeLogMessage(solicitud->id,"La solicitud por QR se ha registrado");
	}
	//printf("Hola\n");
	//writeLogMessage("1111","5");
	sleep(4);
	pthread_mutex_lock(&mutexColaSolicitudes);
	//Copio el id
	strcpy(id,solicitud->id);
	enAtencion=solicitud->atendido;
	//Mientras esté sin atender
	while(enAtencion==0){
		printf("%d\n",enAtencion);
		//Invitación
		aleatorio = rand()%101;
		if(solicitud->tipo==0){
			printf("%d rand\n",aleatorio);
			if(aleatorio<=10){
				//TODO Escribir en log
				printf("ID %s : La solicitud por invitacion se ha eliminado por cansarse de esperar\n", solicitud->id);
				writeLogMessage(solicitud->id,"La solicitud por invitacion se ha eliminado por cansarse de esperar");
				inicializarSolicitud(solicitud);
				pthread_mutex_unlock(&mutexColaSolicitudes);
				pthread_exit(NULL); 
			}
		//QR
		} else {
			if(aleatorio<=30){
				//TODO Escribir en log
				printf("ID %s : La solicitud por QR se ha eliminado por no considerarse muy fiable\n", solicitud->id);
				writeLogMessage(solicitud->id,"La solicitud por QR se ha eliminado por no considerarse muy fiable");
				inicializarSolicitud(solicitud);
				pthread_mutex_unlock(&mutexColaSolicitudes);
				pthread_exit(NULL); 
			}
		}
		//Descarte fallo aplicacion
		aleatorio = rand()%101;
		if(aleatorio<=15){
			//TODO Escribir en log
			printf("ID %s : La solicitud se ha eliminado por fallo de la aplicacion\n", solicitud->id);
			writeLogMessage(solicitud->id,"La solicitud se ha eliminado por fallo de la aplicacion");	
			inicializarSolicitud(solicitud);
			pthread_mutex_unlock(&mutexColaSolicitudes);
			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&mutexColaSolicitudes);
		sleep(4);
		pthread_mutex_lock(&mutexColaSolicitudes);
		enAtencion=solicitud->atendido;
		
	}
	//enAtencion=solicitud->atendido;
	//Mientras me esten atendiendo esperaré. No he soltado el mutex cuando salí del bucle anterior.
	while(enAtencion==1){
		pthread_mutex_unlock(&mutexColaSolicitudes);
		sleep(2);
		pthread_mutex_lock(&mutexColaSolicitudes);
		enAtencion=solicitud->atendido;
	}
	//Al salir de este bucle no he soltado el mutex. En este punto lo desbloqueo, y decido si me uno a una actividad social o no (Si puedo).
	enAtencion=solicitud->atendido;
	pthread_mutex_unlock(&mutexColaSolicitudes);
	if(enAtencion == 2){
		participo=rand()%2;
		//0->Si Participo 1->No Participo
		if(participo==0){
			//Libero hueco en solicitudes
			printf("ID %s : El usuario esta preparado para la actividad\n", solicitud->id);
			writeLogMessage(solicitud->id,"El usuario esta preparado para la actividad");
			pthread_mutex_lock(&mutexColaSolicitudes);
			inicializarSolicitud(solicitud);
			pthread_mutex_unlock(&mutexColaSolicitudes);
			pthread_mutex_lock(&mutexColaSocial);
			while(listaCerrada==true){
				pthread_mutex_unlock(&mutexColaSocial);
				sleep(3);
				pthread_mutex_lock(&mutexColaSocial);
			}
			//Copio el id del usuario a la actividad
			idUsuariosActividad[contadorActividadesCola]= sacarNumero(id);
			//Incremento el numero de actividades
			contadorActividadesCola++;
			if(contadorActividadesCola==4){
				//Aviso al coordinador que soy el ultimo
				pthread_cond_signal(&condActividades);
			}
			pthread_mutex_unlock(&mutexColaSocial);
			//Libero espacio en la cola
			pthread_mutex_lock(&mutexColaSolicitudes);
			inicializarSolicitud(solicitud);
			pthread_mutex_unlock(&mutexColaSolicitudes);
			pthread_exit(NULL);
			
		} else {
			//Libero espacio en cola de solicitudes
			inicializarSolicitud(solicitud);
		}
		pthread_exit(NULL);
	} else if(enAtencion==3){
		//Solicitud con antecedentes. Libero el hueco en la cola y finalizo hilo.
		pthread_mutex_lock(&mutexColaSolicitudes);
		inicializarSolicitud(solicitud);
		pthread_mutex_unlock(&mutexColaSolicitudes);
		pthread_exit(NULL);
	}	
}
void *accionesAtendedor(void *ptrs){
	int tipoEvaluacion, id, contador=0, calculo, espera, apto, posicion=0, cafe=0;
	char atendedor[25], salida[150];
	//printf("Exito %d", *(int *)ptrs);
	sprintf(atendedor,"Atendedor_%d",*(int *)ptrs);
	do{
		contador=0;//😂️
		apto=true;
		cafe++;
		do{
			if(contador%2==0 || contador==0){
					pthread_mutex_lock(&mutexColaSolicitudes);
			}
					
			contador++;
			id=-1;
			for(int i=0;i<tamCola;i++){
				if((colaSolicitudes[i].tipo==tipoEvaluacion || tipoEvaluacion==3) && colaSolicitudes[i].atendido==0){
					if((id>sacarNumero(colaSolicitudes[i].id) || id==-1)){
						if(sacarNumero(colaSolicitudes[i].id)!=0){
							id=sacarNumero(colaSolicitudes[i].id);
							posicion=i;
						}				
					}
				}
			}
			if(id==-1){
				tipoEvaluacion=3;
				if(contador%2==0){
					//printf("%d-Par: %d\n",*(int *)ptrs,id);
					pthread_mutex_unlock(&mutexColaSolicitudes);
					sleep(1);
				}
			}
		}while(id==-1);
		colaSolicitudes[posicion].atendido=1;
		pthread_mutex_unlock(&mutexColaSolicitudes);
		printf("Atendedor_%d: comienza a procesar la solicitud_%d\n",*(int *)ptrs,id);
		sprintf(salida,"comienza a procesar la solicitud_%d",id);
		writeLogMessage(atendedor,salida);
		srand(time(NULL));
		calculo=rand()%10+1;
		if(calculo<=7){
			espera=rand()%4+1;
			printf("Atendedor_%d: todo correcto al procesar la solicitud_%d\n",*(int *)ptrs,id);
			sprintf(salida,"todo correcto al procesar la solicitud_%d",id);
			writeLogMessage(atendedor,salida);
		}else if(calculo<=9){
			espera=rand()%5+2;
			printf("Atendedor_%d: ha ocurrido un error inerperado al procesar los datos de la solicitud_%d\n",*(int *)ptrs,id);
			sprintf(salida,"ha ocurrido un error inerperado al procesar los datos de la solicitud_%d",id);
			writeLogMessage(atendedor,salida);
		}else{
			printf("Atendedor_%d: el usuario de solicitud_%d posee antecedentes\n",*(int *)ptrs,id);
			sprintf(salida,"el usuario de solicitud_%d posee antecedentes",id);
			writeLogMessage(atendedor,salida);
			espera=rand()%5+6;
			apto==false;
		}
		sleep(espera);
		printf("Atendedor_%d: el tiempo necesario para atender la solicitud_%d ha sido %d\n",*(int *)ptrs,id,espera);
		sprintf(salida,"el tiempo necesario para atender la solicitud_%d ha sido %d",id,espera);
		writeLogMessage(atendedor,salida);
		pthread_mutex_lock(&mutexColaSolicitudes);
			if(apto==false){
				colaSolicitudes[posicion].atendido=3;
			} else{
				colaSolicitudes[posicion].atendido=2;
			}
		pthread_mutex_unlock(&mutexColaSolicitudes);
		if(cafe%5==0){
			printf("Atendedor_%d: procede a tomarse un cafe\n",*(int *)ptrs);
			sprintf(salida,"procede a tomarse un cafe");
			writeLogMessage(atendedor,salida);
			sleep(10);
		}
	}while(true);
}
void *accionesCoordinadorSocial(){

	pthread_mutex_lock(&mutexColaSocial);
	pthread_cond_wait(&condActividades, &mutexColaSocial);

	listaCerrada = true;
	writeLogMessage("", "Lista cerrada");

	pthread_t usuario1, usuario2, usuario3, usuario4;

	pthread_create(&usuario1, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[0]);
	pthread_create(&usuario2, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[1]);
	pthread_create(&usuario3, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[2]);
	pthread_create(&usuario4, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[3]);


	pthread_cond_wait(&condActividades, &mutexColaSocial);

	listaCerrada = false;
	writeLogMessage("", "Lista abierta de nuevo");

	pthread_mutex_unlock(&mutexColaSocial);
}
void *usuarioEnActividad(void *id){
	char idTemp[50];
	sprintf(id,"%d",*(int *)id);
	
	pthread_mutex_lock(&mutexColaSocial);	
		printf("[UsuarioID: %d -> Se ha unido al Tsunami]\n", *(int *)id);
		writeLogMessage(id, "Se ha unido al Tsunami");
	pthread_mutex_unlock(&mutexColaSocial);	
	
	sleep(3);

	pthread_mutex_lock(&mutexColaSocial);
		printf("[UsuarioID: %d -> Se ha marchado del Tsunami]\n", *(int *)id);
		writeLogMessage(id, "Se ha marchado del Tsunami");
	pthread_mutex_unlock(&mutexColaSocial);

	pthread_mutex_lock(&mutexColaSolicitudes);
	contadorActividadesCola--;
	if(contadorActividadesCola==0){
		pthread_cond_signal(&condActividades);
	}
	pthread_mutex_unlock(&mutexColaSolicitudes);
	//TODO Escribir en el log
	pthread_exit(NULL);
}

//Escribimos en el log
void writeLogMessage (char *id , char *msg) {
	int i;
	pthread_mutex_lock(&mutexLog);
	if(logFile==NULL){
		logFile = fopen ("log.txt", "w") ;
	} else {
		logFile = fopen ("log.txt", "a") ;
	}
	// Calculamos la hora actual
	time_t now = time (0) ;
	struct tm *tlocal = localtime (&now) ;
	char stnow [23];
	//strcpy(stnow,"");
	strftime (stnow, 23, " %d/ %m/ %y %H: %M: %S " , tlocal) ;
	// Escribimos en el log. La primera vez de cada ejecución, borrará el log.txt en caso de que exista.
	fprintf (logFile , "[%s] %s : %s\n" , stnow , id , msg) ;
	fclose (logFile);
	pthread_mutex_unlock(&mutexLog); 

}
void manejadoraSolicitud(int signal){
	nuevaSolicitud(signal);
		
}
void manejadoraFinalizar(int signal){
	//Temporal
	exit(0);
}
/**
 *@author Marcos Ferreras
 *Inicializa los campos de la estructura solicitud a 0.  
*/
void inicializarSolicitud(Solicitud* solicitud){
	strcpy(solicitud->id, "0");
	solicitud->atendido=0;
	solicitud->tipo=0;
	solicitud->hilo=0;
}
int sacarNumero(char *id){
	int numero=0;
	int posicion_=0, contador=0;
	for (int i=0;i<strlen(id);i++){
		if(id[i]>='0' && id[i]<='9'){
			numero=numero*10 + id[i] - '0';
		}
	}
	return numero;
}

