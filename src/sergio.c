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
void *accionesCoordinadorSocial(void *ptr);
void writeLogMessage (char *id , char *msg);

pthread_mutex_t mutexLog;
pthread_mutex_t mutexColaSolicitudes;
pthread_mutex_t mutexColaSocial;

FILE *logFile;

typedef struct actividad{
	char idUsuario[50];
	pthread_t hilo;
} Actividad;

void inicializarActividad(Actividad* actividad);
Actividad colaActividadSocial[4];

//Para saber cuantos hay en cola
int contadorActividadesCola;
//Para saber 
int listaCerrada;

pthread_cond_t condicion, usuariosActividad[4];

/*
 *	Main
 */
int main(){

	int i;

	contadorActividadesCola=0;
	listaCerrada=false;
	logFile=NULL;

	for(i=0;i<4;i++){
		inicializarActividad(&colaActividadSocial[i]);
	}

	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexColaSolicitudes, NULL);
	//Encargados de las solicitudes de:
	pthread_t atendedor_1, atendedor_2, atendedor_3, coordinador;
	//Creamos el hilo del coordinador
	pthread_create(&coordinador, NULL, accionesCoordinadorSocial, NULL);

	// inicializamos las condiciones de espera de los hilos
	for(i = 0; i < 4; i++){
		if (pthread_cond_init(&usuariosActividad[i], NULL)!=0) exit(-1);
	}

	// inicializamos la condicion de espera del hilo principal
	if (pthread_cond_init(&condicion, NULL)!=0) exit(-1);
	
	listaCerrada = true;
	pthread_t usuario1, usuario2, usuario3, usuario4;

	pthread_mutex_lock(&mutexColaSocial);
	pthread_create(&usuario1, NULL, accionesCoordinadorSocial, (void *) &colaActividadSocial[0].idUsuario);
	pthread_create(&usuario2, NULL, accionesCoordinadorSocial, (void *) &colaActividadSocial[1].idUsuario);
	pthread_create(&usuario3, NULL, accionesCoordinadorSocial, (void *) &colaActividadSocial[2].idUsuario);
	pthread_create(&usuario4, NULL, accionesCoordinadorSocial, (void *) &colaActividadSocial[3].idUsuario);

	//pthread_cond_wait(&condicion, &mutexColaSocial);---------------------------------------
	printf("Va a comenzar la actividad\n");
	pthread_cond_signal(&usuariosActividad[0]);
	pthread_mutex_unlock(&mutexColaSocial);
}

void *accionesCoordinadorSocial(void *ptr){
	int i;
	pthread_mutex_lock(&mutexColaSocial);	
	
	// Comprobamos si ha finalizado su hilo precedente
	pthread_cond_wait(&usuariosActividad[*(char *)ptr-1], &mutexColaSocial);

	printf("[UsuarioID: %d -> Ha iniciado la actividad]\n", *(char *)ptr);

	// enviamos señal de finalización del hilo
	pthread_cond_signal(&usuariosActividad[*(char *)ptr]);

	pthread_mutex_unlock(&mutexColaSocial);
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

/**
 *@author Marcos Ferreras
*/
void inicializarActividad(Actividad* actividad){
	pthread_mutex_lock(&mutexColaSocial);
	strcpy(actividad->idUsuario, "0");
	pthread_mutex_unlock(&mutexColaSocial);
}
