//Bibliotecas------------------------------------------------------------------
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//----------------------------------------------------------------------------

//Constantes------------------------------------------------------------------
#define ARESTA   20//ESCALA
#define LARGURA  540//JANELA
#define ALTURA   260//JANELA
#define LINHAS   11//MAPA
#define COLUNAS  27//MAPA
//----------------------------------------------------------------------------

//Estruturas-------------------------------------------------------------------

//ESTRUTURA PARA PAREDES INDESTRUTÍVEIS/DESTRUTÍVEIS
typedef struct
{
    int posicoes_X[297];//vetor para as coordenadas X das paredes 'W' ou 'D' // 297 caso tivessemos apenas paredes em todo o mapa
    int posicoes_Y[297];//vetor para as coordenadas Y das paredes 'W' ou 'D' // 297 caso tivessemos apenas paredes em todo o mapa
    int qntd;//conta quantas paredes existem no mapa
} PAREDES;

//ESTRUTURA PARA DADOS RELATIVOS AO JOGADOR
typedef struct
{
    int pos_dinamicaPersX;//posicao do personagem no eixc X
    int pos_dinamicaPersY;//posicao do personagem no eixo Y
    int persdx;//deslocamento do personagem no eixo X
    int persdy;//deslocamento do personagem no eixo Y
} PERSONAGEM;

//ESTRUTURA PARA AS POÇÕES
typedef struct
{
    int posicoes_Xp[297];//vetor para as coordenadas X das poçoes 'P' //297 caso tivessemos apenas poçoes em todo o mapa
    int posicoes_Yp[297];//vetor para as coordenadas Y das pocoes 'P' //297 caso tivessemos apenas poçoes em todo o mapa
    int qntdP;//conta quantas pocoes existem no mapa
} CONSUMIVEL;

//ESTRUTURA PARA CONTADORES DE INFORMAÇÕES
typedef struct
{
    int pontuacao;//calcula o SCORE do player
    int vidas;//calcula a quantidade de vidas do player
    int nivel;//mostra em qual nivel o jogador se encontra
    int bombas;//numero de bombas no arsenal do jogador
} CONTADORES;

//ESTRUTURA PARA INFORMAÇÕES RELATIVAS À BOMBA E SUA EXPLOSÃO
typedef struct
{
    bool bomba;//verifica se a bomba está ativada ou não
    int pos_x_bomba;//guarda a posicao x de quando a bomba foi plantada
    int pos_y_bomba;//guarda a posicao y de quando a bomba foi plantada
    float contador_de_bomba;//conta frames por segundo para a bomba
    int contador_de_explosao;//conta frames por segundo para a explosão
    bool explosao;//verifica se a explosão está ativada ou não
    bool timer;//verifica se deve ou não começar a contagem do tempo de existência da bomba/explosão
} BOMBA;

//ESTRUTURA PARA INFORMAÇÕES RELATIVAS AOS SERES E AOS MONSTROS
typedef struct
{
    int desl;//direcao do movimento da criatura
    int qtd_passos;//conta os passos de cada criatura
    char direcao_desl;//qual o eixo de deslocamento das criaturas
    int posX, posY;//posicoes das criaturas
    bool vivo;//verifica se a criatura está viva ou não
} MONSTROS;

//ESTRUTURA PARA SALVAR AS POSIÇÕES INICIAIS DO JOGADOR/CRIATURAS
typedef struct
{
    int posX_monstro[50], posY_monstro[50];
    int posX_seres[50], posY_seres[50];//guarda as coordenadas iniciais das criaturas na fase atual
    int posicaoInicialX, posicaoInicialY;//guarda as posições iniciais do jogador na fase inicial
} INICIAL;

//ESTRUTURA PARA VERIFICAR PARA QUE LADO O PERSONAGEM ESTÁ VIRADO
typedef struct
{
    bool jogDireita, jogEsquerda, jogCima, jogBaixo;//orientação do corpo do jogador (para fins de textura)
} VISAO;

//ESTRUTURA GERAL, QUE CONTÉM OUTRAS ESTRUTURAS E VARIÁVEIS INDISPENSÁVEIS PARA CARREGAR OU SALVAR O JOGO
typedef struct
{
    char mapa[11][27];//mapa interno do jogo
    BOMBA bomba[3];//bombas (3 no máximo no arsenal)
    MONSTROS seres[50];//coloca um limite de '50'
    MONSTROS monstros[50];//coloca um limite de '50'
    CONTADORES info;
    CONSUMIVEL pocao;
    PERSONAGEM jogador;
    int contaseres;//variavel para contar o número de seres
    int contamonstros;//variavel para contar o número de monstros
    char nivel[11];//nome do arquivo texto (MAPA) em que o jogador se encontra
    PAREDES indestrutiveis;
    PAREDES destrutiveis;
} ESTADO;

