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
void *accionesCoordinadorSocial();
void *usuarioEnActividad();
void writeLogMessage (char *id , char *msg);

pthread_mutex_t mutexLog;
pthread_mutex_t mutexColaSolicitudes;
pthread_mutex_t mutexColaSocial;

pthread_cond_t condActividades;
pthread_cond_t condUsuarios;
pthread_cond_t usuarioInicio[5];

FILE *logFile;

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

	int i;

	contadorActividadesCola=4;//el contador se suma en marcos.c
	listaCerrada=false;
	logFile=NULL;
	// inicializamos la condicion de espera del hilo del coordinador	
	//if (pthread_cond_init(&usuario[i], NULL)!=0) exit(-1);
	// inicializamos las condiciones de espera de los usuarios
	//for(i=0; i<=4; i++)
	//	if (pthread_cond_init(&usuarioInicio[i], NULL)!=0) exit(-1);

	for(i=0;i<4;i++){
		idUsuariosActividad[i]=0;
	}

	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexColaSocial,NULL);

	//Encargado de la actividad
	pthread_t coordinador;

	//Creamos el hilo del coordinador
	pthread_create(&coordinador, NULL, accionesCoordinadorSocial, NULL);
	sleep(3);//simula la practica
	//llamada d marcos.c
	pthread_cond_signal(&condActividades);

	while(true){
		pause();
	}
}

void *accionesCoordinadorSocial(){
	int i, M = 0;
	int idUsuariosActividad[4] = {1,2,3,4};

	pthread_mutex_lock(&mutexColaSocial);
	pthread_cond_wait(&condActividades, &mutexColaSocial);

	listaCerrada = true;

	pthread_t usuario1, usuario2, usuario3, usuario4;

	pthread_create(&usuario1, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[0]);
	pthread_create(&usuario2, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[1]);
	pthread_create(&usuario3, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[2]);
	pthread_create(&usuario4, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[3]);

	printf("Tsunami democratico iniciado\n\n");
	pthread_cond_wait(&condActividades, &mutexColaSocial);
	sleep(1);
	printf("\nTsunami democratico finalizado existosamente\n\n");

	listaCerrada = false;

	pthread_mutex_unlock(&mutexColaSocial);
}

void *usuarioEnActividad(void *id){

	printf("[UsuarioID: %d -> Se ha unido al Tsunami]\n", *(int *)id);
	sleep(3);
	printf("[UsuarioID: %d -> Se ha marchado del Tsunami]\n", *(int *)id);

	pthread_mutex_lock(&mutexColaSocial);	
	contadorActividadesCola--;
	if(contadorActividadesCola==0){
		pthread_cond_signal(&condActividades);
	}
	//TODO Escribir en el log
	pthread_mutex_unlock(&mutexColaSocial);	
	pthread_exit(NULL);
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
