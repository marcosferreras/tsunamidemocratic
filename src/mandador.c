#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
void ejecutar(int senial1,int senial2,int alternar,int pid);
void menu(int pid);
int main(int argc, char *argv[]){

//Función principal.
	if(argv[1] == NULL){
		//Error
		printf("El número de argumentos no coincide con los solicitados\n");
		printf("Introduzca:\n");
		printf("./mandador pid\n");
	}else{
		//Defino las variables que voy a utilizar
		//Asigno el pid
		int pid = atoi(argv[1]);
		menu(pid);
	}
	return 0;
}

void menu(int pid){
int senial1 = 0,senial2 = 0,alternar = 0, defecto = 0;
	printf("Prefiere probar con los valores predefinidos?\n");
	printf("1- Si\n2- No\n");
	scanf("%d",&alternar);
	if(defecto == 2){
		printf("Indique el numero de señales SIGUSR1\n");
		scanf("%d",&senial1);
		printf("Indique el numero de señales SIGUSR2\n");
		scanf("%d",&senial2);
		printf("Desea altenarlas?\n");
		printf("1- Si\n2- No\n");
		scanf("%d",&alternar);
		ejecutar(senial1,senial2,alternar,pid);
	}else{
		printf("Valores por defecto,30,30,1\n");
		ejecutar(30,30,1,pid);
	}
}

void ejecutar(int senial1,int senial2,int alternar,int pid){
	int i = 0;
	int max = senial1+senial2;
	char cmd1[100];
	char cmd2[100];
	sprintf(cmd1,"kill -10 %d",pid);
	sprintf(cmd2,"kill -12 %d",pid);
	int valor1 = 0,valor2 = 0;
	for(i = 0;i < max;i++){
		if(i < senial1){
			valor1 = system(cmd1);
		}
		if(i < senial2){
			valor2 = system(cmd2);
		}
	}





}
