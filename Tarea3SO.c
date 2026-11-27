#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    int id;
    int size;
    int *bloques;
    int nBloques;
    int bloqueIndice;
    int nIndices;
} Archivo;

typedef struct {
    int ocupado;
    int fileid;
    int siguiente;
} Bloque;

double fragmentacionExterna(Bloque *disco, int bloques, int metodo,
                            int *mayorHueco, int *huecos) {
    int libresTotal = 0;
    int largo = 0;

    *mayorHueco = 0;
    *huecos = 0;

    for (int i = 0; i < bloques; i++) {
        if (disco[i].ocupado == 0) {
            if (largo == 0) {
                (*huecos)++;
            }

            largo++;
            libresTotal++;

            if (largo > *mayorHueco) {
                *mayorHueco = largo;
            }
        } else {
            largo = 0;
        }
    }

    if (metodo != 1 || libresTotal == 0) {
        return 0;
    }

    return 100.0 * (1.0 - (double)*mayorHueco / libresTotal);
}

int main() {
    int tdisco = 0;
    int tbloque = 0;
    int metodo = 0;
    int tarchivo = 0;
    int bloquesR = 0;
    int archivoMin = 0;
    int archivoMax = 0;
    int capacidadArchivos = 10;
    int cantidadArchivos = 0;
    int t = 0;
    int creados = 0;
    int eliminados = 0;
    int totalAccesos = 0;
    int cantidadAccesos = 0;
    double sumaFrag = 0;
    double maxFrag = 0;
    int registrosFrag = 0;
    int mayorHueco = 0;
    int huecos = 0;

    srand(time(NULL));

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

    printf("Elige un método de asignación\n");
    printf("1. Contigua\n");
    printf("2. Enlazada\n");
    printf("3. Indexada\n");
    scanf("%d", &metodo);

    if (metodo < 1 || metodo > 3) {
        printf("Método inválido, debe ser 1, 2 o 3\n");
        return 1;
    }

    printf("Mínimo del archivo en KB?\n");
    scanf("%d", &archivoMin);

    printf("Máximo del archivo en KB    ?\n");
    scanf("%d", &archivoMax);

    if (archivoMin <= 0 || archivoMax < archivoMin) {
        printf("Rango mal especificado\n");
        return 1;
    }

    int bloques = (tdisco * 1024) / tbloque;
    int libres = bloques;

    printf("Cantidad de bloques: %d\n", bloques);

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
        disco[i].fileid = -1;
        disco[i].siguiente = -1;
    }

    while (1) {
        if (t >= 30 && (t - 30) % 5 == 0) {
            if (cantidadArchivos == 0) {
                printf("\n[t=%ds] No hay archivos para eliminar\n", t);
            } else {
                int elegido = rand() % cantidadArchivos;
                Archivo *e = &archivos[elegido];
                int liberados = e->nBloques;

                for (int k = 0; k < e->nBloques; k++) {
                    disco[e->bloques[k]].ocupado = 0;
                    disco[e->bloques[k]].fileid = -1;
                    disco[e->bloques[k]].siguiente = -1;
                }

                int idx = e->bloqueIndice;

                while (idx != -1) {
                    int sig = disco[idx].siguiente;
                    disco[idx].ocupado = 0;
                    disco[idx].fileid = -1;
                    disco[idx].siguiente = -1;
                    liberados++;
                    idx = sig;
                }

                libres += liberados;

                printf("\n[t=%ds] Se elimina el archivo %d (%d KB), se liberan %d bloques | libres: %d/%d\n",
                       t, e->id, e->size, liberados, libres, bloques);

                free(e->bloques);

                archivos[elegido] = archivos[cantidadArchivos - 1];
                cantidadArchivos--;
                eliminados++;

                double frag = fragmentacionExterna(disco, bloques, metodo, &mayorHueco, &huecos);

                sumaFrag += frag;
                registrosFrag++;

                if (frag > maxFrag) {
                    maxFrag = frag;
                }

                printf("Huecos libres: %d , mayor hueco: %d bloques , frag. externa: %.2f %%\n",
                       huecos, mayorHueco, frag);
            }
        }

        if (t >= 30 && (t - 30) % 5 == 0) {
            if (cantidadArchivos == 0) {
                printf("[t=%ds] No hay archivos para acceder\n", t);
            } else {
                Archivo *x = &archivos[rand() % cantidadArchivos];

                int kbElegido = rand() % x->size;
                int byteElegido = rand() % 1024;
                long long posicion = (long long)kbElegido * 1024 + byteElegido;

                int datosPorBloque = tbloque * 1024;

                if (metodo == 2) {
                    datosPorBloque -= 4;
                }

                int bloqueLogico = posicion / datosPorBloque;
                int desplazamiento = posicion % datosPorBloque;
                int fisico = -1;
                int accesos = 0;
                int indiceUsado = -1;
                int cualIndice = 0;

                if (metodo == 1) {
                    fisico = x->bloques[0] + bloqueLogico;
                    accesos = 1;
                } else if (metodo == 2) {
                    fisico = x->bloques[0];
                    accesos = 1;

                    for (int k = 0; k < bloqueLogico; k++) {
                        fisico = disco[fisico].siguiente;
                        accesos++;
                    }
                } else {
                    int porIndice = tbloque * 1024 / 4 - 1;
                    cualIndice = bloqueLogico / porIndice;

                    int idx = x->bloqueIndice;
                    accesos = 1;

                    for (int k = 0; k < cualIndice; k++) {
                        idx = disco[idx].siguiente;
                        accesos++;
                    }

                    fisico = x->bloques[bloqueLogico];
                    accesos++;
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

        if (t % 2 != 0) {
            sleep(1);
            t++;
            continue;
        }

        if (cantidadArchivos == capacidadArchivos) {
            capacidadArchivos *= 2;

            Archivo *nuevo = realloc(archivos, capacidadArchivos * sizeof(Archivo));

            if (nuevo == NULL) {
                printf("Error al reservar memoria\n");
                break;
            }

            archivos = nuevo;
        }

        Archivo *a = &archivos[cantidadArchivos];
        a->id = creados + 1;
        a->size = archivoMin + rand() % (archivoMax - archivoMin + 1);
        a->bloques = NULL;
        a->nBloques = 0;
        a->bloqueIndice = -1;
        a->nIndices = 0;

        int asignado = 0;

        printf("\n[t=%ds] Archivo %d: %d KB\n", t, a->id, a->size);

        tarchivo = a->size;

        if (tarchivo > tdisco * 1024) {
            printf("El tamaño del archivo supera al del disco\n");
            break;
        }

        if (metodo == 1) {
            bloquesR = ceil((double)tarchivo / tbloque);

            int inicioActual = -1;
            int longitudActual = 0;
            int mejorInicio = -1;
            int mejorSize = bloques + 1;

            for (int i = 0; i < bloques; i++) {
                if (disco[i].ocupado == 0) {
                    if (longitudActual == 0) {
                        inicioActual = i;
                    }

                    longitudActual++;
                } else {
                    if (longitudActual >= bloquesR && longitudActual < mejorSize) {
                        mejorSize = longitudActual;
                        mejorInicio = inicioActual;
                    }

                    longitudActual = 0;
                }
            }

            if (longitudActual >= bloquesR && longitudActual < mejorSize) {
                mejorSize = longitudActual;
                mejorInicio = inicioActual;
            }

            if (mejorInicio == -1) {
                if (libres >= bloquesR) {
                    printf("Fragmentación externa: hay %d bloques libres pero no %d seguidos\n",
                           libres, bloquesR);
                } else {
                    printf("No cabe el archivo\n");
                }
            } else {
                printf("Best espacio comienza en: %d\n", mejorInicio);
                printf("Tamaño del espacio: %d bloques\n", mejorSize);

                a->bloques = malloc(bloquesR * sizeof(int));

                if (a->bloques == NULL) {
                    printf("Error reserva de memoria\n");
                    break;
                }

                a->nBloques = bloquesR;

                for (int i = mejorInicio; i < mejorInicio + bloquesR; i++) {
                    disco[i].ocupado = 1;
                    disco[i].fileid = a->id;
                    a->bloques[i - mejorInicio] = i;
                }

                printf("Archivo %d ocupa los bloques %d a %d\n",
                       a->id, mejorInicio, mejorInicio + bloquesR - 1);

                libres -= bloquesR;
                asignado = 1;
            }
        } else if (metodo == 2) {
            bloquesR = ceil(((double)tarchivo * 1024) / (tbloque * 1024 - 4));

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
                        disco[a->bloques[k]].siguiente = a->bloques[k + 1];
                    }
                }

                printf("Archivo %d (%d bloques): ", a->id, bloquesR);

                for (int b = a->bloques[0]; b != -1; b = disco[b].siguiente) {
                    printf("%d%s", b, disco[b].siguiente != -1 ? "->" : "\n");
                }

                libres -= bloquesR;
                asignado = 1;
            }
        } else if (metodo == 3) {
            int datos = ceil((double)tarchivo / tbloque);
            int punteros = tbloque * 1024 / 4;
            int porIndice = punteros - 1;
            int nIndices = (datos + porIndice - 1) / porIndice;

            bloquesR = datos + nIndices;

            if (bloquesR > libres) {
                printf("No cabe el archivo\n");
            } else {
                a->bloques = malloc(datos * sizeof(int));

                if (a->bloques == NULL) {
                    printf("Error de reserva de memoria\n");
                    break;
                }

                int indicesListos = 0;
                int encontrados = 0;
                int anterior = -1;

                for (int i = 0; i < bloques && encontrados < datos; i++) {
                    if (disco[i].ocupado == 0) {
                        if (indicesListos < nIndices) {
                            if (anterior == -1) {
                                a->bloqueIndice = i;
                            } else {
                                disco[anterior].siguiente = i;
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

        if (!asignado) {
            break;
        }

        cantidadArchivos++;
        creados++;

        double frag = fragmentacionExterna(disco, bloques, metodo, &mayorHueco, &huecos);

        sumaFrag += frag;
        registrosFrag++;

        if (frag > maxFrag) {
            maxFrag = frag;
        }

        printf("Bloques libres: %d/%d , huecos: %d , mayor hueco: %d , frag. externa: %.2f %%\n",
               libres, bloques, huecos, mayorHueco, frag);

        sleep(1);
        t++;
    }

    int ocupados = 0;

    for (int i = 0; i < bloques; i++) {
        ocupados += disco[i].ocupado;
    }

    if (ocupados != bloques - libres) {
        printf("El conteo de bloques libres no coincide\n");
    }

    int datosPorBloque = tbloque * 1024;

    if (metodo == 2) {
        datosPorBloque -= 4;
    }

    long long kbGuardados = 0;
    long long fragInterna = 0;

    for (int i = 0; i < cantidadArchivos; i++) {
        kbGuardados += archivos[i].size;
        fragInterna += (long long)archivos[i].nBloques * datosPorBloque
                       - (long long)archivos[i].size * 1024;
    }

    double fragFinal = fragmentacionExterna(disco, bloques, metodo, &mayorHueco, &huecos);

    printf("\nESTADÍSTICAS\n");
    printf("Fin de la simulación en tiempo t=%ds\n", t);

    printf("Método: %s , %d bloques de %d KB\n",
           metodo == 1 ? "contigua" : metodo == 2 ? "enlazada" : "indexada",
           bloques, tbloque);

    printf("\nArchivos\n");
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
    printf("Al final: %.2f %% , promedio: %.2f %% , máxima: %.2f %% (%d mediciones)\n",
           fragFinal, registrosFrag > 0 ? sumaFrag / registrosFrag : 0.0,
           maxFrag, registrosFrag);
    printf("Huecos libres: %d , mayor hueco: %d bloques\n", huecos, mayorHueco);

    if (metodo != 1) {
        printf("en enlazada e indexada no hay fragmentación externa, cualquier bloque libre sirve\n");
    }

    printf("\n Accesos \n");

    if (cantidadAccesos > 0) {
        printf("Accesos: %d , promedio de accesos para encontrar el bloque: %.2f\n",
               cantidadAccesos, (double)totalAccesos / cantidadAccesos);
    } else {
        printf("Accesos: 0, se demoró menos de 30 segundos\n");
    }

    printf(".................................\n");

    for (int i = 0; i < cantidadArchivos; i++) {
        free(archivos[i].bloques);
    }

    free(archivos);
    free(disco);

    return 0;
}