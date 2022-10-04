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

//Protótipos------------------------------------------------------------------
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

//Funcão Principal------------------------------------------------------------
int main()
{
    //DECLARAÇÃO DE ALGUMAS VARIÁVEIS
    char arqbin[50] = "dadosSalvos.bin";//nome do arquivo binário para salvar e/ou carregar informações
    int i, j, k = 0;//variaveis auxiliares
    bool vida_extra = false; //bônus de vida
    int danoX[5];//raio de dano da bomba
    int danoY[5];//raio de dano da bomba
    bool perdeVida = false;//verifica se o jogador deve perder vida
    bool fim_de_jogo = false;//verifica se acabou o número de níveis
    int serCapturado;//número do ser que foi encostado pelo jogador
    int monstroCapturado;//número do monstro explodido pelo jogador
    int contador_de_mov_criatura = 0;//conta frames para decidir se a criatura deve mover ou não
    bool menu = false;//verifica se o 'menu' está aberto ou fechado
    bool trocaNivel = true;//se verdadeiro, faz avançar a fase
    VISAO visao;//variável para fins de textura do jogador
    visao.jogDireita = false, visao.jogEsquerda = false, visao.jogCima = false, visao.jogBaixo = true;
    ESTADO estadoCarregado;//variável do tipo ESTADO(estrutura principal)
    INICIAL posInicial;
    TEXTURAS textura;
    iniciaVariaveisEstadoCarregado(&estadoCarregado);//zera as variáveis do tipo estruturado ESTADO

    InitWindow(LARGURA, ALTURA, "O jogo");
    SetTargetFPS(60);
    carregaTexturas(&textura);

    //LOOP PRINCIPAL DO JOGO
    while (!WindowShouldClose() && estadoCarregado.info.vidas > 0 && fim_de_jogo == false)//O JOGO SÓ CONTINUA ENQUANTO 'ESC' NÃO FOR PRESSIONADO *E* ENQUANTO O JOGADOR TIVER PELO MENOS '1' VIDA
    {
        //FAZ O AVANÇO DE FASE (NO CASO DO PRIMEIRO JOGO, JÁ FARÁ O AVANÇO DA FASE '0' PARA A FASE '1'(PRIMEIRA EXISTENTE))
        if(trocaNivel == true)
        {
            estadoCarregado.info.nivel++;//já começa no nivel '1'
            trocaNivel = false;//já torna falso, para a condição não ficar se repetindo durante o while

            //OPERA O ARQUIVO TEXTO (MAPA)
            FILE* fase;
            sprintf(estadoCarregado.nivel, "nivel%d.txt", estadoCarregado.info.nivel);//define o nome do arquivo a partir do contador de niveis
            fase = fopen(estadoCarregado.nivel, "r");//abre o arquivo
            if(fase == NULL) //verifica se foi bem sucedido
            {
                fim_de_jogo = true;//se tornar verdadeiro, quer dizer que 1.A abertura do arquivo não teve sucesso ou 2.Acabaram-se os níveis .txt e o jogador ganhou (em ambos os casos apresenta a mensagem 'WIN')
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

            //REINICIA ALGUMAS VARIÁVEIS
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
            if(IsKeyPressed(KEY_N))//só não está funcionando para os monstros
            {
                menu = false; //fecha o menu

                estadoCarregado.info.nivel = 1; //começa o nível em '1'

                //OPERA O ARQUIVO TEXTO (MAPA)
                FILE* fase;
                sprintf(estadoCarregado.nivel, "nivel%d.txt", estadoCarregado.info.nivel);//define o nome do arquivo a partir do contador de niveis
                fase = fopen(estadoCarregado.nivel, "r");//abre o arquivo
                if(fase == NULL) //verifica se foi bem sucedido
                {
                    fim_de_jogo = true;//se tornar verdadeiro, quer dizer que 1.A abertura do arquivo não teve sucesso ou 2.Acabaram-se os níveis .txt e o jogador ganhou (em ambos os casos apresenta a mensagem 'WIN')
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

                //REINICIA ALGUMAS VARIÁVEIS E FUNÇÕES
                iniciaVariaveisEstadoCarregado(&estadoCarregado);
                estadoCarregado.info.nivel = 1;
                variaveisParaProximaFase(&estadoCarregado, contador_de_mov_criatura, &posInicial);
            }

            //CARREGA O ÚLTIMO JOGO SALVO
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

            //SALVA O JOGO NA POSIÇÃO ATUAL
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
            contador_de_mov_criatura++;//faz a contagem do tempo para a movimentação das criaturas

            //CONDIÇÕES
            if(IsKeyPressed(KEY_TAB))
                menu = true;//se TAB for pressionado abre um menu

            //REINICIA OS DESLOCAMENTOS DO PERSONAGEM
            estadoCarregado.jogador.persdx = 0;
            estadoCarregado.jogador.persdy = 0;

            //IDENTIFICA OS COMANDOS DE MOVIMENTO DO JOGADOR (PODENDO SER NA CONFIGURAÇÃO A-W-S-D OU NAS SETAS)
            //para cada orientação, torna verdadeiro o lado para qual o personagem se voltou, e torna falso todos os outros.
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
            if(estadoCarregado.info.bombas>0) //se no arsenal tiver mais do que uma bomba, é possivel plantar uma bomba
            {
                if(IsKeyPressed(KEY_B))
                {
                    if(estadoCarregado.bomba[0].bomba == false)//se a bomba '0' não estiver plantada, é possivel plantá-la
                        funcBomba(&estadoCarregado, 0);
                    else if(estadoCarregado.bomba[1].bomba == false)//se a bomba '1' não estiver plantada E a bomba '0' já estiver plantada, é possivel plantá-la
                        funcBomba(&estadoCarregado, 1);
                    else //se a bomba '2' não estiver plantada E a bomba '0' e '1' já estiverem plantadas, é possivel plantá-la
                        funcBomba(&estadoCarregado, 2);
                }
            }

            for(i = 0; i < 3; i++) //procura se o contador de alguma das 3 bombas chegou ao final
            {
                if((estadoCarregado.bomba[i].contador_de_explosao == 40) && (estadoCarregado.bomba[i].explosao == true)) //descobre se chegou a hora da explosão acabar
                {
                    estadoCarregado.bomba[i].explosao = false;//desliga a explosão
                }
                if(estadoCarregado.bomba[i].explosao == true) //procura se a explosao de alguma das bombas foi iniciada
                {
                    estadoCarregado.bomba[i].contador_de_explosao++;//comeca o contador de quadros daquela bomba
                }
                if(estadoCarregado.bomba[i].timer == true) //procura se o timer de alguma das 3 bombas foi inicializado
                {
                    estadoCarregado.bomba[i].contador_de_bomba++;//comeca o contador de quadros daquela bomba
                }
                if(estadoCarregado.bomba[i].contador_de_bomba < 180 && estadoCarregado.bomba[i].contador_de_bomba > 1) //enquanto o contador não chegar a 180 frames, ou seja, 3 segundos
                {
                    estadoCarregado.bomba[i].bomba = true;//a bomba esta plantada
                }
                if(estadoCarregado.bomba[i].contador_de_bomba == 180) //quando chega ao final
                {
                    explosao(&estadoCarregado, danoX, danoY, &perdeVida, i);//cria a explosao de dano
                    estadoCarregado.bomba[i].bomba = false;//a bomba passa a não existir mais
                    estadoCarregado.bomba[i].explosao = true;//sinaliza que ocorre uma explosao grafica
                    estadoCarregado.bomba[i].contador_de_explosao = 0;
                    estadoCarregado.info.bombas+=1;//reabastece o arsenal
                    for(i = 0; i < LINHAS; i++) //percorre a matriz mapa e verifica onde há paredes destrutiveis
                    {
                        for(j = 0; j < COLUNAS; j++)
                        {
                            if(estadoCarregado.mapa[i][j] == 'D')//onde no mapa for uma parede destrutível
                            {
                                for(k = 0; k < 5; k++)
                                    if((i == (danoY[k]/ARESTA)) && (j == (danoX[k]/ARESTA))) //compara as posicoes das paredes destrutiveis com o RAIO DE DANO DA BOMBA que é em forma de '+'
                                    {
                                        estadoCarregado.mapa[i][j] = ' ';//se existir, substitui aquela parede por um espaço vazio
                                        estadoCarregado.info.pontuacao+=10; //ganha 10 pontos por parede destruida
                                        initJogo(&estadoCarregado);//reinicializa o mapa, para guardar as alteraçoes
                                    }
                            }
                        }

                    }
                }
            }

            //MOVIMENTAÇÃO
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
                if((estadoCarregado.info.pontuacao >= (1000 * i)) && (vida_extra == false)) //quando a pontuação for multipla de 1000, ganha uma vida extra (em um máximo de 10.000)
                {
                    vida_extra = !vida_extra;//para não ficar repetindo
                    estadoCarregado.info.vidas += 1;//ganha uma vida
                }
            }
            if(perdeVida == true)
            {
                estadoCarregado.info.vidas -= 1;
                perdeVida = false;
                if(estadoCarregado.info.pontuacao >= 100)
                    estadoCarregado.info.pontuacao -= 100;//se tiver pontos suficientes para perder, perde
                else if(estadoCarregado.info.pontuacao < 100) //se não tiver pontos suficientes, zera o score, visando não torná-lo negativo
                    estadoCarregado.info.pontuacao = 0;
                for(i = 0; i < estadoCarregado.contamonstros; i ++)//atualiza os monstros para as suas posições iniciais
                {
                    estadoCarregado.monstros[i].posX = posInicial.posX_monstro[i];
                    estadoCarregado.monstros[i].posY = posInicial.posY_monstro[i];
                }
                for(i = 0; i < estadoCarregado.contaseres; i++)//atualiza os seres para as suas posições iniciais
                {
                    estadoCarregado.seres[i].posX = posInicial.posX_seres[i];
                    estadoCarregado.seres[i].posY = posInicial.posY_seres[i];
                }
                estadoCarregado.jogador.pos_dinamicaPersX = posInicial.posicaoInicialX;
                estadoCarregado.jogador.pos_dinamicaPersY = posInicial.posicaoInicialY;//atualiza o jogador para sua posição inicial
            }

            //VERIFICA SE DEVE HAVER MOVIMENTAÇÃO DOS MONSTROS
            if(contador_de_mov_criatura % 50 == 0) //movimenta as criaturas em multiplos de 50
                moveCriaturas(&estadoCarregado);

            if(estadoCarregado.contaseres == avancaNivel(estadoCarregado.contaseres-1, estadoCarregado))//chama função recursiva para verificar quantidade de seres vivos ainda
                trocaNivel = true;

        }
        //GRÁFICOS
        desenhaJogo(&estadoCarregado, menu, danoX, danoY, visao, textura);
    }
    descarregaTexturas(&textura);
    //MODOS DE ENCERRAMENTO DE JOGO
    //se o laço for finalizado por motivos de falta de vidas, escreve "game over" e fecha a janela
    if(estadoCarregado.info.vidas == 0)
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("GAME OVER", 90, 70, 60, RED);
        EndDrawing();
        _sleep(2000);
    }
    //se o laço for finalizado por vitória do jogador ou por erro de abertura de arquivo, escreve "win" e fecha a janela
    if(fim_de_jogo == true)
    {
        BeginDrawing();
        ClearBackground(BLUE);
        DrawText("WIN!", 165, 80, 95, RED);
        EndDrawing();
        _sleep(2000);
    }

    //se for apenas clicado "ESC" a janela só fecha
    CloseWindow();
    return 0;
}

