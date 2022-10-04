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

//ESTRUTURA PARA PAREDES INDESTRUT�VEIS/DESTRUT�VEIS
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

//ESTRUTURA PARA AS PO��ES
typedef struct
{
    int posicoes_Xp[297];//vetor para as coordenadas X das po�oes 'P' //297 caso tivessemos apenas po�oes em todo o mapa
    int posicoes_Yp[297];//vetor para as coordenadas Y das pocoes 'P' //297 caso tivessemos apenas po�oes em todo o mapa
    int qntdP;//conta quantas pocoes existem no mapa
} CONSUMIVEL;

//ESTRUTURA PARA CONTADORES DE INFORMA��ES
typedef struct
{
    int pontuacao;//calcula o SCORE do player
    int vidas;//calcula a quantidade de vidas do player
    int nivel;//mostra em qual nivel o jogador se encontra
    int bombas;//numero de bombas no arsenal do jogador
} CONTADORES;

//ESTRUTURA PARA INFORMA��ES RELATIVAS � BOMBA E SUA EXPLOS�O
typedef struct
{
    bool bomba;//verifica se a bomba est� ativada ou n�o
    int pos_x_bomba;//guarda a posicao x de quando a bomba foi plantada
    int pos_y_bomba;//guarda a posicao y de quando a bomba foi plantada
    float contador_de_bomba;//conta frames por segundo para a bomba
    int contador_de_explosao;//conta frames por segundo para a explos�o
    bool explosao;//verifica se a explos�o est� ativada ou n�o
    bool timer;//verifica se deve ou n�o come�ar a contagem do tempo de exist�ncia da bomba/explos�o
} BOMBA;

//ESTRUTURA PARA INFORMA��ES RELATIVAS AOS SERES E AOS MONSTROS
typedef struct
{
    int desl;//direcao do movimento da criatura
    int qtd_passos;//conta os passos de cada criatura
    char direcao_desl;//qual o eixo de deslocamento das criaturas
    int posX, posY;//posicoes das criaturas
    bool vivo;//verifica se a criatura est� viva ou n�o
} MONSTROS;

//ESTRUTURA PARA SALVAR AS POSI��ES INICIAIS DO JOGADOR/CRIATURAS
typedef struct
{
    int posX_monstro[50], posY_monstro[50];
    int posX_seres[50], posY_seres[50];//guarda as coordenadas iniciais das criaturas na fase atual
    int posicaoInicialX, posicaoInicialY;//guarda as posi��es iniciais do jogador na fase inicial
} INICIAL;

//ESTRUTURA PARA VERIFICAR PARA QUE LADO O PERSONAGEM EST� VIRADO
typedef struct
{
    bool jogDireita, jogEsquerda, jogCima, jogBaixo;//orienta��o do corpo do jogador (para fins de textura)
} VISAO;