//ESTRUTURA DE TEXTURAS (do tipo 'Texture2D'- da raylib)
typedef struct
{
    Texture2D monstro;
    Texture2D ser;
    Texture2D bomba;
    Texture2D meio;
    Texture2D esq;
    Texture2D dir;
    Texture2D cima;
    Texture2D baixo;
    Texture2D paredeDes;
    Texture2D paredeInd;
    Texture2D playerDireita;
    Texture2D playerEsquerda;
    Texture2D playerCima;
    Texture2D playerBaixo;
    Texture2D pocao;
} TEXTURAS;
//----------------------------------------------------------------------------

void quantosSeres(ESTADO *estadoCarregado)//recebe um ponteiro para a struct principal e atualiza a variavel 'contaseres' com o número de seres da atual fase
{
    estadoCarregado->contaseres = 0;
    int linha, coluna;
    //descobre quantos monstros existem no mapa
    for(linha = 0; linha < LINHAS; linha++){
        for(coluna = 0; coluna < COLUNAS; coluna++){
            if(estadoCarregado->mapa[linha][coluna] == 'K')
            {
                estadoCarregado->contaseres+=1;
            }
        }
    }
}

void iniSeres(ESTADO *estadoCarregado, INICIAL *posInicial)//recebe um ponteiro para a struct principal e um ponteiro para a struct que guarda as posições iniciais, atualiza as posicoes iniciais de cada ser no mapa e as posições dinamicas e zera a quantidade de passos de cada ser.
{
    int linha, coluna;
    int i;
    i = 0;
    // acha posicao inicial x e y de cada ser
    for(linha = 0; linha < LINHAS; linha++){
        for(coluna = 0; coluna < COLUNAS; coluna++){
            if(estadoCarregado->mapa[linha][coluna] == 'K')
            {
                if(estadoCarregado->seres[i].vivo == true)
                {
                    estadoCarregado->seres[i].posX = coluna;
                    posInicial->posX_seres[i] = coluna;
                    estadoCarregado->seres[i].posY = linha;
                    posInicial->posY_seres[i] = linha;
                    i++;
                }
            }
        }
    }

    // inicializar qtd passos em zero
    for(i = 0; i < estadoCarregado->contaseres; i++)
        estadoCarregado->seres[i].qtd_passos = 0;

}

void quantosMonstros(ESTADO *estadoCarregado) //recebe um pointeiro para a struct principal e atualiza a variavel 'contamonstros' com o número de monstros da atual fase
{
    estadoCarregado->contamonstros = 0;
    int linha, coluna;
    //descobre quantos monstros existem no mapa
    for(linha = 0; linha < LINHAS; linha++){
        for(coluna = 0; coluna < COLUNAS; coluna++){
            if(estadoCarregado->mapa[linha][coluna] == 'M')
            {
                estadoCarregado->contamonstros+=1;
            }
        }
    }
}

void iniMonstros(ESTADO *estadoCarregado, INICIAL *posInicial) //recebe um ponteiro para a struct principal e um ponteiro para a struct que guarda as posições iniciais, atualiza as posicoes iniciais de cada monstro no mapa e as posições dinamicas e zera a quantidade de passos de cada monstro.
{
    int linha, coluna;
    int i;
    i = 0;
    // acha posicao inicial x e y de cada monstro
    for(linha = 0; linha < LINHAS; linha++){
        for(coluna = 0; coluna < COLUNAS; coluna++){
            if(estadoCarregado->mapa[linha][coluna] == 'M')
            {
                if(estadoCarregado->monstros[i].vivo==true)
                {
                    estadoCarregado->monstros[i].posX = coluna;
                    posInicial->posX_monstro[i] = coluna;
                    estadoCarregado->monstros[i].posY = linha;
                    posInicial->posY_monstro[i] = linha;
                    i++;
                }
            }
        }
    }

    // inicializar qtd passos em zero
    for(i = 0; i < estadoCarregado->contamonstros; i++)
        estadoCarregado->monstros[i].qtd_passos = 0;
}

// gera direcao em que monstros ou seres irao se mover na matriz/mapa, retornado x ou y
char geraDirecao()
{
    char direcaoNaMatriz;
    int valorXY;

    valorXY = GetRandomValue(0,1);

    if(valorXY)
        direcaoNaMatriz = 'x';
    else
        direcaoNaMatriz = 'y';
    return direcaoNaMatriz;

}


// gera deslocamento, dado a direcao atual e o deslocamento atual
// retorna novo deslocamento
int geraDeslocamento()
{
    int deslocamento;
    do
    {
        deslocamento = GetRandomValue(-1,1);
    }
    while(deslocamento == 0);
    return deslocamento;
}

