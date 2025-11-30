#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Configurações e Constantes ---
#define TAM_FILA 5
#define TAM_PILHA 3

// Tipos de peças do Tetris
const char TIPOS_PECAS[] = {'I', 'J', 'L', 'O', 'S', 'T', 'Z'};

// --- Estruturas de Dados ---

typedef struct {
    int id;
    char tipo;
} Peca;

typedef struct {
    Peca itens[TAM_FILA];
    int inicio;
    int fim;
    int total; // Auxiliar para facilitar contagem
} FilaCircular;

typedef struct {
    Peca itens[TAM_PILHA];
    int topo;
} Pilha;

// Estrutura para salvar o estado (Para o "Desfazer")
typedef struct {
    FilaCircular fila;
    Pilha pilha;
    int pontuacao;
} EstadoJogo;

// --- Variáveis Globais de Jogo ---
int idGerador = 1; // Auto-incremento para IDs
int pontuacao = 0;

// --- Protótipos ---
void inicializarFila(FilaCircular* f);
void inicializarPilha(Pilha* p);
Peca gerarPeca();
void enfileirarAutomatico(FilaCircular* f); // Enfileira nova peça random
Peca desenfileirar(FilaCircular* f); // Tira da frente
int empilhar(Pilha* p, Peca peca); // Retorna 1 se ok, 0 se cheia
int desempilhar(Pilha* p, Peca* pecaSaiu); // Retorna 1 se ok, 0 se vazia
void trocarFrenteTopo(FilaCircular* f, Pilha* p);
void inverterFila(FilaCircular* f);
void salvarEstado(EstadoJogo* backup, FilaCircular f, Pilha p, int pts);
void restaurarEstado(EstadoJogo* backup, FilaCircular* f, Pilha* p, int* pts);
void exibirInterface(FilaCircular f, Pilha p);

