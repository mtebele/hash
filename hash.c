#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define TAM_INICIAL 128

/*
 * NOMBRE: Matias Tebele | PADRON: 95372
 * NOMBRE: Lucas Perea | PADRON: 95368
 */

/******************************************************************
 *                IMPLEMENTACION DE LOS TIPOS DE DATOS
 ******************************************************************/

typedef struct nodo_hash {
    const char* clave;
    void* dato;
    status_t estado;
} nodo_hash_t;

struct hash {
    size_t cantidad;
    size_t tamanio;
    hash_destruir_dato_t destruir_dato;
    nodo_hash_t** tabla;
};

struct hash_iter {
    const hash_t* hash;
    size_t pos;
};

/******************************************************************
 *                		FUNCIONES AUXILIARES
 ******************************************************************/
 
 // Función hash DJB2
size_t fhash(const char* clave, size_t tam)
{
   unsigned long number = 5381;
   int c;
   while ((c = *clave++))
	   number = ((number << 5) + number) + c;
   return number % tam;
}

// Clona una cadena con memoria dinámica
char *strdup(const char *s)
{
    char *n = malloc(strlen(s) + 1);
    if(n == NULL) return NULL;
    strcpy(n, s);
    return n;
}

// Crea un nodo
nodo_hash_t* nodo_hash_crear(const char *clave, void *dato)
{
	nodo_hash_t* nodo = malloc(sizeof(nodo_hash_t));
	if (nodo == NULL) return NULL;
	nodo->clave = strdup(clave);
	nodo->dato = dato;
	nodo->estado = OCUPADO;
	return nodo;
}

// Obtiene la próxima posición a insertar
size_t hash_obtener_posicion(hash_t* hash, const char* clave)
{
	size_t pos = fhash(clave, hash->tamanio);
	
	// Mientras exista el nodo y esté ocupado, avanzo de posición
	while (hash->tabla[pos] && hash->tabla[pos]->estado == OCUPADO) {
		if (strcmp(hash->tabla[pos]->clave, clave) == 0)
			return pos;
		pos++;
		// Si se termina la tabla, vuelvo a comenzar.
		if (pos >= hash->tamanio) pos = 0;
	}
	return pos;
}

// Redimensiona el hash
bool hash_redimensionar(hash_t *hash, size_t tamanio)
{
	nodo_hash_t** tabla = calloc(tamanio, sizeof(nodo_hash_t));
	if (tabla == NULL) return false;
		
	// Re-hasheo. 
	for (int i = 0; i < hash->tamanio; i++) {
		if (hash->tabla[i]) {
			if (hash->tabla[i]->estado == OCUPADO) {				
				const char* clave = hash->tabla[i]->clave;
				size_t pos = fhash(clave, tamanio);
				while (tabla[pos]) {
					pos++;
					if (pos >= tamanio) pos = 0;
				}
				tabla[pos] = hash->tabla[i];
			}
			else {
				free((char*)hash->tabla[i]->clave);
				free(hash->tabla[i]);
			}
		}
	}
	
	nodo_hash_t** aux = hash->tabla;
	hash->tabla = tabla;
	hash->tamanio = tamanio;
	free(aux);
	return true;
}

/******************************************************************
 *                IMPLEMENTACION DE LAS PRIMITIVAS
 ******************************************************************/

hash_t *hash_crear(hash_destruir_dato_t destruir_dato)
{
	hash_t* hash = malloc(sizeof(hash_t));
	if (hash == NULL) return NULL;
	hash->tabla = calloc(TAM_INICIAL, sizeof(nodo_hash_t));
	if (hash->tabla == NULL) {
		free(hash);
		return NULL;
	}
	hash->cantidad = 0;
	hash->tamanio = TAM_INICIAL;
	hash->destruir_dato = destruir_dato;
	return hash;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato)
{
	// Si estoy usando mas de 3/4, redimensiono al doble.
	if (hash->cantidad >= hash->tamanio * 3/4)
		if (!hash_redimensionar(hash, hash->tamanio * 2)) return false;

	size_t pos = hash_obtener_posicion(hash, clave);
	if (hash_pertenece(hash, clave)) {
		if (hash->destruir_dato != NULL) 
			hash->destruir_dato(hash->tabla[pos]->dato);
		hash->tabla[pos]->dato = dato;
	}		
	else {
		nodo_hash_t* nodo_nuevo = nodo_hash_crear(clave, dato);
		hash->tabla[pos] = nodo_nuevo;
		hash->cantidad++;
	}
	return true;
}