bool canMove(int pos_x, int pos_y, int desl, char mapa[][COLUNAS], char direcao, BOMBA bomba[])//recebe as posições X, Y, os deslocamentos, mapa, direção do deslocamento e todas as bombas
{                                                                                               //retorna 'true' se as criaturas podem se deslocar
    bool move;
    int i;
    // assumimos inicialmente q pode mover
    move = true;
    if(direcao == 'x')
        pos_x = pos_x + desl;
    else // senao eh x eh y
        pos_y = pos_y + desl;
    if(mapa[pos_y][pos_x] == 'W')
        move = false;
    if(mapa[pos_y][pos_x] == 'D')
        move = false;
    for(i = 0; i < 2; i++)
    {
        if((pos_y*ARESTA == bomba[i].pos_y_bomba)&&(pos_x*ARESTA == bomba[i].pos_x_bomba))
            move = false;
    }
    return move;
}

void moveCriaturas(ESTADO *estadoCarregado)//recebe um ponteiro para a struct principal e atualiza os valores de direção, deslocamento, posição, contagem de passos dos monstros/seres
{
    int i;
    //char direcao;
    //int desl;

    // MONSTROS
    for(i = 0; i < estadoCarregado->contamonstros; i++)
    {
        // gera direcao de movimento do monstro i
        if(estadoCarregado->monstros[i].vivo==true)
        {
            if(estadoCarregado->monstros[i].qtd_passos == 0)
            {
                estadoCarregado->monstros[i].direcao_desl = geraDirecao();
                estadoCarregado->monstros[i].desl = geraDeslocamento();
            }
            // verifica se pode mover e se qtdpassos está de acordo
            if(canMove(estadoCarregado->monstros[i].posX, estadoCarregado->monstros[i].posY, estadoCarregado->monstros[i].desl, estadoCarregado->mapa, estadoCarregado->monstros[i].direcao_desl, estadoCarregado->bomba)&&(estadoCarregado->monstros[i].qtd_passos < 5))
            {
                if(estadoCarregado->monstros[i].direcao_desl == 'x')
                    estadoCarregado->monstros[i].posX = estadoCarregado->monstros[i].posX + estadoCarregado->monstros[i].desl;
                else
                    estadoCarregado->monstros[i].posY = estadoCarregado->monstros[i].posY + estadoCarregado->monstros[i].desl;

                estadoCarregado->monstros[i].qtd_passos = estadoCarregado->monstros[i].qtd_passos + 1;
            }
            else // se nao estiver de acordo zera contador de passos e gera novo deslocamento lá em cima
                estadoCarregado->monstros[i].qtd_passos = 0;
        }
    }
    // SERES
    for(i = 0; i < estadoCarregado->contaseres; i++)
    {
        // gera direcao de movimento do ser i
        if(estadoCarregado->seres[i].vivo==true)
        {
            if(estadoCarregado->seres[i].qtd_passos == 0)
            {
                estadoCarregado->seres[i].direcao_desl = geraDirecao();
                estadoCarregado->seres[i].desl = geraDeslocamento();
            }
            if(canMove(estadoCarregado->seres[i].posX, estadoCarregado->seres[i].posY, estadoCarregado->seres[i].desl, estadoCarregado->mapa, estadoCarregado->seres[i].direcao_desl, estadoCarregado->bomba)&&(estadoCarregado->seres[i].qtd_passos < 5))
            {
                if(estadoCarregado->seres[i].direcao_desl == 'x')
                    estadoCarregado->seres[i].posX = estadoCarregado->seres[i].posX + estadoCarregado->seres[i].desl;
                else
                    estadoCarregado->seres[i].posY = estadoCarregado->seres[i].posY + estadoCarregado->seres[i].desl;

                estadoCarregado->seres[i].qtd_passos = estadoCarregado->seres[i].qtd_passos + 1;
            }
            else
                estadoCarregado->seres[i].qtd_passos = 0;
        }
    }
}

void posicaoJogador(ESTADO *estadoCarregado, INICIAL *posInicial)//recebe um ponteiro para a struct principal e para a struct das posições iniciais, atualiza a posição dinâmica do jogador de acordo com o 'J' no mapa
{                                                               //e guarda essa posição em outra variável também, que será útil ao longo do jogo.
    int x = 0, y = 0;
    int i, j;

    for(i = 0; i < LINHAS; i++) //percorre a matriz mapa e identifica a posicao inicial do jogador, onde há paredes indestrutiveis e quantas sao elas
    {
        for(j = 0; j < COLUNAS; j++)
        {
            if(estadoCarregado->mapa[i][j] == 'J')
            {
                estadoCarregado->jogador.pos_dinamicaPersX = x;
                estadoCarregado->jogador.pos_dinamicaPersY = y;//guarda a posicao inicial do jogador
            }
            x = x + ARESTA;//vai uma coluna pro lado
        }
        y = y + ARESTA;//vai uma linha pra baixo
        x = 0;//e volta para a primeira coluna
    }
    posInicial->posicaoInicialX = estadoCarregado->jogador.pos_dinamicaPersX;
    posInicial->posicaoInicialY = estadoCarregado->jogador.pos_dinamicaPersY;
}

