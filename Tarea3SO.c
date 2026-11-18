#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <time.h> //para la semilla de rand()


typedef struct {
    int id;   //así podemos saber en qué archivos estamos y qué y cuantos bloques ocupamoss
    int size;
    int *bloques;
    int nBloques;
    int bloqueIndice; //nuevo: solo para indexada, el bloque que guarda los punteros, -1 si no tiene
} Archivo;

//para guardar y luego usar los archivos los voy a guardar en un arreglo dinámico
//osea poder usar realloc cuando sea necesario
//vi que también se podían representar como lista enlazada, pero alargaba mucho la tarea

typedef struct { //para representar el disco
int ocupado; //bloque libre o ocupado? como un bitmap
int fileid;   // para saber qué archivo corresponde a cada bloque
int siguiente; //nuevo: para enlazada, el puntero que guarda el bloque hacia el siguiente, -1 si es el último
} Bloque;

int main(){

int tdisco = 0; //tamaño del disco
int tbloque = 0; //tamaño de cada bloque
int metodo = 0; //para el método de asignación
int tarchivo = 0; //tamaño de archivo
int bloquesR = 0; 
int archivoMin = 0;  //max y min archivo
int archivoMax = 0;
int capacidadArchivos = 10;// cuantos se pueden registrar al inicio
int cantidadArchivos = 0; //cuantos están registrados actualmente 

srand(time(NULL)); //nuevo: para que el tamaño del archivo cambie en cada ejecución


printf("Mínimo del archivo?\n");
scanf("%d", &archivoMin);

printf("Máximo del archivo?\n");
scanf("%d", &archivoMax);

if(archivoMin <= 0 || archivoMax < archivoMin){
    printf("Error de rango\n");
    return 1;
}

printf("Tamaño de disco en MB:\n");
scanf(" %d",&tdisco);
printf("Tamaño de bloques en KB:\n");
scanf("%d",&tbloque);

//para sacar la cantidad de bloques voy a pasarlo todo a KB, multiplicar por 1024 es mejor que dividir por 1024, ahorras decimales
if(tdisco <= 0 || tbloque <= 0 || tdisco*1024 < tbloque){
    printf("Error: tamaño de disco o de bloque inválido\n"); //antes salía sin avisar
    return 1; //antes era -1, pero el código de salida tiene que ir de 0 a 255
}

int bloques = (tdisco*1024)/tbloque;
//acá podemos ocupar arreglos o malloc

//según leí si el usuario se pasa con el tamaño del disco a varios GB puede provocar un stack overflow
//el heap normalmente puede crecer más, entonces podemos usar memoria dinámica en vez de un arreglo
printf("Cantidad de bloques: %d\n", bloques); //le faltaba el espacio antes del número

Bloque *disco = malloc(bloques * sizeof(Bloque));

Archivo *archivos = malloc(capacidadArchivos * sizeof(Archivo));

if (disco == NULL || archivos == NULL) {
    printf("Error de reserva de memoria\n");
    free(disco);
    free(archivos);
    return 1;
}

for (int i = 0; i < bloques; i++) {
    disco[i].ocupado = 0;
    disco[i].fileid = -1;  //inicialización de campos como ninguno ocupado y todos los fileid en -1
    disco[i].siguiente = -1; //y ningún bloque apunta a otro todavía
}
///////////////////ARCHIVO DE PRUEBA///////////////////////////////////////////////////
//ya no es de 12 KB fijo: el tamaño sale al azar dentro del rango que se pidió al inicio
archivos[0].id = 1;
archivos[0].size = archivoMin + rand() % (archivoMax - archivoMin + 1); // tamaño en KB
archivos[0].bloques = NULL;     // aún no tiene bloques asignados
archivos[0].nBloques = 0;
archivos[0].bloqueIndice = -1;

cantidadArchivos++;


printf("Archivo %d: %d KB\n",archivos[0].id, archivos[0].size);


////////////////////////////////////////////////////////////////////////////////después

printf("Elige un método de asignación\n");
printf("1. Contigua\n"); //antes era escribir ahora opciones
printf("2. Enlazada\n");
printf("3. Indexada\n");
scanf("%d", &metodo);

tarchivo = archivos[0].size; //lo saqué del if de contigua, en indexada quedaba en 0 y siempre daba 1 bloque

if(metodo == 1){
    bloquesR = ceil((double)tarchivo/tbloque); //el double porque si no trunca antes de hacerle ceil 10/4 = 2 -> ceil(2) = 2
//revisar las siguientes bloquesr posiciones

//17nov ahora puedo continuar con esto, usaré best fit
//si voy recorriendo el disco buscando cadenas de "bits" libres entonces puedo encontrar la cadena 
//que le da el mejor fit, con un recorrido debería bastar
    int inicioActual = -1;
    int longitudActual = 0;
    int mejorInicio = -1;
    int mejorSize = bloques + 1;

    for (int i = 0; i < bloques; i++) {

        if (disco[i].ocupado == 0) { //si está libre
            if (longitudActual == 0) {  
                inicioActual = i;  //y la longitud es 0, es una nueva cadena, hace que sea la posicion actual
            }
            longitudActual++; //le suma el lugar libre al contador
        }
        else { // si no está libre
            if (longitudActual >= bloquesR && longitudActual < mejorSize) { 
                // si la longitud actual es igual o mayor a los bloques que se necesitan reservar
                //y la longitud actual es menor que el mejor tamaño
                mejorSize = longitudActual; //nos quedamos con el actual
                mejorInicio = inicioActual;
            }

            longitudActual = 0; //encontró un bloque ocupado, evalúa el espacio y reinicia el counter
        }
    }

    //comprobar el último espacio si termina con espacios libres,  ya que nunca encontrará
    //otro 1
    if (longitudActual >= bloquesR && longitudActual < mejorSize) {
        mejorSize = longitudActual;
        mejorInicio = inicioActual;
    }

    //resultado
    if (mejorInicio == -1) {
        printf("No hay espacio para el archivo\n");
    }
    else {
        printf("Best espacio comienza en: %d\n", mejorInicio);
        printf("Tamaño del espacio: %d bloques\n", mejorSize);

        //antes solo marcaba el disco, ahora el archivo también guarda qué bloques usa
        archivos[0].bloques = malloc(bloquesR * sizeof(int));
        if (archivos[0].bloques == NULL) {
            printf("Error de reserva de memoria\n");
            free(archivos);
            free(disco);
            return 1;
        }
        archivos[0].nBloques = bloquesR;

//mejorinicio + bloquesR da el final de la cadena + 1 entonces menor que

    for (int i = mejorInicio; i < mejorInicio + bloquesR; i++) { 
        disco[i].ocupado = 1;
        disco[i].fileid = archivos[0].id;
        archivos[0].bloques[i - mejorInicio] = i; //posición i - mejorInicio = bloque lógico
        //para todos los espacios requeridos los llena con 1nos y llena el fileid de los bloques
        }
        printf("Archivo %d ocupa los bloques %d a %d\n", archivos[0].id, mejorInicio, mejorInicio + bloquesR - 1);
    }






}else if(metodo == 2){

    //a este hay que tener en cuenta que el puntero ocupa 4 bytes entonces es lo mismo de arriba pero al tamaño de bloque le restas los 4 bytes del puntero
    //ahora sí: lo paso todo a bytes, cada bloque deja tbloque*1024 - 4 bytes para datos
    bloquesR = ceil(((double)tarchivo*1024)/(tbloque*1024 - 4));

    //acá no hace falta best fit, los bloques no tienen que estar seguidos,
    //así que tomo los primeros libres que encuentre
    archivos[0].bloques = malloc(bloquesR * sizeof(int));
    if (archivos[0].bloques == NULL) {
        printf("Error de reserva de memoria\n");
        free(archivos);
        free(disco);
        return 1;
    }
    int encontrados = 0;
    for (int i = 0; i < bloques && encontrados < bloquesR; i++) {
        if (disco[i].ocupado == 0) {
            archivos[0].bloques[encontrados] = i;
            encontrados++;
        }
    }

    if (encontrados < bloquesR) {
        printf("No hay espacio para el archivo\n");
        free(archivos[0].bloques);
        archivos[0].bloques = NULL;
    }
    else {
        archivos[0].nBloques = bloquesR;
        for (int k = 0; k < bloquesR; k++) {
            disco[archivos[0].bloques[k]].ocupado = 1;
            disco[archivos[0].bloques[k]].fileid = archivos[0].id;
            if (k + 1 < bloquesR) {
                disco[archivos[0].bloques[k]].siguiente = archivos[0].bloques[k + 1]; //el puntero al siguiente
            }
        }
        //muestro la cadena siguiendo los punteros del disco, como se leería de verdad
        printf("Archivo %d (%d bloques): ", archivos[0].id, bloquesR);
        for (int b = archivos[0].bloques[0]; b != -1; b = disco[b].siguiente) {
            printf("%d%s", b, disco[b].siguiente != -1 ? "->" : "\n");
        }
    }

}else if(metodo == 3){
    bloquesR = ceil((double)tarchivo/tbloque) + 1; //+1 por el bloque índice, ahora sí con el tamaño real
    int datos = bloquesR - 1;
    int punteros = tbloque * 1024 / 4; //cuantos punteros de 4 bytes caben en el bloque índice

    if (datos > punteros) {
        printf("El archivo no cabe en un solo bloque índice (%d punteros)\n", punteros);
    }
    else {
        archivos[0].bloques = malloc(datos * sizeof(int));
        if (archivos[0].bloques == NULL) {
            printf("Error de reserva de memoria\n");
            free(archivos);
            free(disco);
            return 1;
        }
        //igual que en enlazada tomo los primeros libres:
        //el primero que encuentro queda como bloque índice y los demás son de datos
        int encontrados = -1; //-1 significa que todavía falta el índice
        for (int i = 0; i < bloques && encontrados < datos; i++) {
            if (disco[i].ocupado == 0) {
                if (encontrados == -1) {
                    archivos[0].bloqueIndice = i;
                } else {
                    archivos[0].bloques[encontrados] = i;
                }
                encontrados++;
            }
        }

        if (encontrados < datos) {
            printf("No hay espacio para el archivo\n");
            free(archivos[0].bloques);
            archivos[0].bloques = NULL;
            archivos[0].bloqueIndice = -1;
        }
        else {
            archivos[0].nBloques = datos;
            disco[archivos[0].bloqueIndice].ocupado = 1;
            disco[archivos[0].bloqueIndice].fileid = archivos[0].id;
            printf("Archivo %d: índice en el bloque %d | datos: ", archivos[0].id, archivos[0].bloqueIndice);
            for (int k = 0; k < datos; k++) {
                disco[archivos[0].bloques[k]].ocupado = 1;
                disco[archivos[0].bloques[k]].fileid = archivos[0].id;
                printf("%d ", archivos[0].bloques[k]);
            }
            printf("\n");
        }
    }
}else{
    printf("Método inválido, debe ser 1, 2 o 3\n"); //antes quedaba vacío
}

//cuento cuántos quedaron ocupados para revisar que el bitmap quedó bien
int ocupados = 0;
for (int i = 0; i < bloques; i++) {
    ocupados += disco[i].ocupado;
}
printf("Archivos registrados: %d | bloques ocupados: %d/%d\n", cantidadArchivos, ocupados, bloques);


free(archivos[0].bloques); //ahora el archivo tiene memoria propia, hay que liberarla
free(archivos);
free(disco);
    return 0;
}