void *hash_borrar(hash_t *hash, const char *clave)
{
	if (hash_cantidad(hash) == 0) return NULL;
		
	if (hash->cantidad < (hash->tamanio / 4) && (hash->tamanio / 2) >= TAM_INICIAL)				
		hash_redimensionar(hash, hash->tamanio / 2);	
	
	size_t pos = fhash(clave, hash->tamanio);
	while (hash->tabla[pos] && hash->tabla[pos]->estado != VACIO) {	
		bool encontreClave = (strcmp(hash->tabla[pos]->clave, clave) == 0);	
		if (hash->tabla[pos]->estado == OCUPADO && encontreClave) {
			hash->tabla[pos]->estado = BORRADO;
			hash->cantidad--;
			void* dato = hash->tabla[pos]->dato;
			
			// Si estoy usando menos de 1/4, redimensiono a la mitad.
			if (hash->cantidad < (hash->tamanio / 4) && (hash->tamanio / 2) >= TAM_INICIAL)				
				hash_redimensionar(hash, hash->tamanio / 2);

			return dato;
		}
		pos++;
		if (pos >= hash->tamanio) pos = 0;
	}
	return NULL;
}

void *hash_obtener(const hash_t *hash, const char *clave)
{
	if (hash_cantidad(hash) == 0) return NULL;
	size_t pos = fhash(clave, hash->tamanio);
	while (hash->tabla[pos] && hash->tabla[pos]->estado != VACIO) {
		bool encontreClave = (strcmp(hash->tabla[pos]->clave, clave) == 0);
		if (hash->tabla[pos]->estado == OCUPADO && encontreClave)
			return hash->tabla[pos]->dato;
		pos++;
	}
	return NULL;
}

bool hash_pertenece(const hash_t *hash, const char *clave)
{
	return (hash_obtener(hash, clave) != NULL);
}

size_t hash_cantidad(const hash_t *hash)
{
	return hash->cantidad;
}

void hash_destruir(hash_t *hash)
{
	for (int i = 0; i < hash->tamanio; i++) {
		if (hash->tabla[i]) {
			if (hash->destruir_dato != NULL) 
				hash->destruir_dato(hash->tabla[i]->dato);
			free((char*)hash->tabla[i]->clave);
			free(hash->tabla[i]);
		}
	}
	free(hash->tabla);
	free(hash);
}

hash_iter_t *hash_iter_crear(const hash_t *hash)
{
	hash_iter_t* hash_iter = malloc(sizeof(hash_iter_t));
	if (hash_iter == NULL) return NULL;
	hash_iter->hash = hash;
	if (hash_cantidad(hash) == 0)
		hash_iter->pos = hash->tamanio;
	else {
		int i = 0;
		while ((!hash->tabla[i] || hash->tabla[i]->estado != OCUPADO) && (i < hash->tamanio))
			i++;
		hash_iter->pos = i;
	}
	return hash_iter;
}

bool hash_iter_avanzar(hash_iter_t *iter)
{
	if (hash_iter_al_final(iter)) return false;
	iter->pos++;
	while ((!iter->hash->tabla[iter->pos] || iter->hash->tabla[iter->pos]->estado != OCUPADO)
			&& (iter->pos < iter->hash->tamanio))
		iter->pos++;
	return true;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter)
{
	return (hash_iter_al_final(iter)) ? NULL : iter->hash->tabla[iter->pos]->clave;
}

bool hash_iter_al_final(const hash_iter_t *iter)
{
	return (iter->pos == iter->hash->tamanio);
}

void hash_iter_destruir(hash_iter_t* iter)
{
	free(iter);
}