void initJogo(ESTADO *estadoCarregado)//recebe um ponteiro para a struct principal e retorna as posições e a quantidade de cada poção, parede indestrutivel e parede destrutivel
{
    int x = 0, y = 0;
    int aux = 0, aux2 = 0, aux3 = 0;
    int i, j;

    for(i = 0; i < LINHAS; i++) //percorre a matriz mapa e identifica a posicao inicial do jogador, onde há paredes indestrutiveis e quantas sao elas
    {
        for(j = 0; j < COLUNAS; j++)
        {
            if(estadoCarregado->mapa[i][j] == 'W')
            {
                estadoCarregado->indestrutiveis.posicoes_X[aux] = x;
                estadoCarregado->indestrutiveis.posicoes_Y[aux] = y;//guarda as coordenadas das paredes indestrutiveis
                aux++;
            }
            if(estadoCarregado->mapa[i][j] == 'P')
            {
                estadoCarregado->pocao.posicoes_Xp[aux2] = x;
                estadoCarregado->pocao.posicoes_Yp[aux2] = y;//guarda as coordenadas das pocoes
                aux2++;
            }
            if(estadoCarregado->mapa[i][j] == 'D')
            {
                estadoCarregado->destrutiveis.posicoes_X[aux3] = x;
                estadoCarregado->destrutiveis.posicoes_Y[aux3] = y;//guarda as coordenadas das paredes destrutiveis
                aux3++;
            }
            x = x + ARESTA;
        }
        y = y + ARESTA;
        x = 0;
    }
    estadoCarregado->indestrutiveis.qntd = aux;//quantas paredes indestrutiveis
    estadoCarregado->pocao.qntdP = aux2;//quantas pocoes
    estadoCarregado->destrutiveis.qntd = aux3;//quantas paredes destrutiveis
}

void desenhaMapaEstatico(char mapa[][COLUNAS], TEXTURAS textura)//recebe o mapa e a struct de texturas e retorna a representação gráfica de cada elemento
{                                                               //que não se movimenta, ou seja, paredes e poções.
    int x = 0, y = 0;
    int i, j;

    //desenha graficamente o mapa estático, que não terá atualizações, ou seja, as paredes indestrutiveis
    for(i = 0; i < LINHAS; i++)
    {
        for(j = 0; j < COLUNAS; j++)
        {
            if(mapa[i][j] == 'W')
            {
                DrawTexture(textura.paredeInd, x, y, GRAY);//desenha um cubo azul onde há parede indestruitivel
            }
            if(mapa[i][j] == 'P')
            {
                DrawTexture(textura.pocao, x, y, PINK);//desenha as pocoes
            }
            if(mapa[i][j] == 'D')
            {
                DrawTexture(textura.paredeDes, x, y, LIGHTGRAY);//desenha paredes destrutiveis
            }
            x = x + ARESTA; //posição no eixo horizontal do próximo elemento
        }
        y = y + ARESTA;
        x = 0;//desce uma linha e volta para a coluna '0'.
    }
}

