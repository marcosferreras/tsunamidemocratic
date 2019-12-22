//Resumen
/*
Dudas:
*/
/*
Trabajando en:
	Escribir los logs
*/
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

//Estructuras

//Estructura de actividad
typedef struct actividad{
	char idUsuario[50];
	pthread_t hilo;
} Actividad;
//Estructura de solicitud
typedef struct solicitud {
	char id[50];
	//0->Sin atender 1->En proceso de atención 2->Atendido 3->Atendido con Antecedentes
	int atendido;
	//Tipo 0->Invitación 1->QR
	int tipo;
	pthread_t hilo;
} Solicitud;


//Vector con el identificador de los atendedores
int id[3]={1,2,3};
//Defino las funciones
void *accionesAtendedor(void *ptr);
void *accionesSolicitud(void *ptr);
void *accionesCoordinadorSocial(void *ptr);
void manejadoraSolicitud(int signal);
void manejadoraFinalizar(int signal);
void writeLogMessage (char *id , char *msg);
void inicializarSolicitud(Solicitud* solicitud);
void inicializarActividad(Actividad* actividad);
//Cola de solicitudes
Solicitud colaSolicitudes[tamCola];
Actividad colaActividadSocial[4];
//Para la asignacion de ID a las solicitudes
int contadorSolicitudes;
//Para saber cuantos hay en cola
int contadorSolicitudesCola;
int contadorActividadesCola;
//Para saber 
int listaCerrada;
//Mutex
pthread_mutex_t mutexLog;
pthread_mutex_t mutexColaSolicitudes;
pthread_mutex_t mutexColaSocial;
//Archivo para los logs
FILE *logFile;
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
	contadorSolicitudesCola=0;
	contadorActividadesCola=0;
	listaCerrada=false;
	logFile=NULL;
	for(i=0;i<tamCola;i++){
		inicializarSolicitud(&colaSolicitudes[i]);
	}
	for(i=0;i<4;i++){
		inicializarActividad(&colaActividadSocial[i]);
	}
	//Inicializo los mutex
	pthread_mutex_init(&mutexColaSocial,NULL);
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexColaSolicitudes, NULL);
	//Encargados de las solicitudes:
	pthread_t atendedor_1, atendedor_2, atendedor_3;
	//Creamos los hilos de los atendedores
	pthread_create(&atendedor_1, NULL, accionesAtendedor, (void *) &tipoAtendedor[0]);
	pthread_create(&atendedor_2, NULL, accionesAtendedor, (void *) &tipoAtendedor[1]);
	pthread_create(&atendedor_3, NULL, accionesAtendedor, (void *) &tipoAtendedor[2]);
	//Creamos el hilo del coordinador
	pthread_create(&coordinador, NULL, accionesCoordinadorSocial, NULL);
	sleep(1);
}

/*
 *	Funciones
 */

//Funcion del hilo encargado de las invitaciones
void *despachoInvitador(void *ptr) {
	printf("Soy el encargado de las invitaciones\n");
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
