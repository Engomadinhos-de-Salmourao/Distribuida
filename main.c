#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Definição da struct
typedef struct {
    int codigo;
    char *nome;
    float preco;
    int quantidade;
} Produto;

// Funções obrigatórias (você define os parâmetros)
void adicionar_produto(Produto** prods, int* tam);
void listar_produtos(Produto* prods, int tam);
Produto* buscar_produto(Produto* prods, int tam, int code);
void atualizar_estoque(Produto* prods, int tam);
void remover_produto(Produto** prods, int* tam);
void liberar_memoria(Produto** prods, int* tam);

int main() {
    // Produtos:
    Produto* produtos = NULL;
    int tamanho = 0;
    
    // Printando menu:
    printf("Menu:\n1. Adicionar produto\n2. Listar produtos\n3. Buscar produto\n4. Atualizar estoque\n5. Remover produto\n6. Sair\n");
    
    bool rodando = true;
    while (rodando) {
        // Pedindo opção:
        int opt = 0;
        printf("\nOpção: ");
        scanf("%d", &opt);
        while (getchar() != '\n');

        switch (opt) {
            case 1:
                printf("\n--- Adicionar Produto ---\n");
                adicionar_produto(&produtos, &tamanho);
                break;
            case 2:
                printf("\n--- Lista de Produtos ---\n");
                listar_produtos(produtos, tamanho);
                break;
            case 3:
                printf("\n--- Buscar Produto---\n");
                // Descobrindo código
                int code = 0;
                printf("Insira o código do produto: ");
                scanf("%d", &code);

                Produto* produto = buscar_produto(produtos, tamanho, code);

                if (produto != NULL) {
                    printf("Produto encontrado.\n");
                }
                else {
                    printf("Produto não encontrado.\n");
                }
                break;
            case 4:
                printf("\n--- Atualizar Estoque ---\n");
                atualizar_estoque(produtos, tamanho);
                break;
            case 5:
                printf("\n--- Remover Produto ---\n");
                remover_produto(&produtos, &tamanho);
                break;
            case 6:
                printf("\n");
                liberar_memoria(&produtos, &tamanho);
                rodando = false;
                printf("Programa encerrado.\n");
                break;
            default:
                printf("Opção Inválida!\n");
                break;
        }
    
    }
    return 0;
};

void adicionar_produto(Produto** prods, int* tam) {
    // Iniciando novo produto.
    Produto prod;
    int code;

    // Verificando código do produto.
    if (*tam == 0) {
        code = 1;
    }
    else {
        code = (*prods)[*tam - 1].codigo + 1;
    }

     // Produto:
    char buffer_nome[100];
    float preco;
    int qtd;

    // Pegando o nome:
    printf("Nome: ");
    fgets(buffer_nome, sizeof(buffer_nome), stdin);
    
    // Removendo "\n" do final:
    buffer_nome[strcspn(buffer_nome, "\n")] = '\0';

    char* nome = strdup(buffer_nome);
    if (nome == NULL) {
        printf("Erro de memória ao alocar nome\n");
        return;
    }

    // Pegando o preco
    printf("Preco: ");
    scanf("%f", &preco);

    // Pegando a quantidade
    printf("Quantidade: ");
    scanf("%d", &qtd);

    // Limpar o buffer do teclado após o scanf
    while (getchar() != '\n');

    // Adicionando valores do novo produto.
    prod.codigo = code;
    prod.nome = nome;
    prod.preco = preco;
    prod.quantidade = qtd;


    Produto *temp = realloc(*prods, sizeof(Produto) * ((*tam) + 1));
    if (temp != NULL) {
        (*tam)++;
        *prods = temp;

        (*prods)[(*tam) - 1] = prod;
        printf("Produto adicionado com códgo %d!\n", code);
    }
    else {
        free(prod.nome);
        printf("Erro de memória!\n");
    }

    return;
}

void listar_produtos(Produto* prods, int tam) {
    if (tam == 0) {
        printf("O estoque esta vazio.\n");
        return;
    }
    printf("+--------+----------------------+------------+-----+----------------+\n");
    printf("| %-6s | %-20s | %-11s | %-3s | %-14s |\n", "Código", "Nome", "Preço", "Qtd", "Valor Estoque");
    printf("+--------+----------------------+------------+-----+----------------+\n");
    float total = 0;
    for (int i = 0; i < tam; i++) {
        float preco = prods[i].preco;
        int qtd = prods[i].quantidade;
        float v_estoque = preco * qtd;
        total += v_estoque;

        printf("| %6d | %-20s | %10.2f | %3d | %14.2f |\n", 
            prods[i].codigo, 
            prods[i].nome, 
            preco, 
            qtd, 
            v_estoque);
    }
    printf("+--------+----------------------+------------+-----+----------------+\n");
    printf("Valor total do estoque: R$ %-10.2f\n", total);

    return;
}

Produto* buscar_produto(Produto* prods, int tam, int code) {

    for (int i = 0; i < tam; i++) {
        if (prods[i].codigo == code) {
            return &prods[i];
        }
    }
    return NULL;
}

void atualizar_estoque(Produto* prods, int tam) {
    // Pedindo o código
    int code = 0;
    printf("Código do produto: ");
    scanf("%d", &code);

    // Encontrando o produto:
    Produto* produto = buscar_produto(prods, tam, code);

    // Atualizando: 
    if (produto != NULL) {
        int qtd = 0;
        printf("Nova quantidade: ");
        scanf("%d", &qtd);
        produto->quantidade = qtd;

        printf("Estoque atualizado com sucesso!\n");
    }

    return;
}

void remover_produto(Produto** prods, int* tam) {
    // Pedindo o código
    int code, encontrado = -1;
    printf("Código do produto: ");
    scanf("%d", &code);

    for (int i = 0; i < (*tam); i++) {
        if ((*prods)[i].codigo == code) {
           encontrado = i;
           break;
        }
    }

    if (encontrado == -1) {
        printf("Produto não encontrado.\n");
        return;
    } else {
        printf("Produto \"%s\" removido com sucesso!\n", (*prods)[encontrado].nome);
    }

    // Liberando o espaço do nome
    free((*prods)[encontrado].nome);

    // Tampando o "Buraco" deixado pela remoção
        // Trazemos os elementos da direita para a esquerda.
    for (int i = encontrado; i < (*tam) - 1; i++) {
        (*prods)[i] = (*prods)[i + 1];
    }

    // Ajustando tamanho (o inteiro) da lista
    (*tam)--;

    if (*tam > 0) {
        // Ajustando tamanho (removendo espaços vazios) da lista
        Produto* temp = realloc(*prods, sizeof(Produto) * (*tam));
        if (temp != NULL) {
            *prods = temp;
        }
        // Se falhar em reallocar, podemos ignorar o erro.
            // Um espaço vazio no fim da lista não interfere no código...
    } else {
        // Se a lista ficou vazia
        free(*prods);
        *prods = NULL;
    }

}

void liberar_memoria(Produto** prods, int* tam) {
    for (int i = 0; i < (*tam); i++) {
        printf("Memória do produto \"%s\" liberada.\n", (*prods)[i].nome);
        free((*prods)[i].nome);
    }

    free(*prods);
    printf("Vetor de produtos liberado\n");
    *prods = NULL;
    *tam = 0;
}