void desenhaJogo(ESTADO *estadoCarregado, bool menu, int danoX[], int danoY[], VISAO visao, TEXTURAS textura)//recebe um ponteiro para a struct principal, a booleana do menu, vetores que representam o raio de dano da bomba,
{                                                                                                            //struct de orientação do personagem e struct de texturas. Retorna a representação gráfica dos elementos
    int i;                                                                                                   //dinâmicos, do menu e das informações projetadas na tela.

    //faz qualquer atualização grafica
    BeginDrawing();
    ClearBackground(LIGHTGRAY);
    desenhaMapaEstatico(estadoCarregado->mapa, textura);//desenha o mapa
    if(visao.jogBaixo == true)
        DrawTexture(textura.playerBaixo, estadoCarregado->jogador.pos_dinamicaPersX, estadoCarregado->jogador.pos_dinamicaPersY, RAYWHITE);//desenha o personagem
    if(visao.jogCima == true)
        DrawTexture(textura.playerCima, estadoCarregado->jogador.pos_dinamicaPersX, estadoCarregado->jogador.pos_dinamicaPersY, RAYWHITE);//desenha o personagem
    if(visao.jogEsquerda == true)
        DrawTexture(textura.playerEsquerda, estadoCarregado->jogador.pos_dinamicaPersX, estadoCarregado->jogador.pos_dinamicaPersY, RAYWHITE);//desenha o personagem
    if(visao.jogDireita == true)
        DrawTexture(textura.playerDireita, estadoCarregado->jogador.pos_dinamicaPersX, estadoCarregado->jogador.pos_dinamicaPersY, RAYWHITE);//desenha o personagem

    for(i = 0; i < estadoCarregado->contaseres; i++)
    {
        if(estadoCarregado->seres[i].vivo == true)
            DrawTexture(textura.ser, estadoCarregado->seres[i].posX*ARESTA, estadoCarregado->seres[i].posY*ARESTA, RAYWHITE);//desenha os seres
    }

    for(i = 0; i < estadoCarregado->contamonstros; i++)
    {
        if(estadoCarregado->monstros[i].vivo == true)
            DrawTexture(textura.monstro, estadoCarregado->monstros[i].posX*ARESTA, estadoCarregado->monstros[i].posY*ARESTA, RAYWHITE);//desenha os monstros
    }

    for(i = 0; i < 3; i++)//verifica se alguma das 3 bombas está plantada
        if(estadoCarregado->bomba[i].bomba == true) //se estiver
        {
            DrawTexture(textura.bomba, estadoCarregado->bomba[i].pos_x_bomba, estadoCarregado->bomba[i].pos_y_bomba, RAYWHITE);//desenha uma bomba amarela
        }
    for(i = 0; i < 3; i++)
        if(estadoCarregado->bomba[i].explosao == true) //se estiver em sua fase de explosao
        {
            DrawTexture(textura.dir,danoX[0], danoY[0], RAYWHITE);//desenha um '+' rapidamente, mostrando o raio de dano
            DrawTexture(textura.esq,danoX[1], danoY[1], RAYWHITE);
            DrawTexture(textura.baixo,danoX[2], danoY[2], RAYWHITE);
            DrawTexture(textura.cima,danoX[3], danoY[3], RAYWHITE);
            DrawTexture(textura.meio,danoX[4], danoY[4], RAYWHITE);
        }
    if(menu) //por enquanto só exibe, as opcoes nao sao funcionais ainda
    {
        DrawRectangle(40, 40, 460, 140, BEIGE);
        DrawText("MENU", 45, 45, 30, BLACK);
        DrawText("Pegue os azuis \n e corra dos \n vermelhos!", 45, 80, 18, GRAY);
        DrawText("(N)Novo Jogo", 210, 50, 20, BLACK);
        DrawText("(C)Carregar Jogo", 210, 75, 20, BLACK);
        DrawText("(S)Salvar Jogo", 210, 100, 20, BLACK);
        DrawText("(Q)Sair do Jogo", 210, 125, 20, BLACK);
        DrawText("(V)Voltar", 210, 150, 20, BLACK);
    }

    DrawText(TextFormat("Score: %d", estadoCarregado->info.pontuacao), 5, 225, 30, GRAY);//vai contando o SCORE do player
    DrawText(TextFormat("Vidas: %d", estadoCarregado->info.vidas), 410, 238, 20, BLACK);//vai contando a VIDA do player
    DrawText(TextFormat("Bombas: %d", estadoCarregado->info.bombas), 410, 220, 20, BLACK);//vai contando o ARSENAL de bombas do player
    DrawText(TextFormat("Lvl:%d", estadoCarregado->info.nivel), 225, 227, 25, RED);//informa o nivel em que o jogador se encotnra (??)
    EndDrawing();
}

int podeMover(ESTADO estadoCarregado)//recebe a struct principal e retorna '0' caso o usuário consiga se mover e '1' caso haja alguma colisão
{
    //ve se o personagem consegue se mover, ou seja, se não vai ocupar o mesmo espaço que outra parede indestrutivel
    int colidiu = 0;
    int i;

    for(i = 0; i < estadoCarregado.indestrutiveis.qntd; i++) //verifica se há colisao com paredes indestrutiveis
    {
        if(((estadoCarregado.jogador.pos_dinamicaPersX + estadoCarregado.jogador.persdx)== estadoCarregado.indestrutiveis.posicoes_X[i])&&((estadoCarregado.jogador.pos_dinamicaPersY + estadoCarregado.jogador.persdy)== estadoCarregado.indestrutiveis.posicoes_Y[i]))
        {
            colidiu = 1;
        }
    }
    for(i = 0; i < estadoCarregado.destrutiveis.qntd; i++) //verifica se há colisao com paredes destrutiveis
    {
        if(((estadoCarregado.jogador.pos_dinamicaPersX + estadoCarregado.jogador.persdx)== estadoCarregado.destrutiveis.posicoes_X[i])&&((estadoCarregado.jogador.pos_dinamicaPersY + estadoCarregado.jogador.persdy)== estadoCarregado.destrutiveis.posicoes_Y[i]))
        {
            colidiu = 1;
        }
    }

    for(i = 0; i < 3; i++)
    {
        if(estadoCarregado.bomba[i].bomba==true)
            if(((estadoCarregado.jogador.pos_dinamicaPersX + estadoCarregado.jogador.persdx)== estadoCarregado.bomba[i].pos_x_bomba)&&(estadoCarregado.jogador.pos_dinamicaPersY + estadoCarregado.jogador.persdy)== estadoCarregado.bomba[i].pos_y_bomba)
                colidiu = 1;
    }
    return colidiu;
}