// --- Função Principal ---
int main() {
    srand(time(NULL));

    FilaCircular fila;
    Pilha pilha;
    EstadoJogo backup;
    int temBackup = 0; // Flag para saber se pode desfazer

    // Inicialização
    inicializarFila(&fila);
    inicializarPilha(&pilha);

    // Preenche a fila inicial
    for(int i=0; i<TAM_FILA; i++) {
        enfileirarAutomatico(&fila);
    }

    int opcao;
    do {
        exibirInterface(fila, pilha);
        printf("\n[1] Jogar (Prox)\n");
        printf("[2] Reservar (Pilha)\n");
        printf("[3] Usar Reserva\n");
        printf("[4] Trocar (Fila <-> Pilha)\n");
        printf("[5] Desfazer Ultima Jogada\n");
        printf("[6] Inverter Fila\n");
        printf("[0] Sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);

        // Salva estado antes de qualquer modificação (exceto Sair e Desfazer)
        if (opcao >= 1 && opcao <= 4 || opcao == 6) {
            salvarEstado(&backup, fila, pilha, pontuacao);
            temBackup = 1;
        }

        switch(opcao) {
            case 1: // Jogar
                desenfileirar(&fila); // Tira a peça da frente (Joga)
                enfileirarAutomatico(&fila); // Repõe no fim
                pontuacao += 10;
                break;

            case 2: // Reservar
                if (pilha.topo < TAM_PILHA - 1) {
                    Peca p = desenfileirar(&fila);
                    empilhar(&pilha, p);
                    enfileirarAutomatico(&fila); // Fila nunca fica vazia
                    printf("Peca reservada!\n");
                } else {
                    printf("ERRO: Pilha de reserva cheia!\n");
                    // Se deu erro, restauramos o backup para não contar como jogada válida
                    restaurarEstado(&backup, &fila, &pilha, &pontuacao); 
                }
                break;

            case 3: // Usar Reserva
                {
                    Peca p;
                    if (desempilhar(&pilha, &p)) {
                        printf("Voce usou a peca reservada [%c #%d]\n", p.tipo, p.id);
                        pontuacao += 15;
                    } else {
                        printf("ERRO: Nenhuma peca reservada!\n");
                        restaurarEstado(&backup, &fila, &pilha, &pontuacao);
                    }
                }
                break;

            case 4: // Trocar
                trocarFrenteTopo(&fila, &pilha);
                break;

            case 5: // Desfazer
                if (temBackup) {
                    restaurarEstado(&backup, &fila, &pilha, &pontuacao);
                    printf("Jogada desfeita!\n");
                    temBackup = 0; // Evita desfazer infinito (apenas 1 nível)
                } else {
                    printf("Nada para desfazer.\n");
                }
                break;
            
            case 6: // Inverter
                inverterFila(&fila);
                pontuacao -= 5; // Custo estratégico
                break;

            case 0:
                printf("Encerrando Tetris Stack...\n");
                break;
            default:
                printf("Opcao invalida.\n");
        }

        // Pequena pausa visual (opcional)
        // system("pause"); // Windows
        // getchar(); 

    } while(opcao != 0);

    return 0;
}

// --- Implementação das Funções ---

void inicializarFila(FilaCircular* f) {
    f->inicio = 0;
    f->fim = -1; // Começa antes do 0
    f->total = 0;
}

void inicializarPilha(Pilha* p) {
    p->topo = -1; // Pilha vazia
}

Peca gerarPeca() {
    Peca p;
    p.id = idGerador++;
    int idx = rand() % 7;
    p.tipo = TIPOS_PECAS[idx];
    return p;
}

void enfileirarAutomatico(FilaCircular* f) {
    // Lógica Circular: (fim + 1) % TAM
    f->fim = (f->fim + 1) % TAM_FILA;
    f->itens[f->fim] = gerarPeca();
    
    if (f->total < TAM_FILA) f->total++;
}

Peca desenfileirar(FilaCircular* f) {
    Peca p = f->itens[f->inicio];
    // Lógica Circular
    f->inicio = (f->inicio + 1) % TAM_FILA;
    f->total--; // Teoricamente diminui, mas no jogo logo em seguida inserimos outra
    return p;
}

int empilhar(Pilha* p, Peca peca) {
    if (p->topo >= TAM_PILHA - 1) return 0; // Cheia
    p->topo++;
    p->itens[p->topo] = peca;
    return 1;
}

int desempilhar(Pilha* p, Peca* pecaSaiu) {
    if (p->topo < 0) return 0; // Vazia
    *pecaSaiu = p->itens[p->topo];
    p->topo--;
    return 1;
}

// Funcionalidade Mestre: Troca direta entre estruturas
void trocarFrenteTopo(FilaCircular* f, Pilha* p) {
    if (p->topo < 0) {
        printf("Erro: Precisa ter peca na reserva para trocar.\n");
        return;
    }
    
    // Swap
    Peca aux = f->itens[f->inicio];
    f->itens[f->inicio] = p->itens[p->topo];
    p->itens[p->topo] = aux;
    
    printf("Troca realizada com sucesso!\n");
}

// Funcionalidade Mestre: Inverte a ordem visual da fila
void inverterFila(FilaCircular* f) {
    Peca temp[TAM_FILA];
    
    // 1. Copia para array temporário na ordem correta
    int idx = f->inicio;
    for(int i = 0; i < TAM_FILA; i++) {
        temp[i] = f->itens[idx];
        idx = (idx + 1) % TAM_FILA;
    }
    
    // 2. Devolve para a fila original de trás pra frente
    // Reinicializa índices para simplificar
    f->inicio = 0;
    f->fim = TAM_FILA - 1;
    
    for(int i = 0; i < TAM_FILA; i++) {
        f->itens[i] = temp[(TAM_FILA - 1) - i]; // Inverte
    }
    printf("Fila Invertida!\n");
}

// Salva cópia exata das structs
void salvarEstado(EstadoJogo* backup, FilaCircular f, Pilha p, int pts) {
    backup->fila = f;
    backup->pilha = p;
    backup->pontuacao = pts;
}

void restaurarEstado(EstadoJogo* backup, FilaCircular* f, Pilha* p, int* pts) {
    *f = backup->fila;
    *p = backup->pilha;
    *pts = backup->pontuacao;
}

void exibirInterface(FilaCircular f, Pilha p) {
    printf("\n==================================\n");
    printf(" PONTUACAO: %d\n", pontuacao);
    printf("==================================\n");
    
    // Exibir Pilha (Reserva)
    printf(" [ RESERVA (Pilha) ]\n");
    if (p.topo == -1) printf("    (Vazio)\n");
    else {
        for(int i = p.topo; i >= 0; i--) {
            printf("    | [%c #%02d] |\n", p.itens[i].tipo, p.itens[i].id);
        }
    }
    printf("    +--------+\n\n");
    
    // Exibir Fila (Próximas Peças)
    printf(" [ PROXIMAS (Fila Circular) ]\n");
    printf(" SAIDA -> ");
    
    int count = 0;
    int i = f.inicio;
    while (count < TAM_FILA) { // Mostra sempre 5 pois está sempre cheia
        // Marca visual para a primeira peça (que será jogada)
        if (count == 0) printf("{%c #%02d} ", f.itens[i].tipo, f.itens[i].id);
        else printf("[%c #%02d] ", f.itens[i].tipo, f.itens[i].id);
        
        i = (i + 1) % TAM_FILA;
        count++;
    }
    printf("<- ENTRADA\n");
    printf("==================================\n");
}