//ESTRUTURA GERAL, QUE CONT�M OUTRAS ESTRUTURAS E VARI�VEIS INDISPENS�VEIS PARA CARREGAR OU SALVAR O JOGO
typedef struct
{
    char mapa[11][27];//mapa interno do jogo
    BOMBA bomba[3];//bombas (3 no m�ximo no arsenal)
    MONSTROS seres[50];//coloca um limite de '50'
    MONSTROS monstros[50];//coloca um limite de '50'
    CONTADORES info;
    CONSUMIVEL pocao;
    PERSONAGEM jogador;
    int contaseres;//variavel para contar o n�mero de seres
    int contamonstros;//variavel para contar o n�mero de monstros
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

//Prot�tipos------------------------------------------------------------------
void carregaTexturas();
void descarregaTexturas();
int  avancaNivel();
int  carregaJogo();
void leMapa();
int  salvaJogo();
void funcBomba();
void quantosSeres();
void quantosMonstros();
void iniMonstros();
void iniSeres();
int  moveParaSer();
int  moveParaMonstro();
bool canMove();
int  geraDeslocamento();
char geraDirecao();
void moveCriaturas();
void colhePocao();
void posicaoJogador();
void initJogo();
void desenhaMapaEstatico();
void desenhaJogo();
int  podeMover();
int  moveParaPocao();
void explosao();
void iniciaVariaveisEstadoCarregado();
void variaveisParaProximaFase();
//----------------------------------------------------------------------------

//Func�o Principal------------------------------------------------------------
int main()
{
    //DECLARA��O DE ALGUMAS VARI�VEIS
    char arqbin[50] = "dadosSalvos.bin";//nome do arquivo bin�rio para salvar e/ou carregar informa��es
    int i, j, k = 0;//variaveis auxiliares
    bool vida_extra = false; //b�nus de vida
    int danoX[5];//raio de dano da bomba
    int danoY[5];//raio de dano da bomba
    bool perdeVida = false;//verifica se o jogador deve perder vida
    bool fim_de_jogo = false;//verifica se acabou o n�mero de n�veis
    int serCapturado;//n�mero do ser que foi encostado pelo jogador
    int monstroCapturado;//n�mero do monstro explodido pelo jogador
    int contador_de_mov_criatura = 0;//conta frames para decidir se a criatura deve mover ou n�o
    bool menu = false;//verifica se o 'menu' est� aberto ou fechado
    bool trocaNivel = true;//se verdadeiro, faz avan�ar a fase
    VISAO visao;//vari�vel para fins de textura do jogador
    visao.jogDireita = false, visao.jogEsquerda = false, visao.jogCima = false, visao.jogBaixo = true;
    ESTADO estadoCarregado;//vari�vel do tipo ESTADO(estrutura principal)
    INICIAL posInicial;
    TEXTURAS textura;
    iniciaVariaveisEstadoCarregado(&estadoCarregado);//zera as vari�veis do tipo estruturado ESTADO

    InitWindow(LARGURA, ALTURA, "O jogo");
    SetTargetFPS(60);
    carregaTexturas(&textura);

    //LOOP PRINCIPAL DO JOGO
    while (!WindowShouldClose() && estadoCarregado.info.vidas > 0 && fim_de_jogo == false)//O JOGO S� CONTINUA ENQUANTO 'ESC' N�O FOR PRESSIONADO *E* ENQUANTO O JOGADOR TIVER PELO MENOS '1' VIDA
    {
        //FAZ O AVAN�O DE FASE (NO CASO DO PRIMEIRO JOGO, J� FAR� O AVAN�O DA FASE '0' PARA A FASE '1'(PRIMEIRA EXISTENTE))
        if(trocaNivel == true)
        {
            estadoCarregado.info.nivel++;//j� come�a no nivel '1'
            trocaNivel = false;//j� torna falso, para a condi��o n�o ficar se repetindo durante o while

            //OPERA O ARQUIVO TEXTO (MAPA)
            FILE* fase;
            sprintf(estadoCarregado.nivel, "nivel%d.txt", estadoCarregado.info.nivel);//define o nome do arquivo a partir do contador de niveis
            fase = fopen(estadoCarregado.nivel, "r");//abre o arquivo
            if(fase == NULL) //verifica se foi bem sucedido
            {
                fim_de_jogo = true;//se tornar verdadeiro, quer dizer que 1.A abertura do arquivo n�o teve sucesso ou 2.Acabaram-se os n�veis .txt e o jogador ganhou (em ambos os casos apresenta a mensagem 'WIN')
            }
            else
            {
                for(i = 0; i < LINHAS; i++)
                {
                    for(j = 0; j < COLUNAS; j++)
                    {
                        estadoCarregado.mapa[i][j] = getc(fase);//passa a matriz de caracteres do arquivo texto (MAPA) para uma matriz variavel do programa
                    }
                    getc(fase);//ignora '\n'
                }
            }
            fclose(fase);//fecha o arquivo texto

            //REINICIA ALGUMAS VARI�VEIS
            variaveisParaProximaFase(&estadoCarregado, contador_de_mov_criatura, &posInicial);
        }

        //SE MENU ESTIVER ABERTO
        if(menu)
        {
            //SAI DO MENU
            if(IsKeyPressed(KEY_V))
                menu = false;

            //FECHA O JOGO
            if(IsKeyPressed(KEY_Q))//fecha o jogo
                CloseWindow();

            //CRIA UM NOVO JOGO
            if(IsKeyPressed(KEY_N))//s� n�o est� funcionando para os monstros
            {
                menu = false; //fecha o menu

                estadoCarregado.info.nivel = 1; //come�a o n�vel em '1'

                //OPERA O ARQUIVO TEXTO (MAPA)
                FILE* fase;
                sprintf(estadoCarregado.nivel, "nivel%d.txt", estadoCarregado.info.nivel);//define o nome do arquivo a partir do contador de niveis
                fase = fopen(estadoCarregado.nivel, "r");//abre o arquivo
                if(fase == NULL) //verifica se foi bem sucedido
                {
                    fim_de_jogo = true;//se tornar verdadeiro, quer dizer que 1.A abertura do arquivo n�o teve sucesso ou 2.Acabaram-se os n�veis .txt e o jogador ganhou (em ambos os casos apresenta a mensagem 'WIN')
                }
                else
                {
                    for(i = 0; i < LINHAS; i++)
                    {
                        for(j = 0; j < COLUNAS; j++)
                        {
                            estadoCarregado.mapa[i][j] = getc(fase);//passa a matriz de caracteres do arquivo texto (MAPA) para uma matriz variavel do programa
                        }
                        getc(fase);//ignora '\n'
                    }
                }
                fclose(fase);//fecha o arquivo texto

                //REINICIA ALGUMAS VARI�VEIS E FUN��ES
                iniciaVariaveisEstadoCarregado(&estadoCarregado);
                estadoCarregado.info.nivel = 1;
                variaveisParaProximaFase(&estadoCarregado, contador_de_mov_criatura, &posInicial);
            }

            //CARREGA O �LTIMO JOGO SALVO
            if(IsKeyPressed(KEY_C))
            {
                menu = false;//fecha o menu
                if(carregaJogo(&estadoCarregado, arqbin)==1)
                {
                    printf("Jogo carregado com sucesso!\n");
                    leMapa(estadoCarregado);
                    initJogo(&estadoCarregado);
                }
                else
                    printf("Erro ao carregar jogo!\n");
            }

            //SALVA O JOGO NA POSI��O ATUAL
            if(IsKeyPressed(KEY_S))
            {
                menu = false;//fecha o menu
                if(salvaJogo(estadoCarregado, arqbin)==1)
                {
                    printf("Jogo salvo com sucesso!\n");
                }
                else
                    printf("Problema ao salvar jogo!\n");
            }

        }
        else  //SOMENTE SE O MENU ESTIVER DESATIVADO, VAI ATUALIZAR O JOGO
        {

            //TIMERS
            contador_de_mov_criatura++;//faz a contagem do tempo para a movimenta��o das criaturas

            //CONDI��ES
            if(IsKeyPressed(KEY_TAB))
                menu = true;//se TAB for pressionado abre um menu

            //REINICIA OS DESLOCAMENTOS DO PERSONAGEM
            estadoCarregado.jogador.persdx = 0;
            estadoCarregado.jogador.persdy = 0;

            //IDENTIFICA OS COMANDOS DE MOVIMENTO DO JOGADOR (PODENDO SER NA CONFIGURA��O A-W-S-D OU NAS SETAS)
            //para cada orienta��o, torna verdadeiro o lado para qual o personagem se voltou, e torna falso todos os outros.
            if(IsKeyPressed(KEY_D)||IsKeyPressed(KEY_RIGHT))
            {
                estadoCarregado.jogador.persdx = ARESTA;
                visao.jogBaixo = false;
                visao.jogCima = false;
                visao.jogEsquerda = false;
                visao.jogDireita = true;
            }
            else if(IsKeyPressed(KEY_A)||IsKeyPressed(KEY_LEFT))
            {
                estadoCarregado.jogador.persdx =- ARESTA;
                visao.jogBaixo = false;
                visao.jogCima = false;
                visao.jogEsquerda = true;
                visao.jogDireita = false;
            }
            else if(IsKeyPressed(KEY_W)||IsKeyPressed(KEY_UP))
            {
                estadoCarregado.jogador.persdy =- ARESTA;
                visao.jogBaixo = false;
                visao.jogCima = true;
                visao.jogEsquerda = false;
                visao.jogDireita = false;
            }
            else if(IsKeyPressed(KEY_S)||IsKeyPressed(KEY_DOWN))
            {
                estadoCarregado.jogador.persdy = ARESTA;
                visao.jogBaixo = true;
                visao.jogCima = false;
                visao.jogEsquerda = false;
                visao.jogDireita = false;
            }

            //BOMBAS
            if(estadoCarregado.info.bombas>0) //se no arsenal tiver mais do que uma bomba, � possivel plantar uma bomba
            {
                if(IsKeyPressed(KEY_B))
                {
                    if(estadoCarregado.bomba[0].bomba == false)//se a bomba '0' n�o estiver plantada, � possivel plant�-la
                        funcBomba(&estadoCarregado, 0);
                    else if(estadoCarregado.bomba[1].bomba == false)//se a bomba '1' n�o estiver plantada E a bomba '0' j� estiver plantada, � possivel plant�-la
                        funcBomba(&estadoCarregado, 1);
                    else //se a bomba '2' n�o estiver plantada E a bomba '0' e '1' j� estiverem plantadas, � possivel plant�-la
                        funcBomba(&estadoCarregado, 2);
                }
            }

            for(i = 0; i < 3; i++) //procura se o contador de alguma das 3 bombas chegou ao final
            {
                if((estadoCarregado.bomba[i].contador_de_explosao == 40) && (estadoCarregado.bomba[i].explosao == true)) //descobre se chegou a hora da explos�o acabar
                {
                    estadoCarregado.bomba[i].explosao = false;//desliga a explos�o
                }
                if(estadoCarregado.bomba[i].explosao == true) //procura se a explosao de alguma das bombas foi iniciada
                {
                    estadoCarregado.bomba[i].contador_de_explosao++;//comeca o contador de quadros daquela bomba
                }
                if(estadoCarregado.bomba[i].timer == true) //procura se o timer de alguma das 3 bombas foi inicializado
                {
                    estadoCarregado.bomba[i].contador_de_bomba++;//comeca o contador de quadros daquela bomba
                }
                if(estadoCarregado.bomba[i].contador_de_bomba < 180 && estadoCarregado.bomba[i].contador_de_bomba > 1) //enquanto o contador n�o chegar a 180 frames, ou seja, 3 segundos
                {
                    estadoCarregado.bomba[i].bomba = true;//a bomba esta plantada
                }
                if(estadoCarregado.bomba[i].contador_de_bomba == 180) //quando chega ao final
                {
                    explosao(&estadoCarregado, danoX, danoY, &perdeVida, i);//cria a explosao de dano
                    estadoCarregado.bomba[i].bomba = false;//a bomba passa a n�o existir mais
                    estadoCarregado.bomba[i].explosao = true;//sinaliza que ocorre uma explosao grafica
                    estadoCarregado.bomba[i].contador_de_explosao = 0;
                    estadoCarregado.info.bombas+=1;//reabastece o arsenal
                    for(i = 0; i < LINHAS; i++) //percorre a matriz mapa e verifica onde h� paredes destrutiveis
                    {
                        for(j = 0; j < COLUNAS; j++)
                        {
                            if(estadoCarregado.mapa[i][j] == 'D')//onde no mapa for uma parede destrut�vel
                            {
                                for(k = 0; k < 5; k++)
                                    if((i == (danoY[k]/ARESTA)) && (j == (danoX[k]/ARESTA))) //compara as posicoes das paredes destrutiveis com o RAIO DE DANO DA BOMBA que � em forma de '+'
                                    {
                                        estadoCarregado.mapa[i][j] = ' ';//se existir, substitui aquela parede por um espa�o vazio
                                        estadoCarregado.info.pontuacao+=10; //ganha 10 pontos por parede destruida
                                        initJogo(&estadoCarregado);//reinicializa o mapa, para guardar as altera�oes
                                    }
                            }
                        }

                    }
                }
            }

            //MOVIMENTA��O
            if(podeMover(estadoCarregado)== 0) //se puder mover
            {
                if(moveParaPocao(estadoCarregado.jogador, &estadoCarregado.pocao)== 0) //se for para cima de uma pocao
                {
                    int pontos = 50;
                    colhePocao(&estadoCarregado, pontos);//realiza o processo de captura da pocao
                    estadoCarregado.pocao.qntdP -= 1;//diminui uma pocao do vetor de pocoes
                    initJogo(&estadoCarregado);//reinicializa o mapa
                }

                if(moveParaSer(estadoCarregado, &serCapturado)== 0) //se for para cima de um ser
                {
                    if(estadoCarregado.seres[serCapturado].vivo == true)//e o ser estiver vivo
                    {
                        estadoCarregado.info.pontuacao += 10; //pontua
                        estadoCarregado.seres[serCapturado].vivo = false;//ser morto
                    }
                }
                if(moveParaMonstro(estadoCarregado, &monstroCapturado)== 0) //se for para cima de um monstro
                {
                    if(estadoCarregado.monstros[monstroCapturado].vivo == true)//e estiver vivo
                    {
                        perdeVida = true;
                    }
                }

                estadoCarregado.jogador.pos_dinamicaPersX += estadoCarregado.jogador.persdx;
                estadoCarregado.jogador.pos_dinamicaPersY += estadoCarregado.jogador.persdy;//desloca o personagem

            }

            //VIDA EXTRA
            for(i = 1; i < 10; i++)
            {
                if((estadoCarregado.info.pontuacao >= (1000 * i)) && (vida_extra == false)) //quando a pontua��o for multipla de 1000, ganha uma vida extra (em um m�ximo de 10.000)
                {
                    vida_extra = !vida_extra;//para n�o ficar repetindo
                    estadoCarregado.info.vidas += 1;//ganha uma vida
                }
            }
            if(perdeVida == true)
            {
                estadoCarregado.info.vidas -= 1;
                perdeVida = false;
                if(estadoCarregado.info.pontuacao >= 100)
                    estadoCarregado.info.pontuacao -= 100;//se tiver pontos suficientes para perder, perde
                else if(estadoCarregado.info.pontuacao < 100) //se n�o tiver pontos suficientes, zera o score, visando n�o torn�-lo negativo
                    estadoCarregado.info.pontuacao = 0;
                for(i = 0; i < estadoCarregado.contamonstros; i ++)//atualiza os monstros para as suas posi��es iniciais
                {
                    estadoCarregado.monstros[i].posX = posInicial.posX_monstro[i];
                    estadoCarregado.monstros[i].posY = posInicial.posY_monstro[i];
                }
                for(i = 0; i < estadoCarregado.contaseres; i++)//atualiza os seres para as suas posi��es iniciais
                {
                    estadoCarregado.seres[i].posX = posInicial.posX_seres[i];
                    estadoCarregado.seres[i].posY = posInicial.posY_seres[i];
                }
                estadoCarregado.jogador.pos_dinamicaPersX = posInicial.posicaoInicialX;
                estadoCarregado.jogador.pos_dinamicaPersY = posInicial.posicaoInicialY;//atualiza o jogador para sua posi��o inicial
            }

            //VERIFICA SE DEVE HAVER MOVIMENTA��O DOS MONSTROS
            if(contador_de_mov_criatura % 50 == 0) //movimenta as criaturas em multiplos de 50
                moveCriaturas(&estadoCarregado);

            if(estadoCarregado.contaseres == avancaNivel(estadoCarregado.contaseres-1, estadoCarregado))//chama fun��o recursiva para verificar quantidade de seres vivos ainda
                trocaNivel = true;

        }
        //GR�FICOS
        desenhaJogo(&estadoCarregado, menu, danoX, danoY, visao, textura);
    }
    descarregaTexturas(&textura);
    //MODOS DE ENCERRAMENTO DE JOGO
    //se o la�o for finalizado por motivos de falta de vidas, escreve "game over" e fecha a janela
    if(estadoCarregado.info.vidas == 0)
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("GAME OVER", 90, 70, 60, RED);
        EndDrawing();
        _sleep(2000);
    }
    //se o la�o for finalizado por vit�ria do jogador ou por erro de abertura de arquivo, escreve "win" e fecha a janela
    if(fim_de_jogo == true)
    {
        BeginDrawing();
        ClearBackground(BLUE);
        DrawText("WIN!", 165, 80, 95, RED);
        EndDrawing();
        _sleep(2000);
    }

    //se for apenas clicado "ESC" a janela s� fecha
    CloseWindow();
    return 0;
}

