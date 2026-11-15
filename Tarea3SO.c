#include <stdio.h>
#include <unistd.h>
#include <string.h>




int main(){
int tdisco = 0;
int tbloque = 0;
char metodo[10] = "";

printf("Tamaño de disco en MB:");
scanf("%d",&tdisco);
printf("Tamaño de bloques en KB:");
scanf("%d",&tbloque);
printf("Asignación:");
scanf(" %s",metodo);

printf("Has elegido para el tamaño del disco: %d\n", tdisco);
printf("Para cada bloque: %d\n", tbloque);
printf("Y el método es: %s\n", metodo);

printf("Hola");
    return 0;
}