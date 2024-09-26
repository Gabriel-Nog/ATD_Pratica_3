#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// item C
#define BUF_SIZE 3 // tamanho do buffer

typedef struct {
  int buf[BUF_SIZE];     // definindo o buffer
  size_t len;            // controla o número de itens do buffer
  pthread_mutex_t mutex; // variável mutex
  pthread_cond_t
      can_produce; // variável condicional indicando que item(s) foram removidos
  pthread_cond_t can_consume; // variável condicional indicando que item(s)
                              // foram adicionados
} buffer_t;

void *producer(void *arg) {

  /*Lê os argumentos do buffer_t inicializado no main*/
  buffer_t *buffer = (buffer_t *)arg;

  /*Bloqueia o mutex para que nenhum outro thread entre na região crítica*/
  pthread_mutex_lock(&buffer->mutex);

  /* testa se o buffer está cheio */
  while (buffer->len == BUF_SIZE) {
    // é bloqueado e espera por um sinal que itens foram removidos
    printf("Buffer cheio, produtor aguardando...\n");
    pthread_cond_wait(&buffer->can_produce, &buffer->mutex);
  }

  // Se o buffer não está cheio, produz o item aleatoriamente
  // Rand gera um número aleatório entre 0 e 10
  int item = rand() % 11;
  printf("Produzido: %d\n", item);

  // insere item no buffer
  buffer->buf[buffer->len] = item;
  ++buffer->len; // incrementa o contador

  // sinaliza p/ consumidor despertar que um item foi produzido
  pthread_cond_signal(&buffer->can_consume);
  // desbloqueia o mutex (libera a trava)
  pthread_mutex_unlock(&buffer->mutex);

  return NULL;
}

void *consumer(void *arg) {

  /*Lê os argumentos do buffer_t inicializado no main*/
  buffer_t *buffer = (buffer_t *)arg;

  /*Bloqueia o mutex para que nenhum outro thread entre na região crítica*/
  pthread_mutex_lock(&buffer->mutex);

  /* testa se o buffer está vazio */
  while (buffer->len == 0) {
    // é bloqueado e espera por um sinal que itens foram produzidos
    printf("Buffer vazio, consumidor aguardando...\n");
    pthread_cond_wait(&buffer->can_consume, &buffer->mutex);
  }

  // decrementa o contador
  --buffer->len;
  printf("Consumido: %d\n", buffer->buf[buffer->len]);

  // sinaliza p/ produtor despertar que um item foi consumido
  pthread_cond_signal(&buffer->can_produce);

  // desbloqueia o mutex (libera a trava)
  pthread_mutex_unlock(&buffer->mutex);

  return NULL;
}

int main(int argc, char *argv[]) {
  // Para m produtores e n consumidores
  int m, n;
  // Produtor
  printf("Qual o total de produtores?\n");
  scanf("%d", &m);
  // Consumidor
  printf("Qual o total de consumidores?\n");
  scanf("%d", &n);
  // Iniciando..
  printf("Para %d produtores e %d consumidores\n", m, n);
  // inicializa elementos do buffer_t
  buffer_t buffer = {.len = 0,
                     .mutex = PTHREAD_MUTEX_INITIALIZER,
                     .can_produce = PTHREAD_COND_INITIALIZER,
                     .can_consume = PTHREAD_COND_INITIALIZER};

  pthread_t prod[m], cons[n]; // declara produtor consumidor

  for (int i = 0; i < m; i++) {
    pthread_create(&prod[i], NULL, producer, (void *)&buffer); // cria produtor
  }
  for (int i = 0; i < n; i++) {
    pthread_create(&cons[i], NULL, consumer, (void *)&buffer); // cria
                                                               // consumidor
  }

  for (int i = 0; i < m; i++) {
    pthread_join(prod[i], NULL); // espera o thread produtor terminar
  }

  for (int i = 0; i < n; i++) {
    pthread_join(cons[i], NULL); // espera o thread consumidor terminar
  }

  printf("Programa finalizado\n");
  return 0;
}