void explosao(ESTADO *estadoCarregado, int danoX[], int danoY[], bool* perdeVida, int i)//recebe um ponteiro para a struct principal, vetores de raio de dano da bomba, um ponteiro para a booleana que faz com que o jogador perca uma vida, e 'i' que representa qual a bomba.
{                                                                                       //retorna os vetores de raio de dano atualizados, o valor da booleana 'perdeVida', o estado do monstro(vivo/morto) e pontuação.
    int a, k;
    //define o raio de dano a partir da posicao de onde a bomba foi plantada, ou seja, um quadrado a cima, um abaixo, um a esquerda, e um a direita, e claro, no mesmo lugar em que foi plantada.
    danoX[0] = estadoCarregado->bomba[i].pos_x_bomba + ARESTA;
    danoX[1] = estadoCarregado->bomba[i].pos_x_bomba - ARESTA;
    danoX[2] = estadoCarregado->bomba[i].pos_x_bomba;
    danoX[3] = estadoCarregado->bomba[i].pos_x_bomba;
    danoX[4] = estadoCarregado->bomba[i].pos_x_bomba;
    danoY[0] = estadoCarregado->bomba[i].pos_y_bomba;
    danoY[1] = estadoCarregado->bomba[i].pos_y_bomba;
    danoY[2] = estadoCarregado->bomba[i].pos_y_bomba + ARESTA;
    danoY[3] = estadoCarregado->bomba[i].pos_y_bomba - ARESTA;
    danoY[4] = estadoCarregado->bomba[i].pos_y_bomba;

    for(a = 0; a < 5; a++)
    {
        if((danoX[a] == estadoCarregado->jogador.pos_dinamicaPersX) && (danoY[a] == estadoCarregado->jogador.pos_dinamicaPersY)) //se alguma coordenada do raio de dano coincidir com a posicao do jogador
        {
            *perdeVida = true;
        }
    }

    for(k = 0; k < 5; k++)
    {
        for(a = 0; a < estadoCarregado->contamonstros; a++)
        {
            if(estadoCarregado->monstros[a].vivo == true)
                if((danoX[k] == ((estadoCarregado->monstros[a].posX)*ARESTA)) && (danoY[k] == (estadoCarregado->monstros[a].posY)*ARESTA))
                {
                    estadoCarregado->monstros[a].vivo = false;
                    estadoCarregado->info.pontuacao += 100; //(?)
                }
        }
    }
}

void colhePocao(ESTADO *estadoCarregado, int pontos)//recebe um ponteiro para a struct principal e um valor de pontos, atualiza a matriz caracter e pontua.
{
    int j, i;

    j = (estadoCarregado->jogador.pos_dinamicaPersX)/ARESTA;
    i = (estadoCarregado->jogador.pos_dinamicaPersY)/ARESTA;//Se as posicoes dinamicas são o produto da aresta pela posicao na matriz caracter, logo as posicoes na matriz caracter serao a divisao das posiçoes dinamicas pela aresta.

    estadoCarregado->mapa[i][j] = ' ';//onde há pocao e o está em cima dessa posicao, nossa matriz caracter tera esse 'P' trocado por um ' ', ou seja, irá liberar um espaço de livre.
    estadoCarregado->info.pontuacao +=  pontos;//contabiliza 50 pontos por pocao pega
}

int moveParaPocao(PERSONAGEM jogador, CONSUMIVEL *pocao)//recebe a struct jogador e um ponteiro para a struct das poções e retorna '1' caso o jogador não tenha se movido para uma poção
{                                                       // e retorna '0' quanto tiver.
    //ve se o personagem vai ocupar as  mesmas coordenadas de uma pocao
    int colidiu = 1;
    int i;
    for(i = 0; i < pocao->qntdP; i++)
    {
        if(((jogador.pos_dinamicaPersX)== pocao->posicoes_Xp[i])&&((jogador.pos_dinamicaPersY)== pocao->posicoes_Yp[i]))
        {
            colidiu = 0;//vai ocupar
        }
    }
    return colidiu;
}

int moveParaSer(ESTADO estadoCarregado, int *serCapturado)//recebe a struct principal e um ponteiro para um ser específico
{                                                         //retorna '0' se encostou no ser e '1' caso contrário. retorna também,
                                                          //o número do ser encostado.
    //ve se o personagem vai ocupar as  mesmas coordenadas de uma pocao
    int colidiu = 1;
    int i, achou;
    for(i = 0; i < estadoCarregado.contaseres; i++)
    {
        if(((estadoCarregado.jogador.pos_dinamicaPersX) == estadoCarregado.seres[i].posX * ARESTA)&&((estadoCarregado.jogador.pos_dinamicaPersY)== estadoCarregado.seres[i].posY * ARESTA)) //
        {
            colidiu = 0;//vai ocupar
            achou = i;
        }
    }
    *serCapturado = achou;
    return colidiu;
}

