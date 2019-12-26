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
int contadorSolicitudesCola;
int contadorActividadesCola;
int listaCerrada;

void *accionesAtendedor(void * a);
void inicializarSolicitud(Solicitud* solicitud);
int sacarNumero(char *id);
void writeLogMessage (char *id , char *msg);

int main(){
	int tipoAtendedor[3]={1,2,3};
	srand(time(NULL));
	for(int i=0;i<tamCola;i++){
		sprintf(colaSolicitudes[i].id, "Solicitud_%d", rand()%50+1);
		colaSolicitudes[i].tipo=rand()%2+1;
	}
	pthread_t atendedor_1, atendedor_2, atendedor_3;
	pthread_create(&atendedor_1, NULL, accionesAtendedor, (void *) &tipoAtendedor[0]);
	pthread_create(&atendedor_2, NULL, accionesAtendedor, (void *) &tipoAtendedor[1]);
	pthread_create(&atendedor_3, NULL, accionesAtendedor, (void *) &tipoAtendedor[2]);
	sleep(10);
}


void *accionesAtendedor(void *ptrs){
	int aux, num, contador=0, calculo, espera, apto, posicion=0, cafe=0;	
	//printf("Exito %d", *(int *)ptrs);
	do{
		apto=true;
		cafe++;
		do{
			if(contador%2==0){
				aux=*(int *)ptrs;
				pthread_mutex_lock(&mutexColaSolicitudes);
			}		
			contador++;
			num=-1;
			for(int i=0;i<tamCola;i++){
				if((colaSolicitudes[i].tipo==aux || aux==3) && colaSolicitudes[i].atendido==0){
					if(num>sacarNumero(colaSolicitudes[i].id) || num==-1){
						num=sacarNumero(colaSolicitudes[i].id);
						posicion=i;
					}
				}
			}
			if(num==-1){
				aux=3;
				if(contador%2==0){
					pthread_mutex_unlock(&mutexColaSolicitudes);
					sleep(1);
			}
			}
		}while(num==-1);
		colaSolicitudes[posicion].atendido==1;
		pthread_mutex_unlock(&mutexColaSolicitudes);
		srand(time(NULL));
		calculo=rand()%10+1;
		if(calculo<=7){
				espera=rand()%4+1;
		}else if(calculo<=9){
			espera=rand()%5+2;
		}else{
			espera=rand()%5+6;
			apto==false;
		}
		sleep(espera);
		pthread_mutex_lock(&mutexColaSolicitudes);
			if(apto==false){
				inicializarSolicitud(&colaSolicitudes[posicion]);
			} else{
				colaSolicitudes[posicion].atendido==2;
			}
		pthread_mutex_unlock(&mutexColaSolicitudes);
		if(cafe%5==0) sleep(10);
	}while(true);
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








































