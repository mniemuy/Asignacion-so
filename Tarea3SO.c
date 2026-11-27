#include <stdio.h>
#include <stdlib.h>
#include <math.h> //por el ceil hay que compilar con -lm al final, si no da error al linkear
#include <unistd.h>
#include <time.h> //para el srand con la hora, time(NULL) da los segundos desde 1970

typedef struct {
    int id; //así podemos saber en qué archivos estamos y qué y cuantos bloques ocupamoss
    int size;
    int *bloques;
    int nBloques;
    int bloqueIndice; //solo lo usa indexada, es el primer bloque donde van los punteros (-1 si no tiene)
    int nIndices; //24nov cuantos bloques índice usa, si no caben en uno se encadenan
} Archivo;

//para guardar y luego usar los archivos los voy a guardar en un arreglo dinámico
//osea poder usar realloc cuando sea necesario
//vi que también se podían representar como lista enlazada, pero alargaba mucho la tarea

typedef struct { //para representar el disco
    int ocupado; //bloque libre o ocupado? como un bitmap
    int fileid; // para saber qué archivo corresponde a cada bloque
    int siguiente; //para enlazada, a qué bloque apunta este, -1 si es el último
                   //en indexada lo uso igual pero para encadenar los bloques índice
} Bloque;

//recorre el bitmap contando los huecos (bloques libres seguidos), igual que el best fit
//mayorHueco y huecos van con puntero para que la función me pueda devolver más de un valor
double fragmentacionExterna(Bloque *disco, int bloques, int metodo,
                            int *mayorHueco, int *huecos) {
    int libresTotal = 0;
    int largo = 0; //largo del hueco que voy recorriendo

    *mayorHueco = 0;
    *huecos = 0;

    for (int i = 0; i < bloques; i++) {
        if (disco[i].ocupado == 0) {
            if (largo == 0) {
                (*huecos)++; //empieza un hueco nuevo
            }

            largo++;
            libresTotal++;

            if (largo > *mayorHueco) {
                *mayorHueco = largo;
            }
        } else {
            largo = 0; //se cortó el hueco
        }
    }

    if (metodo != 1 || libresTotal == 0) {
        return 0;
    }

    return 100.0 * (1.0 - (double)*mayorHueco / libresTotal);
}

