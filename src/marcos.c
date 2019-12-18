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
pthread_mutex_t mutexLog;
pthread_mutex_t mutexColaSolicitudes;
FILE *logFile;
typedef struct solicitud {
	char id[50];
	//0->Sin atender 1->En proceso de atención 2->Atendido
	int atendido;
	//Tipo 0->Invitación 1->QR
	int tipo;
	pthread_t hilo;
} Solicitud;
void inicializarSolicitud(Solicitud* solicitud);
//Cola de solicitudes
Solicitud colaSolicitudes[tamCola];
//Para la asignacion de ID a las solicitudes
int contadorSolicitudes;

/*
 *	Main
 */

int main(){
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
	logFile=NULL;
	for(i=0;i<tamCola;i++){
		inicializarSolicitud(&colaSolicitudes[i]);
	}
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexColaSolicitudes, NULL);
	//Encargados de las solicitudes de:
	pthread_t atendedor_1, atendedor_2, atendedor_3;
	//Creamos los hilos de los "usuarios destacados"
	pthread_create(&atendedor_1, NULL, accionesAtendedor, (void *) &tipoAtendedor[0]);
	pthread_create(&atendedor_2, NULL, accionesAtendedor, (void *) &tipoAtendedor[1]);
	pthread_create(&atendedor_3, NULL, accionesAtendedor, (void *) &tipoAtendedor[2]);
	//PRUEBA pthread_create(&atendedor_3, NULL, accionesSolicitud, (void *) &colaSolicitudes[0]);
	//PRUEBA sleep(5);
	//PRUEBA strcpy(colaSolicitudes[1].id, "1");
	//PRUEBA pthread_create(&atendedor_3, NULL, accionesSolicitud, (void *) &colaSolicitudes[1]);
	//Espera por señales de forma indefinda. No es espera activa!!!
	while(true){
		pause();
	}
}

/*
 *	Funciones
 */

/*//Funcion del hilo encargado de las invitaciones
void *despachoInvitador(void *ptr) {
	int i = 0;
	printf("Soy el encargado de las invitaciones\n");
	writeLogMessage("1111",Hola%d);
	writeLogMessage("1111","5");
	writeLogMessage("1111","6");
}

//Funcion del hilo encargado de los codigos QR
void *despachoQR(void *ptr) {
	//Números aleatorios
	srand(time(NULL));
	int num;
	//Genero disparo aleatorio
	num = (0+(rand()%(100)-0)+1);
	//Descarto el 40% de las invitaciones
	if(num <=40){
		//Poco fiable
		if(num <=30){
			printf("--Solicitud descartada por falta de fiabilidad--\n");	
		//Se canso de esperar
		}else{
			printf("--El solicitante se ha cansado de esperar--\n");	
		}
	}
	//Descarto un 15% (aplicacion mal hecha)
	num = (0+(rand()%(100)-0)+1);
	if(num <= 15){
		printf("--La solicitud ha sido rechazada con exito--\n");
	}
	printf("Soy el encargado de los codigos QR\n");
}

//Funcion del AtendedorPRO
void *despachoAtendedorPRO(void *ptr) {
	//Números aleatorios
	srand(time(NULL));
	int num;
	//Genero el numero aleatorio
	num = (0+(rand()%(100)-0)+1);
	printf("Soy el atendedorPRO\n");
}*/
void *accionesAtendedor(void *ptr){
	char cadena[100];
	//printf("gfdgxf\n");
	//writeLogMessage("sdfsdfsdfsdfsd","fdfsdfsdfsdf");

	
}
/**
 * Función que se encarga de las acciones de la solicitud
 * @autor Marcos Ferreras
*/
void *accionesSolicitud(void *structSolicitud){
	//char idSolicitud[50], cadenaLog[100];
	Solicitud* solicitud = (Solicitud *)structSolicitud;
	//0->Sin atender 1->En proceso de atención 2->Atendido
	int enAtencion=0;
	int aleatorio;
	int participo;
	
	//strcpy(solicitud->id,"1");
	//if(strcmp("0\0",solicitud->id)==0)	
	//	printf("Hola\n");
	
	//writeLogMessage("1","La solicitud se ha recibido");
	//printf("Hola\n");
	//writeLogMessage("1111","5");
	//TODO Escribir en log
	printf("ID %s : La solicitud de tipo %d ha sido registrada\n", solicitud->id, solicitud->tipo);
	printf("ID %s : La solicitud de tipo %d esta esperando (4 segundos)\n", solicitud->id, solicitud->tipo);
	sleep(4);
	pthread_mutex_lock(&mutexColaSolicitudes);
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
				printf("ID %s : La solicitud de tipo %d se ha eliminado por cansarse de esperar\n", solicitud->id, solicitud->tipo);
				inicializarSolicitud(solicitud);
				pthread_exit(NULL); 
			}
		//QR
		} else {
			if(aleatorio<=30){
				//TODO Escribir en log
				printf("ID %s : La solicitud de tipo %d se ha eliminado por no considerarse muy fiable\n", solicitud->id, solicitud->tipo);
				inicializarSolicitud(solicitud);
				pthread_exit(NULL); 
			}
		}
		//Descarte fallo aplicacion
		aleatorio = rand()%101;
		if(aleatorio<=15){
			//TODO Escribir en log
			printf("ID %s : La solicitud de tipo %d se ha eliminado por fallo de la aplicacion\n", solicitud->id, solicitud->tipo);
			inicializarSolicitud(solicitud);
			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&mutexColaSolicitudes);
		sleep(4);
		pthread_mutex_lock(&mutexColaSolicitudes);
		enAtencion=solicitud->atendido;
		
	}
	//enAtencion=solicitud->atendido;
	//Mientras me esten atendiendo esperaré. No he soltado el mutex cuando salí del bucle anterior.
	while(enAtencion==2){
		pthread_mutex_unlock(&mutexColaSolicitudes);
		sleep(2);
		pthread_mutex_lock(&mutexColaSolicitudes);
		enAtencion=solicitud->atendido;
	}
	//Al salir de este bucle no he soltado el mutex. En este punto lo desbloqueo, y decido si me uno a una actividad social o no.
	pthread_mutex_unlock(&mutexColaSolicitudes);
	participo=rand()%2;
	//0->No participo 1->Si participo
	
	
	
}

//Escribimos en el log
void writeLogMessage (char *id , char *msg) {
	pthread_mutex_lock(&mutexLog);
	if(logFile==NULL){
		logFile = fopen ("log.txt", "w") ;
	} else {
		logFile = fopen ("log.txt", "a") ;
	}
	// Calculamos la hora actual
	time_t now = time (0) ;
	struct tm *tlocal = localtime (&now) ;
	char stnow [19];
	strftime (stnow, 19, " %d/ %m/ %y %H: %M: %S " , tlocal) ;
	// Escribimos en el log. La primera vez de cada ejecución, borrará el log.txt en caso de que exista.
	fprintf (logFile , "[%s] %s : %s\n" , stnow , id , msg) ;
	fclose (logFile);
	pthread_mutex_unlock(&mutexLog); 

}
void manejadoraSolicitud(int signal){
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
	pthread_mutex_lock(&mutexColaSolicitudes);
	strcpy(solicitud->id, "0");
	solicitud->atendido=0;
	solicitud->tipo=0;
	solicitud->hilo=0;
	pthread_mutex_unlock(&mutexColaSolicitudes);
}