int moveParaMonstro(ESTADO estadoCarregado, int *monstroCapturado)  //recebe a struct principal e um ponteiro para um monstro específico
                                                                    //retorna '0' se encostou no ser e '1' caso contrário. retorna também,
                                                                    //o número do ser encostado.
{
    //ve se o personagem vai ocupar as  mesmas coordenadas de uma pocao
    int colidiu = 1;
    int i, achou;
    for(i = 0; i < estadoCarregado.contamonstros; i++)
    {
        if(((estadoCarregado.jogador.pos_dinamicaPersX) == estadoCarregado.monstros[i].posX * ARESTA)&&((estadoCarregado.jogador.pos_dinamicaPersY)== estadoCarregado.monstros[i].posY * ARESTA)) //
        {
            colidiu = 0;//vai ocupar
            achou = i;
        }
    }
    *monstroCapturado = achou;
    return colidiu;
}

void funcBomba(ESTADO *estadoCarregado, int qualBomba)//recebe um ponteiro para a struct principal e uma variável que sinaliza qual bomba está sendo plantada
{                                                     //retorna uma atualização do valor de variáveis
    estadoCarregado->bomba[qualBomba].contador_de_bomba = 0;//zera o contador de quadros
    estadoCarregado->bomba[qualBomba].timer = true;//sinaliza para o timer começar a contar
    estadoCarregado->bomba[qualBomba].pos_x_bomba = estadoCarregado->jogador.pos_dinamicaPersX;
    estadoCarregado->bomba[qualBomba].pos_y_bomba = estadoCarregado->jogador.pos_dinamicaPersY;//lê as posicoes de onde a bomba será plantada
    estadoCarregado->info.bombas -= 1;//diminui uma bomba do arsenal
}

int carregaJogo(ESTADO *estadoCarregado, char arqbin[])//recebe a struct principal e o nome do a rquivo binário
{                                                      //retorna '1' caso tenha êxito na abertura do arquivo e '0' caso contrário
    int status = 0;                                    //carrega toda a struct ESTADO do arquivo
    FILE *arq_bin;

    if(!(arq_bin = fopen (arqbin, "rb")))
        printf("Erro na leitura dos dados do jogo salvo!\n");
    else
    {
        if(fread(estadoCarregado, sizeof(ESTADO), 1, arq_bin)==1)
            status = 1;
        fclose(arq_bin);
    }
    return status;
}

void leMapa(ESTADO estadoCarregado)//recebe a struct principal e retorna a matriz caracter para a variável mapa a partir de um arquivo texto
{

    int i, j; //aux
    FILE *arq_txt;

    if(!(arq_txt = fopen (estadoCarregado.nivel, "r")))
        printf("Erro na leitura do mapa do jogo salvo!\n");
    else
    {
        for(i = 0; i < LINHAS; i++)
        {
            for(j = 0; j < COLUNAS; j++)
            {
                estadoCarregado.mapa[i][j] = getc(arq_txt);
            }
        }
        fclose(arq_txt);
    }
}

int salvaJogo(ESTADO estadoCarregado, char arqbin[])//recebe a struct principal e o nome do arquivo binário
{                                                   //retorna '1' caso tenha êxito na abertura do arquivo e '0' caso contrário
    int status = 0;                                 //grava toda a struct ESTADO no arquivo
    FILE *arq_bin;

    if(!(arq_bin = fopen(arqbin, "wb")))
        printf("Erro na abertura do arquivo para salvar dados!\n");
    else
    {
        if(fwrite(&estadoCarregado, sizeof(ESTADO), 1, arq_bin)==1)
            status = 1;
        fclose(arq_bin);
    }
    return status;
}

void iniciaVariaveisEstadoCarregado(ESTADO *estadoCarregado)//recebe um ponteiro para a struct principal e atualiza os valores para poder começar um novo jogo
{
    int i;

    estadoCarregado->destrutiveis.qntd = 0;//começa a contagem de paredes destrutiveis em '0'
    estadoCarregado->indestrutiveis.qntd = 0;//começa a contagem de paredes indestrutiveis em '0'
    estadoCarregado->pocao.qntdP = 0;//começa a contagem de poções em '0'
    estadoCarregado->contaseres = 0;//começa a contagem de seres em '0'
    estadoCarregado->contamonstros = 0;//começa a contagem de monstros em '0'
    estadoCarregado->info.bombas = 3;//começa o número de bombas no arsenal do jogador em '3'
    estadoCarregado->info.pontuacao = 0;//começa o SCORE do jogador em '0'
    estadoCarregado->info.vidas = 3;//jogador começa com '3' vidas
    estadoCarregado->info.nivel = 0;//jogador começa na fase '0'
    for(i = 0; i < 3; i++)
    {
        estadoCarregado->bomba[i].bomba = false;//começa todas as bombas como 'não ativadas'
        estadoCarregado->bomba[i].timer = false;//começa o sinalizador de contagem como 'não ativado'
        estadoCarregado->bomba[i].explosao = false;//começa as explosões de todas as bombas como 'não ativadas'
    }
}