int main() {
    int tdisco = 0; //tamaño del disco
    int tbloque = 0; //tamaño de cada bloque
    int metodo = 0; //para el método de asignación
    int tarchivo = 0; //tamaño de archivo
    int bloquesR = 0;
    int archivoMin = 0; //max y min archivo
    int archivoMax = 0;
    int capacidadArchivos = 10; // cuantos se pueden registrar al inicio
    int cantidadArchivos = 0; //cuantos están registrados actualmente
    int t = 0; //tiempo que lleva la simulación, ahora va de 1 en 1
    int creados = 0; //cuantos se crearon en total, ya no es lo mismo que cantidadArchivos porque ahora se borran
    int eliminados = 0; //cuantos se borraron
    int totalAccesos = 0; //suma de los accesos que se hicieron, para sacar el promedio
    int cantidadAccesos = 0; //cuantas veces se accedió a un archivo
    double sumaFrag = 0; //para el registro de la fragmentación externa: suma, máxima y cuantas mediciones
    double maxFrag = 0;
    int registrosFrag = 0;
    int mayorHueco = 0; //estos dos los llena la función de fragmentación
    int huecos = 0;

    srand(time(NULL));

    //24nov ahora pido los datos disco, bloque, método y al final el rango
    printf("Tamaño de disco en MB:\n");
    scanf(" %d", &tdisco);
    printf("Tamaño de bloques en KB:\n");
    scanf("%d", &tbloque);

    if (tdisco <= 0 || tbloque <= 0 ||
        tdisco > 1048576 || tbloque > 1048576 ||
        tdisco * 1024 < tbloque) {
        printf("Error, tamaño de disco o de bloque inválido\n");
        return 1;
    }

    //1048576 x 1024 = 1073741824
    //subí esto antes del ciclo, el método se elige una vez y sirve para todos los archivos
    printf("Elige un método de asignación\n");
    printf("1. Contigua\n"); //antes era escribir ahora opciones
    printf("2. Enlazada\n");
    printf("3. Indexada\n");
    scanf("%d", &metodo);

    if (metodo < 1 || metodo > 3) {
        printf("Método inválido, debe ser 1, 2 o 3\n"); //antes estaba al final en el else
        return 1;
    }

    //el rango lo bajé para acá, antes era lo primero que se pedía
    printf("Mínimo del archivo en KB?\n");
    scanf("%d", &archivoMin);

    printf("Máximo del archivo en KB    ?\n");
    scanf("%d", &archivoMax);

    if (archivoMin <= 0 || archivoMax < archivoMin) {
        printf("Rango mal especificado\n");
        return 1;
    }

    int bloques = (tdisco * 1024) / tbloque;
    int libres = bloques; //llevo la cuenta de los libres, así sé si falta espacio o es fragmentación

    printf("Cantidad de bloques: %d\n", bloques); //le faltaba un espacio

    //igual con un disco gigante y bloques chicos son demasiados bloques, el malloc puede pasar
    //pero después al recorrerlos se queda sin ram, así que pongo un tope de unos 16 millones
    if (bloques > 16777216) {
        printf("Son demasiados bloques, usar bloques más grandes\n");
        return 1;
    }

    Bloque *disco = malloc(bloques * sizeof(Bloque));
    Archivo *archivos = malloc(capacidadArchivos * sizeof(Archivo));

    if (disco == NULL || archivos == NULL) {
        printf("Error al reservar memoria\n");
        free(disco);
        free(archivos);
        return 1;
    }

    for (int i = 0; i < bloques; i++) {
        disco[i].ocupado = 0;
        disco[i].fileid = -1; //inicialización de campos como ninguno ocupado y todos los fileid en -1
        disco[i].siguiente = -1; //y que nadie apunte a nadie todavía
    }

    //18nov ya no es un solo archivo de prueba, ahora se crea uno cada 2 segundos con tamaño random
    //dentro del rango y sigue hasta que uno no quepa, ahí se termina todo
    //el while (1) da vueltas para siempre, se sale con break cuando un archivo no cabe
    //19nov ahora da una vuelta por segundo, porque las eliminaciones van cada 5 y el 35 o el 45 no son pares
    while (1) {
        //19nov después de los 30 segundos, cada 5 segundos se elimina un archivo random
        //(t - 30) % 5 == 0 se cumple en 30, 35, 40... osea cada 5 desde los 30
        if (t >= 30 && (t - 30) % 5 == 0) {
            if (cantidadArchivos == 0) {
                printf("\n[t=%ds] No hay archivos para eliminar\n", t);
            } else {
                int elegido = rand() % cantidadArchivos; //uno al azar entre los que están en el disco
                Archivo *e = &archivos[elegido];
                int liberados = e->nBloques;

                //sus bloques vuelven a 0 en el bitmap y quedan sin dueño
                for (int k = 0; k < e->nBloques; k++) {
                    disco[e->bloques[k]].ocupado = 0;
                    disco[e->bloques[k]].fileid = -1;
                    disco[e->bloques[k]].siguiente = -1; //en enlazada también se borra el puntero
                }

                //en indexada falta soltar los bloques índice, ahora pueden ser varios
                //así que voy siguiendo la cadena desde el primero hasta que llegue a -1
                int idx = e->bloqueIndice;

                while (idx != -1) {
                    int sig = disco[idx].siguiente; //lo guardo antes porque abajo lo borro
                    disco[idx].ocupado = 0;
                    disco[idx].fileid = -1;
                    disco[idx].siguiente = -1;
                    liberados++;
                    idx = sig;
                }

                libres += liberados;

                printf("\n[t=%ds] Se elimina el archivo %d (%d KB), se liberan %d bloques | libres: %d/%d\n",
                       t, e->id, e->size, liberados, libres, bloques);

                free(e->bloques); //su arreglo de bloques ya no sirve

                //para no dejar un hoyo en el arreglo el último archivo pasa a su lugar,
                //el orden da lo mismo porque cada uno tiene su id
                archivos[elegido] = archivos[cantidadArchivos - 1];
                cantidadArchivos--;
                eliminados++;

                //al borrar quedan huecos, así que acá también se mide la fragmentación
                double frag = fragmentacionExterna(disco, bloques, metodo, &mayorHueco, &huecos);
                //en enlazada e indexada sirve cualquier libre, la clase dice que no hay fragmentación externa

                sumaFrag += frag;
                registrosFrag++;

                if (frag > maxFrag) {
                    maxFrag = frag;
                }

                printf("Huecos libres: %d , mayor hueco: %d bloques , frag. externa: %.2f %%\n",
                       huecos, mayorHueco, frag);
            }
        }

        //23nov en los mismos segundos de la eliminación se accede a una posición de un archivo random
        if (t >= 30 && (t - 30) % 5 == 0) {
            if (cantidadArchivos == 0) {
                printf("[t=%ds] No hay archivos para acceder\n", t);
            } else {
                Archivo *x = &archivos[rand() % cantidadArchivos];

                //la posición lógica es un byte del archivo, entre 0 y su tamaño en bytes - 1
                //lo paso a long long porque size * 1024 se puede pasar del int con archivos grandes
                //El sorteo asume RAND_MAX = 2147483647
                int kbElegido = rand() % x->size;
                int byteElegido = rand() % 1024;
                long long posicion = (long long)kbElegido * 1024 + byteElegido;

                int datosPorBloque = tbloque * 1024;

                if (metodo == 2) {
                    datosPorBloque -= 4; //en enlazada cada bloque tiene 4 bytes menos por el puntero
                }

                int bloqueLogico = posicion / datosPorBloque; //en qué bloque del archivo cae la posición
                int desplazamiento = posicion % datosPorBloque; //y en qué byte de ese bloque
                int fisico = -1;
                int accesos = 0;
                int indiceUsado = -1; //solo para indexada, en qué bloque índice estaba el puntero
                int cualIndice = 0;

                if (metodo == 1) {
                    //contigua: inicio + bloque lógico, se llega directo con 1 acceso
                    fisico = x->bloques[0] + bloqueLogico;
                    accesos = 1;
                } else if (metodo == 2) {
                    //enlazada: parto del primero y voy siguiendo los punteros del disco,
                    //cada bloque que leo es un acceso, osea bloque lógico + 1 accesos
                    fisico = x->bloques[0];
                    accesos = 1;

                    for (int k = 0; k < bloqueLogico; k++) {
                        fisico = disco[fisico].siguiente;
                        accesos++;
                    }
                } else {
                    //cada índice guarda punteros - 1 bloques porque la última entrada apunta al siguiente índice
                    int porIndice = tbloque * 1024 / 4 - 1;
                    cualIndice = bloqueLogico / porIndice; //0 si está en el primero, 1 en el segundo...

                    int idx = x->bloqueIndice;
                    accesos = 1; //leer el primer índice

                    for (int k = 0; k < cualIndice; k++) {
                        idx = disco[idx].siguiente; //salto al siguiente índice, otro acceso
                        accesos++;
                    }

                    //la entrada bloqueLogico % porIndice de ese índice dice el bloque físico
                    fisico = x->bloques[bloqueLogico];
                    accesos++; //y leer el dato
                    indiceUsado = idx;
                }

                printf("[t=%ds] Acceso al archivo %d (%d KB):  posición lógica %lld B -> bloque lógico %d, byte %d -> bloque físico %d , accesos: %d\n",
                       t, x->id, x->size, posicion, bloqueLogico, desplazamiento, fisico, accesos);

                if (metodo == 3) {
                    printf("(el puntero estaba en el índice %d de %d del archivo, que es el bloque %d)\n",
                           cualIndice + 1, x->nIndices, indiceUsado);
                }

                totalAccesos += accesos;
                cantidadAccesos++;
            }
        }

        //los archivos se siguen creando cada 2 segundos, en los impares solo se espera
        //el continue se salta lo que queda del while y pasa a la siguiente vuelta
        if (t % 2 != 0) {
            sleep(1);
            t++;
            continue;
        }

        //acá entra el realloc que tenía pensado, si se llena el arreglo lo duplico
        //si falla devuelve NULL pero el viejo sigue ahí, por eso lo guardo primero en nuevo y no en archivos
        if (cantidadArchivos == capacidadArchivos) {
            capacidadArchivos *= 2;

            Archivo *nuevo = realloc(archivos, capacidadArchivos * sizeof(Archivo));

            if (nuevo == NULL) {
                printf("Error al reservar memoria\n");
                break;
            }

            archivos = nuevo;
        }

        //con este puntero me ahorro escribir archivos[cantidadArchivos] a cada rato
        //como a es puntero se usa -> en vez del punto, a->id es lo mismo que archivos[cantidadArchivos].id
        Archivo *a = &archivos[cantidadArchivos];
        a->id = creados + 1; //antes era cantidadArchivos + 1, pero al borrar se repetían los id

        //rand() % (archivoMax - archivoMin + 1) da algo entre 0 y archivoMax - archivoMin, al sumarle el mínimo queda dentro del rango
        a->size = archivoMin + rand() % (archivoMax - archivoMin + 1); // tamaño en KB
        a->bloques = NULL; // aún no tiene bloques asignados
        a->nBloques = 0;
        a->bloqueIndice = -1;
        a->nIndices = 0;

        int asignado = 0; //se pone en 1 si el método logra meter el archivo

        printf("\n[t=%ds] Archivo %d: %d KB\n", t, a->id, a->size);

        tarchivo = a->size; //estaba dentro del if de contigua y en indexada quedaba en 0, por eso siempre daba 1 bloque

        if (tarchivo > tdisco * 1024) {
            printf("El tamaño del archivo supera al del disco\n");
            break;
        }

        if (metodo == 1) {
            bloquesR = ceil((double)tarchivo / tbloque); //el double porque si no trunca antes de hacerle ceil 10/4 = 2 -> ceil(2) = 2
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
                        inicioActual = i; //y la longitud es 0, es una nueva cadena, hace que sea la posicion actual
                    }

                    longitudActual++; //le suma el lugar libre al contador
                } else { // si no está libre
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
                //si hay libres de sobra pero ningún hueco alcanza es fragmentación externa
                //si no, derechamente no hay espacio
                if (libres >= bloquesR) {
                    printf("Fragmentación externa: hay %d bloques libres pero no %d seguidos\n",
                           libres, bloquesR);
                } else {
                    printf("No cabe el archivo\n");
                }
            } else {
                printf("Best espacio comienza en: %d\n", mejorInicio);
                printf("Tamaño del espacio: %d bloques\n", mejorSize);

                //antes solo marcaba el disco, ahora el archivo también se acuerda de sus bloques
                a->bloques = malloc(bloquesR * sizeof(int));

                if (a->bloques == NULL) {
                    printf("Error reserva de memoria\n");
                    break;
                }

                a->nBloques = bloquesR;

                //mejorinicio + bloquesR da el final de la cadena + 1 entonces menor que
                for (int i = mejorInicio; i < mejorInicio + bloquesR; i++) {
                    disco[i].ocupado = 1;
                    disco[i].fileid = a->id;
                    a->bloques[i - mejorInicio] = i; //i - mejorInicio parte en 0, osea es el bloque lógico
                    //para todos los espacios requeridos los llena con 1nos y llena el fileid de los bloques
                }

                printf("Archivo %d ocupa los bloques %d a %d\n",
                       a->id, mejorInicio, mejorInicio + bloquesR - 1);

                libres -= bloquesR;
                asignado = 1;
            }
        } else if (metodo == 2) {
            //a este hay que tener en cuenta que el puntero ocupa 4 bytes entonces es lo mismo de arriba pero al tamaño de bloque le restas los 4 bytes del puntero
            //hecho, paso todo a bytes y a cada bloque le resto los 4 del puntero
            bloquesR = ceil(((double)tarchivo * 1024) / (tbloque * 1024 - 4));

            //acá no sirve el best fit porque no tienen que estar seguidos, tomo los primeros libres que pille
            //primero veo si alcanzan los libres, así no pido memoria para después tener que soltarla
            if (bloquesR > libres) {
                printf("No cabe el archivo\n");
            } else {
                a->bloques = malloc(bloquesR * sizeof(int));

                if (a->bloques == NULL) {
                    printf("Error reservando memoria\n");
                    break;
                }

                int encontrados = 0;

                for (int i = 0; i < bloques && encontrados < bloquesR; i++) {
                    if (disco[i].ocupado == 0) {
                        a->bloques[encontrados] = i;
                        encontrados++;
                    }
                }

                a->nBloques = bloquesR;

                for (int k = 0; k < bloquesR; k++) {
                    disco[a->bloques[k]].ocupado = 1;
                    disco[a->bloques[k]].fileid = a->id;

                    if (k + 1 < bloquesR) {
                        disco[a->bloques[k]].siguiente = a->bloques[k + 1]; //el bloque guarda a cuál sigue
                    }
                }

                // guardo todos en a->bloques 
                //pero la muestro siguiendo los punteros del disco y no el arreglo, como se leería de verdad (acceso secuencial)
                printf("Archivo %d (%d bloques): ", a->id, bloquesR);

                for (int b = a->bloques[0]; b != -1; b = disco[b].siguiente) {
                    //el ? : es un if corto: si hay siguiente imprime la flecha y si no el salto de línea
                    printf("%d%s", b, disco[b].siguiente != -1 ? "->" : "\n");
                }

                libres -= bloquesR;
                asignado = 1;
            }
        } else if (metodo == 3) {
            int datos = ceil((double)tarchivo / tbloque); //bloques de datos, ahora sí con el tamaño del archivo
            int punteros = tbloque * 1024 / 4; //cuantos punteros de 4 bytes caben en un bloque índice (los 4 bytes los asumo yo, son 32 bits, alcanza para numerar muchísimos bloques)

            //24nov antes si no cabía en un índice se terminaba todo, ahora se encadenan varios índices
            //la última entrada de cada índice apunta al siguiente, así que cada uno guarda punteros - 1 datos
            int porIndice = punteros - 1;
            int nIndices = (datos + porIndice - 1) / porIndice; //ceil con enteros

            bloquesR = datos + nIndices;

            if (bloquesR > libres) { //lo mismo que en enlazada, primero ver si alcanza
                printf("No cabe el archivo\n");
            } else {
                a->bloques = malloc(datos * sizeof(int));

                if (a->bloques == NULL) {
                    printf("Error de reserva de memoria\n");
                    break;
                }

                //también tomo los primeros libres, los primeros que encuentro quedan de índice
                //y el resto son datos, a->bloques vendría siendo lo que está escrito adentro de los índices
                int indicesListos = 0;
                int encontrados = 0;
                int anterior = -1; //el índice anterior, para encadenarlo con el nuevo

                for (int i = 0; i < bloques && encontrados < datos; i++) {
                    if (disco[i].ocupado == 0) {
                        if (indicesListos < nIndices) {
                            if (anterior == -1) {
                                a->bloqueIndice = i; //el primero queda en el archivo
                            } else {
                                disco[anterior].siguiente = i; //los demás se cuelgan del anterior
                            }

                            disco[i].ocupado = 1;
                            disco[i].fileid = a->id;
                            anterior = i;
                            indicesListos++;
                        } else {
                            a->bloques[encontrados] = i;
                            encontrados++;
                        }
                    }
                }

                a->nBloques = datos;
                a->nIndices = nIndices;

                //muestro los índices siguiendo la cadena, igual que en enlazada
                printf("Archivo %d: índice(s) ", a->id);

                for (int b = a->bloqueIndice; b != -1; b = disco[b].siguiente) {
                    printf("%d%s", b, disco[b].siguiente != -1 ? "->" : "");
                }

                printf("    datos: ");

                for (int k = 0; k < datos; k++) {
                    disco[a->bloques[k]].ocupado = 1;
                    disco[a->bloques[k]].fileid = a->id;
                    printf("%d ", a->bloques[k]);
                }

                printf("\n");

                libres -= bloquesR;
                asignado = 1;
            }
        }

        //si no se pudo meter el archivo se termina la simulación nomás
       if (!asignado) {
    break;
        }

        cantidadArchivos++; //ahora lo cuento solo si quedó asignado
        creados++;

        //cada vez que entra un archivo se mide la fragmentación externa y se guarda para el promedio
        double frag = fragmentacionExterna(disco, bloques, metodo, &mayorHueco, &huecos);
        //lo mismo que al eliminar, solo en contigua cuenta

        sumaFrag += frag;
        registrosFrag++;

        if (frag > maxFrag) {
            maxFrag = frag;
        }

        printf("Bloques libres: %d/%d , huecos: %d , mayor hueco: %d , frag. externa: %.2f %%\n",
               libres, bloques, huecos, mayorHueco, frag);

        sleep(1); //antes era sleep(2), ahora espera 1 segundo por vuelta, mientras duerme el proceso no ocupa cpu
        t++;
    }

    int ocupados = 0;

    for (int i = 0; i < bloques; i++) {
        ocupados += disco[i].ocupado;
    }

    if (ocupados != bloques - libres) {
        printf("El conteo de bloques libres no coincide\n");
    }

    //23nov ordené todo lo del final como estadísticas utilización del disco, creados y eliminados, fragmentación externa y promedio de accesos
    //sumo lo que pesan los archivos que quedaron y la fragmentación interna (lo que sobra en el último bloque)
    int datosPorBloque = tbloque * 1024;

    if (metodo == 2) {
        datosPorBloque -= 4; //igual que en el acceso, en enlazada cada bloque tiene 4 bytes menos
    }

    long long kbGuardados = 0;
    long long fragInterna = 0; //en bytes

    for (int i = 0; i < cantidadArchivos; i++) {
        kbGuardados += archivos[i].size;
        fragInterna += (long long)archivos[i].nBloques * datosPorBloque
                       - (long long)archivos[i].size * 1024;
    }

    double fragFinal = fragmentacionExterna(disco, bloques, metodo, &mayorHueco, &huecos);

    printf("\nESTADÍSTICAS\n");
    printf("Fin de la simulación en tiempo t=%ds\n", t);

    //el ? : de nuevo, para poner el nombre del método en vez del número
    printf("Método: %s , %d bloques de %d KB\n",
           metodo == 1 ? "contigua" : metodo == 2 ? "enlazada" : "indexada",
           bloques, tbloque);

    printf("\nArchivos\n");

    //ahora que se borran archivos muestro creados y eliminados aparte de los que quedaron
    printf("Creados: %d , eliminados: %d , en disco: %d\n",
           creados, eliminados, cantidadArchivos);

    printf("\nUso del disco\n");
    printf("Bloques ocupados: %d/%d (%.2f %%)\n",
           ocupados, bloques, 100.0 * ocupados / bloques);
    printf("Datos guardados: %lld KB de %d KB que tiene el disco en bloques\n",
           kbGuardados, bloques * tbloque);
    printf("Fragmentación interna: %lld bytes sin usar en los últimos bloques\n",
           fragInterna);

    printf("\nFragmentación externa\n");

    //el promedio y la máxima salen del registro que se fue llenando en cada creación y eliminación
    printf("Al final: %.2f %% , promedio: %.2f %% , máxima: %.2f %% (%d mediciones)\n",
           fragFinal, registrosFrag > 0 ? sumaFrag / registrosFrag : 0.0,
           maxFrag, registrosFrag);
    printf("Huecos libres: %d , mayor hueco: %d bloques\n", huecos, mayorHueco);

    if (metodo != 1) {
        printf("en enlazada e indexada no hay fragmentación externa, cualquier bloque libre sirve\n");
    }

    printf("\n Accesos \n");

    //el promedio solo si hubo accesos, si se termina antes de los 30 segundos no hay ninguno y sería dividir por 0
    if (cantidadAccesos > 0) {
        printf("Accesos: %d , promedio de accesos para encontrar el bloque: %.2f\n",
               cantidadAccesos, (double)totalAccesos / cantidadAccesos);
    } else {
        printf("Accesos: 0, se demoró menos de 30 segundos\n");
    }

    printf(".................................\n");

    //ahora hay que liberar los bloques de cada archivo
    for (int i = 0; i < cantidadArchivos; i++) {
        free(archivos[i].bloques);
    }

    free(archivos);
    free(disco);

    return 0;
}