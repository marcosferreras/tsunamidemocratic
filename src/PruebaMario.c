#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define tamCola 15
#define true 1
#define false 0

typedef struct solicitud {
	char id[50];
	int atendido;
	int tipo;
	pthread_t hilo;
} Solicitud;
Solicitud colaSolicitudes[tamCola];

pthread_mutex_t mutexLog;
pthread_mutex_t mutexColaSolicitudes;
pthread_mutex_t mutexColaSocial;

int contadorSolicitudes;
int contadorSolicitudesCola=15;
int contadorActividadesCola;
int listaCerrada;

void inicializarSolicitud(Solicitud* solicitud);
void eliminar(int n);


int main(){
	int tipoAtendedor[3]={1,2,3};
	srand(time(NULL));
	for(int i=0;i<tamCola;i++){
		sprintf(colaSolicitudes[i].id, "Solicitud_%d", i);
		colaSolicitudes[i].tipo=rand()%2+1;
	}
	printf("Datos Base:\n");
	for(int i=0;i<contadorSolicitudesCola;i++){
		printf("%s\n",colaSolicitudes[i].id);
		printf("tipo: %d\n",colaSolicitudes[i].tipo);
		printf("\n");
	}
	eliminar(4);
	printf("Datos nuevos\n\t==x==\n");
	for(int i=0;i<contadorSolicitudesCola;i++){
		printf("%s\n",colaSolicitudes[i].id);
		printf("tipo: %d\n",colaSolicitudes[i].tipo);
		printf("\n");
	}
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

void eliminar(int n){
	pthread_mutex_lock(&mutexColaSolicitudes);
	for(int i=n;i<contadorSolicitudesCola-1;i++){
		colaSolicitudes[i]=colaSolicitudes[i+1];
	}
	contadorSolicitudesCola--;
	pthread_mutex_unlock(&mutexColaSolicitudes);
	inicializarSolicitud(&colaSolicitudes[tamCola]);
}



