void variaveisParaProximaFase(ESTADO *estadoCarregado, int contador_de_mov_criatura, INICIAL *posInicial)//recebe um ponteiro para a struct principal, um contador e um ponteiro para a struct das posições iniciais
{                                                                                                        //retorna uma atualização dos valores das variaveis e reinicia algumas funções,
    int i;                                                                                               //visando o começo de uma nova fase
    for(i = 0; i < 3; i++)
    {
        estadoCarregado->bomba[i].bomba = false;//bombas desativadas
        estadoCarregado->bomba[i].timer = false;//contador de existência de bomba/explosão desativado
        estadoCarregado->bomba[i].contador_de_bomba = 0;//contado de frames para a existência da bomba em '0'
        estadoCarregado->bomba[i].contador_de_explosao = 0;//contado de frames para a existência da explosão em '0'
        estadoCarregado->bomba[i].explosao = false;
    }
    estadoCarregado->info.bombas = 3;//contador de bombas em '3'
    estadoCarregado->indestrutiveis.qntd = 0;//contador de paredes indestrutiveis em '0'
    estadoCarregado->destrutiveis.qntd = 0;//contador de paredes destrutíveis em '0'
    estadoCarregado->pocao.qntdP = 0;//contador de poções em '0'
    contador_de_mov_criatura = 0;

    //REINICIA ALGUMAS FUNÇÕES
    posicaoJogador(estadoCarregado, posInicial);
    quantosSeres(estadoCarregado);
    quantosMonstros(estadoCarregado);

    for(i = 0; i < estadoCarregado->contamonstros; i++)
    {
        estadoCarregado->monstros[i].vivo = true;
    }
    for(i = 0; i < estadoCarregado->contaseres; i++)
    {
        estadoCarregado->seres[i].vivo = true;
    }

    iniSeres(estadoCarregado, posInicial);
    iniMonstros(estadoCarregado, posInicial);
    initJogo(estadoCarregado);
}

int avancaNivel(int num_seres, ESTADO estadoCarregado)//funcao recursiva que recebe o numero de seres existentes na fase atual e a struct principal (para acessar a booleana 'vivo') e
{
    //retorna o numero de seres mortos.
    int achou = 0;

    if(num_seres == 0)
        if(estadoCarregado.seres[num_seres].vivo == false)
            return 1;
        else
            return 0;
    else if(estadoCarregado.seres[num_seres].vivo == false)
        achou++;
    return achou = achou + avancaNivel(num_seres - 1, estadoCarregado);
}

void carregaTexturas(TEXTURAS *textura)//recebe um ponteiro para a struct de texturas e retorna o carregamento delas
{
    textura->monstro = LoadTexture("texturas\\monstro.png");
    textura->ser = LoadTexture("texturas\\ser.png");
    textura->bomba = LoadTexture("texturas\\bomb1.png");
    textura->meio = LoadTexture("texturas\\exp meio.png");
    textura->esq = LoadTexture("texturas\\exp esq.png");
    textura->dir = LoadTexture("texturas\\exp direita.png");
    textura->cima = LoadTexture("texturas\\exp cima.png");
    textura->baixo = LoadTexture("texturas\\exp baixo.png");
    textura->paredeDes = LoadTexture("texturas\\destrutivel1.png");
    textura->paredeInd = LoadTexture("texturas\\indestrutivel1.png");
    textura->playerDireita = LoadTexture("texturas\\jogador dir1.png");
    textura->playerEsquerda = LoadTexture("texturas\\jogador esq1.png");
    textura->playerCima = LoadTexture("texturas\\jogador ci1.png");
    textura->playerBaixo = LoadTexture("texturas\\jogador bai1.png");
    textura->pocao = LoadTexture("texturas\\pocao.png");
}

void descarregaTexturas(TEXTURAS *textura)//recebe um ponteiro para a struct de texturas e descarrega elas da GPU
{
    UnloadTexture(textura->monstro);
    UnloadTexture(textura->ser);
    UnloadTexture(textura->bomba);
    UnloadTexture(textura->meio);
    UnloadTexture(textura->esq);
    UnloadTexture(textura->dir);
    UnloadTexture(textura->cima);
    UnloadTexture(textura->baixo);
    UnloadTexture(textura->paredeDes);
    UnloadTexture(textura->paredeInd);
    UnloadTexture(textura->playerDireita);
    UnloadTexture(textura->playerEsquerda);
    UnloadTexture(textura->playerCima);
    UnloadTexture(textura->playerBaixo);
    UnloadTexture(textura->pocao);
}
