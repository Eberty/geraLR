/*
*-----------------------------------------------------------------------
*
*   Arquivo   : analex.c
*   Criado    : 2009-06-05
*   Modificado: 2016-07-17
*
*   DESCRICAO:
*
*
*   USO:
*   lidas -d lidas.mel -p prog01.lda svgOrigem.svg
*
*-----------------------------------------------------------------------
*/

/*                    */
/* This module's name */
/*                    */

/*
*-----------------------------------------------------------------------
* INCLUDE FILES
*-----------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

#include "mystrings.h"
#include "error.h"
#include "report.h"
#include "lexan-PT.h"
#include "commline.h"

/*
*-----------------------------------------------------------
* Definições para a Leitura de Ids do fonte SVG
*-----------------------------------------------------------
*/
int mostrar = 0,
    tempoInt = 0,
    tempoIntPor = 0,
    flagInicio = 0;
double tempoDec = 0,
       tempoDecPor = 0;

void buscaIdsSVG(const char *fileSVG);

typedef enum grupoTransicaoSVG {letra, digito, underline, menorQue, maiorQue, letra_i, letra_d, igual, branco, quebraDeLinha, aspas, outros, fimDeArquivo} e_grupoTransicaoSVG;

typedef struct
{
  int estadoFinal;   /* 0 = Não é estado final. 1 = É estado final*/
  int contaCaracter; /* Contabiliza o caracter lido */
}estado;

typedef struct registroId{
  char* lexema;
  struct registroId *proximo;
}t_registroId;

typedef struct {
  t_registroId  *p_primeiro;               /* Ponteiro p/ primeiro Id encontrado na entrada SVG */
  t_registroId  *p_ultimo;                 /* Ponteiro p/ ultimo Id encontrado na entrada SVG   */
}listaDeIds;

listaDeIds listaIds;

int pegaId(int cursor, int tabelaTransicaoId[6][12], estado estadosId[6]);

void fazTransicao(int *estadoAtualIdx, int tabelaTransicaoId[6][12], e_grupoTransicaoSVG transicao);
e_grupoTransicaoSVG defineGrupoTransicao(char c);
char pegarProximoChar(int cursor);

void inicializarListaDeIds(void);
void inicializarRegistroListaDeIds(t_registroId *registro, char *lexema);
void inserirNaListaDeIds(t_registroId *novoRegistro);

int buscarNaListaDeIds(char *iden);
void destroi_listaIds(void);

char* lerArquivo(const char * nomeDoArquivo);

/*
*-----------------------------------------------------------
* Definições para a saída do html com animações
*-----------------------------------------------------------
*/

char* htmlHeader[46] = {"<!DOCTYPE html>\n",
                   "<html lang=\"en\">\n",
                   "    <head>\n",
                   "        <meta charset=\"utf-8\">\n",
                   "        <script src=\"snap.svg-min.js\"></script>\n",
                   "        <script>\n",
                   "        window.onload = function () {\n",
                   "            var s = Snap(\"#SvgAnimado\");\n",
                   "            var funcIdex=0;\n",
                   "            var timeoutAnim = function(time, nextElem){\n",
                   "				s.animate({},time, nextElem);\n",
                   "			}\n",
                   "			var nextFrame = function(){\n",
                   "			if(funcIdex <= ultimaAnim){\n",
                   "					funcs[funcIdex]();\n",
                   "					funcIdex++;\n",
                   "				}\n",
                   "			}\n",
                   "			var previousFrame = function(){\n",
                   "				while(funcIdex>0 && names[funcIdex-1] == null){\n",
                   "					funcIdex--;\n",
                   "				}\n",
                   "				if(funcIdex > 0 && names[funcIdex-1] != null){\n",
                   "					if(s.select(\"#\"+names[funcIdex-1]).attr(\"opacity\")==0)\n",
                   "						s.select(\"#\"+names[funcIdex-1]).animate({opacity:1}, 200);\n",
                   "					else\n",
                   "						s.select(\"#\"+names[funcIdex-1]).animate({opacity: 0}, 200);\n",
                   "					funcIdex--;\n",
                   "				}\n",
                   "			}\n",
                   "			var showElem = function(elem, time, next){\n",
                   "				if(next == null)\n",
                   "					s.select(\"#\"+elem).animate({opacity: 1}, time);\n",
                   "				else{\n",
                   "					s.select(\"#\"+elem).animate({opacity: 1}, time, timeoutAnim(time, next));\n",
                   "					funcIdex++;\n",
                   "				}\n",
                   "			}\n",
                   "			var hideElem = function(elem, time, next){\n",
                   "				if(next == null)\n",
                   "					s.select(\"#\"+elem).animate({opacity: 0}, time);\n",
                   "				else{\n",
                   "					s.select(\"#\"+elem).animate({opacity: 0}, time, timeoutAnim(time, next));\n",
                   "					funcIdex++;\n",
                   "				}\n",
                   "			}\n"};

char* htmlBody[20] = {"\n			document.onmousedown = function (e) {\n",
                      "				nextFrame();\n",
                      "			}\n",
                      "			document.onkeydown = function (e) { \n",
                      "			 switch(e.which) {\n",
                      "			   case 27: case 37: case 65: // left\n",
                      "			        previousFrame();\n",
                      "			   break;\n",
                      "			   case 32: case 39: case 68: // right\n",
                      "					nextFrame();\n",
                      "			   break;\n",
                      "			   default: return;\n",
                      "			   }\n",
                      "			   e.preventDefault(); // prevent the default action (scroll / move caret)\n",
                      "			};\n",
                      "        };\n",
                      "        </script>\n",
                      "    </head>\n",
                      "    <body>\n",
                      "		<svg id=\"SvgAnimado\" width=\"2000px\" height=\"2000px\">\n"
                      };

char *htmlFooter = "		</svg>\n"
                   "    </body>\n"
                   "</html>\n";

char *fonteSVG;

typedef struct frame
{
  char* nome;
  int flagJuntos;
  int flagInicio;
  int tempoApos;        /*em milisegundos           */
  int tempoPor;         /*em milisegundos           */
  int acao;             /*1 = mostrar || 0 = apagar */
  struct frame *proximo;
}t_frame;

t_frame *novoFrame;

typedef struct
{
  t_frame *p_primeiro;
  t_frame *p_ultimo;
  int quantidadeDeFrames;
}listaDeFrames;

listaDeFrames listaFrames;

typedef struct defIden
{
  char* iden;
  char* cadeia;
  struct defIden *proximo;
}t_defIden;

typedef struct
{
 t_defIden *p_primeiro;
 t_defIden *p_ultimo;
}listaIdentificadoresDefinidos;

listaIdentificadoresDefinidos listaDefs;

void inicializarListaDeFrames(void);
void inicializarRegistroListaDeFrames(t_frame *registro, char *nome, int flagJuntos, int flagIni, int tempoApos, int tempoPor, int acao);
void inserirNaListaDeFrames(t_frame *novoRegistro);

void destroi_listaFrames(void);

void inicializarListaDeIdentificadores(void);
void inicializarRegistroListaDeIdentificadores(t_defIden *registro, char *iden, char *cadeia);
void inserirNaListaDeIdentificadores(t_defIden *novoRegistro);

int identificarDefinido(char *iden);
void destroi_listaDefs(void);

void escreveSaidaHtml(const char* nomeDoArquivoFonte);

void escreveParaArquivo(char *nomeDoArquivo, char *conteudo);
void concatenaEmArquivo(char *nomeDoArquivo, char *conteudo);

/* Estrutura para armazenar identificadores utilizados (util para inicialização da animação) */
typedef struct idenUtilizado
{
  char* iden;
  int acaoInicial; /* 1 = mostrar; 0 = apagar; */
  struct idenUtilizado *proximo;
}t_idenUtilizado;

typedef struct
{
 t_idenUtilizado *p_primeiro;
 t_idenUtilizado *p_ultimo;
}listaIdenUtilizados;

listaIdenUtilizados listaIdenUt;

void inicializarListaIdenUtilizados(void);
void inicializarRegistroListaIdenUtilizados(t_idenUtilizado *registro, char *iden, int acaoInicial);
void inserirNaListaDeIdenUtilizados(t_idenUtilizado *novoRegistro);

int idenJaFoiUtilizado(char *iden);
void destroi_listaIdsUtils(void);

/*
*-----------------------------------------------------------------------
* Definicoes para operacoes de entrada e saida
*-----------------------------------------------------------------------
*/

/*
*-----------------------------------------------------------------------
* Para gerar impressoes adicionais de estruturas de dados internas,
* descomente o #define a seguir
*-----------------------------------------------------------------------

#define GERA_SAIDA_DEBUG

*/

/*                                      */
/* Definicoes para os arquivos de saida */
/*                                      */

#define TAM_MAX_NOME_ARQUIVO         512

#define EXTENSAO_SAIDA_META_TOKENS   ".mtk"
#define EXTENSAO_SAIDA_META_ERROS    ".mer"

#define EXTENSAO_SAIDA_TOKENS        ".tok"
#define EXTENSAO_SAIDA_ERROS         ".err"
#define EXTENSAO_SAIDA_TABSIMB       ".tbl"

char
  *progName = NULL,
  arq_Entrada_Ortografia_Nome [TAM_MAX_NOME_ARQUIVO] = "",
  arq_Saida_Meta_Tokens_Nome  [TAM_MAX_NOME_ARQUIVO] = "",
  arq_Saida_Meta_Erros_Nome   [TAM_MAX_NOME_ARQUIVO] = "",
  arq_Entrada_Programa_Nome   [TAM_MAX_NOME_ARQUIVO] = "",
  arq_Saida_Erros_Nome        [TAM_MAX_NOME_ARQUIVO] = "",
  arq_Saida_Tokens_Nome       [TAM_MAX_NOME_ARQUIVO] = "",
  arq_Saida_TabSimb_Nome      [TAM_MAX_NOME_ARQUIVO] = "",
  arq_Entrada_SVG_Nome        [TAM_MAX_NOME_ARQUIVO] = "";

FILE
  *arq_Entrada_Ortografia_Ptr = NULL,
  *arq_Saida_Meta_Tokens_Ptr  = NULL,
  *arq_Saida_Meta_Erros_Ptr   = NULL,
  *arq_Entrada_Programa_Ptr   = NULL,
  *arq_Saida_Erros_Ptr        = NULL,
  *arq_Saida_Tokens_Ptr       = NULL,
  *arq_Saida_TabSimb_Ptr      = NULL,
  *arq_Entrada_SVG_Ptr        = NULL;

/*                                          */
/* Definicoes para as impressoes das saidas */
/*                                          */

#define SEPARADOR_TABELA_DETALHE_META_TOKENS   '|'
#define SEPARADOR_TABELA_RESUMO_META_TOKENS    '|'
#define TITULO_ULT_COL_RESUMO_META_TOKENS      "USOS"

#define SEPARADOR_TABELA_DETALHE_TOKENS   '|'
#define SEPARADOR_TABELA_RESUMO_TOKENS    '|'
#define TITULO_ULT_COL_LISTA_TOKENS       "POS TAB SIMB"
#define TITULO_ULT_COL_RESUMO_TOKENS      "USOS"

#define TITULO_ULT_COL_TAB_SIMBOLOS   "POS NA ENTRADA (linha,coluna)"
#define SEPARADOR_TABELA_SIMBOLOS     '|'

#define TAM_MAX_TRACOS_TABELAS   10000

/* Formatos de impressao para posicoes no arquivo de entrada */

#define FORMATO_POS_ENTRADA_LIN       "%3d"
#define FORMATO_POS_ENTRADA_COL       "%3d"
#define FORMATO_POS_ENTRADA_LIN_COL   "(%3d,%3d)"

/* Largura (nr. de caracteres) de diversos campos impressos nas saidas */

size_t
  tamPosEntradaLin    = 0,
  tamPosEntradaCol    = 0,
  tamPosEntradaLinCol = 0;

char
  tracos_tab_meta_tokens        [TAM_MAX_TRACOS_TABELAS] = "",
  tracos_tab_resumo_meta_tokens [TAM_MAX_TRACOS_TABELAS] = "",
  tracos_tab_tokens             [TAM_MAX_TRACOS_TABELAS] = "",
  tracos_tab_resumo_tokens      [TAM_MAX_TRACOS_TABELAS] = "",
  tracos_tab_simbolos           [TAM_MAX_TRACOS_TABELAS] = "";

/*
*--------------------------------------------------------------------------------------------
* Definicoes para processamento do arquivo texto contendo a ortografia da linguagem LiDAS
*--------------------------------------------------------------------------------------------
*/

/* Meta-comandos (comandos da linguagem Melinda) distinguem entre caixa alta e caixa baixa? */

#define META_COMANDOS_CASE_SENSITIVE  FALSE

/* Lista de meta-tokens (tokens usados na definicao da ortografia de LiDAS) */

#define TAM_LISTA_META_TOKENS      15
#define COD_PRIMEIRA_META_PAL_RES  1
#define COD_ULTIMA_META_PAL_RES    ((TAM_LISTA_META_TOKENS)-1)

/* Tamanhos para declaracao de arrays */

#define TAM_MAX_CADEIA_META_TOKEN  30
#define TAM_MAX_REGEX_META_TOKEN   (4*(TAM_MAX_CADEIA_META_TOKEN))
#define TAM_MAX_ROTULO_META_TOKEN  30
#define TAM_MAX_META_STRING        50

/* Expressoes regulares dos meta-comentarios na definicao da ortografia de LiDAS */

#define META_REGEX_COMENTARIO        "/#"      /* Inicia comentario de linha */
#define META_REGEX_ABRE_COMENTARIO   "\\(#"    /* Abre   comentario de bloco */
#define META_REGEX_FECHA_COMENTARIO  "#\\)"    /* Fecha  comentario de bloco */

/* Expressoes regulares dos meta-literais na definicao da ortografia de LiDAS */

#define META_REGEX_STRING_ASPAS_SIMPLES  "('[^']*')"
#define META_REGEX_STRING_ASPAS_DUPLAS   "(\"[^\"]*\")"

/* Numero maximo de comandos (palavras reservadas) em LiDAS */

#define NRO_MAX_COMANDOS  100

/* Formatos de impressao para meta-tokens (codigos numericos e cadeias) */

#define FORMATO_CODIGO_META_TOKEN   "%3d"
#define FORMATO_ROTULO_META_TOKEN   "%-30s"
#define FORMATO_NR_USOS_META_TOKEN  "%3d"

typedef enum {
  metaTk_abre = 1        ,
  metaTk_caixa           ,
  metaTk_comandos        ,
  metaTk_comentarios     ,
  metaTk_decimal         ,
  metaTk_distinguem      ,
  metaTk_fecha           ,
  metaTk_identificador   ,
  metaTk_identificadores ,
  metaTk_ignoram         ,
  metaTk_inteiro         ,
  metaTk_lexemas         ,
  metaTk_linha           ,
  metaTk_string          ,
  metaTk_metaString
}
  t_codigoMetaToken;

/* Largura (nr. de caracteres) de diversos campos impressos nas saidas */

size_t
  tamCodigoMetaToken   = 0,
  tamNrUsosMetaToken   = 0;

/* Lista de tokens RECONHECIVEIS na definicao da ortografia da linguagem LiDAS;   */

typedef struct {
  t_codigoMetaToken  codigoMetaToken;                              /* Codigo numerico unico do meta token                        */
  char               cadeiaMetaToken [TAM_MAX_CADEIA_META_TOKEN];  /* Como o meta token aparece na entrada (apenas minusculas)   */
  char               regexMetaToken  [TAM_MAX_REGEX_META_TOKEN];   /* Como o meta token sera reconhecido (sua expressao regular) */
  char               rotuloMetaToken [TAM_MAX_ROTULO_META_TOKEN];  /* Como o meta token sera impresso na saida                   */
  unsigned int       usosMetaToken;                                /* Quantas vezes o meta token apareceu na entrada             */

}
  t_listaMetaTokens;

t_listaMetaTokens
  listaMetaTokens [TAM_LISTA_META_TOKENS] =
  {
    {metaTk_abre            , "abre"               , "" ,                     "metaTk_abre"             , 0},
    {metaTk_caixa           , "caixa"              , "" ,                     "metaTk_caixa"            , 0},
    {metaTk_comandos        , "comandos"           , "" ,                     "metaTk_comandos"         , 0},
    {metaTk_comentarios     , "comentarios"        , "" ,                     "metaTk_comentarios"      , 0},
    {metaTk_decimal         , "decimal"            , "" ,                     "metaTk_decimal"          , 0},
    {metaTk_distinguem      , "distinguem"         , "" ,                     "metaTk_distinguem"       , 0},
    {metaTk_fecha           , "fecha"              , "" ,                     "metaTk_fecha"            , 0},
    {metaTk_identificador   , "identificador"      , "" ,                     "metaTk_identificador"    , 0},
    {metaTk_identificadores , "identificadores"    , "" ,                     "metaTk_identificadores"  , 0},
    {metaTk_ignoram         , "ignoram"            , "" ,                     "metaTk_ignoram"          , 0},
    {metaTk_inteiro         , "inteiro"            , "" ,                     "metaTk_inteiro"          , 0},
    {metaTk_lexemas         , "lexemas"            , "" ,                     "metaTk_lexemas"          , 0},
    {metaTk_linha           , "linha"              , "" ,                     "metaTk_linha"            , 0},
    {metaTk_string          , "string"             , "" ,                     "metaTk_string"           , 0},
    {metaTk_metaString      , ""                   , LEXAN_REGEX_DOUBLE_STR , "metaTk_metaString"       , 0}
  };

unsigned int
  totMetaTokens = 0,  /* Numero de meta-tokens encontrados na especificacao ortografica de LiDAS */
  totComandos   = 0,  /* Numero de comandos (palavras reservadas) na especificacao de LiDAS      */
  totMetaErros  = 0;  /* Numero de erros sintaticos na especificacao ortografica de LiDAS        */

LEXAN_t_tokenLocation
  emptyMetaTokenLocation = {0,0};

/*
*--------------------------------------------------------------------------------------------
* Definicoes para processamento do arquivo texto contendo o programa (codigo fonte) em LiDAS
*--------------------------------------------------------------------------------------------
*/

/* Tamanhos para declaracao de arrays */

#define TAM_MAX_LISTA_TOKENS   200  /* O nr real de tokens na linguagem LiDAS so sera sabido qdo o arquivo de definicao for lido */
#define TAM_MAX_CADEIA_TOKEN   TAM_MAX_CADEIA_META_TOKEN
#define TAM_MAX_ROTULO_TOKEN   TAM_MAX_ROTULO_META_TOKEN
#define TAM_MAX_REGEX_TOKEN    (4*(TAM_MAX_CADEIA_TOKEN)*(NRO_MAX_COMANDOS))

/* Formatos de impressao para tokens (codigos numericos e cadeias) */

#define FORMATO_CODIGO_TOKEN   FORMATO_CODIGO_META_TOKEN
#define FORMATO_ROTULO_TOKEN   FORMATO_ROTULO_META_TOKEN
#define FORMATO_NR_USOS_TOKEN  FORMATO_NR_USOS_META_TOKEN

/* Formatos de impressao para lexemas (codigos numericos e cadeias) */

#define FORMATO_LEXEMA_STRING   "%s"
#define FORMATO_LEXEMA_INTEIRO  "%ld"
#define FORMATO_LEXEMA_DOUBLE   "%f"

/* Formato de impressao para posicoes na tabela de simbolos */

#define FORMATO_POS_TABSIMB   "%3d"

/* Token final-de-arquivo (end-of-file) */

#define CADEIA_TK_EOF  "EOF"
#define ROTULO_TK_EOF  "tk_EOF"

typedef int
  t_codigoToken;

/* Largura (nr. de caracteres) de diversos campos impressos nas saidas */

unsigned int
  tamCodigoToken = 0,
  tamPosTabSimb  = 0,
  tamNrUsosToken = 0;

/* Lista de tokens RECONHECIVEIS em determinado programa escrito em LiDAS.                                    */
/* Nao confundir com:                                                                                            */
/*  - meta-tokens (tokens na definicao da ortografia da linguagem LiDAS), armazenados em listaMetaTokens[]    */
/*  - tokens encontrados no codigo fonte escrito em LiDAS (nao sao armazenados em nenhuma estrutura de dados) */

typedef struct {
  t_codigoToken     codigoToken;                         /* Codigo numerico unico do token                        */
  LEXAN_t_tokenType tipoToken;                           /* Tipo do token (um dos varios tipos pre-definidos)     */
  char              cadeiaToken [TAM_MAX_CADEIA_TOKEN];  /* Como o token aparece na entrada (apenas minusculas)   */
  char              regexToken  [TAM_MAX_REGEX_TOKEN];   /* Como o token sera reconhecido (sua expressao regular) */
  char              rotuloToken [TAM_MAX_ROTULO_TOKEN];  /* Como o token sera impresso na saida                   */
  unsigned int      usosToken;                           /* Quantas vezes o token apareceu na entrada             */
}
  t_listaTokens;

t_listaTokens
  listaTokens [TAM_MAX_LISTA_TOKENS];

int
  tamListaTokens = 0,  /* Numero de tokens distintos na linguagem LiDAS         */
  totTokens      = 0;  /* Numero de tokens encontrados no codigo fonte em LiDAS */

LEXAN_t_tokenLocation
  emptyTokenLocation = {0,0};

/* Meta-literais encontrados na definicao da ortografia da linguagem LiDAS */

char
  *lidas_regex_comentario_linha           ,
  *lidas_regex_comentario_abre            ,
  *lidas_regex_comentario_fecha           ,
  *lidas_regex_lexema_identificador       ,
  *lidas_regex_lexema_inteiro             ,
  *lidas_regex_lexema_decimal             ,
  *lidas_regex_lexema_string              ,
  *lidas_regex_TOKEN_double               ,
  *lidas_regex_TOKEN_resWord              ,
  *lidas_rotulo_lexema_identificador      ,
  *lidas_rotulo_lexema_inteiro            ,
  *lidas_rotulo_lexema_decimal            ,
  *lidas_rotulo_lexema_string             ,
  *lidas_cadeia_comando [NRO_MAX_COMANDOS],
  *lidas_regex_comando  [NRO_MAX_COMANDOS],
  *lidas_rotulo_comando [NRO_MAX_COMANDOS];

/* Array usado por lexan.c, sera alocado com malloc dependendo dos meta-tokens encontrados */

LEXAN_t_tokenSpecs
  *p_tokenSpecs;

int
  totTokenSpecs = 0;

/*
*-----------------------------------------------------------------------------------------------
* Definicoes para processamento de caixa alta e caixa baixa em identificadores e comandos
*-----------------------------------------------------------------------------------------------
*/

typedef enum {
  t_caixa_indefinido,
  t_caixa_distinguem,
  t_caixa_ignoram
}
  t_caixa;

t_caixa
  caixa_Identificadores = t_caixa_indefinido,
  caixa_Comandos        = t_caixa_indefinido;

/*
*-----------------------------------------------------------------------------------------------
* Definicoes para tabela de simbolos do programa em LiDAS
*-----------------------------------------------------------------------------------------------
*/

/* Tamanhos para declaracao de arrays */

#define TAM_MAX_LEXEMA   50
#define TAM_MAX_TABSIMB  5000

/* Tamanhos para declaracao de arrays */

struct posEntrada {
  unsigned int linha;
  unsigned int coluna;
  struct posEntrada *proxEntrada;
};

typedef struct posEntrada t_posEntrada;

typedef struct {
  t_codigoToken  codigoToken;              /* Codigo numerico do respectivo token        */
  char           lexema [TAM_MAX_LEXEMA];  /* Cadeia do lexema                           */
  unsigned int   usosToken;                /* Quantas vezes o lexema ocorre na entrada   */
  t_posEntrada  *p_primeira;               /* Ponteiro p/ primeira ocorrências do lexema */
  t_posEntrada  *p_ultima;                 /* Ponteiro p/ ultima ocorrências do lexema   */
}
  t_tabSimb;

t_tabSimb
  tabSimb [TAM_MAX_TABSIMB];

unsigned int
  totLexemas = 0;

/*
*---------------------------------------------------------------------
* Function prototypes
*---------------------------------------------------------------------
*/

char *strdup (const char*);

Bool cadeiasEquivalentes (char *cadeia1, char *cadeia2, Bool caseSensitive);

Bool tokenTemLexema (LEXAN_t_tokenType token);

t_codigoMetaToken cadeiaMetaToken2codigoMetaToken (char *cadeia, Bool caseSensitive);

char *cadeia2regex (char *cadeia, Bool caseSensitive);

void imprime_linha_entrada_ortografia (int nroLinha, char *linha);

void imprime_cabecalho_meta_erros (char *nomeArqEntradaOrtografia);

void imprime_meta_erro (int nroLinha, int nroColuna, char *strErro);

void imprime_cabecalho_meta_tokens (char *nomeArqEntradaOrtografia);

void imprime_detalhes_meta_token (
  LEXAN_t_tokenLocation metaTokenLocation,
  LEXAN_t_tokenType         metaToken,
  LEXAN_t_tokenVal      tokenVal );

void imprime_resumo_meta_tokens (void);

void imprime_resumo_meta_token (
  t_codigoMetaToken  codigoMetaToken,
  int                usosMetaToken);

void inicializa_tabSimb (void);

int  instala_na_tabSimb (
  t_codigoToken          codigoToken,
  char                  *lexema,
  LEXAN_t_tokenLocation  tokenLocation,
  Bool                   caseSensitive );

void imprime_tabSimb (char *nomeArqEntrada);

void destroi_tabSimb (void);

t_codigoToken tipoToken2codigoToken (LEXAN_t_tokenType tipoToken);

t_codigoToken cadeiaToken2codigoToken (char *cadeia, Bool caseSensitive);

void imprime_linha_entrada_programa (int nroLinha, char *linha);

void imprime_cabecalho_erros  (char *nomeArqEntradaPrograma);

void imprime_cabecalho_tokens (char *nomeArqEntradaPrograma);

void imprime_detalhes_token (
  LEXAN_t_tokenLocation tokenLocation,
  LEXAN_t_tokenType     token,
  t_codigoToken         codigoToken,
  LEXAN_t_tokenVal      tokenVal,
  int                   posTabSimb );

void imprime_resumo_token (
  char            *rotuloToken,
  t_codigoToken    codigoToken,
  int              infoToken );

void imprime_resumo_tokens (void);

void insere_novo_token (
  LEXAN_t_tokenType  token,
  char          *cadeiaNovoToken,
  char          *regexNovoToken,
  char          *rotuloNovoToken );

void prepara_arquivos_saida (void);

void processa_linhaDeComando (int argc, char *argv[]);

Bool processa_definicoesDeTokens (void);

void processa_fonteEmLidas (void);

int faz_analiseSintatica (void);

int tkDefinaEncontrado(void);
int tkMostreApagueEncontrado(void);
int tkUmAUmEncontrado(char *nome, int umAum);

void prepara_para_sair (void);

int  main (int argc, char *argv[]);

/*
*----------------------------------------------------------------------------------
* Dadas duas cadeias (strings), verifica se sao iguais, considerando ou ignorando
* diferencas entre maiusculas e minusculas
*----------------------------------------------------------------------------------
*/

Bool cadeiasEquivalentes (char *cadeia1, char *cadeia2, Bool caseSensitive)
{
 int
   iChar,
   tam1,
   tam2;
 char
   *p1 = cadeia1,
   *p2 = cadeia2;

 tam1 = strlen (cadeia1);
 tam2 = strlen (cadeia2);
 if (tam1 != tam2)
   return (FALSE);
 for (iChar = 0; iChar < tam1; iChar++, p1++, p2++) {
   if (tolower(*p1) != tolower(*p2))
     return (FALSE);
   if (caseSensitive && (*p1 != *p2))
     return (FALSE);
 }
 return (TRUE);
}

/*
*---------------------------------------------------------------------
* Retorna um Booleano indicando se determinado token possui lexema
*---------------------------------------------------------------------
*/

Bool tokenTemLexema (LEXAN_t_tokenType token)
{
  switch (token) {
    case (LEXAN_token_iden)       :
    case (LEXAN_token_longInt)    :
    case (LEXAN_token_double)     :
    case (LEXAN_token_single_str) :
    case (LEXAN_token_double_str) :
      return (TRUE);
    case (LEXAN_token_resWord)    :
    case (LEXAN_token_delim)      :
    case (LEXAN_token_EOF)        :
      return (FALSE);
    case (LEXAN_token_LINE_COMMENT)      :
    case (LEXAN_token_OPEN_COMMENT)  :
    case (LEXAN_token_CLOSE_COMMENT) :
    case (LEXAN_token_NULL)         :
    default                   :
      errno = 0;
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Tipo de token invalido (%d)", token);
      ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
}

/*
*---------------------------------------------------------------------
* Toma como entrada uma cadeia (string) e retorna um ponteiro para a
* expressao regular equivalente, armazenada em um string estatico.
* E necessario duplicar o string retornado com strdup().
*---------------------------------------------------------------------
*/

char *cadeia2regex (char *cadeia, Bool caseSensitive)
{
  unsigned int
    iChar;
  static char
    regex[1000];
  char
    *p_cadeia,
    *p_regex;

  memset (regex, 0, 1000);
  p_regex = regex;
  for (iChar = 0, p_cadeia = cadeia; iChar < strlen(cadeia); iChar++, p_cadeia++ ) {

    /* We need to escape meta symbols (characters that have special meanings in regular expressions) */

    if ( (*p_cadeia == '+') ||
         (*p_cadeia == '*') ||
         (*p_cadeia == '|') ||
         (*p_cadeia == '.') ||
         (*p_cadeia == '^') ||
         (*p_cadeia == '$') ||
         (*p_cadeia == '(') ||
         (*p_cadeia == ')') ||
         (*p_cadeia == '{') ||
         (*p_cadeia == '}') )
    {
      *(p_regex++) = '\\';
      *(p_regex++) = *p_cadeia;
    }
    else if ((tolower (*p_cadeia) == toupper (*p_cadeia)) || caseSensitive)
      *(p_regex++) = *p_cadeia;
    else {
      *(p_regex++) = '[';
      *(p_regex++) = tolower (*p_cadeia);
      *(p_regex++) = toupper (*p_cadeia);
      *(p_regex++) = ']';
    }
  }
  return (regex);
}

/*
*---------------------------------------------------------------------
* Preparacoes diversas para posterior impressao das saidas
*---------------------------------------------------------------------
*/

#ifdef strAuxLen
# undef strAuxLen
#endif
#define strAuxLen  30

void prepara_arquivos_saida (void)
{
  unsigned int
    iChar,
    iComando;
  char
    strAux [strAuxLen];

  lidas_regex_comentario_linha      = STRING_allocate();
  lidas_regex_comentario_abre       = STRING_allocate();
  lidas_regex_comentario_fecha      = STRING_allocate();
  lidas_regex_lexema_identificador  = STRING_allocate();
  lidas_regex_lexema_inteiro        = STRING_allocate();
  lidas_regex_lexema_decimal        = STRING_allocate();
  lidas_regex_lexema_string         = STRING_allocate();
  lidas_regex_TOKEN_double          = STRING_allocate();
  lidas_regex_TOKEN_resWord         = STRING_allocate();
  lidas_rotulo_lexema_identificador = STRING_allocate();
  lidas_rotulo_lexema_inteiro       = STRING_allocate();
  lidas_rotulo_lexema_decimal       = STRING_allocate();
  lidas_rotulo_lexema_string        = STRING_allocate();

  for (iComando = 0; iComando < NRO_MAX_COMANDOS; iComando++) {
    lidas_cadeia_comando[iComando] = STRING_allocate();
    lidas_regex_comando [iComando] = STRING_allocate();
    lidas_rotulo_comando[iComando] = STRING_allocate();
  }

  /*                                                           */
  /* Calcula a largura de diversos campos impressos nas saidas */
  /*                                                           */

  snprintf (strAux, strAuxLen, FORMATO_CODIGO_META_TOKEN,   0);     tamCodigoMetaToken  = strlen (strAux);
  snprintf (strAux, strAuxLen, FORMATO_NR_USOS_META_TOKEN,  0);     tamNrUsosMetaToken  = strlen (strAux);
  snprintf (strAux, strAuxLen, FORMATO_CODIGO_TOKEN,        0);     tamCodigoToken      = strlen (strAux);
  snprintf (strAux, strAuxLen, FORMATO_NR_USOS_TOKEN,       0);     tamNrUsosToken      = strlen (strAux);
  snprintf (strAux, strAuxLen, FORMATO_POS_TABSIMB,         0);     tamPosTabSimb       = strlen (strAux);
  snprintf (strAux, strAuxLen, FORMATO_POS_ENTRADA_LIN,     0);     tamPosEntradaLin    = strlen (strAux);
  snprintf (strAux, strAuxLen, FORMATO_POS_ENTRADA_COL,     0);     tamPosEntradaCol    = strlen (strAux);
  snprintf (strAux, strAuxLen, FORMATO_POS_ENTRADA_LIN_COL, 0, 0);  tamPosEntradaLinCol = strlen (strAux);

  /*                                                                                 */
  /* Constroi strings do tipo "+------+------+" para cabecalhos de tabelas impressas */
  /*                                                                                 */

  /* Lista de meta-tokens reconhecidos na definicao ortografica de LiDAS */

  strcpy (tracos_tab_meta_tokens, "+");
  for (iChar = 0; iChar < tamPosEntradaLin + 2; iChar++)
    strcat (tracos_tab_meta_tokens, "-");
  strcat (tracos_tab_meta_tokens, "+");
  for (iChar = 0; iChar < tamPosEntradaCol + 2; iChar++)
    strcat (tracos_tab_meta_tokens, "-");
  strcat (tracos_tab_meta_tokens, "+");
  for (iChar = 0; iChar < tamCodigoMetaToken + 2; iChar++)
    strcat (tracos_tab_meta_tokens, "-");
  strcat (tracos_tab_meta_tokens, "+");
  for (iChar = 0; iChar < TAM_MAX_ROTULO_META_TOKEN + 2; iChar++)
    strcat (tracos_tab_meta_tokens, "-");
  strcat (tracos_tab_meta_tokens, "+");
  for (iChar = 0; iChar < TAM_MAX_META_STRING + 2; iChar++)
    strcat (tracos_tab_meta_tokens, "-");
  strcat (tracos_tab_meta_tokens, "+");

  /* Resumo dos meta-tokens reconhecidos na definicao ortografica de LiDAS */

  strcpy (tracos_tab_resumo_meta_tokens, "+");
  for (iChar = 0; iChar < tamCodigoMetaToken + 2; iChar++)
    strcat (tracos_tab_resumo_meta_tokens, "-");
  strcat (tracos_tab_resumo_meta_tokens, "+");
  for (iChar = 0; iChar < TAM_MAX_ROTULO_META_TOKEN + 2; iChar++)
    strcat (tracos_tab_resumo_meta_tokens, "-");
  strcat (tracos_tab_resumo_meta_tokens, "+");
  for (iChar = 0; iChar < strlen(TITULO_ULT_COL_RESUMO_META_TOKENS) + 2; iChar++)
    strcat (tracos_tab_resumo_meta_tokens, "-");
  strcat (tracos_tab_resumo_meta_tokens, "+");

  /* Lista de tokens reconhecidos no codigo fonte em LiDAS */

  strcpy (tracos_tab_tokens, "+");
  for (iChar = 0; iChar < tamPosEntradaLin + 2; iChar++)
    strcat (tracos_tab_tokens, "-");
  strcat (tracos_tab_tokens, "+");
  for (iChar = 0; iChar < tamPosEntradaCol + 2; iChar++)
    strcat (tracos_tab_tokens, "-");
  strcat (tracos_tab_tokens, "+");
  for (iChar = 0; iChar < tamCodigoToken + 2; iChar++)
    strcat (tracos_tab_tokens, "-");
  strcat (tracos_tab_tokens, "+");
  for (iChar = 0; iChar < TAM_MAX_ROTULO_META_TOKEN + 2; iChar++)
    strcat (tracos_tab_tokens, "-");
  strcat (tracos_tab_tokens, "+");
  for (iChar = 0; iChar < TAM_MAX_LEXEMA + 2; iChar++)
    strcat (tracos_tab_tokens, "-");
  strcat (tracos_tab_tokens, "+");
  for (iChar = 0; iChar < strlen(TITULO_ULT_COL_LISTA_TOKENS) + 2; iChar++)
    strcat (tracos_tab_tokens, "-");
  strcat (tracos_tab_tokens, "+");

  /* Resumo de tokens reconhecidos no codigo fonte em LiDAS */

  strcpy (tracos_tab_resumo_tokens, "+");
  for (iChar = 0; iChar < tamCodigoToken + 2; iChar++)
    strcat (tracos_tab_resumo_tokens, "-");
  strcat (tracos_tab_resumo_tokens, "+");
  for (iChar = 0; iChar < TAM_MAX_ROTULO_TOKEN + 2; iChar++)
    strcat (tracos_tab_resumo_tokens, "-");
  strcat (tracos_tab_resumo_tokens, "+");
  for (iChar = 0; iChar < strlen(TITULO_ULT_COL_RESUMO_TOKENS) + 2; iChar++)
    strcat (tracos_tab_resumo_tokens, "-");
  strcat (tracos_tab_resumo_tokens, "+");
}

#undef strAuxLen

/*
*---------------------------------------------------------------------
*
* PROCESSAMENTO DE META-TOKENS NA DEFINICAO ORTOGRAFICA DE LiDAS
*
*---------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
* Dada uma cadeia (string), verifica se e uma palavra reservada ou
* um delimitador; se for, retorna o codigo do respectivo meta-token,
* senao retorna zero.
*---------------------------------------------------------------------
*/

t_codigoMetaToken cadeiaMetaToken2codigoMetaToken (char *cadeia, Bool caseSensitive)
{
 int
   iMetaToken;
 char
   inicioCadeia,
   fimCadeia;

 /* Eh um meta-string? */

 inicioCadeia = cadeia[0];
 fimCadeia = cadeia[strlen(cadeia)-1];
 if ((inicioCadeia == fimCadeia) && ((inicioCadeia == '"') || (inicioCadeia == '\'')))
   return (metaTk_metaString);

 /* Nao e um meta-string. Sera uma palavra reservada? */

 for (iMetaToken = COD_PRIMEIRA_META_PAL_RES; iMetaToken <= COD_ULTIMA_META_PAL_RES; iMetaToken++)
   if (cadeiasEquivalentes (cadeia, listaMetaTokens[iMetaToken-1].cadeiaMetaToken, caseSensitive))
     return (listaMetaTokens[iMetaToken-1].codigoMetaToken);
 return (0);
}

/*
*---------------------------------------------------------------------------
* Imprime cabecalho da lista de erros na definicao ortografica de LiDAS
*---------------------------------------------------------------------------
*/

void imprime_cabecalho_meta_erros (char *nomeArqEntradaOrtografia)
{
  fprintf (arq_Saida_Meta_Erros_Ptr, "LISTA DE ERROS EM \"%s\"", nomeArqEntradaOrtografia);
  REPORT_newLine (arq_Saida_Meta_Erros_Ptr, 2);
  fflush  (NULL);
}

/*
*---------------------------------------------------------------------
* Imprime cabecalho da lista de tokens reconhecidos
*---------------------------------------------------------------------
*/

void imprime_cabecalho_meta_tokens (char *nomeArqEntradaOrtografia)
{
  char
    *strAux = NULL;

  strAux = STRING_allocate();

  fprintf (arq_Saida_Meta_Tokens_Ptr, "LISTA DE META-TOKENS RECONHECIDOS EM \"%s\"", nomeArqEntradaOrtografia);
  REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 2);

  /*                                                         */
  /* Imprime cabecalho da tabela de meta-tokens reconhecidos */
  /*                                                         */

  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", tracos_tab_meta_tokens);
  REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "LINHA");
  (void) STRING_justify (&strAux, tamPosEntradaLin, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "COLUNA");
  (void) STRING_justify (&strAux, tamPosEntradaCol, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "CODIGO");
  (void) STRING_justify (&strAux, tamCodigoMetaToken, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "META-TOKEN");
  (void) STRING_justify (&strAux, TAM_MAX_ROTULO_META_TOKEN, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "LEXEMA");
  (void) STRING_justify (&strAux, TAM_MAX_META_STRING, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);

  REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 1);

  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", tracos_tab_meta_tokens);
  REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 1);
  fflush  (NULL);
  STRING_release (&strAux);
}

/*
*---------------------------------------------------------------------
* Imprime uma linha na lista de meta-tokens reconhecidos
*---------------------------------------------------------------------
*/

void imprime_detalhes_meta_token (
  LEXAN_t_tokenLocation  metaTokenLocation,
  LEXAN_t_tokenType      metaToken,
  LEXAN_t_tokenVal       tokenVal )
{
  static unsigned int
    linhaAtual = 0;
  char
    *strLinhaAtual = NULL,
    *strMetaString = NULL,
    *strAuxiliar   = NULL;
  t_codigoMetaToken
    codigoMetaToken;

  strLinhaAtual = STRING_allocate();
  strMetaString = STRING_allocate();
  strAuxiliar   = STRING_allocate();

  (void) STRING_set (&strLinhaAtual, ' ', tamPosEntradaLin);
  if ((metaTokenLocation.lineNumber != 0) && (linhaAtual != metaTokenLocation.lineNumber))
    snprintf (strLinhaAtual, tamPosEntradaLin + 1, FORMATO_POS_ENTRADA_LIN, metaTokenLocation.lineNumber);

  linhaAtual = metaTokenLocation.lineNumber;

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  fprintf  (arq_Saida_Meta_Tokens_Ptr, "%s", strLinhaAtual);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  fprintf  (arq_Saida_Meta_Tokens_Ptr, FORMATO_POS_ENTRADA_COL, metaTokenLocation.columnNumber);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  switch (metaToken) {
    case (LEXAN_token_resWord):
      codigoMetaToken = cadeiaMetaToken2codigoMetaToken (tokenVal.tokenStr, META_COMANDOS_CASE_SENSITIVE);
      fprintf (arq_Saida_Meta_Tokens_Ptr, FORMATO_CODIGO_META_TOKEN, codigoMetaToken);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
      if (codigoMetaToken == 0) {
        {
          char formatoErro[] = "Pal reservada desconhecida: %s";
          size_t strAuxLen = strlen(formatoErro) + strlen(tokenVal.tokenStr);
          (void) STRING_set (&strAuxiliar, ' ', strAuxLen);
          snprintf (strAuxiliar, strAuxLen + 1, formatoErro, tokenVal.tokenStr);
        }
        fprintf (arq_Saida_Erros_Ptr, "ERRO em %s: %s \n", arq_Entrada_Ortografia_Nome, strAuxiliar);
        (void) STRING_justify (&strAuxiliar, TAM_MAX_ROTULO_META_TOKEN, ' ', STRING_t_justify_left);
        fprintf (arq_Saida_Meta_Tokens_Ptr, FORMATO_ROTULO_META_TOKEN, strAuxiliar);
      }
      else
        fprintf (arq_Saida_Meta_Tokens_Ptr, FORMATO_ROTULO_META_TOKEN, listaMetaTokens[codigoMetaToken-1].rotuloMetaToken);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', TAM_MAX_META_STRING);
      break;
    case (LEXAN_token_single_str):
    case (LEXAN_token_double_str):
      codigoMetaToken = metaTk_metaString;
      (void) STRING_set (&strMetaString, ' ', strlen(tokenVal.tokenStr));
      snprintf (strMetaString, strlen(tokenVal.tokenStr) + 1, "%s", tokenVal.tokenStr);
      (void) STRING_justify (&strMetaString, TAM_MAX_META_STRING, ' ', STRING_t_justify_left);
      fprintf (arq_Saida_Meta_Tokens_Ptr, FORMATO_CODIGO_META_TOKEN, codigoMetaToken);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
      fprintf (arq_Saida_Meta_Tokens_Ptr, FORMATO_ROTULO_META_TOKEN, listaMetaTokens[codigoMetaToken-1].rotuloMetaToken);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);
      REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
      fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", strMetaString);
      break;
    case (LEXAN_token_iden)         :
    case (LEXAN_token_longInt)      :
    case (LEXAN_token_double)       :
    case (LEXAN_token_delim)        :
    case (LEXAN_token_EOF)          :
    case (LEXAN_token_NULL)         :
    case (LEXAN_token_LINE_COMMENT) :
    case (LEXAN_token_OPEN_COMMENT) :
    case (LEXAN_token_CLOSE_COMMENT):
      errno = 0;
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Tipo de meta-token invalido (%d)", metaToken);
      ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
    default:
      errno = 0;
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Tipo de meta-token desconhecido (%d)", metaToken);
      ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_META_TOKENS, 1);
  REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 1);
  fflush  (NULL);
  STRING_release (&strAuxiliar);
  STRING_release (&strMetaString);
  STRING_release (&strLinhaAtual);
}

/*
*---------------------------------------------------------------------
* Imprime resumo sobre meta-token
*---------------------------------------------------------------------
*/

void imprime_resumo_meta_token (t_codigoMetaToken codigoMetaToken, int usosMetaToken)
{
  unsigned int
    larguraInfoCol = 0;
  char
    *strNrUsosMetaToken = NULL;

  strNrUsosMetaToken = STRING_allocate();

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_META_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  fprintf (arq_Saida_Meta_Tokens_Ptr, FORMATO_CODIGO_META_TOKEN, codigoMetaToken);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_META_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  if (codigoMetaToken > 0)
    fprintf (arq_Saida_Meta_Tokens_Ptr, FORMATO_ROTULO_META_TOKEN, listaMetaTokens[codigoMetaToken - 1].rotuloMetaToken);
  else
    fprintf (arq_Saida_Meta_Tokens_Ptr, FORMATO_ROTULO_META_TOKEN, "TOTAL");
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_META_TOKENS, 1);

  larguraInfoCol = strlen(TITULO_ULT_COL_RESUMO_META_TOKENS);
  larguraInfoCol = GREATEST(larguraInfoCol, tamNrUsosMetaToken);
  (void) STRING_set (&strNrUsosMetaToken, 0, tamNrUsosMetaToken);
  snprintf (strNrUsosMetaToken, tamNrUsosMetaToken + 1, FORMATO_NR_USOS_META_TOKEN, usosMetaToken);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  (void) STRING_justify (&strNrUsosMetaToken, larguraInfoCol, ' ', STRING_t_justify_centre);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", strNrUsosMetaToken);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_META_TOKENS, 1);

  REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 1);
  fflush  (NULL);
  STRING_release (&strNrUsosMetaToken);
}

/*
*---------------------------------------------------------------------
* Imprime tabela com resumo de meta-tokens reconhecidos
*---------------------------------------------------------------------
*/

void imprime_resumo_meta_tokens (void)
{
 unsigned int
   iMetaToken,
   larguraInfoCol;
  char
    *strAux = NULL;

  strAux = STRING_allocate();

  REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 1);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "RESUMO DE META-TOKENS RECONHECIDOS");
  REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 2);

  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", tracos_tab_resumo_meta_tokens);
  REPORT_newLine     (arq_Saida_Meta_Tokens_Ptr, 1);

  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_META_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "CODIGO");
  (void) STRING_justify (&strAux, tamCodigoToken, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_META_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "META-TOKEN");
  (void) STRING_justify (&strAux, TAM_MAX_ROTULO_META_TOKEN, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_META_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, TITULO_ULT_COL_RESUMO_META_TOKENS);
  larguraInfoCol = strlen(TITULO_ULT_COL_RESUMO_META_TOKENS);
  larguraInfoCol = GREATEST(larguraInfoCol, tamNrUsosMetaToken);
  (void) STRING_justify (&strAux, larguraInfoCol, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Meta_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_META_TOKENS, 1);
  REPORT_newLine     (arq_Saida_Meta_Tokens_Ptr, 1);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", tracos_tab_resumo_meta_tokens);
  REPORT_newLine     (arq_Saida_Meta_Tokens_Ptr, 1);

  for (iMetaToken = totMetaTokens = 0; iMetaToken < TAM_LISTA_META_TOKENS; iMetaToken++) {
    totMetaTokens += listaMetaTokens[iMetaToken].usosMetaToken;
    imprime_resumo_meta_token (
      listaMetaTokens[iMetaToken].codigoMetaToken,
      listaMetaTokens[iMetaToken].usosMetaToken );
  }

  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", tracos_tab_resumo_meta_tokens);
  REPORT_newLine    (arq_Saida_Meta_Tokens_Ptr, 1);
  imprime_resumo_meta_token (0, totMetaTokens);
  fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", tracos_tab_resumo_meta_tokens);
  REPORT_newLine    (arq_Saida_Meta_Tokens_Ptr, 1);
  fflush  (NULL);
  STRING_release (&strAux);
}

/*
*--------------------------------------------------------------------------------------------
* Imprime linha corrente no arquivo de erros de definicao ortografica da linguagem LiDAS
*--------------------------------------------------------------------------------------------
*/

void imprime_linha_entrada_ortografia (int nroLinha, char *linha)
{
  fprintf (arq_Saida_Meta_Erros_Ptr, "[%4d] %s", nroLinha, linha);
  fflush  (NULL);
}

/*
*--------------------------------------------------------------------------------------------
* Imprime mensagem de erro no arquivo de erros de definicao ortografica da linguagem LiDAS
*--------------------------------------------------------------------------------------------
*/

void imprime_meta_erro (int nroLinha, int nroColuna, char *strErro)
{
  fprintf (arq_Saida_Meta_Erros_Ptr, "       ERRO lin %d, col %d: %s\n", nroLinha, nroColuna, strErro);
  totMetaErros++;
  fflush  (NULL);
}

/*
*---------------------------------------------------------------------
*
* PROCESSAMENTO DE TOKENS NO CÓDIGO FONTE EM LiDAS
*
*---------------------------------------------------------------------
*/

/*
*----------------------------------------------------------------------------------
* Dado um tipo pre-definido de token, retorna o codigo do token deste tipo
* encontrado no programa LiDAS, ou zero se nenhum foi encontrado.
*
* Mas e se tiverem sido encontrados mais de um token deste tipo?????
*----------------------------------------------------------------------------------
*/

t_codigoToken tipoToken2codigoToken (LEXAN_t_tokenType tipoToken)
{
 int
   iToken,
   tokensDesseTipo;
 t_codigoToken
   codigoToken = 0;

 for (iToken = tokensDesseTipo = 0; iToken < tamListaTokens; iToken++)
   if (listaTokens[iToken].tipoToken == tipoToken) {
     codigoToken = listaTokens[iToken].codigoToken;
     tokensDesseTipo++;
   }
 if (tokensDesseTipo == 1)
   return (codigoToken);
 errno = 0;
 if (tokensDesseTipo > 1) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Ha %d tokens do tipo %s; impossivel determinar seu codigo", tokensDesseTipo, LEXAN_tokenTypeNames[tipoToken]);
   ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (0);
}

/*
*----------------------------------------------------------------------------------
* Dada uma cadeia (string), verifica se e uma palavra reservada ou um delimitador.
* Se for, retorna o codigo do respectivo token; caso contrario, retorna zero.
*----------------------------------------------------------------------------------
*/

t_codigoToken cadeiaToken2codigoToken (char *cadeia, Bool caseSensitive)
{
 int
   iToken;

 for (iToken = 0; iToken < tamListaTokens; iToken++)
   if (cadeiasEquivalentes (cadeia, listaTokens[iToken].cadeiaToken, caseSensitive))
     return (listaTokens[iToken].codigoToken);
 return (0);
}

/*
*---------------------------------------------------------------------------
* Imprime cabecalho da lista de erros lexicos no codigo fonte em LiDAS
*---------------------------------------------------------------------------
*/

void imprime_cabecalho_erros (char *nomeArqEntrada)
{
  fprintf (arq_Saida_Erros_Ptr, "LISTA DE ERROS LEXICOS EM \"%s\"", nomeArqEntrada);
  REPORT_newLine (arq_Saida_Erros_Ptr, 2);
  fflush  (NULL);
}

/*
*---------------------------------------------------------------------
* Imprime cabecalho da lista de tokens reconhecidos
*---------------------------------------------------------------------
*/

void imprime_cabecalho_tokens (char *nomeArqEntrada)
{
  char
    *strAux = NULL;

  strAux = STRING_allocate();

  fprintf (arq_Saida_Tokens_Ptr, "LISTA DE TOKENS RECONHECIDOS EM \"%s\"", nomeArqEntrada);
  REPORT_newLine (arq_Saida_Tokens_Ptr, 2);

  /*                                                    */
  /* Imprime cabecalho da tabela de tokens reconhecidos */
  /*                                                    */

  fprintf (arq_Saida_Tokens_Ptr, "%s", tracos_tab_tokens);
  REPORT_newLine (arq_Saida_Tokens_Ptr, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "LINHA");
  (void) STRING_justify (&strAux, tamPosEntradaLin, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "COLUNA");
  (void) STRING_justify (&strAux, tamPosEntradaCol, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "CODIGO");
  (void) STRING_justify (&strAux, tamCodigoMetaToken, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "TOKEN");
  (void) STRING_justify (&strAux, TAM_MAX_ROTULO_TOKEN, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "LEXEMA");
  (void) STRING_justify (&strAux, TAM_MAX_LEXEMA, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  fprintf (arq_Saida_Tokens_Ptr, TITULO_ULT_COL_LISTA_TOKENS);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_newLine (arq_Saida_Tokens_Ptr, 1);

  fprintf (arq_Saida_Tokens_Ptr, "%s", tracos_tab_tokens);
  REPORT_newLine (arq_Saida_Tokens_Ptr, 1);
  fflush  (NULL);
  STRING_release (&strAux);
}

/*
*-------------------------------------------------------------------------------
* Imprime uma linha na lista de tokens reconhecidos no codigo fonte em LiDAS
*-------------------------------------------------------------------------------
*/

void imprime_detalhes_token (
  LEXAN_t_tokenLocation tokenLocation,
  LEXAN_t_tokenType     token,
  t_codigoToken         codigoToken,
  LEXAN_t_tokenVal      tokenVal,
  int                   posTabSimb )
{
  static unsigned int
    linhaAtual = 0;
  unsigned int
    larguraPosTabSimb;
  char
    *strLexema     = NULL,
    *strPosTabSimb = NULL;

  strPosTabSimb = STRING_allocate();
  (void) STRING_set (&strPosTabSimb, ' ', tamPosTabSimb);
  strLexema = STRING_allocate();
  (void) STRING_set (&strLexema, ' ', TAM_MAX_LEXEMA);

  larguraPosTabSimb = strlen (TITULO_ULT_COL_LISTA_TOKENS);
  larguraPosTabSimb = GREATEST (larguraPosTabSimb, tamPosTabSimb);
  if (tokenTemLexema (token)) {
    if (token == LEXAN_token_longInt)
      snprintf (strLexema, TAM_MAX_LEXEMA + 1, FORMATO_LEXEMA_INTEIRO, tokenVal.longIntVal);
    else if (token == LEXAN_token_double)
      snprintf (strLexema, TAM_MAX_LEXEMA + 1, FORMATO_LEXEMA_DOUBLE, tokenVal.doubleVal);
    else
      snprintf (strLexema, TAM_MAX_LEXEMA + 1, FORMATO_LEXEMA_STRING, tokenVal.tokenStr);
    if (posTabSimb == 0) {
      errno = 0;
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Lexema do token %s (%s) nao instalado na tabela de simbolos", listaTokens[codigoToken-1].rotuloToken, tokenVal.tokenStr);
      ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
    }
    else {
      (void) STRING_set (&strPosTabSimb, ' ', tamPosTabSimb);
      snprintf (strPosTabSimb, tamPosTabSimb + 1, FORMATO_POS_TABSIMB, posTabSimb);
    }
  }
  else {
    snprintf (strLexema, TAM_MAX_LEXEMA + 1, FORMATO_LEXEMA_STRING, "");
    (void) STRING_set (&strPosTabSimb, ' ', tamPosTabSimb);
  }

  (void) STRING_justify (&strLexema,     TAM_MAX_LEXEMA,    ' ', STRING_t_justify_left);
  (void) STRING_justify (&strPosTabSimb, larguraPosTabSimb, ' ', STRING_t_justify_centre);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  if ((tokenLocation.lineNumber == 0) || (linhaAtual == tokenLocation.lineNumber))
    REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', tamPosEntradaLin);
  else
    fprintf  (arq_Saida_Tokens_Ptr, FORMATO_POS_ENTRADA_LIN, tokenLocation.lineNumber);

  linhaAtual = tokenLocation.lineNumber;

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  fprintf  (arq_Saida_Tokens_Ptr, FORMATO_POS_ENTRADA_COL, tokenLocation.columnNumber);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  fprintf (arq_Saida_Tokens_Ptr, FORMATO_CODIGO_TOKEN, codigoToken);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  fprintf (arq_Saida_Tokens_Ptr, FORMATO_ROTULO_TOKEN, listaTokens[codigoToken-1].rotuloToken);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  fprintf (arq_Saida_Tokens_Ptr, "%s", strLexema);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  fprintf (arq_Saida_Tokens_Ptr, "%s", strPosTabSimb);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_newLine (arq_Saida_Tokens_Ptr, 1);
  fflush  (NULL);
  STRING_release (&strLexema);
  STRING_release (&strPosTabSimb);
}

/*
*---------------------------------------------------------------------
* Imprime detalhes sobre tokens reconhecidos
*---------------------------------------------------------------------
*/

void imprime_resumo_token (
  char           *rotuloToken,
  t_codigoToken   codigoToken,
  int             infoToken )
{
  unsigned int
    tamInfoToken,
    larguraInfoCol;
  char
    *strInfoToken     = NULL,
    *formatoInfoToken = NULL;

  strInfoToken     = STRING_allocate();
  formatoInfoToken = STRING_allocate();

  (void) STRING_copy (&formatoInfoToken, FORMATO_NR_USOS_TOKEN);
  tamInfoToken = tamNrUsosToken;
  larguraInfoCol = GREATEST(strlen(TITULO_ULT_COL_RESUMO_TOKENS), tamInfoToken);
  (void) STRING_set (&strInfoToken, 0, larguraInfoCol);
  snprintf (strInfoToken, larguraInfoCol, formatoInfoToken, infoToken);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  fprintf (arq_Saida_Tokens_Ptr, FORMATO_CODIGO_TOKEN, codigoToken);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  fprintf (arq_Saida_Tokens_Ptr, FORMATO_ROTULO_TOKEN, rotuloToken);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  (void) STRING_justify (&strInfoToken, larguraInfoCol, ' ', STRING_t_justify_centre);
  fprintf (arq_Saida_Tokens_Ptr, "%s", strInfoToken);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_DETALHE_TOKENS, 1);
  REPORT_newLine (arq_Saida_Tokens_Ptr, 1);
  fflush  (NULL);
  STRING_release (&strInfoToken);
  STRING_release (&formatoInfoToken);
}

/*
*---------------------------------------------------------------------
* Imprime tabela com resumo de tokens reconhecidos
*---------------------------------------------------------------------
*/

void imprime_resumo_tokens (void)
{
 int
   iToken;
  char
    *strAux = NULL;

  strAux = STRING_allocate ();
  REPORT_newLine (arq_Saida_Tokens_Ptr, 1);
  fprintf  (arq_Saida_Tokens_Ptr, "  RESUMO");
  REPORT_newLine (arq_Saida_Tokens_Ptr, 2);

  fprintf  (arq_Saida_Tokens_Ptr, "%s", tracos_tab_resumo_tokens);
  REPORT_newLine     (arq_Saida_Tokens_Ptr, 1);

  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "CODIGO");
  (void) STRING_justify (&strAux, tamCodigoToken, ' ', STRING_t_justify_left);
  fprintf  (arq_Saida_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "TOKEN");
  (void) STRING_justify (&strAux, TAM_MAX_ROTULO_TOKEN, ' ', STRING_t_justify_left);
  fprintf  (arq_Saida_Tokens_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_TOKENS, 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  fprintf  (arq_Saida_Tokens_Ptr, TITULO_ULT_COL_RESUMO_TOKENS);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_Tokens_Ptr, SEPARADOR_TABELA_RESUMO_TOKENS, 1);
  REPORT_newLine     (arq_Saida_Tokens_Ptr, 1);

  fprintf  (arq_Saida_Tokens_Ptr, "%s", tracos_tab_resumo_tokens);
  REPORT_newLine     (arq_Saida_Tokens_Ptr, 1);

  imprime_resumo_token (ROTULO_TK_EOF, cadeiaToken2codigoToken (CADEIA_TK_EOF, caixa_Comandos == t_caixa_distinguem), 1);
  totTokens = 1;

  for (iToken = 1; iToken < tamListaTokens; iToken++) {
    imprime_resumo_token (
      listaTokens[iToken].rotuloToken,
      listaTokens[iToken].codigoToken,
      listaTokens[iToken].usosToken );
    totTokens += listaTokens[iToken].usosToken;
  }

  fprintf (arq_Saida_Tokens_Ptr, "%s", tracos_tab_resumo_tokens);
  REPORT_newLine (arq_Saida_Tokens_Ptr, 1);
  imprime_resumo_token ("TOTAL", 0, totTokens);
  fprintf (arq_Saida_Tokens_Ptr, "%s", tracos_tab_resumo_tokens);
  REPORT_newLine (arq_Saida_Tokens_Ptr, 1);
  fflush (NULL);
  STRING_release (&strAux);
}

/*
*---------------------------------------------------------------------
* Imprime linha corrente no arquivo de erros lexicos
*---------------------------------------------------------------------
*/

void imprime_linha_entrada_programa (int nroLinha, char *linha)
{
  fprintf (arq_Saida_Erros_Ptr, "[%4d] %s", nroLinha, linha);
  fflush  (NULL);
}

/*
*----------------------------------------------------------------------------------------------
* Insere um novo token na lista de tokens que podem ser encontrados em um programa em LiDAS
*----------------------------------------------------------------------------------------------
*/

void insere_novo_token (
  LEXAN_t_tokenType  token,
  char              *cadeiaNovoToken,
  char              *regexNovoToken,
  char              *rotuloNovoToken )
{
  t_listaTokens
    *p_listaTokens;

  p_listaTokens = &listaTokens[tamListaTokens];
  tamListaTokens++;
  p_listaTokens->codigoToken = tamListaTokens;
  p_listaTokens->tipoToken = token;

  strcpy (p_listaTokens->cadeiaToken, cadeiaNovoToken);
  strcpy (p_listaTokens->regexToken,  regexNovoToken );
  strcpy (p_listaTokens->rotuloToken, rotuloNovoToken);

/*
  STRING_SAFER_STRCPY (p_listaTokens->cadeiaToken, cadeiaNovoToken, TAM_MAX_CADEIA_TOKEN)
  STRING_SAFER_STRCPY (p_listaTokens->regexToken,  regexNovoToken , TAM_MAX_REGEX_TOKEN)
  STRING_SAFER_STRCPY (p_listaTokens->rotuloToken, rotuloNovoToken, TAM_MAX_ROTULO_TOKEN)
*/
  p_listaTokens->usosToken = 0;
}

/*
*---------------------------------------------------------------------
* Inicializa a tabela de simbolos
*---------------------------------------------------------------------
*/

void inicializa_tabSimb (void)
{
  unsigned int
    iLexema;

  for (iLexema = 0; iLexema < totLexemas; iLexema++) {
    tabSimb[iLexema].codigoToken = 0;
    memset (tabSimb[iLexema].lexema, 0, TAM_MAX_LEXEMA);
    tabSimb[iLexema].usosToken = 0;
    tabSimb[iLexema].p_primeira  = NULL;
    tabSimb[iLexema].p_ultima    = NULL;
  }
}

/*
*---------------------------------------------------------------------
* Instalacao de identificador na tabela de simbolos
*---------------------------------------------------------------------
*/

int instala_na_tabSimb (
  t_codigoToken          codigoToken,
  char                  *lexema,
  LEXAN_t_tokenLocation  tokenLocation,
  Bool                   caseSensitive )
{
  unsigned int
    iLexema;
  t_posEntrada
    *p_posEntrada = NULL;

  /* Se o lexema ainda nao existe na tabela de simbolos entao adicione-o */

  for (iLexema = 0; iLexema < totLexemas; iLexema++)
    if ((codigoToken == tabSimb[iLexema].codigoToken) && cadeiasEquivalentes(lexema, tabSimb[iLexema].lexema, caseSensitive))
      break;

  if (iLexema == totLexemas) {
    tabSimb[totLexemas].codigoToken = codigoToken;
    strcpy (tabSimb[totLexemas].lexema, lexema);
    totLexemas++;
  }

  /* Atualize os demais campos da tabela */

  tabSimb[iLexema].usosToken++;
  errno = 0;
  p_posEntrada = (t_posEntrada *) malloc ((size_t) sizeof (t_posEntrada));
  if (p_posEntrada == NULL)
    ERROR_no_memory (errno, progName, __func__, "p_posEntrada");
  p_posEntrada->linha  = tokenLocation.lineNumber;
  p_posEntrada->coluna = tokenLocation.columnNumber;
  p_posEntrada->proxEntrada = NULL;
  if (tabSimb[iLexema].p_primeira == NULL)
    tabSimb[iLexema].p_primeira = tabSimb[iLexema].p_ultima = p_posEntrada;
  else {
    tabSimb[iLexema].p_ultima->proxEntrada = p_posEntrada;
    tabSimb[iLexema].p_ultima = p_posEntrada;
  }
  return (iLexema);
}

/*
*---------------------------------------------------------------------
* Imprime a tabela de simbolos
*---------------------------------------------------------------------
*/

void imprime_tabSimb (char *nomeArqEntrada)
{
  unsigned int
    iChar,
    iLexema,
    iOcorr,
    tamLexemaMaisLongo,
    maiorNroOcorrencias,
    larguraUmaOcorr,
    larguraOcorrencias,
    larguraUltimaCol;
  char
    *strAux = NULL;
  t_posEntrada
    *p_ocorr;

  strAux = STRING_allocate();

  /* Calcula o tamanho do lexema mais longo e o maior nro de usosToken do mesmo lexema */

  for (iLexema = tamLexemaMaisLongo = maiorNroOcorrencias = 0; iLexema < totLexemas; iLexema++) {
    tamLexemaMaisLongo = GREATEST(tamLexemaMaisLongo, strlen(tabSimb[iLexema].lexema));
    maiorNroOcorrencias = GREATEST(maiorNroOcorrencias, tabSimb[iLexema].usosToken);
  }
  larguraUmaOcorr = tamPosEntradaLinCol;
  larguraOcorrencias = (maiorNroOcorrencias - 1) * (larguraUmaOcorr + 1) + larguraUmaOcorr;
  larguraUltimaCol = GREATEST(strlen(TITULO_ULT_COL_TAB_SIMBOLOS), larguraOcorrencias);


  /* Constroi strings do tipo "+------+------+" para cabecalho da tabela de simbolos */

  strcpy (tracos_tab_simbolos, "+");
  for (iChar = 0; iChar < tamPosTabSimb + 2; iChar++)
    strcat (tracos_tab_simbolos, "-");
  strcat (tracos_tab_simbolos, "+");
  for (iChar = 0; iChar < TAM_MAX_ROTULO_TOKEN + 2; iChar++)
    strcat (tracos_tab_simbolos, "-");
  strcat (tracos_tab_simbolos, "+");
  for (iChar = 0; iChar < tamLexemaMaisLongo + 2; iChar++)
    strcat (tracos_tab_simbolos, "-");
  strcat (tracos_tab_simbolos, "+");
  for (iChar = 0; iChar < larguraUltimaCol + 2; iChar++)
    strcat (tracos_tab_simbolos, "-");
  strcat (tracos_tab_simbolos, "+");

  /* Imprime cabecalho da saida, inclusive da tabela de simbolos */

  fprintf (arq_Saida_TabSimb_Ptr, "TABELA DE SIMBOLOS - \"%s\"", nomeArqEntrada);
  REPORT_newLine (arq_Saida_TabSimb_Ptr, 2);

  fprintf (arq_Saida_TabSimb_Ptr, "%s", tracos_tab_simbolos);
  REPORT_newLine (arq_Saida_TabSimb_Ptr, 1);

  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, SEPARADOR_TABELA_SIMBOLOS, 1);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "POSICAO");
  (void) STRING_justify (&strAux, tamPosTabSimb, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_TabSimb_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, SEPARADOR_TABELA_SIMBOLOS, 1);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "TOKEN");
  (void) STRING_justify (&strAux, TAM_MAX_ROTULO_TOKEN, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_TabSimb_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, SEPARADOR_TABELA_SIMBOLOS, 1);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, "LEXEMA");
  (void) STRING_justify (&strAux, tamLexemaMaisLongo, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_TabSimb_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, SEPARADOR_TABELA_SIMBOLOS, 1);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
  (void) STRING_copy (&strAux, TITULO_ULT_COL_TAB_SIMBOLOS);
  (void) STRING_justify (&strAux, larguraUltimaCol, ' ', STRING_t_justify_left);
  fprintf (arq_Saida_TabSimb_Ptr, "%s", strAux);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
  REPORT_repeat_char (arq_Saida_TabSimb_Ptr, SEPARADOR_TABELA_SIMBOLOS, 1);
  REPORT_newLine (arq_Saida_TabSimb_Ptr, 1);

  fprintf (arq_Saida_TabSimb_Ptr, "%s", tracos_tab_simbolos);
  REPORT_newLine (arq_Saida_TabSimb_Ptr, 1);

  for (iLexema = 0; iLexema < totLexemas; iLexema++) {
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, SEPARADOR_TABELA_SIMBOLOS, 1);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);

    fprintf (arq_Saida_TabSimb_Ptr, FORMATO_POS_TABSIMB, iLexema + 1);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, SEPARADOR_TABELA_SIMBOLOS, 1);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);

    (void) STRING_copy (&strAux, listaTokens[tabSimb[iLexema].codigoToken - 1].rotuloToken);
    (void) STRING_justify (&strAux, TAM_MAX_ROTULO_TOKEN, ' ', STRING_t_justify_left);
    fprintf (arq_Saida_TabSimb_Ptr, "%s", strAux);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, SEPARADOR_TABELA_SIMBOLOS, 1);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);

    (void) STRING_copy (&strAux, tabSimb[iLexema].lexema);
    (void) STRING_justify (&strAux, tamLexemaMaisLongo, ' ', STRING_t_justify_left);
    fprintf (arq_Saida_TabSimb_Ptr, "%s", strAux);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, SEPARADOR_TABELA_SIMBOLOS, 1);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', 1);
    for (iOcorr = 0, p_ocorr = tabSimb[iLexema].p_primeira; iOcorr < maiorNroOcorrencias; iOcorr++) {
      if (iOcorr < tabSimb[iLexema].usosToken) {
        fprintf (arq_Saida_TabSimb_Ptr, FORMATO_POS_ENTRADA_LIN_COL, p_ocorr->linha, p_ocorr->coluna);
        fprintf (arq_Saida_TabSimb_Ptr, " ");
        p_ocorr = p_ocorr->proxEntrada;
      }
      else
        REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', larguraUmaOcorr + 1);
    }
    if (larguraOcorrencias < larguraUltimaCol)
      REPORT_repeat_char (arq_Saida_TabSimb_Ptr, ' ', larguraUltimaCol - larguraOcorrencias);
    REPORT_repeat_char (arq_Saida_TabSimb_Ptr, SEPARADOR_TABELA_SIMBOLOS, 1);
    REPORT_newLine (arq_Saida_TabSimb_Ptr, 1);
  }

  fprintf (arq_Saida_TabSimb_Ptr, "%s", tracos_tab_simbolos);
  REPORT_newLine (arq_Saida_TabSimb_Ptr, 1);
  fflush (NULL);
  STRING_release (&strAux);
}

/*
*--------------------------------------------------------------------------
* Libera memoria dinamica alocada para a tabela de simbolos, lista de IDs,
* lista de frames, lista de identificadores e lista de IDs utilizados.
*--------------------------------------------------------------------------
*/

void destroi_tabSimb (void)
{
 unsigned int
   iLexema;
 t_posEntrada
   *p_ocorrencia_atual,
   *p_proxima_ocorrencia;

 for (iLexema = 0; iLexema < totLexemas; iLexema++) {
  p_ocorrencia_atual = tabSimb[iLexema].p_primeira;
  while (p_ocorrencia_atual) {
    p_proxima_ocorrencia = p_ocorrencia_atual->proxEntrada;
    free (p_ocorrencia_atual);
    p_ocorrencia_atual = p_proxima_ocorrencia;
  }
 }
}

void destroi_listaIds (void)
{
 t_registroId *registro_atual,
              *registro_proximo;
 if(listaIds.p_primeiro != NULL){
  registro_atual = listaIds.p_primeiro;
  while(registro_atual){
   registro_proximo = registro_atual->proximo;
   free(registro_atual);
   registro_atual = registro_proximo;
  }
 }
}

void destroi_listaFrames (void)
{
 t_frame *registro_atual,
         *registro_proximo;
 if(listaFrames.p_primeiro != NULL){
  registro_atual = listaFrames.p_primeiro;
  while(registro_atual){
   registro_proximo = registro_atual->proximo;
   free(registro_atual);
   registro_atual = registro_proximo;
  }
 }
}

void destroi_listaDefs (void)
{
 t_defIden *registro_atual,
           *registro_proximo;
 if(listaDefs.p_primeiro != NULL){
  registro_atual = listaDefs.p_primeiro;
  while(registro_atual){
   registro_proximo = registro_atual->proximo;
   free(registro_atual);
   registro_atual = registro_proximo;
  }
 }
}

void destroi_listaIdsUtils(void)
{
 t_idenUtilizado *registro_atual,
                 *registro_proximo;
 if(listaIdenUt.p_primeiro != NULL){
  registro_atual = listaIdenUt.p_primeiro;
  while(registro_atual){
   registro_proximo = registro_atual->proximo;
   free(registro_atual);
   registro_atual = registro_proximo;
  }
 }
}

/*
*---------------------------------------------------------------------
* Processa opcoes e argumentos na linha de comando
*---------------------------------------------------------------------
*/

void processa_linhaDeComando (int argc, char *argv[])
{
 Bool
   commLineOK;
 int
   optUses;
 char
   *argStr = NULL;

 /* If there was an error in the command line */
 /* then display program usage info and abort */

 progName = COMMLINE_get_program_short_name (argv[0]);
 commLineOK = COMMLINE_parse_commLine (argc, argv,
   4,
      1, 'a', "ajuda",     COMMLINE_opt_arg_none,   0, COMMLINE_OPT_FREE_USE, 0,
      2, 'u', "uso",       COMMLINE_opt_arg_none,   0, COMMLINE_OPT_FREE_USE, 0,
      3, 'd', "definicao", COMMLINE_opt_arg_string, 1, 1,                     0,
      4, 'p', "programa",  COMMLINE_opt_arg_string, 1, 1,                     0,
   1,
     COMMLINE_opt_arg_string);

 if (! commLineOK) {
   COMMLINE_display_usage();
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 }

 /* If the user asked for help then provide it and exit */

 if (! COMMLINE_optId2optUses (1, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (optUses > 0) {
   printf("\n");
   printf("Analisador lexico para LiDAS. Toma como entrada 2 arquivos texto.\n");
   printf("Uso:\n");
   printf("  %s -d arq1 -p arq2\n", progName);
   printf("onde\n");
   printf("  arq1 contem a definicao de tokens da linguagem LiDAS e\n");
   printf("  arq2 contem o programa (codigo fonte) em LiDAS a ser analisado.\n");
   printf("\n");
   printf("Gera como saida 3 arquivos texto com as seguintes extensoes e conteudos:\n");
   printf("  %s - Erros lexicos encontrados no programa em arq1\n", EXTENSAO_SAIDA_ERROS);
   printf("  %s - Tokens reconhecidos no programa em arq1\n", EXTENSAO_SAIDA_TOKENS);
   printf("  %s - Tabela de simbolos do programa em arq1\n", EXTENSAO_SAIDA_TABSIMB);
   printf("\n");
   printf("Exemplo de uso:\n");
   printf("  %s -d LiDAS-2011-1.mel -p teste01.lda svgOrigem.svg\n", progName);
   printf(" \n");
   exit (EXIT_SUCCESS);
 }

 /* If the user asked for program usage info then provide it and exit */

 if (! COMMLINE_optId2optUses (2, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (optUses > 0) {
   COMMLINE_display_usage();
   exit (EXIT_SUCCESS);
 }

 /* Arquivo de definicao de tokens da linguagem LiDAS */

 if (! COMMLINE_optId2optUses (3, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (optUses == 1) {
   errno = 0;
   if (! COMMLINE_optUse2optArg (3, 1, &argStr))
     ERROR_short_fatal_error (COMMLINE_get_commLine_error());
   if ((strcpy (arq_Entrada_Ortografia_Nome, argStr)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcpy (arq_Entrada_Ortografia_Nome, %s)", argStr);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   free ((char *) argStr);
   errno = 0;
   if ((arq_Entrada_Ortografia_Ptr = fopen (arq_Entrada_Ortografia_Nome, "r")) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro ao abrir \"%s\" para leitura", arq_Entrada_Ortografia_Nome);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
 }

 /* Arquivo contendo o programa de entrada (codigo fonte) em LiDAS */

 if (! COMMLINE_optId2optUses (4, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (optUses == 1) {
   errno = 0;
   if (! COMMLINE_optUse2optArg (4, 1, &argStr))
     ERROR_short_fatal_error (COMMLINE_get_commLine_error());
   if ((strcpy (arq_Entrada_Programa_Nome, argStr)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcpy (arq_Entrada_Programa_Nome, %s)", argStr);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   free ((char *) argStr);
   errno = 0;
   if ((arq_Entrada_Programa_Ptr = fopen (arq_Entrada_Programa_Nome, "r")) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro ao abrir \"%s\" para leitura", arq_Entrada_Programa_Nome);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
 }

 /* Argumento obrigatório: arquivo SVG */

 if (! COMMLINE_argPos2argVal (1, &argStr))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 errno = 0;
 if ((strcpy (arq_Entrada_SVG_Nome, argStr)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (arq_Entrada_SVG_Nome, %s) failed", argStr);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 free (argStr);
 argStr = NULL;
 errno = 0;
 if ((arq_Entrada_SVG_Ptr = fopen (arq_Entrada_SVG_Nome, "r")) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open file \"%s\" for reading", arq_Entrada_SVG_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }

 /* Erros na definicao ortografica de LiDAS: Construa o nome do arquivo e obtenha o handle */

 errno = 0;
 if ((strcpy (arq_Saida_Meta_Erros_Nome, arq_Entrada_Ortografia_Nome)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcpy (arq_Saida_Meta_Erros_Nome, %s)", arq_Entrada_Ortografia_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if ((strcat (arq_Saida_Meta_Erros_Nome, EXTENSAO_SAIDA_ERROS)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcat (arq_Saida_Meta_Erros_Nome, %s)", EXTENSAO_SAIDA_ERROS);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if ((arq_Saida_Meta_Erros_Ptr = fopen (arq_Saida_Meta_Erros_Nome, "w")) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro ao abrir \"%s\" para escrita", arq_Saida_Meta_Erros_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }

 /* Meta-tokens reconhecidos: Construa o nome do arquivo e obtenha o handle */

 errno = 0;
 if ((strcpy (arq_Saida_Meta_Tokens_Nome, arq_Entrada_Ortografia_Nome)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcpy (arq_Saida_Meta_Tokens_Nome, %s)", arq_Entrada_Ortografia_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if ((strcat (arq_Saida_Meta_Tokens_Nome, EXTENSAO_SAIDA_META_TOKENS)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcat (arq_Saida_Meta_Tokens_Nome, %s)", EXTENSAO_SAIDA_META_TOKENS);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if ((arq_Saida_Meta_Tokens_Ptr = fopen (arq_Saida_Meta_Tokens_Nome, "w")) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro ao abrir \"%s\" para escrita", arq_Saida_Meta_Tokens_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }

 /* Erros lexicos no fonte em LiDAS: Construa o nome do arquivo e obtenha o handle */

 errno = 0;
 if ((strcpy (arq_Saida_Erros_Nome, arq_Entrada_Programa_Nome)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcpy (arq_Saida_Erros_Nome, %s)", arq_Entrada_Programa_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if ((strcat (arq_Saida_Erros_Nome, EXTENSAO_SAIDA_ERROS)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcat (arq_Saida_Erros_Nome, %s)", EXTENSAO_SAIDA_ERROS);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if ((arq_Saida_Erros_Ptr = fopen (arq_Saida_Erros_Nome, "w")) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro ao abrir \"%s\" para escrita", arq_Saida_Erros_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }

 /* Tokens reconhecidos: Construa o nome do arquivo e obtenha o handle */

 errno = 0;
 if ((strcpy (arq_Saida_Tokens_Nome, arq_Entrada_Programa_Nome)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcpy (arq_Saida_Tokens_Nome, %s)", arq_Entrada_Programa_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if ((strcat (arq_Saida_Tokens_Nome, EXTENSAO_SAIDA_TOKENS)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcat (arq_Saida_Tokens_Nome, %s)", EXTENSAO_SAIDA_TOKENS);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if ((arq_Saida_Tokens_Ptr = fopen (arq_Saida_Tokens_Nome, "w")) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro ao abrir \"%s\" para escrita", arq_Saida_Tokens_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }

 /* Tabela de simbolos: Construa o nome do arquivo e obtenha o handle */

 errno = 0;
 if ((strcpy (arq_Saida_TabSimb_Nome, arq_Entrada_Programa_Nome)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcpy (arq_Saida_TabSimb_Nome, %s)", arq_Entrada_Programa_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if ((strcat (arq_Saida_TabSimb_Nome, EXTENSAO_SAIDA_TABSIMB)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em strcat (arq_Saida_TabSimb_Nome, %s)", EXTENSAO_SAIDA_TABSIMB);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if ((arq_Saida_TabSimb_Ptr = fopen (arq_Saida_TabSimb_Nome, "w")) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro ao abrir \"%s\" para escrita", arq_Saida_TabSimb_Nome);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }

 /* Linha de comando toda processada; libere memoria alocada dinamicamente antes de retornar */

 COMMLINE_free_commLine_data();
}

/*
*---------------------------------------------------------------------------------------
* Processa o arquivo de entrada contendo as definicoes de tokens da linguagem LiDAS
*---------------------------------------------------------------------------------------
*/

#define totMetaTokenSpecs                  6
#define TAM_MAX_DESCR_META_TOKEN_ESPERADO  100

Bool processa_definicoesDeTokens (void) {
 unsigned int
   iChar,
   iComando,
   iMetaToken,
   comprCadeiaAtual,
   comprRotuloAtual,
   comprCadeiaAnterior,
   comprRotuloAnterior;
 int
   iTokenSpecs,
   nroLinhaAtual    = -1,
   nroLinhaAnterior = -1,
   nroColunaAtual   = -1,
   indiceMetaTokenEsperado = 0;
 t_codigoToken
   codigoMetaToken;
 Bool
   tokenOK,
   mudouDeLinha = FALSE,
   metaTokensOK = TRUE;
 Bool                                    /* Meta-tokens esperados no arquivo de entrada:   */
   espero_Caixa               = TRUE,    /*   essa palavra reservada                       */
   espero_Identificadores     = FALSE,   /*   essa palavra reservada                       */
   espero_Comandos_1          = FALSE,   /*   essa palavra reservada                       */
   espero_Distinguem          = FALSE,   /*   essa palavra reservada                       */
   espero_Ignoram             = FALSE,   /*   essa palavra reservada                       */
   espero_Comentarios         = FALSE,   /*   essa palavra reservada                       */
   espero_Linha               = FALSE,   /*   essa palavra reservada                       */
   espero_Abre                = FALSE,   /*   essa palavra reservada                       */
   espero_Fecha               = FALSE,   /*   essa palavra reservada                       */
   espero_Lexemas             = FALSE,   /*   essa palavra reservada                       */
   espero_Identificador       = FALSE,   /*   essa palavra reservada                       */
   espero_Inteiro             = FALSE,   /*   essa palavra reservada                       */
   espero_Decimal             = FALSE,   /*   essa palavra reservada                       */
   espero_String              = FALSE,   /*   essa palavra reservada                       */
   espero_Comandos_2          = FALSE,   /*   essa palavra reservada                       */
   espero_meta_string         = FALSE,   /*   um meta-string qualquer                      */
   espero_fim_de_arquivo      = FALSE;   /*   token_EOF                                    */
                                         /*                                                */
 Bool                                    /* Meta-tokens encontrados no arquivo de entrada: */
   achei_Identificadores      = FALSE,   /*   essa palavra reservada                       */
   achei_Comandos_1           = FALSE,   /*   essa palavra reservada                       */
   achei_Linha                = FALSE,   /*   essa palavra reservada                       */
   achei_Abre                 = FALSE,   /*   essa palavra reservada                       */
   achei_Fecha                = FALSE,   /*   essa palavra reservada                       */
   achei_Identificador        = FALSE,   /*   essa palavra reservada                       */
   achei_Inteiro              = FALSE,   /*   essa palavra reservada                       */
   achei_Decimal              = FALSE,   /*   essa palavra reservada                       */
   achei_String               = FALSE;   /*   essa palavra reservada                       */

 /*
 Bool                                    / * Meta-tokens encontrados no arquivo de entrada: * /
   achei_Caixa                = FALSE,   / *   essa palavra reservada                       * /
   achei_Distinguem           = FALSE,   / *   essa palavra reservada                       * /
   achei_Ignoram              = FALSE,   / *   essa palavra reservada                       * /
   achei_Comentarios          = FALSE,   / *   essa palavra reservada                       * /
   achei_Lexemas              = FALSE,   / *   essa palavra reservada                       * /
   achei_Comandos_2           = FALSE;   / *   essa palavra reservada                       * /
   achei_meta_string          = FALSE;   / *   um meta-string qualquer                      * /
 */
                                         /*                                                */
                                         /*                                                */
 Bool                                    /* Partes da entrada encontradas ate agora        */
   achei_regex_linha          = FALSE,   /*   COMENTARIOS > ER de comentario de linha      */
   achei_regex_abre           = FALSE,   /*   COMENTARIOS > ER de comentario de bloco      */
   achei_regex_fecha          = FALSE,   /*   COMENTARIOS > ER de comentario de bloco      */
   achei_rotulo_identificador = FALSE,   /*   LEXEMAS > rotulo de Identificador            */
   achei_rotulo_inteiro       = FALSE,   /*   LEXEMAS > rotulo de Inteiro                  */
   achei_rotulo_decimal       = FALSE,   /*   LEXEMAS > rotulo de Decimal                  */
   achei_rotulo_string        = FALSE,   /*   LEXEMAS > rotulo de String                   */
   achei_regex_identificador  = FALSE,   /*   LEXEMAS > ER de Identificador                */
   achei_regex_inteiro        = FALSE,   /*   LEXEMAS > ER de Inteiro                      */
   achei_regex_decimal        = FALSE,   /*   LEXEMAS > ER de Decimal                      */
   achei_regex_string         = FALSE,   /*   LEXEMAS > ER de String                       */
   achei_cadeia_comando       = FALSE;   /*   COMANDOS > cadeia de um comando qualquer     */
                                         /*                                                */
 Bool                                    /* Parte do arquivo de entrada em processamento:  */
   estou_em_Caixa             = FALSE,   /*   Secao CAIXA                                  */
   estou_em_Comentarios       = FALSE,   /*   Secao COMENTARIOS                            */
   estou_em_Lexemas           = FALSE,   /*   Secao LEXEMAS                                */
   estou_em_Comandos          = FALSE,   /*   Secao COMANDOS                               */
   estou_em_Identificador     = FALSE,   /*   Secao LEXEMAS > Identificado                 */
   estou_em_Inteiro           = FALSE,   /*   Secao LEXEMAS > Inteiro                      */
   estou_em_Decimal           = FALSE,   /*   Secao LEXEMAS > Decimal                      */
   estou_em_String            = FALSE;   /*   Secao LEXEMAS > String                       */
 t_caixa
   caixa;
 LEXAN_t_lexJobId
   lexJobId;
 LEXAN_t_tokenType
   metaToken,
   tipoString;
 LEXAN_t_tokenVal
   tokenVal;
 LEXAN_t_tokenLocation
   metaTokenLocation;
 char
   descrMetaTokenEsperado[16][TAM_MAX_DESCR_META_TOKEN_ESPERADO] = {
     "'Caixa'",                                                                /*  0 */
     "'Identificadores' ou 'Comandos'",                                        /*  1 */
     "'Identificadores'",                                                      /*  2 */
     "'Comandos'",                                                             /*  3 */
     "'Distinguem' ou 'Ignoram'",                                              /*  4 */
     "'Comentarios'",                                                          /*  5 */
     "'Linha' ou 'Abre' ou 'Fecha'",                                           /*  6 */
     "'Linha'",                                                                /*  7 */
     "'Abre'",                                                                 /*  8 */
     "'Fecha'",                                                                /*  9 */
     "um meta-string",                                                         /* 10 */
     "'Lexemas'",                                                              /* 11 */
     "'Identificador' ou 'Inteiro' ou 'Decimal' ou 'String'",                  /* 12 */
     "'Comandos'",                                                             /* 13 */
     "'Identificador' ou 'Inteiro' ou 'Decimal' ou 'String' ou 'Comandos'"     /* 14 */
   };
 char
   *meta_token_regex    = NULL,
   *copiaCadeiaAtual    = NULL,
   *copiaRotuloAtual    = NULL,
   *copiaCadeiaAnterior = NULL,
   *copiaRotuloAnterior = NULL,
   *pLinhaAtual         = NULL,
   *p_regex             = NULL;

#define TAM_MAX_STR_ERRO  500

 char
   strErro       [TAM_MAX_STR_ERRO] = "",
   strMetaString [TAM_MAX_LEXEMA]   = "";
 LEXAN_t_tokenSpecs
   metaTokenSpecs [totMetaTokenSpecs] = {
     {LEXAN_token_resWord,       NULL,                            1, 1},
     {LEXAN_token_single_str,    META_REGEX_STRING_ASPAS_SIMPLES, 1, 1},
     {LEXAN_token_double_str,    META_REGEX_STRING_ASPAS_DUPLAS,  1, 1},
     {LEXAN_token_LINE_COMMENT,  META_REGEX_COMENTARIO,           0, 0},
     {LEXAN_token_OPEN_COMMENT,  META_REGEX_ABRE_COMENTARIO,      0, 0},
     {LEXAN_token_CLOSE_COMMENT, META_REGEX_FECHA_COMENTARIO,     0, 0}
   };

 meta_token_regex    = STRING_allocate();
 copiaCadeiaAtual    = STRING_allocate();
 copiaRotuloAtual    = STRING_allocate();
 copiaCadeiaAnterior = STRING_allocate();
 copiaRotuloAnterior = STRING_allocate();

 /* Construa a expressao regular de cada palavra reservada                            */
 /* Poderia ter inicializado esse campo da estrutura com constantes no proprio codigo */

 for (iMetaToken = 0; iMetaToken < TAM_LISTA_META_TOKENS - 1; iMetaToken++) {
   p_regex = cadeia2regex (listaMetaTokens[iMetaToken].cadeiaMetaToken, META_COMANDOS_CASE_SENSITIVE);
   strcpy (listaMetaTokens[iMetaToken].regexMetaToken, p_regex);
 }

 #ifdef GERA_SAIDA_DEBUG
   {
    int iMetaToken;
	printf ("TAM_LISTA_META_TOKENS = %d \n\n", TAM_LISTA_META_TOKENS);
    for (iMetaToken = 0; iMetaToken < TAM_LISTA_META_TOKENS; iMetaToken++) {
      printf ("listaMetaTokens[%d].codigoMetaToken = %d \n",   iMetaToken, listaMetaTokens[iMetaToken].codigoMetaToken);
      printf ("listaMetaTokens[%d].cadeiaMetaToken = %s \n",   iMetaToken, listaMetaTokens[iMetaToken].cadeiaMetaToken);
      printf ("listaMetaTokens[%d].regexMetaToken  = %s \n",   iMetaToken, listaMetaTokens[iMetaToken].regexMetaToken);
      printf ("listaMetaTokens[%d].rotuloMetaToken = %s \n",   iMetaToken, listaMetaTokens[iMetaToken].rotuloMetaToken);
      printf ("listaMetaTokens[%d].usosMetaToken   = %d \n\n", iMetaToken, listaMetaTokens[iMetaToken].usosMetaToken);
    }
   }
 #endif

 /* Construa a expressao regular para todas as palavras reservadas juntas */

 (void) STRING_copy (&meta_token_regex, "(");
 for (iMetaToken = 0; iMetaToken < TAM_LISTA_META_TOKENS - 2; iMetaToken++) {
   if (strlen(listaMetaTokens[iMetaToken].regexMetaToken) > 0) {
     (void) STRING_concatenate (&meta_token_regex, listaMetaTokens[iMetaToken].regexMetaToken);
     (void) STRING_concatenate (&meta_token_regex, "|");
   }
 }
 if (strlen(listaMetaTokens[TAM_LISTA_META_TOKENS - 2].regexMetaToken) > 0) {
   (void) STRING_concatenate (&meta_token_regex, listaMetaTokens[TAM_LISTA_META_TOKENS - 2].regexMetaToken);
   (void) STRING_concatenate (&meta_token_regex, ")");
 }
 errno = 0;
 metaTokenSpecs[0].regex = (char *) strdup (meta_token_regex);
 if (metaTokenSpecs[0].regex == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em metaTokenSpecs[0].regex = strdup (\"%s\")", meta_token_regex);
   ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 #ifdef GERA_SAIDA_DEBUG
   {
    int iMetaToken;
 	printf ("totMetaTokenSpecs = %d \n\n", totMetaTokenSpecs);
    for (iMetaToken = 0; iMetaToken < totMetaTokenSpecs; iMetaToken++) {
      printf ("metaTokenSpecs[%d].type       = %d \n",   iMetaToken, metaTokenSpecs[iMetaToken].type);
      printf ("metaTokenSpecs[%d].regex      = %s \n",   iMetaToken, metaTokenSpecs[iMetaToken].regex);
      printf ("metaTokenSpecs[%d].firstGroup = %d \n",   iMetaToken, metaTokenSpecs[iMetaToken].firstGroup);
      printf ("metaTokenSpecs[%d].lastGroup  = %d \n\n", iMetaToken, metaTokenSpecs[iMetaToken].lastGroup);
    }
   }
 #endif

 imprime_cabecalho_meta_tokens (arq_Entrada_Ortografia_Nome);
 imprime_cabecalho_meta_erros  (arq_Entrada_Ortografia_Nome);

 /* Insira o token EOF na lista de tokens da linguagem LiDAS */

 insere_novo_token (LEXAN_token_EOF, CADEIA_TK_EOF, "", ROTULO_TK_EOF);

 lexJobId = LEXAN_start_job (
   (char *)               arq_Entrada_Ortografia_Nome,
   (reg_syntax_t)         LEXAN_REGEX_SYNTAX,
   (LEXAN_t_tokenCase)    LEXAN_t_keepCase,
   (int)                  totMetaTokenSpecs,
   (LEXAN_t_tokenSpecs *) metaTokenSpecs );

 tokenOK = LEXAN_get_token (
  (LEXAN_t_lexJobId)     lexJobId,
  (LEXAN_t_tokenType *) &metaToken,
  (LEXAN_t_tokenVal *)  &tokenVal );

 while (tokenOK && (metaToken != LEXAN_token_EOF)) {
   metaTokenLocation = LEXAN_get_token_location (lexJobId);
   nroLinhaAtual = metaTokenLocation.lineNumber;
   nroColunaAtual = metaTokenLocation.columnNumber;
   pLinhaAtual = LEXAN_get_token_context (lexJobId);
   mudouDeLinha = (nroLinhaAtual != nroLinhaAnterior);
   if (mudouDeLinha) {
     imprime_linha_entrada_ortografia (nroLinhaAtual, pLinhaAtual);
     nroLinhaAnterior = nroLinhaAtual;
   }
   imprime_detalhes_meta_token (metaTokenLocation, metaToken, tokenVal);

   /*                                         */
   /* Analise sintatica do arquivo de entrada */
   /*                                         */

   codigoMetaToken = cadeiaMetaToken2codigoMetaToken (tokenVal.tokenStr, META_COMANDOS_CASE_SENSITIVE);
   if (codigoMetaToken > 0)
     listaMetaTokens[codigoMetaToken-1].usosMetaToken++;

   /*                               */
   /* Encontrado meta-token "Caixa" */
   /*                               */


   if (espero_Caixa && (codigoMetaToken == metaTk_caixa)) {
     estou_em_Caixa = TRUE;
     /*
     achei_Caixa    = TRUE;
     */

     /* Proximo meta-token esperado: "Identificadores" ou "Comandos" */

     espero_Caixa            = FALSE;
     espero_Identificadores  = TRUE;
     espero_Comandos_1       = TRUE;
     indiceMetaTokenEsperado = 1;
   }

   /*                                                       */
   /* Encontrado meta-token "Identificadores' ou 'Comandos" */
   /*                                                       */

   else if ( (espero_Identificadores || espero_Comandos_1) &&
             (codigoMetaToken == metaTk_identificadores || codigoMetaToken == metaTk_comandos) ) {
     if ( ((codigoMetaToken == metaTk_identificadores) && achei_Identificadores) ||
          ((codigoMetaToken == metaTk_comandos) && achei_Comandos_1) ) {
       snprintf (strErro, TAM_MAX_STR_ERRO, "Palavra reservada '%s' usada mais de uma vez", tokenVal.tokenStr);
       imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
       metaTokensOK = FALSE;
     }
     if (codigoMetaToken == metaTk_identificadores)
       achei_Identificadores = TRUE;
     else
       achei_Comandos_1 = TRUE;

     /* Proximo meta-token esperado: "Distinguem" ou "Ignoram" */

     espero_Identificadores  = FALSE;
     espero_Comandos_1       = FALSE;
     espero_Distinguem       = TRUE;
     espero_Ignoram          = TRUE;
     indiceMetaTokenEsperado = 4;
   }

   /*                                                 */
   /* Encontrado meta-token "Distinguem" ou "Ignoram" */
   /*                                                 */

   else if ( (espero_Distinguem || espero_Ignoram) &&
             (codigoMetaToken == metaTk_distinguem || codigoMetaToken == metaTk_ignoram) ) {
     if (codigoMetaToken == metaTk_distinguem) {
       /*
       achei_Distinguem = TRUE;
       achei_Ignoram    = FALSE;
       */
       caixa = t_caixa_distinguem;
     }
     else {
       /*
       achei_Distinguem = FALSE;
       achei_Ignoram    = TRUE;
       */
       caixa = t_caixa_ignoram;
     }
     espero_Distinguem  = FALSE;
     espero_Ignoram     = FALSE;

     /* Ja havia processado caixa de identificadores e de comandos? */

     if ((caixa_Identificadores != t_caixa_indefinido) && (caixa_Comandos != t_caixa_indefinido)) {
       snprintf (strErro, TAM_MAX_STR_ERRO, "Palavra reservada '%s' usada mais de uma vez", tokenVal.tokenStr);
       imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
       metaTokensOK = FALSE;
     }

     /* Ja havia processado caixa somente de comandos? */

     else if ((caixa_Identificadores == t_caixa_indefinido) && (caixa_Comandos != t_caixa_indefinido)) {
       caixa_Identificadores   = caixa;
       estou_em_Caixa          = FALSE;
       espero_Comentarios      = TRUE;
       indiceMetaTokenEsperado = 5;
     }

     /* Ja havia processado caixa somente de identificadores? */

     else if ((caixa_Identificadores != t_caixa_indefinido) && (caixa_Comandos == t_caixa_indefinido)) {
       caixa_Comandos          = caixa;
       estou_em_Caixa          = FALSE;
       espero_Comentarios      = TRUE;
       indiceMetaTokenEsperado = 5;
     }

     /* Nao havia processado caixa nem de identificadores, nem de comandos */
     /* Estou agora processando caixa de comandos                          */

     else if ((! achei_Identificadores) && achei_Comandos_1) {
       caixa_Comandos          = caixa;
       espero_Identificadores  = TRUE;
       indiceMetaTokenEsperado = 2;
     }

     /* Nao havia processado caixa nem de identificadores, nem de comandos */
     /* Estou agora processando caixa de identificadores                   */

     else if (achei_Identificadores && (! achei_Comandos_1)) {
       caixa_Identificadores   = caixa;
       espero_Comandos_1       = TRUE;
       indiceMetaTokenEsperado = 3;
     }

     /* Nao havia processado caixa nem de identificadores, nem de comandos */
     /* Nao estou processando agora caixa nem de um, nem de outro -- ERRO! */
     /* Isso nunca deveria acontecer                                       */

     else {
       snprintf (strErro, TAM_MAX_STR_ERRO, "Palavra reservada '%s' deve seguir 'Identificadores' ou 'Comandos'", tokenVal.tokenStr);
       imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
       metaTokensOK = FALSE;
     }
   }

  /*                                     */
  /* Encontrado meta-token "Comentarios" */
  /*                                     */

   else if (espero_Comentarios && (codigoMetaToken == metaTk_comentarios)) {
     estou_em_Comentarios = TRUE;
     /*
     achei_Comentarios    = TRUE;
     */
     espero_Comentarios   = FALSE;

     /* Proximo meta-token esperado: "Linha" ou "Abre" ou "Fecha" */

     espero_Linha            = TRUE;
     espero_Abre             = TRUE;
     espero_Fecha            = TRUE;
     indiceMetaTokenEsperado = 6;
   }

  /*                                 */
  /* Encontrado meta-token "Lexemas" */
  /*                                 */

   else if (espero_Lexemas && (codigoMetaToken == metaTk_lexemas)) {
     estou_em_Lexemas = TRUE;
     /*
     achei_Lexemas    = TRUE;
     */

     /* Proximo meta-token esperado: "Identificador" ou "Inteiro" ou "Decimal" ou "String" */

     espero_Lexemas          = FALSE;
     espero_Identificador    = TRUE;
     espero_Inteiro          = TRUE;
     espero_Decimal          = TRUE;
     espero_String           = TRUE;
     indiceMetaTokenEsperado = 12;
   }

  /*                                  */
  /* Encontrado meta-token "Comandos" */
  /*                                  */

   else if (espero_Comandos_2 && (codigoMetaToken == metaTk_comandos)) {
     estou_em_Lexemas  = FALSE;
     estou_em_Comandos = TRUE;
     /*
     achei_Comandos_2  = TRUE;
     */

     /* Proximo meta-token esperado: metaTk_metaString */

     espero_Comandos_2       = FALSE;
     espero_meta_string      = TRUE;
     indiceMetaTokenEsperado = 10;
   }

   /*                                                    */
   /* Encontrado meta-token "Linha" ou "Abre" ou "Fecha" */
   /*                                                    */

   else if ( ( espero_Linha ||
               espero_Abre  ||
               espero_Fecha ) &&
             ( codigoMetaToken == metaTk_linha ||
               codigoMetaToken == metaTk_abre  ||
               codigoMetaToken == metaTk_fecha ) ) {

     /* Ja havia processado esse tipo de meta-comentario? */

     if ( ((codigoMetaToken == metaTk_linha) && achei_Linha) ||
          ((codigoMetaToken == metaTk_abre)  && achei_Abre)  ||
          ((codigoMetaToken == metaTk_fecha) && achei_Fecha) ) {
       snprintf (strErro, TAM_MAX_STR_ERRO, "Palavra reservada '%s' usada mais de uma vez", tokenVal.tokenStr);
       imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
       metaTokensOK = FALSE;
     }

     /* Ainda nao havia processado esse tipo de meta-comentario; processar agora */

     else {
       if (codigoMetaToken == metaTk_linha)
         achei_Linha = TRUE;
       else if (codigoMetaToken == metaTk_abre)
         achei_Abre = TRUE;
       else
         achei_Fecha = TRUE;

       /* Proximo meta-token esperado: metaTk_metaString */

       espero_Linha            = FALSE;
       espero_Abre             = FALSE;
       espero_Fecha            = FALSE;
       espero_meta_string      = TRUE;
       indiceMetaTokenEsperado = 10;
     }
   }

   /*                                                                             */
   /* Encontrado meta-token "Identificador" ou "Inteiro" ou "Decimal" ou "String" */
   /*                                                                             */

   else if ( ( espero_Identificador ||
               espero_Inteiro       ||
               espero_Decimal       ||
               espero_String ) &&
             ( codigoMetaToken == metaTk_identificador ||
               codigoMetaToken == metaTk_inteiro       ||
               codigoMetaToken == metaTk_decimal       ||
               codigoMetaToken == metaTk_string ) ) {

     /* Ja havia processado esse tipo de meta-comentario? */

     if ( ((codigoMetaToken == metaTk_identificador) && achei_Identificador) ||
          ((codigoMetaToken == metaTk_inteiro)       && achei_Inteiro)       ||
          ((codigoMetaToken == metaTk_decimal)       && achei_Decimal)       ||
          ((codigoMetaToken == metaTk_string)        && achei_String) ) {
       snprintf (strErro, TAM_MAX_STR_ERRO, "Palavra reservada '%s' usada mais de uma vez", tokenVal.tokenStr);
       imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
       metaTokensOK = FALSE;
     }

     /* Ainda nao havia processado esse tipo de meta-lexema; processar agora */

     else {
       if (codigoMetaToken == metaTk_identificador)
         achei_Identificador = estou_em_Identificador = TRUE;
       else if (codigoMetaToken == metaTk_inteiro)
         achei_Inteiro = estou_em_Inteiro = TRUE;
       else if (codigoMetaToken == metaTk_decimal)
         achei_Decimal = estou_em_Decimal = TRUE;
       else
         achei_String = estou_em_String = TRUE;

       /* Proximo meta-token esperado: metaTk_metaString */

       espero_Identificador    = FALSE;
       espero_Inteiro          = FALSE;
       espero_Decimal          = FALSE;
       espero_String           = FALSE;
       espero_meta_string      = TRUE;
       indiceMetaTokenEsperado = 10;
     }
   }

   /*                                         */
   /* Encontrado meta-token metaTk_metaString */
   /*                                         */

   else if (espero_meta_string && (codigoMetaToken == metaTk_metaString)) {
     /*
     achei_meta_string = TRUE;
     */

     /* Descarte as aspas no inicio e fim do string */

     memset (strMetaString, 0, sizeof (strMetaString));
     strncpy (strMetaString, &(tokenVal.tokenStr[1]), strlen (tokenVal.tokenStr) - 2);
     strcpy (tokenVal.tokenStr, strMetaString);

     /* O que fazer com esse string e qual o proximo meta-token esperado dependem de onde estamos no arquivo de entrada */
     /* Meta-literais podem ocorrer em três secoes: COMENTARIOS, LEXEMAS e COMANDOS                                     */

     /*                                                */
     /* O meta-string ocorreu na secao de COMENTARIOS? */
     /*                                                */

     if (estou_em_Comentarios) {

       if (achei_Linha && (! achei_regex_linha)) {
         achei_regex_linha = TRUE;
         (void) STRING_copy (&lidas_regex_comentario_linha, tokenVal.tokenStr);

         /* Proximo meta-token esperado: depende de ainda haver ou nao comentarios a serem definidos */

         espero_meta_string = FALSE;
         espero_Linha        = ! achei_Linha;
         espero_Abre         = ! achei_Abre;
         espero_Fecha        = ! achei_Fecha;
         if (espero_Linha || espero_Abre || espero_Fecha)
           indiceMetaTokenEsperado = 6;
         else {
           estou_em_Comentarios    = FALSE;
           espero_Lexemas          = TRUE;
           indiceMetaTokenEsperado = 11;
         }
       }
       else if (achei_Abre && (! achei_regex_abre)) {
         achei_regex_abre = TRUE;
         (void) STRING_copy (&lidas_regex_comentario_abre, tokenVal.tokenStr);

         /* Proximo meta-token esperado: depende de ainda haver ou nao comentarios a serem definidos */

         espero_meta_string = FALSE;
         espero_Linha        = ! achei_Linha;
         espero_Abre         = ! achei_Abre;
         espero_Fecha        = ! achei_Fecha;
         if (espero_Linha || espero_Abre || espero_Fecha)
           indiceMetaTokenEsperado = 6;
         else {
           estou_em_Comentarios    = FALSE;
           espero_Lexemas          = TRUE;
           indiceMetaTokenEsperado = 11;
         }
       }
       else if (achei_Fecha && (! achei_regex_fecha)) {
         achei_regex_fecha = TRUE;
         (void) STRING_copy (&lidas_regex_comentario_fecha, tokenVal.tokenStr);

         /* Proximo meta-token esperado: depende de ainda haver ou nao comentarios a serem definidos */

         espero_meta_string = FALSE;
         espero_Linha        = ! achei_Linha;
         espero_Abre         = ! achei_Abre;
         espero_Fecha        = ! achei_Fecha;
         if (espero_Linha || espero_Abre || espero_Fecha)
           indiceMetaTokenEsperado = 6;
         else {
           estou_em_Comentarios    = FALSE;
           espero_Lexemas          = TRUE;
           indiceMetaTokenEsperado = 11;
         }
       }

       /* Isso nunca deveria acontecer */

       else {
         snprintf (strErro, TAM_MAX_STR_ERRO, "Meta-string '%s' em local inesperado da secao de comentarios", tokenVal.tokenStr);
         imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
         metaTokensOK = FALSE;
       }

     }

     /*                                            */
     /* O meta-string ocorreu na secao de LEXEMAS? */
     /*                                            */

     else if (estou_em_Lexemas) {
       espero_Comandos_2 = FALSE;

       if (estou_em_Identificador) {

         /* O primeiro string de um lexema e seu rotulo */

         if (! achei_rotulo_identificador) {
           achei_rotulo_identificador = TRUE;
           (void) STRING_copy (&lidas_rotulo_lexema_identificador, tokenVal.tokenStr);

           /* Proximo meta-token esperado: metaTk_metaString */

           espero_meta_string      = TRUE;
           indiceMetaTokenEsperado = 10;
         }

         /* O segundo string de um lexema e sua expressao regular */

         else if (! achei_regex_identificador) {
           achei_regex_identificador = TRUE;
           (void) STRING_copy (&lidas_regex_lexema_identificador, tokenVal.tokenStr);

           /* Proximo meta-token esperado: "Inteiro" ou "Decimal" ou "String" */

           espero_meta_string      = FALSE;
           espero_Comandos_2       = TRUE;
           espero_Identificador    = ! achei_Identificador;
           espero_Inteiro          = ! achei_Inteiro;
           espero_Decimal          = ! achei_Decimal;
           espero_String           = ! achei_String;
           if (espero_Identificador || espero_Inteiro || espero_Decimal || espero_String)
             indiceMetaTokenEsperado = 14;
           else {
             estou_em_Lexemas        = FALSE;
             indiceMetaTokenEsperado = 13;
           }
           estou_em_Identificador = FALSE;
         }

         /* O terceiro string de um lexema e um erro */

         else {
           snprintf (strErro, TAM_MAX_STR_ERRO, "Meta-string '%s' em excesso na secao LEXEMAS > Identificador", tokenVal.tokenStr);
           imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
           metaTokensOK = FALSE;
         }

       }

       else if (estou_em_Inteiro) {

         /* O primeiro string de um lexema e seu rotulo */

         if (! achei_rotulo_inteiro) {
           achei_rotulo_inteiro = TRUE;
           (void) STRING_copy (&lidas_rotulo_lexema_inteiro, tokenVal.tokenStr);

           /* Proximo meta-token esperado: metaTk_metaString */

           espero_meta_string      = TRUE;
           indiceMetaTokenEsperado = 10;
         }

         /* O segundo string de um lexema e sua expressao regular */

         else if (! achei_regex_inteiro) {
           achei_regex_inteiro = TRUE;
           (void) STRING_copy (&lidas_regex_lexema_inteiro, tokenVal.tokenStr);

           /* Proximo meta-token esperado: "Identificador" ou "Decimal" ou "String" */

           espero_meta_string      = FALSE;
           espero_Comandos_2       = TRUE;
           espero_Identificador    = ! achei_Identificador;
           espero_Inteiro          = ! achei_Inteiro;
           espero_Decimal          = ! achei_Decimal;
           espero_String           = ! achei_String;
           if (espero_Identificador || espero_Inteiro || espero_Decimal || espero_String)
             indiceMetaTokenEsperado = 14;
           else {
             estou_em_Lexemas        = FALSE;
             indiceMetaTokenEsperado = 13;
           }
           estou_em_Inteiro = FALSE;
         }

         /* O terceiro string de um lexema e um erro */

         else {
           snprintf (strErro, TAM_MAX_STR_ERRO, "Meta-string '%s' em excesso na secao LEXEMAS > Inteiro", tokenVal.tokenStr);
           imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
           metaTokensOK = FALSE;
         }

       }

       else if (estou_em_Decimal) {

         /* O primeiro string de um lexema e seu rotulo */

         if (! achei_rotulo_decimal) {
           achei_rotulo_decimal = TRUE;
           (void) STRING_copy (&lidas_rotulo_lexema_decimal, tokenVal.tokenStr);

           /* Proximo meta-token esperado: metaTk_metaString */

           espero_meta_string      = TRUE;
           indiceMetaTokenEsperado = 10;
         }

         /* O segundo string de um lexema e sua expressao regular */

         else if (! achei_regex_decimal) {
           achei_regex_decimal = TRUE;
           (void) STRING_copy (&lidas_regex_lexema_decimal, tokenVal.tokenStr);

           /* Proximo meta-token esperado: "Identificador" ou "Inteiro" ou "String" */

           espero_meta_string      = FALSE;
           espero_Comandos_2       = TRUE;
           espero_Identificador    = ! achei_Identificador;
           espero_Inteiro          = ! achei_Inteiro;
           espero_Decimal          = ! achei_Decimal;
           espero_String           = ! achei_String;
           if (espero_Identificador || espero_Inteiro || espero_Decimal || espero_String)
             indiceMetaTokenEsperado = 14;
           else {
             estou_em_Lexemas        = FALSE;
             indiceMetaTokenEsperado = 13;
           }
           estou_em_Decimal = FALSE;
         }

         /* O terceiro string de um lexema e um erro */

         else {
           snprintf (strErro, TAM_MAX_STR_ERRO, "Meta-string '%s' em excesso na secao LEXEMAS > Decimal", tokenVal.tokenStr);
           imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
           metaTokensOK = FALSE;
         }

       }

       else if (estou_em_String) {

         /* O primeiro string de um lexema e seu rotulo */

         if (! achei_rotulo_string) {
           achei_rotulo_string = TRUE;
           (void) STRING_copy (&lidas_rotulo_lexema_string, tokenVal.tokenStr);

           /* Proximo meta-token esperado: metaTk_metaString */

           espero_meta_string      = TRUE;
           indiceMetaTokenEsperado = 10;
         }

         /* O segundo string de um lexema e sua expressao regular */

         else if (! achei_regex_string) {
           achei_regex_string = TRUE;
           (void) STRING_copy (&lidas_regex_lexema_string, tokenVal.tokenStr);

           /* Proximo meta-token esperado: "Identificador" ou "Inteiro" ou "Decimal" */

           espero_meta_string      = FALSE;
           espero_Comandos_2       = TRUE;
           espero_Identificador    = ! achei_Identificador;
           espero_Inteiro          = ! achei_Inteiro;
           espero_Decimal          = ! achei_Decimal;
           espero_String           = ! achei_String;
           if (espero_Identificador || espero_Inteiro || espero_Decimal || espero_String)
             indiceMetaTokenEsperado = 14;
           else {
             estou_em_Lexemas        = FALSE;
             indiceMetaTokenEsperado = 13;
           }
           estou_em_String = FALSE;
         }

         /* O terceiro string de um lexema e um erro */

         else {
           snprintf (strErro, TAM_MAX_STR_ERRO, "Meta-string '%s' em excesso na secao LEXEMAS > String", tokenVal.tokenStr);
           imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
           metaTokensOK = FALSE;
         }

       }

       /* Isso nunca deveria acontecer */

       else {
         snprintf (strErro, TAM_MAX_STR_ERRO, "Meta-string '%s' em local inesperado da secao de lexemas", tokenVal.tokenStr);
         imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
         metaTokensOK = FALSE;
       }

     }

     /*                                             */
     /* O meta-string ocorreu na secao de COMANDOS? */
     /*                                             */

     else if (estou_em_Comandos) {

         /* O primeiro string de um comando e sua cadeia */

         if (! achei_cadeia_comando) {
           achei_cadeia_comando = TRUE;
           if (strlen(tokenVal.tokenStr) > TAM_MAX_CADEIA_META_TOKEN) {
             snprintf (strErro, TAM_MAX_STR_ERRO, "Meta-string %s longo demais para lidas_cadeia_comando[%d]", tokenVal.tokenStr, totComandos);
             imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
             metaTokensOK = FALSE;
           }

           /* Certifique-se que essa cadeia nao foi declarada antes */

           (void) STRING_copy (&copiaCadeiaAtual, tokenVal.tokenStr);
           comprCadeiaAtual = strlen (copiaCadeiaAtual);
           if (caixa_Comandos == t_caixa_ignoram)
             for (iChar = 0; iChar < comprCadeiaAtual; iChar++)
               copiaCadeiaAtual[iChar] = tolower(copiaCadeiaAtual[iChar]);
           for (iComando = 0; iComando < totComandos; iComando++) {
             (void) STRING_copy (&copiaCadeiaAnterior, lidas_cadeia_comando[iComando]);
             comprCadeiaAnterior = strlen (copiaCadeiaAnterior);
             if (comprCadeiaAtual != comprCadeiaAnterior)
               continue;
             if (caixa_Comandos == t_caixa_ignoram)
               for (iChar = 0; iChar < comprCadeiaAnterior; iChar++)
                 copiaCadeiaAnterior[iChar] = tolower(copiaCadeiaAnterior[iChar]);
             if (! strcmp (copiaCadeiaAtual, copiaCadeiaAnterior))
               break;
           }
           if (iComando < totComandos) {
             (void) STRING_clear (&lidas_cadeia_comando[totComandos]);
             snprintf (strErro, TAM_MAX_STR_ERRO, "Cadeia %s duplicada em COMANDOS", tokenVal.tokenStr);
             imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
             metaTokensOK = FALSE;
           }
           else
             (void) STRING_n_copy (&lidas_cadeia_comando[totComandos], tokenVal.tokenStr, TAM_MAX_CADEIA_META_TOKEN-1);

           /* Proximo meta-token esperado: metaTk_metaString */

           espero_meta_string      = TRUE;
           espero_fim_de_arquivo   = FALSE;
           indiceMetaTokenEsperado = 10;
         }

         /* O segundo string de um comando e seu rotulo */

         else {
           if (strlen(tokenVal.tokenStr) > TAM_MAX_ROTULO_META_TOKEN) {
             snprintf (strErro, TAM_MAX_STR_ERRO, "Meta-string %s longo demais para lidas_rotulo_comando[%d]", tokenVal.tokenStr, totComandos);
             imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
             metaTokensOK = FALSE;
           }

           /* Certifique-se que esse rotulo nao foi declarado antes */

           (void) STRING_copy (&copiaRotuloAtual, tokenVal.tokenStr);
           comprRotuloAtual = strlen (copiaRotuloAtual);
           if (caixa_Comandos == t_caixa_ignoram)
             for (iChar = 0; iChar < comprRotuloAtual; iChar++)
               copiaRotuloAtual[iChar] = tolower(copiaRotuloAtual[iChar]);
           for (iComando = 0; iComando < totComandos; iComando++) {
             (void) STRING_copy (&copiaRotuloAnterior, lidas_rotulo_comando[iComando]);
             comprRotuloAnterior = strlen (copiaRotuloAnterior);
             if (comprRotuloAtual != comprRotuloAnterior)
               continue;
             if (caixa_Comandos == t_caixa_ignoram)
               for (iChar = 0; iChar < comprRotuloAnterior; iChar++)
                 copiaRotuloAnterior[iChar] = tolower(copiaRotuloAnterior[iChar]);
             if (! strcmp (copiaRotuloAtual, copiaRotuloAnterior))
               break;
           }
           if (iComando < totComandos) {
             (void) STRING_clear (&lidas_rotulo_comando[totComandos]);
             snprintf (strErro, TAM_MAX_STR_ERRO, "Rotulo %s duplicada em COMANDOS", tokenVal.tokenStr);
             imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
             metaTokensOK = FALSE;
           }
           else
             (void) STRING_n_copy (&lidas_rotulo_comando[totComandos], tokenVal.tokenStr, TAM_MAX_ROTULO_META_TOKEN-1);

           totComandos++;

           /* Proximo meta-token esperado: metaTk_metaString ou LEXAN_token_EOF */

           espero_meta_string      = TRUE;
           espero_fim_de_arquivo   = TRUE;
           indiceMetaTokenEsperado = 10;
           achei_cadeia_comando    = FALSE;
         }

     }

     /*                              */
     /* Isso nunca deveria acontecer */
     /*                              */

     else {
       snprintf (strErro, TAM_MAX_STR_ERRO, "Meta-string \"%s\" em secao inesperada", tokenVal.tokenStr);
       imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
       metaTokensOK = FALSE;
     }
   }

   /*                                              */
   /* Erro sintatico: nao encontrou o que esperava */
   /*                                              */

   else {
     snprintf (strErro, TAM_MAX_STR_ERRO, "Esperava %s ao inves de %s", descrMetaTokenEsperado[indiceMetaTokenEsperado], tokenVal.tokenStr);
     imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
     metaTokensOK = FALSE;
   }

   free (tokenVal.tokenStr);

   tokenOK = LEXAN_get_token (
     (LEXAN_t_lexJobId)     lexJobId,
     (LEXAN_t_tokenType *) &metaToken,
     (LEXAN_t_tokenVal *)  &tokenVal );
 }

 if (! espero_fim_de_arquivo) {
     if (estou_em_Caixa)
       snprintf (strErro, TAM_MAX_STR_ERRO, "Fim de arquivo prematuro, processando CAIXA");
     else if (estou_em_Comentarios)
       snprintf (strErro, TAM_MAX_STR_ERRO, "Fim de arquivo prematuro, processando COMENTARIOS");
     else if (estou_em_Lexemas)
       snprintf (strErro, TAM_MAX_STR_ERRO, "Fim de arquivo prematuro, processando LEXEMAS");
     else if (estou_em_Comandos)
       snprintf (strErro, TAM_MAX_STR_ERRO, "Fim de arquivo prematuro, processando COMANDOS");
     else
       snprintf (strErro, TAM_MAX_STR_ERRO, "Fim de arquivo prematuro, processando secao indefinida");
   imprime_meta_erro (nroLinhaAtual, nroColunaAtual, strErro);
   metaTokensOK = FALSE;
 }

 fprintf (arq_Saida_Meta_Tokens_Ptr, "%s", tracos_tab_meta_tokens);
 REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 2);

 imprime_resumo_meta_tokens();

 REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 1);
 fprintf (arq_Saida_Meta_Tokens_Ptr, "TOTAL DE ERROS: %d", totMetaErros + LEXAN_get_error_count (lexJobId));
 REPORT_newLine (arq_Saida_Meta_Tokens_Ptr, 1);

 REPORT_newLine (arq_Saida_Meta_Erros_Ptr, 2);
 fprintf (arq_Saida_Meta_Erros_Ptr, "TOTAL DE ERROS: %d", totMetaErros + LEXAN_get_error_count (lexJobId));
 fflush(NULL);

 LEXAN_terminate_job (lexJobId);

 /*                                                                                  */
 /* Termine de processar o que foi encontrado na especificacao da linguagem LiDAS    */
 /*                                                                                  */

 totTokenSpecs = 0;

 if (achei_Linha)         totTokenSpecs++;
 if (achei_Abre)          totTokenSpecs++;
 if (achei_Fecha)         totTokenSpecs++;
 if (achei_Identificador) totTokenSpecs++;
 if (achei_Inteiro)       totTokenSpecs++;
 if (achei_Decimal)       totTokenSpecs++;
 if (achei_String)        totTokenSpecs++;
 if (totComandos > 0)     totTokenSpecs++;

 errno = 0;
 if (totTokenSpecs == 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Nenhum meta-token encontrado");
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }

 errno = 0;
 p_tokenSpecs = (LEXAN_t_tokenSpecs *) malloc (totTokenSpecs * sizeof (LEXAN_t_tokenSpecs));
 if (p_tokenSpecs == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em p_tokenSpecs = malloc (%d * %zd)", totTokenSpecs, sizeof (LEXAN_t_tokenSpecs));
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }

 iTokenSpecs = -1;

 if (achei_Linha) {
   ++iTokenSpecs;
   p_tokenSpecs[iTokenSpecs].type       = (LEXAN_t_tokenType) LEXAN_token_LINE_COMMENT;
   p_tokenSpecs[iTokenSpecs].regex      = (char *)            lidas_regex_comentario_linha;
   p_tokenSpecs[iTokenSpecs].firstGroup = (int)               0;
   p_tokenSpecs[iTokenSpecs].lastGroup  = (int)               0;
 }

 if (achei_Abre) {
   ++iTokenSpecs;
   p_tokenSpecs[iTokenSpecs].type       = (LEXAN_t_tokenType) LEXAN_token_OPEN_COMMENT;
   p_tokenSpecs[iTokenSpecs].regex      = (char *)            lidas_regex_comentario_abre;
   p_tokenSpecs[iTokenSpecs].firstGroup = (int)               0;
   p_tokenSpecs[iTokenSpecs].lastGroup  = (int)               0;
 }

 if (achei_Fecha) {
   ++iTokenSpecs;
   p_tokenSpecs[iTokenSpecs].type       = (LEXAN_t_tokenType) LEXAN_token_CLOSE_COMMENT;
   p_tokenSpecs[iTokenSpecs].regex      = (char *)            lidas_regex_comentario_fecha;
   p_tokenSpecs[iTokenSpecs].firstGroup = (int)               0;
   p_tokenSpecs[iTokenSpecs].lastGroup  = (int)               0;
 }

 if (achei_Identificador) {
   ++iTokenSpecs;
   p_tokenSpecs[iTokenSpecs].type       = (LEXAN_t_tokenType) LEXAN_token_iden;
   p_tokenSpecs[iTokenSpecs].regex      = (char *)            lidas_regex_lexema_identificador;
   p_tokenSpecs[iTokenSpecs].firstGroup = (int)               1;
   p_tokenSpecs[iTokenSpecs].lastGroup  = (int)               1;
   insere_novo_token (LEXAN_token_iden, "", lidas_regex_lexema_identificador, lidas_rotulo_lexema_identificador);
 }

 if (achei_Inteiro) {
   ++iTokenSpecs;
   p_tokenSpecs[iTokenSpecs].type       = (LEXAN_t_tokenType) LEXAN_token_longInt;
   p_tokenSpecs[iTokenSpecs].regex      = (char *)            lidas_regex_lexema_inteiro;
   p_tokenSpecs[iTokenSpecs].firstGroup = (int)               1;
   p_tokenSpecs[iTokenSpecs].lastGroup  = (int)               1;
   insere_novo_token (LEXAN_token_longInt, "", lidas_regex_lexema_inteiro, lidas_rotulo_lexema_inteiro);
 }

 if (achei_Decimal) {
   ++iTokenSpecs;
   p_tokenSpecs[iTokenSpecs].type       = (LEXAN_t_tokenType) LEXAN_token_double;
   p_tokenSpecs[iTokenSpecs].regex      = (char *)            lidas_regex_lexema_decimal;
   p_tokenSpecs[iTokenSpecs].firstGroup = (int)               1;
   p_tokenSpecs[iTokenSpecs].lastGroup  = (int)               1;
   insere_novo_token (LEXAN_token_double, "", lidas_regex_lexema_decimal, lidas_rotulo_lexema_decimal);
 }

 if (achei_String) {
   tipoString = (lidas_regex_lexema_string[0] == '"' ? LEXAN_token_double_str : LEXAN_token_single_str);
   ++iTokenSpecs;
   p_tokenSpecs[iTokenSpecs].type       = (LEXAN_t_tokenType) tipoString;
   p_tokenSpecs[iTokenSpecs].regex      = (char *)            lidas_regex_lexema_string;
   p_tokenSpecs[iTokenSpecs].firstGroup = (int)               1;
   p_tokenSpecs[iTokenSpecs].lastGroup  = (int)               1;
   insere_novo_token (tipoString, "", lidas_regex_lexema_string, lidas_rotulo_lexema_string);
 }

 if (totComandos > 0) {
   ++iTokenSpecs;
   (void) STRING_copy (&lidas_regex_TOKEN_resWord, "(");
   for (iComando = 0; iComando < totComandos - 1; iComando++) {
     (void) STRING_copy   (&lidas_regex_comando[iComando], cadeia2regex(lidas_cadeia_comando[iComando], (caixa_Comandos == t_caixa_distinguem)));
     (void) STRING_concatenate (&lidas_regex_TOKEN_resWord, lidas_regex_comando[iComando]);
     (void) STRING_concatenate (&lidas_regex_TOKEN_resWord, "|");
     insere_novo_token (LEXAN_token_resWord, lidas_cadeia_comando[iComando], lidas_regex_comando[iComando], lidas_rotulo_comando[iComando]);
   }
   insere_novo_token (LEXAN_token_resWord, lidas_cadeia_comando[iComando], lidas_regex_comando[iComando], lidas_rotulo_comando[iComando]);
   (void) STRING_copy   (&lidas_regex_comando[iComando], cadeia2regex(lidas_cadeia_comando[iComando], (caixa_Comandos == t_caixa_distinguem)));
   (void) STRING_concatenate (&lidas_regex_TOKEN_resWord, lidas_regex_comando[iComando]);
   (void) STRING_concatenate (&lidas_regex_TOKEN_resWord, ")");
   p_tokenSpecs[iTokenSpecs].type       = (LEXAN_t_tokenType) LEXAN_token_resWord;
   p_tokenSpecs[iTokenSpecs].regex      = (char *)            lidas_regex_TOKEN_resWord;
   p_tokenSpecs[iTokenSpecs].firstGroup = (int)               1;
   p_tokenSpecs[iTokenSpecs].lastGroup  = (int)               1;
 }

 #ifdef GERA_SAIDA_DEBUG
   {
    int iToken;
    printf ("tamListaTokens = %d \n\n", tamListaTokens);
    for (iToken = 0; iToken < tamListaTokens; iToken++) {
      printf ("listaTokens[%d].codigoToken = %d \n",     iToken, listaTokens[iToken].codigoToken);
      printf ("listaTokens[%d].tipoToken   = %d (%s)\n", iToken, listaTokens[iToken].tipoToken, LEXAN_tokenTypeNames[listaTokens[iToken].tipoToken]);
      printf ("listaTokens[%d].cadeiaToken = %s \n",     iToken, listaTokens[iToken].cadeiaToken);
      printf ("listaTokens[%d].regexToken  = %s \n",     iToken, listaTokens[iToken].regexToken);
      printf ("listaTokens[%d].rotuloToken = %s \n",     iToken, listaTokens[iToken].rotuloToken);
      printf ("listaTokens[%d].usosToken   = %d \n\n",   iToken, listaTokens[iToken].usosToken);
    }
    printf ("totTokenSpecs = %d \n\n", totTokenSpecs);
    for (iToken = 0; iToken < totTokenSpecs; iToken++) {
      printf ("p_tokenSpecs[%d].type       = %d (%s)\n", iToken, p_tokenSpecs[iToken].type, LEXAN_tokenTypeNames[p_tokenSpecs[iToken].type]);
      printf ("p_tokenSpecs[%d].regex      = %s \n",     iToken, p_tokenSpecs[iToken].regex);
      printf ("p_tokenSpecs[%d].firstGroup = %d \n",     iToken, p_tokenSpecs[iToken].firstGroup);
      printf ("p_tokenSpecs[%d].lastGroup  = %d \n\n",   iToken, p_tokenSpecs[iToken].lastGroup);
      fflush(NULL);
    }
   }
 #endif

 /* Libere memoria dinamica alocada nesta funcao */

 free (metaTokenSpecs[0].regex);
 STRING_release (&copiaRotuloAnterior);
 STRING_release (&copiaCadeiaAnterior);
 STRING_release (&copiaRotuloAtual);
 STRING_release (&copiaCadeiaAtual);
 STRING_release (&meta_token_regex);

 /* Mais nada a fazer a nao ser retornar */

 return (tokenOK && metaTokensOK);
}

/*
*---------------------------------------------------------------------
* Processa o arquivo de entrada contendo o codigo fonte em LiDAS
*---------------------------------------------------------------------
*/

void processa_fonteEmLidas (void)
{
 int
   posTabSimb,
   nroLinhaAtual    = 0,
   nroLinhaAnterior = 0;
 t_codigoToken
   codigoToken;
 Bool
   linhaAnteriorOK = TRUE,
   linhaAtualOK    = TRUE,
   mudouDeLinha    = FALSE;
 LEXAN_t_lexJobId
   lexJobId;
 LEXAN_t_tokenType
   token;
 LEXAN_t_tokenVal
   tokenVal;
 LEXAN_t_tokenLocation
   tokenLocation;
 char
   *strAux           = NULL,
   *strLinhaAtual    = NULL,
   *strLinhaAnterior = NULL;

 strAux           = STRING_allocate();
 strLinhaAtual    = STRING_allocate();
 strLinhaAnterior = STRING_allocate();

 imprime_cabecalho_tokens (arq_Entrada_Programa_Nome);
 imprime_cabecalho_erros  (arq_Entrada_Programa_Nome);

 lexJobId = LEXAN_start_job (
   (char *)               arq_Entrada_Programa_Nome,
   (reg_syntax_t)         LEXAN_REGEX_SYNTAX,
   (LEXAN_t_tokenCase)    LEXAN_t_keepCase,
   (int)                  totTokenSpecs,
   (LEXAN_t_tokenSpecs *) p_tokenSpecs );

 (void) LEXAN_get_token (
  (LEXAN_t_lexJobId)     lexJobId,
  (LEXAN_t_tokenType *) &token,
  (LEXAN_t_tokenVal *)  &tokenVal );

 while (token != LEXAN_token_EOF) {
   tokenLocation = LEXAN_get_token_location (lexJobId);
   nroLinhaAtual = tokenLocation.lineNumber;
   (void) STRING_copy (&strLinhaAtual, LEXAN_get_token_context (lexJobId));
   mudouDeLinha = (nroLinhaAtual != nroLinhaAnterior);
   if (mudouDeLinha) {
     linhaAnteriorOK = linhaAtualOK;
     linhaAtualOK = (token != LEXAN_token_NULL);
   }
   else
     linhaAtualOK = (linhaAtualOK && (token != LEXAN_token_NULL));
   if (mudouDeLinha) {
     if (linhaAnteriorOK && (nroLinhaAnterior > 0))
       imprime_linha_entrada_programa (nroLinhaAnterior, strLinhaAnterior);
     nroLinhaAnterior = nroLinhaAtual;
     (void) STRING_copy (&strLinhaAnterior, strLinhaAtual);
   }

   (void) STRING_set (&strAux, 0, TAM_MAX_LEXEMA);
   switch (token) {
     case (LEXAN_token_iden):
       codigoToken = tipoToken2codigoToken (token);
       #ifdef GERA_SAIDA_DEBUG
         printf ("%s \"%s\" codigo %d lin %d col %d\n", LEXAN_tokenTypeNames[token], tokenVal.tokenStr, codigoToken, tokenLocation.lineNumber, tokenLocation.columnNumber);
         fflush(NULL);
       #endif
       snprintf (strAux, TAM_MAX_LEXEMA, FORMATO_LEXEMA_STRING, tokenVal.tokenStr);
       posTabSimb = instala_na_tabSimb (codigoToken, strAux, tokenLocation, caixa_Identificadores == t_caixa_distinguem);
       imprime_detalhes_token (tokenLocation, token, codigoToken, tokenVal, posTabSimb+1);
       listaTokens[codigoToken - 1].usosToken++;
       break;
     case (LEXAN_token_longInt):
       codigoToken = tipoToken2codigoToken (token);
       #ifdef GERA_SAIDA_DEBUG
         printf ("%s %ld codigo %d lin %d col %d\n", LEXAN_tokenTypeNames[token], tokenVal.longIntVal, codigoToken, tokenLocation.lineNumber, tokenLocation.columnNumber);
         fflush(NULL);
       #endif
       snprintf (strAux, TAM_MAX_LEXEMA, FORMATO_LEXEMA_INTEIRO, tokenVal.longIntVal);
       posTabSimb = instala_na_tabSimb (codigoToken, strAux, tokenLocation, FALSE);
       imprime_detalhes_token (tokenLocation, token, codigoToken, tokenVal, posTabSimb+1);
       listaTokens[codigoToken - 1].usosToken++;
       break;
     case (LEXAN_token_double):
       codigoToken = tipoToken2codigoToken (token);
       #ifdef GERA_SAIDA_DEBUG
         printf ("%s %ld codigo %d lin %d col %d\n", LEXAN_tokenTypeNames[token], tokenVal.longIntVal, codigoToken, tokenLocation.lineNumber, tokenLocation.columnNumber);
         fflush(NULL);
       #endif
       snprintf (strAux, TAM_MAX_LEXEMA, FORMATO_LEXEMA_DOUBLE, tokenVal.doubleVal);
       posTabSimb = instala_na_tabSimb (codigoToken, strAux, tokenLocation, FALSE);
       imprime_detalhes_token (tokenLocation, token, codigoToken, tokenVal, posTabSimb+1);
       listaTokens[codigoToken - 1].usosToken++;
       break;
     case (LEXAN_token_single_str):
     case (LEXAN_token_double_str):
       codigoToken = tipoToken2codigoToken (token);
       #ifdef GERA_SAIDA_DEBUG
         printf ("%s \"%s\" codigo %d lin %d col %d\n", LEXAN_tokenTypeNames[token], tokenVal.tokenStr, codigoToken, tokenLocation.lineNumber, tokenLocation.columnNumber);
         fflush(NULL);
       #endif
       snprintf (strAux, TAM_MAX_LEXEMA, FORMATO_LEXEMA_STRING, tokenVal.tokenStr);
       posTabSimb = instala_na_tabSimb (codigoToken, strAux, tokenLocation, FALSE);
       imprime_detalhes_token (tokenLocation, token, codigoToken, tokenVal, posTabSimb+1);
       listaTokens[codigoToken - 1].usosToken++;
       break;
     case (LEXAN_token_resWord):
       codigoToken = cadeiaToken2codigoToken (tokenVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
       #ifdef GERA_SAIDA_DEBUG
         printf ("%s \"%s\" codigo %d lin %d col %d\n", LEXAN_tokenTypeNames[token], tokenVal.tokenStr, codigoToken, tokenLocation.lineNumber, tokenLocation.columnNumber);
         fflush(NULL);
       #endif
       if (codigoToken == 0)
         fprintf (arq_Saida_Erros_Ptr, "ERRO: Palavra reservada \"%s\" desconhecida \n", tokenVal.tokenStr);
       else {
         imprime_detalhes_token (tokenLocation, token, codigoToken, tokenVal, 0);
         listaTokens[codigoToken - 1].usosToken++;
       }
       break;
     case (LEXAN_token_NULL):
       imprime_linha_entrada_programa (nroLinhaAtual, LEXAN_get_error_descr (lexJobId));
       break;
     case (LEXAN_token_delim)        :
     case (LEXAN_token_EOF)          :
     case (LEXAN_token_LINE_COMMENT) :
     case (LEXAN_token_OPEN_COMMENT) :
     case (LEXAN_token_CLOSE_COMMENT):
       fprintf (arq_Saida_Tokens_Ptr, "ERRO: Token %d invalido. %s \n", token, LEXAN_get_error_descr (lexJobId));
       break;
     default:
       fprintf (arq_Saida_Tokens_Ptr, "ERRO: Token %d desconhecido. %s \n", token, LEXAN_get_error_descr (lexJobId));
       break;
   }
   free (tokenVal.tokenStr);

   (void) LEXAN_get_token (
     (LEXAN_t_lexJobId)     lexJobId,
     (LEXAN_t_tokenType *) &token,
     (LEXAN_t_tokenVal *)  &tokenVal );
 }

 imprime_linha_entrada_programa (nroLinhaAnterior, strLinhaAnterior);

 tokenLocation = LEXAN_get_token_location (lexJobId);
 imprime_detalhes_token (tokenLocation, LEXAN_token_EOF, tipoToken2codigoToken(LEXAN_token_EOF), tokenVal, 0);
 fprintf (arq_Saida_Tokens_Ptr, "%s", tracos_tab_tokens);
 REPORT_newLine (arq_Saida_Tokens_Ptr, 2);

 imprime_resumo_tokens();

 REPORT_newLine (arq_Saida_Tokens_Ptr, 2);
 fprintf (arq_Saida_Tokens_Ptr, "TOTAL DE ERROS: %d", LEXAN_get_error_count (lexJobId));
 REPORT_newLine (arq_Saida_Tokens_Ptr, 1);

 REPORT_newLine (arq_Saida_Erros_Ptr, 2);
 fprintf (arq_Saida_Erros_Ptr, "TOTAL DE ERROS: %d", LEXAN_get_error_count (lexJobId));

 imprime_tabSimb (arq_Entrada_Programa_Nome);

 LEXAN_terminate_job (lexJobId);
 fflush (NULL);
 STRING_release (&strAux);
 STRING_release (&strLinhaAtual);
 STRING_release (&strLinhaAnterior);
}

/*
*---------------------------------------------------------------------
* Analisador sintatico descendente recursivo
*---------------------------------------------------------------------
*/
t_codigoToken
  codToken;
LEXAN_t_lexJobId
  lxJobId;
LEXAN_t_tokenType
  tok;
LEXAN_t_tokenVal
  tokVal;

int faz_analiseSintatica (void){

 lxJobId = LEXAN_start_job (
  (char *)               arq_Entrada_Programa_Nome,
  (reg_syntax_t)         LEXAN_REGEX_SYNTAX,
  (LEXAN_t_tokenCase)    LEXAN_t_keepCase,
  (int)                  totTokenSpecs,
  (LEXAN_t_tokenSpecs *) p_tokenSpecs );

 (void) LEXAN_get_token (
  (LEXAN_t_lexJobId)     lxJobId,
  (LEXAN_t_tokenType *) &tok,
  (LEXAN_t_tokenVal *)  &tokVal );

 while (tok != LEXAN_token_EOF){
   if(tok == LEXAN_token_resWord)
     codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
   else
     codToken = tipoToken2codigoToken (tok);

   if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_defina") == 0) {
     if(!tkDefinaEncontrado())
      return 0;
   }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_mostre") == 0){
     mostrar = 1;
     if(!tkMostreApagueEncontrado())
      return 0;

   }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_apague") == 0 ){
     mostrar = 0;
     if(!tkMostreApagueEncontrado())
      return 0;
   }else{
     printf("ERRO! 'Defina', 'Mostre' ou 'Apague' esperados.\n");
    return 0;
   }

   (void) LEXAN_get_token (
    (LEXAN_t_lexJobId)     lxJobId,
    (LEXAN_t_tokenType *) &tok,
    (LEXAN_t_tokenVal *)  &tokVal );
 }

 LEXAN_terminate_job (lxJobId);
 fflush (NULL);
 return 1;
}

int tkDefinaEncontrado(void){
 (void) LEXAN_get_token (
   (LEXAN_t_lexJobId)     lxJobId,
   (LEXAN_t_tokenType *) &tok,
   (LEXAN_t_tokenVal *)  &tokVal );
 if(tok == LEXAN_token_resWord)
   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
 else
   codToken = tipoToken2codigoToken (tok);

 if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_IDEN") == 0 ){
   char *iden = tokVal.tokenStr;
   (void) LEXAN_get_token (
    (LEXAN_t_lexJobId)     lxJobId,
    (LEXAN_t_tokenType *) &tok,
    (LEXAN_t_tokenVal *)  &tokVal );
    if(tok == LEXAN_token_resWord)
      codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
    else
      codToken = tipoToken2codigoToken (tok);

    if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_CADEIA") == 0 ){
      char *cadeia = tokVal.tokenStr;
      (void) LEXAN_get_token (
        (LEXAN_t_lexJobId)     lxJobId,
        (LEXAN_t_tokenType *) &tok,
        (LEXAN_t_tokenVal *)  &tokVal );
      if(tok == LEXAN_token_resWord)
        codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
      else
        codToken = tipoToken2codigoToken (tok);

      if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
        if(listaDefs.p_primeiro == NULL){
         t_defIden *novoRegListDef = (t_defIden *)malloc(sizeof(t_defIden));
         inicializarRegistroListaDeIdentificadores(novoRegListDef, iden, cadeia);
         inserirNaListaDeIdentificadores(novoRegListDef);
         printf("DEFINA %s = %s;\n", iden, cadeia);
         return 1;
        }
        else if(!identificarDefinido(iden)){
         t_defIden *novoRegListDef = (t_defIden *)malloc(sizeof(t_defIden));
         inicializarRegistroListaDeIdentificadores(novoRegListDef, iden, cadeia);
         inserirNaListaDeIdentificadores(novoRegListDef);
         printf("DEFINA %s = %s;\n", iden, cadeia);
         return 1;
        }
        else
         printf("ERRO! \"%s\" ja definido!\n", iden);
        return 0;
      }else{
        printf("ERRO! Ponto-e-virgula esperado apos \"Defina %s %s\".\n", iden, cadeia);
        return 0;
      }
    }
    else{
      printf("ERRO! Cadeia ou Expressao Regular esperada apos \"Defina %s\".\n", iden);
      return 0;
    }
 }else{
   printf("ERRO! Identificador esperado apos \"Defina\".\n");
   return 0;
 }
}

int tkMostreApagueEncontrado(void){
 (void) LEXAN_get_token (
     (LEXAN_t_lexJobId)     lxJobId,
     (LEXAN_t_tokenType *) &tok,
     (LEXAN_t_tokenVal *)  &tokVal );
   if(tok == LEXAN_token_resWord)
     codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
   else
     codToken = tipoToken2codigoToken (tok);

   if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_IDEN") == 0 ){
     char *nome = tokVal.tokenStr;
     if(listaDefs.p_primeiro == NULL || !identificarDefinido(nome)){
      printf("ERRO! \"%s\" nao foi definido.\n", nome);
      return 0;
     }
     (void) LEXAN_get_token (
       (LEXAN_t_lexJobId)     lxJobId,
       (LEXAN_t_tokenType *) &tok,
       (LEXAN_t_tokenVal *)  &tokVal );
     if(tok == LEXAN_token_resWord)
       codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
     else
       codToken = tipoToken2codigoToken (tok);

     if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_umAum") == 0 ){
      if(!tkUmAUmEncontrado(nome, 1))
       return 0;

      }else if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_juntos") == 0 ){
       if(!tkUmAUmEncontrado(nome, 0))
        return 0;

      }else if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
        if(identificarDefinido(nome)){
          regex_t re;
          t_defIden *aux = listaDefs.p_primeiro;
          while(aux != NULL){
            if(strcmp(aux->iden, nome) == 0){
             if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
               t_registroId *regId = listaIds.p_primeiro;
               flagInicio = 1;
               while(regId != NULL){
                 if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                   if(listaIdenUt.p_primeiro==NULL){
                    t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                    inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                    inserirNaListaDeIdenUtilizados(novoIden);
                   }
                   else if (!idenJaFoiUtilizado(regId->lexema)){
                    t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                    inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                    inserirNaListaDeIdenUtilizados(novoIden);
                   }
                   novoFrame = (t_frame *)malloc(sizeof(t_frame));
                   inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, 0, 0, mostrar);
                   inserirNaListaDeFrames(novoFrame);
                   if(flagInicio)
                    flagInicio = 0;
                 }
                 regId = regId->proximo;
               }
             }
            }
            aux = aux->proximo;
          }
          regfree(&re);
        }
        if(mostrar)
         printf("Mostre %s;\n", nome);
        else
         printf("Apague %s;\n", nome);
        return 1;

     }else if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_apos") == 0 ){
       (void) LEXAN_get_token (
         (LEXAN_t_lexJobId)     lxJobId,
         (LEXAN_t_tokenType *) &tok,
         (LEXAN_t_tokenVal *)  &tokVal );
       if(tok == LEXAN_token_resWord)
         codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
       else
         codToken = tipoToken2codigoToken (tok);

       if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
           tempoInt = tokVal.longIntVal;
           (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
             (void) LEXAN_get_token (
               (LEXAN_t_lexJobId)     lxJobId,
               (LEXAN_t_tokenType *) &tok,
               (LEXAN_t_tokenVal *)  &tokVal );
             if(tok == LEXAN_token_resWord)
               codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
             else
               codToken = tipoToken2codigoToken (tok);

             if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
               if(identificarDefinido(nome)){
                 regex_t re;
                 t_defIden *aux = listaDefs.p_primeiro;
                 while(aux != NULL){
                   if(strcmp(aux->iden, nome) == 0){
                    if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                      t_registroId *regId = listaIds.p_primeiro;
                      flagInicio = 1;
                      while(regId != NULL){
                        if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                          if(listaIdenUt.p_primeiro==NULL){
                           t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                           inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                           inserirNaListaDeIdenUtilizados(novoIden);
                          }
                          else if (!idenJaFoiUtilizado(regId->lexema)){
                           t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                           inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                           inserirNaListaDeIdenUtilizados(novoIden);
                          }
                          novoFrame = (t_frame *)malloc(sizeof(t_frame));
                          inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, tempoInt*1000, 0, mostrar);
                          inserirNaListaDeFrames(novoFrame);
                          if(flagInicio)
                           flagInicio = 0;
                        }
                        regId = regId->proximo;
                      }
                    }
                   }
                   aux = aux->proximo;
                 }
                 regfree(&re);
               }
               if(mostrar)
                printf("Mostre %s apos %d segundos;\n", nome, tempoInt);
               else
                printf("Apague %s apos %d segundos;\n", nome, tempoInt);
               return 1;

             }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
               (void) LEXAN_get_token (
                 (LEXAN_t_lexJobId)     lxJobId,
                 (LEXAN_t_tokenType *) &tok,
                 (LEXAN_t_tokenVal *)  &tokVal );
               if(tok == LEXAN_token_resWord)
                 codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
               else
                 codToken = tipoToken2codigoToken (tok);

               if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
                 tempoIntPor = tokVal.longIntVal;
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                   (void) LEXAN_get_token (
                     (LEXAN_t_lexJobId)     lxJobId,
                     (LEXAN_t_tokenType *) &tok,
                     (LEXAN_t_tokenVal *)  &tokVal );
                   if(tok == LEXAN_token_resWord)
                     codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                   else
                     codToken = tipoToken2codigoToken (tok);

                   if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                     t_defIden *aux;
                     regex_t re;
                     if(identificarDefinido(nome)){
                       aux = listaDefs.p_primeiro;
                       while(aux != NULL){
                         if(strcmp(aux->iden, nome) == 0){
                          if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                            t_registroId *regId = listaIds.p_primeiro;
                            flagInicio = 1;
                            while(regId != NULL){
                              if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                                if(listaIdenUt.p_primeiro==NULL){
                                 t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                                 inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                                 inserirNaListaDeIdenUtilizados(novoIden);
                                }
                                else if (!idenJaFoiUtilizado(regId->lexema)){
                                 t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                                 inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                                 inserirNaListaDeIdenUtilizados(novoIden);
                                }
                                novoFrame = (t_frame *)malloc(sizeof(t_frame));
                                inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, tempoInt*1000, 0, mostrar);
                                inserirNaListaDeFrames(novoFrame);
                                if(flagInicio)
                                 flagInicio = 0;
                              }
                              regId = regId->proximo;
                            }
                          }
                         }
                         aux = aux->proximo;
                       }
                     }
                     aux = listaDefs.p_primeiro;
                     while(aux != NULL){
                       if(strcmp(aux->iden, nome) == 0){
                        if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                          t_registroId *regId = listaIds.p_primeiro;
                          while(regId != NULL){
                            if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                              novoFrame = (t_frame *)malloc(sizeof(t_frame));
                              inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, tempoIntPor*1000, 0, !mostrar);
                              inserirNaListaDeFrames(novoFrame);
                            }
                            regId = regId->proximo;
                          }
                        }
                       }
                       aux = aux->proximo;
                     }
                     regfree(&re);

                     if(mostrar)
                      printf("Mostre %s apos %d segundos por %d segundos;\n", nome, tempoInt, tempoIntPor);
                     else
                      printf("Apague %s apos %d segundos por %d segundos;\n", nome, tempoInt, tempoIntPor);
                     return 1;
                   }else{
                     printf("ERRO! Ponto-e-virgula esperado apos 'segundos'.\n");
                    return 0;
                   }
                 }else{
                   printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %d\".\n", tempoIntPor);
                   return 0;
                 }

               }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
                 tempoDecPor = tokVal.doubleVal;
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                   (void) LEXAN_get_token (
                     (LEXAN_t_lexJobId)     lxJobId,
                     (LEXAN_t_tokenType *) &tok,
                     (LEXAN_t_tokenVal *)  &tokVal );
                   if(tok == LEXAN_token_resWord)
                     codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                   else
                     codToken = tipoToken2codigoToken (tok);

                   if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                     t_defIden *aux;
                     regex_t re;
                     if(identificarDefinido(nome)){
                        aux = listaDefs.p_primeiro;
                        while(aux != NULL){
                          if(strcmp(aux->iden, nome) == 0){
                           if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                             t_registroId *regId = listaIds.p_primeiro;
                             flagInicio = 1;
                             while(regId != NULL){
                               if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                                 if(listaIdenUt.p_primeiro==NULL){
                                  t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                                  inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                                  inserirNaListaDeIdenUtilizados(novoIden);
                                 }
                                 else if (!idenJaFoiUtilizado(regId->lexema)){
                                  t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                                  inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                                  inserirNaListaDeIdenUtilizados(novoIden);
                                 }
                                 novoFrame = (t_frame *)malloc(sizeof(t_frame));
                                 inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, tempoInt*1000, 0, mostrar);
                                 inserirNaListaDeFrames(novoFrame);
                                 if(flagInicio)
                                  flagInicio = 0;
                               }
                               regId = regId->proximo;
                             }
                           }
                          }
                          aux = aux->proximo;
                        }
                     }
                     aux = listaDefs.p_primeiro;
                     while(aux != NULL){
                       if(strcmp(aux->iden, nome) == 0){
                        if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                          t_registroId *regId = listaIds.p_primeiro;
                          while(regId != NULL){
                            if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                              novoFrame = (t_frame *)malloc(sizeof(t_frame));
                              inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
                              inserirNaListaDeFrames(novoFrame);
                            }
                            regId = regId->proximo;
                          }
                        }
                       }
                       aux = aux->proximo;
                     }
                     regfree(&re);

                     if(mostrar)
                      printf("Mostre %s apos %d segundos por %.2f segundos;\n", nome, tempoInt, tempoDecPor);
                     else
                      printf("Apague %s apos %d segundos por %.2f segundos;\n", nome, tempoInt, tempoDecPor);
                     return 1;
                   }else{
                     printf("ERRO! Ponto-e-virgula esperado apos \"por %.2f segundos\".\n", tempoDecPor);
                     return 0;
                   }
                 }else{
                   printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDecPor);
                   return 0;
                 }
               }else{
                 printf("ERRO! DECIMAL ou INTEIRO esperado apos \"...%d segundos por\".\n", tempoInt);
                 return 0;
               }

             }
             else{
               printf("ERRO! Ponto-e-virgula esperado após \"...apos %d segundos\".\n", tempoInt);
               return 0;
             }
           }else{
             printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...apos %d\".\n", tempoInt);
             return 0;
           }

         }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
           tempoDec = tokVal.doubleVal;
           (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
             (void) LEXAN_get_token (
               (LEXAN_t_lexJobId)     lxJobId,
               (LEXAN_t_tokenType *) &tok,
               (LEXAN_t_tokenVal *)  &tokVal );
             if(tok == LEXAN_token_resWord)
               codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
             else
               codToken = tipoToken2codigoToken (tok);

             if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
               if(identificarDefinido(nome)){
                 regex_t re;
                 t_defIden *aux = listaDefs.p_primeiro;
                 while(aux != NULL){
                   if(strcmp(aux->iden, nome) == 0){
                    if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                      t_registroId *regId = listaIds.p_primeiro;
                      flagInicio = 1;
                      while(regId != NULL){
                        if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                          if(listaIdenUt.p_primeiro==NULL){
                           t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                           inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                           inserirNaListaDeIdenUtilizados(novoIden);
                          }
                          else if (!idenJaFoiUtilizado(regId->lexema)){
                           t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                           inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                           inserirNaListaDeIdenUtilizados(novoIden);
                          }
                          novoFrame = (t_frame *)malloc(sizeof(t_frame));
                          inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, (int)(tempoDec*1000), 0, mostrar);
                          inserirNaListaDeFrames(novoFrame);
                          if(flagInicio)
                           flagInicio = 0;
                        }
                        regId = regId->proximo;
                      }
                    }
                   }
                   aux = aux->proximo;
                 }
                 regfree(&re);
              }
              if(mostrar)
               printf("Mostre %s apos %.2f segundos;\n", nome, tempoDec);
              else
               printf("Apague %s apos %.2f segundos;\n", nome, tempoDec);
              return 1;
             }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
               (void) LEXAN_get_token (
                 (LEXAN_t_lexJobId)     lxJobId,
                 (LEXAN_t_tokenType *) &tok,
                 (LEXAN_t_tokenVal *)  &tokVal );
               if(tok == LEXAN_token_resWord)
                 codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
               else
                 codToken = tipoToken2codigoToken (tok);

               if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
                 tempoIntPor = tokVal.longIntVal;
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                   (void) LEXAN_get_token (
                     (LEXAN_t_lexJobId)     lxJobId,
                     (LEXAN_t_tokenType *) &tok,
                     (LEXAN_t_tokenVal *)  &tokVal );
                   if(tok == LEXAN_token_resWord)
                     codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                   else
                     codToken = tipoToken2codigoToken (tok);

                   if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                     regex_t re;
                     t_defIden *aux;
                     if(identificarDefinido(nome)){
                      aux = listaDefs.p_primeiro;
                      while(aux != NULL){
                        if(strcmp(aux->iden, nome) == 0){
                         if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                           t_registroId *regId = listaIds.p_primeiro;
                           flagInicio = 1;
                           while(regId != NULL){
                             if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                               if(listaIdenUt.p_primeiro==NULL){
                                t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                                inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                                inserirNaListaDeIdenUtilizados(novoIden);
                               }
                               else if (!idenJaFoiUtilizado(regId->lexema)){
                                t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                                inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                                inserirNaListaDeIdenUtilizados(novoIden);
                               }
                               novoFrame = (t_frame *)malloc(sizeof(t_frame));
                               inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, (int)(tempoDec*1000), 0, mostrar);
                               inserirNaListaDeFrames(novoFrame);
                               if(flagInicio)
                                flagInicio = 0;
                             }
                             regId = regId->proximo;
                           }
                         }
                        }
                        aux = aux->proximo;
                      }
                    }
                    aux = listaDefs.p_primeiro;
                     while(aux != NULL){
                       if(strcmp(aux->iden, nome) == 0){
                        if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                          t_registroId *regId = listaIds.p_primeiro;
                          while(regId != NULL){
                            if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                              novoFrame = (t_frame *)malloc(sizeof(t_frame));
                              inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, tempoIntPor*1000, 0, !mostrar);
                              inserirNaListaDeFrames(novoFrame);
                            }
                            regId = regId->proximo;
                          }
                        }
                       }
                       aux = aux->proximo;
                     }
                     regfree(&re);

                     if(mostrar)
                      printf("Mostre %s apos %.2f segundos por %d segundos;\n", nome, tempoDec, tempoIntPor);
                     else
                      printf("Apague %s apos %.2f segundos por %d segundos;\n", nome, tempoDec, tempoIntPor);
                     return 1;
                   }else{
                     printf("ERRO! Ponto-e-virgula esperado apos \"...por %d segundos\".\n", tempoIntPor);
                    return 0;
                   }
                 }else{
                   printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %d\".\n", tempoIntPor);
                  return 0;
                 }

               }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
                 tempoDecPor = tokVal.doubleVal;
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                   (void) LEXAN_get_token (
                     (LEXAN_t_lexJobId)     lxJobId,
                     (LEXAN_t_tokenType *) &tok,
                     (LEXAN_t_tokenVal *)  &tokVal );
                   if(tok == LEXAN_token_resWord)
                     codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                   else
                     codToken = tipoToken2codigoToken (tok);

                   if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                     regex_t re;
                     t_defIden *aux;
                     if(identificarDefinido(nome)){
                      aux = listaDefs.p_primeiro;
                      while(aux != NULL){
                        if(strcmp(aux->iden, nome) == 0){
                         if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                           t_registroId *regId = listaIds.p_primeiro;
                           flagInicio = 1;
                           while(regId != NULL){
                             if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                               if(listaIdenUt.p_primeiro==NULL){
                                t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                                inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                                inserirNaListaDeIdenUtilizados(novoIden);
                               }
                               else if (!idenJaFoiUtilizado(regId->lexema)){
                                t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                                inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                                inserirNaListaDeIdenUtilizados(novoIden);
                               }
                               novoFrame = (t_frame *)malloc(sizeof(t_frame));
                               inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, (int)(tempoDec*1000), 0, mostrar);
                               inserirNaListaDeFrames(novoFrame);
                               if(flagInicio)
                                flagInicio = 0;
                             }
                             regId = regId->proximo;
                           }
                         }
                        }
                        aux = aux->proximo;
                      }
                    }
                    aux = listaDefs.p_primeiro;
                     while(aux != NULL){
                       if(strcmp(aux->iden, nome) == 0){
                        if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                          t_registroId *regId = listaIds.p_primeiro;
                          while(regId != NULL){
                            if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                              novoFrame = (t_frame *)malloc(sizeof(t_frame));
                              inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
                              inserirNaListaDeFrames(novoFrame);
                            }
                            regId = regId->proximo;
                          }
                        }
                       }
                       aux = aux->proximo;
                     }
                     regfree(&re);

                     if(mostrar)
                      printf("Mostre %s apos %.2f segundos por %.2f segundos;\n", nome, tempoDec, tempoDecPor);
                     else
                      printf("Apague %s apos %.2f segundos por %.2f segundos;\n", nome, tempoDec, tempoDecPor);
                     return 1;
                   }else{
                     printf("ERRO! Ponto-e-virgula esperado apos \"...por %.2f segundos\".\n", tempoDecPor);
                     return 0;
                   }
                 }else{
                   printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDecPor);
                   return 0;
                 }
               }else{
                 printf("ERRO! DECIMAL ou INTEIRO esperado apos \"...%.2f segundos por\".\n", tempoDec);
                 return 0;
               }
             }else{
               printf("ERRO! Ponto-e-virgula ou 'por' esperado após \"...apos %.2f segundos\".\n", tempoDec);
               return 0;
             }
           }else{
             printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...apos %.2f\".\n", tempoDec);
             return 0;
           }

         }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_tecla") == 0 ){
           (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
            regex_t re;
            t_defIden *aux;
            if(identificarDefinido(nome)){
              aux = listaDefs.p_primeiro;
              while(aux != NULL){
                if(strcmp(aux->iden, nome) == 0){
                 if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                   t_registroId *regId = listaIds.p_primeiro;
                   flagInicio = 1;
                   while(regId != NULL){
                     if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                       if(listaIdenUt.p_primeiro==NULL){
                        t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                        inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                        inserirNaListaDeIdenUtilizados(novoIden);
                       }
                       else if (!idenJaFoiUtilizado(regId->lexema)){
                        t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                        inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                        inserirNaListaDeIdenUtilizados(novoIden);
                       }
                       novoFrame = (t_frame *)malloc(sizeof(t_frame));
                       inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, 0, 0, mostrar);
                       inserirNaListaDeFrames(novoFrame);
                       if(flagInicio)
                        flagInicio = 0;
                     }
                     regId = regId->proximo;
                   }
                 }
                }
                aux = aux->proximo;
              }
              regfree(&re);
            }
            if(mostrar)
             printf("Mostre \"%s\" apos tecla.\n", nome);
            else
             printf("Mostre \"%s\" apos tecla.\n", nome);
            return 1;

           }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
            (void) LEXAN_get_token (
              (LEXAN_t_lexJobId)     lxJobId,
              (LEXAN_t_tokenType *) &tok,
              (LEXAN_t_tokenVal *)  &tokVal );
            if(tok == LEXAN_token_resWord)
              codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
            else
              codToken = tipoToken2codigoToken (tok);

             if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
               tempoIntPor = tokVal.longIntVal;
               (void) LEXAN_get_token (
                 (LEXAN_t_lexJobId)     lxJobId,
                 (LEXAN_t_tokenType *) &tok,
                 (LEXAN_t_tokenVal *)  &tokVal );
               if(tok == LEXAN_token_resWord)
                 codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
               else
                 codToken = tipoToken2codigoToken (tok);

               if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                   regex_t re;
                   t_defIden *aux;
                   if(identificarDefinido(nome)){
                     aux = listaDefs.p_primeiro;
                     while(aux != NULL){
                       if(strcmp(aux->iden, nome) == 0){
                        if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                          t_registroId *regId = listaIds.p_primeiro;
                          flagInicio = 1;
                          while(regId != NULL){
                            if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                              if(listaIdenUt.p_primeiro==NULL){
                               t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                               inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                               inserirNaListaDeIdenUtilizados(novoIden);
                              }
                              else if (!idenJaFoiUtilizado(regId->lexema)){
                               t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                               inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                               inserirNaListaDeIdenUtilizados(novoIden);
                              }
                              novoFrame = (t_frame *)malloc(sizeof(t_frame));
                              inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, 0, 0, mostrar);
                              inserirNaListaDeFrames(novoFrame);
                              if(flagInicio)
                               flagInicio = 0;
                            }
                            regId = regId->proximo;
                          }
                        }
                       }
                       aux = aux->proximo;
                     }
                   }
                   aux = listaDefs.p_primeiro;
                   while(aux != NULL){
                     if(strcmp(aux->iden, nome) == 0){
                      if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                        t_registroId *regId = listaIds.p_primeiro;
                        while(regId != NULL){
                          if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                            novoFrame = (t_frame *)malloc(sizeof(t_frame));
                            inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, tempoIntPor*1000, 0, !mostrar);
                            inserirNaListaDeFrames(novoFrame);
                          }
                          regId = regId->proximo;
                        }
                      }
                     }
                     aux = aux->proximo;
                   }
                   regfree(&re);

                   if(mostrar)
                    printf("Mostre %s apos tecla por %d segundos;\n", nome, tempoIntPor);
                   else
                    printf("Apague %s apos tecla por %d segundos;\n", nome, tempoIntPor);
                   return 1;

                 }else{
                   printf("ERRO! Ponto-e-virgula esperado apos \"...por %d segundos\".\n", tempoIntPor);
                   return 0;
                 }
                }else{
                  printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %d\".\n", tempoIntPor);
                  return 0;
                }

               }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
                 tempoDecPor = tokVal.doubleVal;
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                   (void) LEXAN_get_token (
                     (LEXAN_t_lexJobId)     lxJobId,
                     (LEXAN_t_tokenType *) &tok,
                     (LEXAN_t_tokenVal *)  &tokVal );
                   if(tok == LEXAN_token_resWord)
                     codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                   else
                     codToken = tipoToken2codigoToken (tok);

                   if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                     regex_t re;
                     t_defIden *aux;
                     if(identificarDefinido(nome)){
                       aux = listaDefs.p_primeiro;
                       while(aux != NULL){
                         if(strcmp(aux->iden, nome) == 0){
                          if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                            t_registroId *regId = listaIds.p_primeiro;
                            flagInicio = 1;
                            while(regId != NULL){
                              if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                                if(listaIdenUt.p_primeiro==NULL){
                                 t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                                 inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                                 inserirNaListaDeIdenUtilizados(novoIden);
                                }
                                else if (!idenJaFoiUtilizado(regId->lexema)){
                                 t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                                 inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                                 inserirNaListaDeIdenUtilizados(novoIden);
                                }
                                novoFrame = (t_frame *)malloc(sizeof(t_frame));
                                inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, 0, 0, mostrar);
                                inserirNaListaDeFrames(novoFrame);
                                if(flagInicio)
                                 flagInicio = 0;
                              }
                              regId = regId->proximo;
                            }
                          }
                         }
                         aux = aux->proximo;
                       }
                     }
                     aux = listaDefs.p_primeiro;
                     while(aux != NULL){
                       if(strcmp(aux->iden, nome) == 0){
                        if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                          t_registroId *regId = listaIds.p_primeiro;
                          while(regId != NULL){
                            if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                              novoFrame = (t_frame *)malloc(sizeof(t_frame));
                              inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
                              inserirNaListaDeFrames(novoFrame);
                            }
                            regId = regId->proximo;
                          }
                        }
                       }
                       aux = aux->proximo;
                     }
                     regfree(&re);

                     if(mostrar)
                      printf("Mostre %s apos tecla por %.2f segundos;\n", nome, tempoDecPor);
                     else
                      printf("Apague %s apos tecla por %.2f segundos;\n", nome, tempoDecPor);
                     return 1;

                   }else{
                     printf("ERRO! Ponto-e-virgula esperado apos \"...por %.2f segundos\".\n", tempoDecPor);
                     return 0;
                   }
                  }else{
                    printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDecPor);
                    return 0;
                  }

                }else{
                  printf("ERRO! DECIMAL ou INTEIRO esperado apos \"...apos tecla por\".\n");
                  return 0;
                }

               }else{
                if(mostrar)
                 printf("ERRO! Ponto-e-virgula ou 'por' esperado apos \"Mostre \"%s\" apos tecla\".\n", nome);
                else
                 printf("ERRO! Ponto-e-virgula ou 'por' esperado apos \"Apague \"%s\" apos tecla\".\n", nome);
                return 0;
               }

         }else{
          if(mostrar)
           printf("ERRO! DECIMAL, INTEIRO ou 'tecla' esperado apos \"Mostre %s apos\".\n",nome);
          else
           printf("ERRO! DECIMAL, INTEIRO ou 'tecla' apos \"Apague %s apos\".\n",nome);
          return 0;
         }

     }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
       (void) LEXAN_get_token (
         (LEXAN_t_lexJobId)     lxJobId,
         (LEXAN_t_tokenType *) &tok,
         (LEXAN_t_tokenVal *)  &tokVal );
       if(tok == LEXAN_token_resWord)
         codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
       else
         codToken = tipoToken2codigoToken (tok);

       if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
         tempoIntPor = tokVal.longIntVal;
         (void) LEXAN_get_token (
           (LEXAN_t_lexJobId)     lxJobId,
           (LEXAN_t_tokenType *) &tok,
           (LEXAN_t_tokenVal *)  &tokVal );
         if(tok == LEXAN_token_resWord)
           codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
         else
           codToken = tipoToken2codigoToken (tok);

         if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
           (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
             regex_t re;
             t_defIden *aux;
             if(identificarDefinido(nome)){
               aux = listaDefs.p_primeiro;
               while(aux != NULL){
                 if(strcmp(aux->iden, nome) == 0){
                  if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                    t_registroId *regId = listaIds.p_primeiro;
                    flagInicio = 1;
                    while(regId != NULL){
                      if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                        if(listaIdenUt.p_primeiro==NULL){
                         t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                         inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                         inserirNaListaDeIdenUtilizados(novoIden);
                        }
                        else if (!idenJaFoiUtilizado(regId->lexema)){
                         t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                         inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                         inserirNaListaDeIdenUtilizados(novoIden);
                        }
                        novoFrame = (t_frame *)malloc(sizeof(t_frame));
                        inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, 0, 0, mostrar);
                        inserirNaListaDeFrames(novoFrame);
                        if(flagInicio)
                         flagInicio = 0;
                      }
                      regId = regId->proximo;
                    }
                  }
                 }
                 aux = aux->proximo;
               }
             }
             aux = listaDefs.p_primeiro;
             while(aux != NULL){
               if(strcmp(aux->iden, nome) == 0){
                if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                  t_registroId *regId = listaIds.p_primeiro;
                  while(regId != NULL){
                    if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                      novoFrame = (t_frame *)malloc(sizeof(t_frame));
                      inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, tempoIntPor*1000, 0, !mostrar);
                      inserirNaListaDeFrames(novoFrame);
                    }
                    regId = regId->proximo;
                  }
                }
               }
               aux = aux->proximo;
             }
             regfree(&re);

             if(mostrar)
              printf("Mostre %s por %d segundos;\n", nome, tempoIntPor);
             else
              printf("Apague %s por %d segundos;\n", nome, tempoIntPor);
             return 1;
           }else{
             printf("ERRO! Ponto-e-virgula esperado apos \"...por %d segundos\".\n", tempoIntPor);
             return 0;
           }
         }else{
           printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %d\".\n", tempoIntPor);
           return 0;
         }

       }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
         tempoDecPor = tokVal.doubleVal;
         (void) LEXAN_get_token (
           (LEXAN_t_lexJobId)     lxJobId,
           (LEXAN_t_tokenType *) &tok,
           (LEXAN_t_tokenVal *)  &tokVal );
         if(tok == LEXAN_token_resWord)
           codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
         else
           codToken = tipoToken2codigoToken (tok);

         if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
           (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
             regex_t re;
             t_defIden *aux;
             if(identificarDefinido(nome)){
               aux = listaDefs.p_primeiro;
               while(aux != NULL){
                 if(strcmp(aux->iden, nome) == 0){
                  if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                    t_registroId *regId = listaIds.p_primeiro;
                    flagInicio = 1;
                    while(regId != NULL){
                      if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                        if(listaIdenUt.p_primeiro==NULL){
                         t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                         inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                         inserirNaListaDeIdenUtilizados(novoIden);
                        }
                        else if (!idenJaFoiUtilizado(regId->lexema)){
                         t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                         inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                         inserirNaListaDeIdenUtilizados(novoIden);
                        }
                        novoFrame = (t_frame *)malloc(sizeof(t_frame));
                        inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, 0, 0, mostrar);
                        inserirNaListaDeFrames(novoFrame);
                        if(flagInicio)
                         flagInicio = 0;
                      }
                      regId = regId->proximo;
                    }
                  }
                 }
                 aux = aux->proximo;
               }
             }
             aux = listaDefs.p_primeiro;
             while(aux != NULL){
               if(strcmp(aux->iden, nome) == 0){
                if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                  t_registroId *regId = listaIds.p_primeiro;
                  while(regId != NULL){
                    if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                      novoFrame = (t_frame *)malloc(sizeof(t_frame));
                      inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
                      inserirNaListaDeFrames(novoFrame);
                    }
                    regId = regId->proximo;
                  }
                }
               }
               aux = aux->proximo;
             }
             regfree(&re);

             if(mostrar)
              printf("Mostre %s por %.2f segundos;\n", nome, tempoDecPor);
             else
              printf("Apague %s por %.2f segundos;\n", nome, tempoDecPor);
             return 1;
           }else{
             printf("ERRO! Ponto-e-virgula esperado apos \"...por %.2f segundos\".\n", tempoDecPor);
             return 0;
           }
         }else{
           printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDecPor);
           return 0;
         }
       }else{
        if(mostrar)
         printf("ERRO! DECIMAL ou INTEIRO esperado apos \"Mostre %s por\".\n", nome);
        else
         printf("ERRO! DECIMAL ou INTEIRO esperado apos \"Apague %s por\".\n", nome);
        return 0;
       }

     }else{
      if(mostrar)
         printf("ERRO! 'apos', por', 'umAum', 'juntos' ou ';' esperado apos \"Mostre %s\".\n", nome);
        else
         printf("ERRO! 'apos', 'por', 'umAum', 'juntos' ou ';' esperado apos \"Apague %s\".\n", nome);
      return 0;
     }

   }else if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_CADEIA") == 0 ){
     char *cadeia = tokVal.tokenStr;
     if(!buscarNaListaDeIds(cadeia)){
      printf("ERRO: Id %s nao definido.", cadeia);
      return 0;
     }
     else if(listaIdenUt.p_primeiro==NULL){
      t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
      inicializarRegistroListaIdenUtilizados(novoIden, cadeia, mostrar);
      inserirNaListaDeIdenUtilizados(novoIden);
     }
     else if (!idenJaFoiUtilizado(cadeia)){
      t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
      inicializarRegistroListaIdenUtilizados(novoIden, cadeia, mostrar);
      inserirNaListaDeIdenUtilizados(novoIden);
     }
     (void) LEXAN_get_token (
       (LEXAN_t_lexJobId)     lxJobId,
       (LEXAN_t_tokenType *) &tok,
       (LEXAN_t_tokenVal *)  &tokVal );
     if(tok == LEXAN_token_resWord)
       codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
     else
       codToken = tipoToken2codigoToken (tok);

     if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
       if(buscarNaListaDeIds(cadeia)){
        novoFrame = (t_frame *)malloc(sizeof(t_frame));
        inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, 0, 0, mostrar);
        inserirNaListaDeFrames(novoFrame);
        if(mostrar)
         printf("Mostre %s;\t\n", cadeia);
        else
         printf("Apague %s;\n", cadeia);
        return 1;
       }
       else
        printf("ERRO: Id %s nao existe no arquivo \".svg\".", cadeia);
       return 0;

     }else if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_apos") == 0 ){
       (void) LEXAN_get_token (
         (LEXAN_t_lexJobId)     lxJobId,
         (LEXAN_t_tokenType *) &tok,
         (LEXAN_t_tokenVal *)  &tokVal );
       if(tok == LEXAN_token_resWord)
         codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
       else
         codToken = tipoToken2codigoToken (tok);

       if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
        tempoInt = tokVal.longIntVal;
        (void) LEXAN_get_token (
          (LEXAN_t_lexJobId)     lxJobId,
          (LEXAN_t_tokenType *) &tok,
          (LEXAN_t_tokenVal *)  &tokVal );
        if(tok == LEXAN_token_resWord)
          codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
        else
          codToken = tipoToken2codigoToken (tok);

        if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
          (void) LEXAN_get_token (
            (LEXAN_t_lexJobId)     lxJobId,
            (LEXAN_t_tokenType *) &tok,
            (LEXAN_t_tokenVal *)  &tokVal );
          if(tok == LEXAN_token_resWord)
            codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
          else
            codToken = tipoToken2codigoToken (tok);

          if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
            novoFrame = (t_frame *)malloc(sizeof(t_frame));
            inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, tempoInt*1000, 0, mostrar);
            inserirNaListaDeFrames(novoFrame);
            if(mostrar)
             printf("Mostre %s apos %d segundos;\n", cadeia, tempoInt);
            else
             printf("Apague %s apos %d segundos;\n", cadeia, tempoInt);
            return 1;

          }else if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
             (void) LEXAN_get_token (
               (LEXAN_t_lexJobId)     lxJobId,
               (LEXAN_t_tokenType *) &tok,
               (LEXAN_t_tokenVal *)  &tokVal );
             if(tok == LEXAN_token_resWord)
               codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
             else
               codToken = tipoToken2codigoToken (tok);

             if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
               tempoIntPor = tokVal.longIntVal;
               (void) LEXAN_get_token (
                 (LEXAN_t_lexJobId)     lxJobId,
                 (LEXAN_t_tokenType *) &tok,
                 (LEXAN_t_tokenVal *)  &tokVal );
               if(tok == LEXAN_token_resWord)
                 codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
               else
                 codToken = tipoToken2codigoToken (tok);

               if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                   novoFrame = (t_frame *)malloc(sizeof(t_frame));
                   inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, tempoInt*1000, 0, mostrar);
                   inserirNaListaDeFrames(novoFrame);

                   novoFrame = (t_frame *)malloc(sizeof(t_frame));
                   inicializarRegistroListaDeFrames(novoFrame, cadeia, 2, 0, tempoIntPor*1000, 0, !mostrar);
                   inserirNaListaDeFrames(novoFrame);

                   if(mostrar)
                    printf("Mostre %s apos %d segundos por %d segundos;\n", cadeia, tempoInt, tempoIntPor);
                   else
                    printf("Apague %s apos %d segundos por %d segundos;\n", cadeia, tempoInt, tempoIntPor);
                   return 1;
                 }else{
                   printf("ERRO! Ponto-e-virgula esperado apos \"...por %d segundos\".\n", tempoIntPor);
                   return 0;
                 }
               }else{
                 printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...segundos por %d\".\n", tempoIntPor);
                 return 0;
               }
             }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
               tempoDecPor = tokVal.doubleVal;
               (void) LEXAN_get_token (
                 (LEXAN_t_lexJobId)     lxJobId,
                 (LEXAN_t_tokenType *) &tok,
                 (LEXAN_t_tokenVal *)  &tokVal );
               if(tok == LEXAN_token_resWord)
                 codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
               else
                 codToken = tipoToken2codigoToken (tok);

               if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                   novoFrame = (t_frame *)malloc(sizeof(t_frame));
                   inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, tempoInt*1000, 0, mostrar);
                   inserirNaListaDeFrames(novoFrame);

                   novoFrame = (t_frame *)malloc(sizeof(t_frame));
                   inicializarRegistroListaDeFrames(novoFrame, cadeia, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
                   inserirNaListaDeFrames(novoFrame);

                   if(mostrar)
                    printf("Mostre %s apos %d segundos por %.2f segundos;\n", cadeia, tempoInt, tempoDecPor);
                   else
                    printf("Apague %s apos %d segundos por %.2f segundos;\n", cadeia, tempoInt, tempoDecPor);
                   return 1;
                 }else{
                   printf("ERRO! Ponto-e-virgula esperado apos \"...por %.2f segundos\".\n", tempoDecPor);
                   return 0;
                 }
               }else{
                 printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDecPor);
                 return 0;
               }
             }else{
               printf("ERRO! DECIMAL ou INTEIRO esperado apos \"...%d segundos por\".\n", tempoInt);
               return 0;
             }

           }else{
             printf("ERRO! Ponto-e-virgula ou 'por' esperado apos \"...apos %d segundos\".\n", tempoInt);
             return 0;
           }
         }else{
           printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...apos %d\".\n", tempoInt);
           return 0;
         }

       }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
         tempoDec = tokVal.doubleVal;
         (void) LEXAN_get_token (
           (LEXAN_t_lexJobId)     lxJobId,
           (LEXAN_t_tokenType *) &tok,
           (LEXAN_t_tokenVal *)  &tokVal );
         if(tok == LEXAN_token_resWord)
           codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
         else
           codToken = tipoToken2codigoToken (tok);

         if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
           (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
             novoFrame = (t_frame *)malloc(sizeof(t_frame));
             inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, (int)(tempoDec*1000), 0, mostrar);
             inserirNaListaDeFrames(novoFrame);

             if(mostrar)
              printf("Mostre %s apos %.2f segundos;\n", cadeia, tempoDec);
             else
              printf("Apague %s apos %.2f segundos;\n", cadeia, tempoDec);
             return 1;

           }else if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
             (void) LEXAN_get_token (
               (LEXAN_t_lexJobId)     lxJobId,
               (LEXAN_t_tokenType *) &tok,
               (LEXAN_t_tokenVal *)  &tokVal );
             if(tok == LEXAN_token_resWord)
               codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
             else
               codToken = tipoToken2codigoToken (tok);

             if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
               tempoIntPor = tokVal.longIntVal;
               (void) LEXAN_get_token (
                 (LEXAN_t_lexJobId)     lxJobId,
                 (LEXAN_t_tokenType *) &tok,
                 (LEXAN_t_tokenVal *)  &tokVal );
               if(tok == LEXAN_token_resWord)
                 codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
               else
                 codToken = tipoToken2codigoToken (tok);

               if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                    novoFrame = (t_frame *)malloc(sizeof(t_frame));
                    inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, (int)(tempoDec*1000), 0, mostrar);
                    inserirNaListaDeFrames(novoFrame);

                    novoFrame = (t_frame *)malloc(sizeof(t_frame));
                    inicializarRegistroListaDeFrames(novoFrame, cadeia, 2, 0, tempoIntPor*1000, 0, !mostrar);
                    inserirNaListaDeFrames(novoFrame);

                   if(mostrar)
                    printf("Mostre %s apos %.2f segundos por %d segundos;\n", cadeia, tempoDec, tempoIntPor);
                   else
                    printf("Apague %s apos %.2f segundos por %d segundos;\n", cadeia, tempoDec, tempoIntPor);
                   return 1;

                 }else{
                   printf("ERRO! Ponto-e-virgula esperado apos \"...por %d segundos\".\n", tempoIntPor);
                   return 0;
                 }
               }else{
                printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...apos %.2f\".\n", tempoDec);
                 return 0;
               }
           }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
             tempoDecPor = tokVal.doubleVal;
             (void) LEXAN_get_token (
               (LEXAN_t_lexJobId)     lxJobId,
               (LEXAN_t_tokenType *) &tok,
               (LEXAN_t_tokenVal *)  &tokVal );
             if(tok == LEXAN_token_resWord)
               codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
             else
               codToken = tipoToken2codigoToken (tok);

             if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
               (void) LEXAN_get_token (
                 (LEXAN_t_lexJobId)     lxJobId,
                 (LEXAN_t_tokenType *) &tok,
                 (LEXAN_t_tokenVal *)  &tokVal );
               if(tok == LEXAN_token_resWord)
                 codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
               else
                 codToken = tipoToken2codigoToken (tok);

               if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                  novoFrame = (t_frame *)malloc(sizeof(t_frame));
                  inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, (int)(tempoDec*1000), 0, mostrar);
                  inserirNaListaDeFrames(novoFrame);

                  novoFrame = (t_frame *)malloc(sizeof(t_frame));
                  inicializarRegistroListaDeFrames(novoFrame, cadeia, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
                  inserirNaListaDeFrames(novoFrame);

                 if(mostrar)
                  printf("Mostre %s apos %.2f segundos por %.2f segundos;\n", cadeia, tempoDec, tempoDecPor);
                 else
                  printf("Apague %s apos %.2f segundos por %.2f segundos;\n", cadeia, tempoDec, tempoDecPor);
                 return 1;
               }else{
                 printf("ERRO! Ponto-e-virgula esperado apos \"...por %.2f segundos\".\n", tempoDecPor);
                 return 0;
               }
             }else{
               printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDecPor);
               return 0;
             }
           }else{
             printf("ERRO! DECIMAL ou INTEIRO esperado apos \"...%.2f segundos por\".\n", tempoDec);
             return 0;
           }
          }else{
            printf("ERRO! Ponto-e-virgula ou 'por' esperado apos \"...apos %.2f segundos\".\n", tempoDec);
            return 0;
          }
         }else{
           printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...apos %.2f\".\n", tempoDec);
           return 0;
         }

       }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_tecla") == 0){
         (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){

            novoFrame = (t_frame *)malloc(sizeof(t_frame));
            inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, 0, 0, mostrar);
            inserirNaListaDeFrames(novoFrame);

            if(mostrar)
             printf("Mostre \"%s\" apos tecla.\n", cadeia);
            else
             printf("Mostre \"%s\" apos tecla.\n", cadeia);
            return 1;

           }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
            (void) LEXAN_get_token (
              (LEXAN_t_lexJobId)     lxJobId,
              (LEXAN_t_tokenType *) &tok,
              (LEXAN_t_tokenVal *)  &tokVal );
            if(tok == LEXAN_token_resWord)
              codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
            else
              codToken = tipoToken2codigoToken (tok);

             if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
               tempoIntPor = tokVal.longIntVal;
               (void) LEXAN_get_token (
                 (LEXAN_t_lexJobId)     lxJobId,
                 (LEXAN_t_tokenType *) &tok,
                 (LEXAN_t_tokenVal *)  &tokVal );
               if(tok == LEXAN_token_resWord)
                 codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
               else
                 codToken = tipoToken2codigoToken (tok);

               if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                  novoFrame = (t_frame *)malloc(sizeof(t_frame));
                  inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, 0, 0, mostrar);
                  inserirNaListaDeFrames(novoFrame);

                  novoFrame = (t_frame *)malloc(sizeof(t_frame));
                  inicializarRegistroListaDeFrames(novoFrame, cadeia, 2, 0, tempoIntPor, 0, !mostrar);
                  inserirNaListaDeFrames(novoFrame);

                  if(mostrar)
                   printf("Mostre %s apos tecla por %d segundos;\n", cadeia, tempoIntPor);
                  else
                   printf("Apague %s apos tecla por %d segundos;\n", cadeia, tempoIntPor);
                  return 1;

                 }else{
                   printf("ERRO! Ponto-e-virgula esperado apos \"...por %d segundos\".\n", tempoIntPor);
                   return 0;
                 }
                }else{
                  printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %d\".\n", tempoIntPor);
                  return 0;
                }

               }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
                 tempoDecPor = tokVal.doubleVal;
                 (void) LEXAN_get_token (
                   (LEXAN_t_lexJobId)     lxJobId,
                   (LEXAN_t_tokenType *) &tok,
                   (LEXAN_t_tokenVal *)  &tokVal );
                 if(tok == LEXAN_token_resWord)
                   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                 else
                   codToken = tipoToken2codigoToken (tok);

                 if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
                   (void) LEXAN_get_token (
                     (LEXAN_t_lexJobId)     lxJobId,
                     (LEXAN_t_tokenType *) &tok,
                     (LEXAN_t_tokenVal *)  &tokVal );
                   if(tok == LEXAN_token_resWord)
                     codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
                   else
                     codToken = tipoToken2codigoToken (tok);

                   if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
                    novoFrame = (t_frame *)malloc(sizeof(t_frame));
                    inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, 0, 0, mostrar);
                    inserirNaListaDeFrames(novoFrame);

                    novoFrame = (t_frame *)malloc(sizeof(t_frame));
                    inicializarRegistroListaDeFrames(novoFrame, cadeia, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
                    inserirNaListaDeFrames(novoFrame);

                    if(mostrar)
                     printf("Mostre %s apos tecla por %.2f segundos;\n", cadeia, tempoDecPor);
                    else
                     printf("Apague %s apos tecla por %.2f segundos;\n", cadeia, tempoDecPor);
                    return 1;

                   }else{
                     printf("ERRO! Ponto-e-virgula esperado apos \"...por %.2f segundos\".\n", tempoDecPor);
                     return 0;
                   }
                  }else{
                    printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDecPor);
                    return 0;
                  }

                }else{
                  printf("ERRO! DECIMAL ou INTEIRO esperado apos \"...apos tecla por\".\n");
                  return 0;
                }

               }else{
                if(mostrar)
                 printf("ERRO! Ponto-e-virgula ou 'por' esperado apos \"Mostre \"%s\" apos tecla\".\n", cadeia);
                else
                 printf("ERRO! Ponto-e-virgula ou 'por' esperado apos \"Apague \"%s\" apos tecla\".\n", cadeia);
                return 0;
               }

       }else{
        if(mostrar)
         printf("ERRO! 'tecla', DECIMAL ou INTEIRO esperado apos \"Mostre %s apos\".\n", cadeia);
        else
         printf("ERRO! 'tecla', DECIMAL ou INTEIRO esperado apos \"Apague %s apos\".\n", cadeia);
        return 0;
       }

     }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
       (void) LEXAN_get_token (
         (LEXAN_t_lexJobId)     lxJobId,
         (LEXAN_t_tokenType *) &tok,
         (LEXAN_t_tokenVal *)  &tokVal );
       if(tok == LEXAN_token_resWord)
         codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
       else
         codToken = tipoToken2codigoToken (tok);

       if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
         tempoIntPor = tokVal.longIntVal;
         (void) LEXAN_get_token (
           (LEXAN_t_lexJobId)     lxJobId,
           (LEXAN_t_tokenType *) &tok,
           (LEXAN_t_tokenVal *)  &tokVal );
         if(tok == LEXAN_token_resWord)
           codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
         else
           codToken = tipoToken2codigoToken (tok);

         if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
           (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
             novoFrame = (t_frame *)malloc(sizeof(t_frame));
             inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, 0, 0, mostrar);
             inserirNaListaDeFrames(novoFrame);

             novoFrame = (t_frame *)malloc(sizeof(t_frame));
             inicializarRegistroListaDeFrames(novoFrame, cadeia, 2, 0, tempoIntPor*1000, 0, !mostrar);
             inserirNaListaDeFrames(novoFrame);

             if(mostrar)
              printf("Mostre %s por %d segundos;\n", cadeia, tempoIntPor);
             else
              printf("Apague %s por %d segundos;\n", cadeia, tempoIntPor);
             return 1;
           }else{
             printf("ERRO! Ponto-e-virgula esperado apos \"...por %d segundos\".\n", tempoIntPor);
             return 0;
           }
         }else{
           printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...%s por %d\".\n", cadeia, tempoIntPor);
           return 0;
         }

       }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
         tempoDecPor = tokVal.doubleVal;
         (void) LEXAN_get_token (
           (LEXAN_t_lexJobId)     lxJobId,
           (LEXAN_t_tokenType *) &tok,
           (LEXAN_t_tokenVal *)  &tokVal );
         if(tok == LEXAN_token_resWord)
           codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
         else
           codToken = tipoToken2codigoToken (tok);

         if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
           (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
             novoFrame = (t_frame *)malloc(sizeof(t_frame));
             inicializarRegistroListaDeFrames(novoFrame, cadeia, 0, 1, 0, 0, mostrar);
             inserirNaListaDeFrames(novoFrame);

             novoFrame = (t_frame *)malloc(sizeof(t_frame));
             inicializarRegistroListaDeFrames(novoFrame, cadeia, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
             inserirNaListaDeFrames(novoFrame);

             if(mostrar)
              printf("Mostre %s por %.2f segundos;\n", cadeia, tempoDecPor);
             else
              printf("Apague %s por %.2f segundos;\n", cadeia, tempoDecPor);
             return 1;

           }else{
             printf("ERRO! Ponto-e-virgula esperado apos \"...por %.2f segundos\".\n", tempoDecPor);
             return 0;
           }
         }else{
           printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDecPor);
           return 0;
         }
       }else{
        if(mostrar)
         printf("ERRO! DECIMAL ou INTEIRO esperado apos \"Mostre %s por\".\n", cadeia);
        else
         printf("ERRO! DECIMAL ou INTEIRO esperado apos \"Apague %s por\".\n", cadeia);
        return 0;
       }

     }else{
      if(mostrar)
       printf("ERRO! Ponto-e-virgula, 'apos' ou 'por' esperado apos \"Mostre %s\".\n", cadeia);
      else
       printf("ERRO! Ponto-e-virgula, 'apos' ou 'por' esperado apos \"Apague %s\".\n", cadeia);
      return 0;
     }

   }else{
    if(mostrar)
     printf("ERRO! Identificador, Cadeia ou Expressao Regular esperado apos \"Mostre\".\n");
    else
     printf("ERRO! Identificador, Cadeia ou Expressao Regular esperado apos \"Apague\".\n");
    return 0;
   }
 return 1;
}

int tkUmAUmEncontrado(char *nome, int umAum){
 (void) LEXAN_get_token (
   (LEXAN_t_lexJobId)     lxJobId,
   (LEXAN_t_tokenType *) &tok,
   (LEXAN_t_tokenVal *)  &tokVal );
 if(tok == LEXAN_token_resWord)
   codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
 else
   codToken = tipoToken2codigoToken (tok);

 if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_apos") == 0 ){
   (void) LEXAN_get_token (
     (LEXAN_t_lexJobId)     lxJobId,
     (LEXAN_t_tokenType *) &tok,
     (LEXAN_t_tokenVal *)  &tokVal );
   if(tok == LEXAN_token_resWord)
     codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
   else
     codToken = tipoToken2codigoToken (tok);

   if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
     tempoInt = tokVal.longIntVal;
     (void) LEXAN_get_token (
       (LEXAN_t_lexJobId)     lxJobId,
       (LEXAN_t_tokenType *) &tok,
       (LEXAN_t_tokenVal *)  &tokVal );
     if(tok == LEXAN_token_resWord)
       codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
     else
       codToken = tipoToken2codigoToken (tok);

     if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
       (void) LEXAN_get_token (
         (LEXAN_t_lexJobId)     lxJobId,
         (LEXAN_t_tokenType *) &tok,
         (LEXAN_t_tokenVal *)  &tokVal );
       if(tok == LEXAN_token_resWord)
         codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
       else
         codToken = tipoToken2codigoToken (tok);

       if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){

         if(identificarDefinido(nome)){
           regex_t re;
           t_defIden *aux = listaDefs.p_primeiro;
           while(aux != NULL){
             if(strcmp(aux->iden, nome) == 0){
              if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                t_registroId *regId = listaIds.p_primeiro;
                flagInicio = 1;
                while(regId != NULL){
                  if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                    if(listaIdenUt.p_primeiro==NULL){
                     t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                     inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                     inserirNaListaDeIdenUtilizados(novoIden);
                    }
                    else if (!idenJaFoiUtilizado(regId->lexema)){
                     t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                     inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                     inserirNaListaDeIdenUtilizados(novoIden);
                    }
                    if(umAum){
                     novoFrame = (t_frame *)malloc(sizeof(t_frame));
                     inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, tempoInt*1000, 0, mostrar);
                     inserirNaListaDeFrames(novoFrame);
                    }
                    else{
                     novoFrame = (t_frame *)malloc(sizeof(t_frame));
                     inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 1, flagInicio, tempoInt*1000, 0, mostrar);
                     inserirNaListaDeFrames(novoFrame);
                    }

                    if(flagInicio)
                     flagInicio = 0;

                  }
                  regId = regId->proximo;
                }
              }
             }
             aux = aux->proximo;
           }
           regfree(&re);
         }
         if(mostrar){
          if(umAum)
           printf("Mostre %s umAum apos %d segundos;\n", nome, tempoInt);
          else
           printf("Mostre %s juntos apos %d segundos;\n", nome, tempoInt);
         }
         else{
           if(umAum)
            printf("Apague %s umAum apos %d segundos;\n", nome, tempoInt);
           else
            printf("Apague %s juntos apos %d segundos;\n", nome, tempoInt);
         }
         return 1;

        }else if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
         (void) LEXAN_get_token (
           (LEXAN_t_lexJobId)     lxJobId,
           (LEXAN_t_tokenType *) &tok,
           (LEXAN_t_tokenVal *)  &tokVal );
         if(tok == LEXAN_token_resWord)
           codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
         else
           codToken = tipoToken2codigoToken (tok);

         if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
           tempoIntPor = tokVal.longIntVal;
           (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
             (void) LEXAN_get_token (
               (LEXAN_t_lexJobId)     lxJobId,
               (LEXAN_t_tokenType *) &tok,
               (LEXAN_t_tokenVal *)  &tokVal );
             if(tok == LEXAN_token_resWord)
               codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
             else
               codToken = tipoToken2codigoToken (tok);

             if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
               regex_t re;
               t_defIden *aux;
               if(identificarDefinido(nome)){
                aux = listaDefs.p_primeiro;
                while(aux != NULL){
                  if(strcmp(aux->iden, nome) == 0){
                   if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                     t_registroId *regId = listaIds.p_primeiro;
                     flagInicio = 1;
                     while(regId != NULL){
                       if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                         if(listaIdenUt.p_primeiro==NULL){
                          t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                          inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                          inserirNaListaDeIdenUtilizados(novoIden);
                         }
                         else if (!idenJaFoiUtilizado(regId->lexema)){
                          t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                          inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                          inserirNaListaDeIdenUtilizados(novoIden);
                         }
                         if(umAum){
                          novoFrame = (t_frame *)malloc(sizeof(t_frame));
                          inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, tempoInt*1000, 0, mostrar);
                          inserirNaListaDeFrames(novoFrame);
                         }
                         else{
                          novoFrame = (t_frame *)malloc(sizeof(t_frame));
                          inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 1, flagInicio, tempoInt*1000, 0, mostrar);
                          inserirNaListaDeFrames(novoFrame);
                         }

                         if(flagInicio)
                          flagInicio = 0;

                       }
                       regId = regId->proximo;
                     }
                   }
                  }
                  aux = aux->proximo;
                }
              }
              aux = listaDefs.p_primeiro;
              while(aux != NULL){
                if(strcmp(aux->iden, nome) == 0){
                  if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                    t_registroId *regId = listaIds.p_primeiro;
                    while(regId != NULL){
                      if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                        novoFrame = (t_frame *)malloc(sizeof(t_frame));
                        inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, tempoIntPor*1000, 0, !mostrar);
                        inserirNaListaDeFrames(novoFrame);
                      }
                      regId = regId->proximo;
                    }
                  }
                }
                aux = aux->proximo;
              }
              regfree(&re);

              if(mostrar){
               if(umAum)
                printf("Mostre %s umAum apos %d segundos por %d segundos;\n", nome, tempoInt, tempoIntPor);
               else
                printf("Mostre %s juntos apos %d segundos por %d segundos;\n", nome, tempoInt, tempoIntPor);
              }

              else{
               if(umAum)
                printf("Apague %s umAum apos %d segundos por %d segundos;\n", nome, tempoInt, tempoIntPor);
               else
                printf("Apague %s juntos apos %d segundos por %d segundos;\n", nome, tempoInt, tempoIntPor);
              }
              return 1;

             }else{
               printf("ERRO! Ponto-e-virgula esperado apos \"...por %d segundos\".\n", tempoIntPor);
               return 0;
             }
           }else{
             printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %d\".\n", tempoIntPor);
             return 0;
           }

         }else if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
           tempoDecPor = tokVal.doubleVal;
           (void) LEXAN_get_token (
             (LEXAN_t_lexJobId)     lxJobId,
             (LEXAN_t_tokenType *) &tok,
             (LEXAN_t_tokenVal *)  &tokVal );
           if(tok == LEXAN_token_resWord)
             codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
           else
             codToken = tipoToken2codigoToken (tok);

           if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
             (void) LEXAN_get_token (
               (LEXAN_t_lexJobId)     lxJobId,
               (LEXAN_t_tokenType *) &tok,
               (LEXAN_t_tokenVal *)  &tokVal );
             if(tok == LEXAN_token_resWord)
               codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
             else
               codToken = tipoToken2codigoToken (tok);

             if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
               regex_t re;
               t_defIden *aux;
               if(identificarDefinido(nome)){
                aux = listaDefs.p_primeiro;
                flagInicio = 1;
                while(aux != NULL){
                  if(strcmp(aux->iden, nome) == 0){
                   if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                     t_registroId *regId = listaIds.p_primeiro;
                     while(regId != NULL){
                       if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                         if(listaIdenUt.p_primeiro==NULL){
                          t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                          inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                          inserirNaListaDeIdenUtilizados(novoIden);
                         }
                         else if (!idenJaFoiUtilizado(regId->lexema)){
                          t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                          inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                          inserirNaListaDeIdenUtilizados(novoIden);
                         }
                         if(umAum){
                          novoFrame = (t_frame *)malloc(sizeof(t_frame));
                          inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, tempoInt*1000, 0, mostrar);
                          inserirNaListaDeFrames(novoFrame);
                         }
                         else{
                          novoFrame = (t_frame *)malloc(sizeof(t_frame));
                          inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 1, flagInicio, tempoInt*1000, 0, mostrar);
                          inserirNaListaDeFrames(novoFrame);
                         }

                         if(flagInicio)
                          flagInicio = 0;

                       }
                       regId = regId->proximo;
                     }
                   }
                  }
                  aux = aux->proximo;
                }
              }
              aux = listaDefs.p_primeiro;
               while(aux != NULL){
                 if(strcmp(aux->iden, nome) == 0){
                  if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                    t_registroId *regId = listaIds.p_primeiro;
                    while(regId != NULL){
                      if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                        novoFrame = (t_frame *)malloc(sizeof(t_frame));
                        inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
                        inserirNaListaDeFrames(novoFrame);
                      }
                      regId = regId->proximo;
                   }
                 }
                }
                aux = aux->proximo;
              }
              regfree(&re);

              if(mostrar){
               if(umAum)
                printf("Mostre %s umAum apos %d segundos por %.2f segundos;\n", nome, tempoInt, tempoDecPor);
               else
                printf("Mostre %s juntos apos %d segundos por %.2f segundos;\n", nome, tempoInt, tempoDecPor);
              }
              else{
               if(umAum)
                printf("Apague %s umAum apos %d segundos por %.2f segundos;\n", nome, tempoInt, tempoDecPor);
               else
                printf("Apague %s juntos apos %d segundos por %.2f segundos;\n", nome, tempoInt, tempoDecPor);
              }
              return 1;

             }else{
               printf("ERRO! Ponto-e-virgula esperado apos \"...por %.2f segundos\".\n", tempoDecPor);
               return 0;
             }
           }else{
             printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDecPor);
             return 0;
           }

         }else{
           printf("ERRO! DECIMAL ou INTEIRO esperado apos \"...%d segundos por\".\n", tempoInt);
           return 0;
         }

       }else{
         printf("ERRO! Ponto-e-virgula ou 'por' esperado apos \"...apos %d segundos\".\n", tempoInt);
         return 0;
       }
     }else{
       printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...apos %d\".\n", tempoInt);
       return 0;
     }

   }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
     tempoDec = tokVal.doubleVal;
     (void) LEXAN_get_token (
       (LEXAN_t_lexJobId)     lxJobId,
       (LEXAN_t_tokenType *) &tok,
       (LEXAN_t_tokenVal *)  &tokVal );
     if(tok == LEXAN_token_resWord)
       codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
     else
       codToken = tipoToken2codigoToken (tok);

     if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
       (void) LEXAN_get_token (
         (LEXAN_t_lexJobId)     lxJobId,
         (LEXAN_t_tokenType *) &tok,
         (LEXAN_t_tokenVal *)  &tokVal );
       if(tok == LEXAN_token_resWord)
         codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
       else
         codToken = tipoToken2codigoToken (tok);

       if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
         if(identificarDefinido(nome)){
           regex_t re;
           t_defIden *aux = listaDefs.p_primeiro;
           flagInicio = 1;
           while(aux != NULL){
             if(strcmp(aux->iden, nome) == 0){
              if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                t_registroId *regId = listaIds.p_primeiro;
                while(regId != NULL){
                  if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                    if(listaIdenUt.p_primeiro==NULL){
                     t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                     inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                     inserirNaListaDeIdenUtilizados(novoIden);
                    }
                    else if (!idenJaFoiUtilizado(regId->lexema)){
                     t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                     inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                     inserirNaListaDeIdenUtilizados(novoIden);
                    }

                    if(umAum){
                     novoFrame = (t_frame *)malloc(sizeof(t_frame));
                     inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, (int)(tempoDec*1000), 0, mostrar);
                     inserirNaListaDeFrames(novoFrame);
                    }
                    else{
                     novoFrame = (t_frame *)malloc(sizeof(t_frame));
                     inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 1, flagInicio, (int)(tempoDec*1000), 0, mostrar);
                     inserirNaListaDeFrames(novoFrame);
                    }

                    if(flagInicio)
                     flagInicio = 0;
                  }
                  regId = regId->proximo;
                }
              }
             }
             aux = aux->proximo;
           }
           regfree(&re);
         }
         if(mostrar){
          if(umAum)
           printf("Mostre %s umAum apos %.2f segundos;\n", nome, tempoDec);
          else
           printf("Mostre %s juntos apos %.2f segundos;\n", nome, tempoDec);
         }
         else{
          if(umAum)
           printf("Apague %s umAum apos %.2f segundos;\n", nome, tempoDec);
          else
           printf("Apague %s juntos apos %.2f segundos;\n", nome, tempoDec);
         }
         return 1;

       }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
        (void) LEXAN_get_token (
          (LEXAN_t_lexJobId)     lxJobId,
          (LEXAN_t_tokenType *) &tok,
          (LEXAN_t_tokenVal *)  &tokVal );
        if(tok == LEXAN_token_resWord)
          codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
        else
          codToken = tipoToken2codigoToken (tok);

        if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
          tempoIntPor = tokVal.longIntVal;
          (void) LEXAN_get_token (
            (LEXAN_t_lexJobId)     lxJobId,
            (LEXAN_t_tokenType *) &tok,
            (LEXAN_t_tokenVal *)  &tokVal );
          if(tok == LEXAN_token_resWord)
            codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
          else
            codToken = tipoToken2codigoToken (tok);

          if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
            (void) LEXAN_get_token (
              (LEXAN_t_lexJobId)     lxJobId,
              (LEXAN_t_tokenType *) &tok,
              (LEXAN_t_tokenVal *)  &tokVal );
            if(tok == LEXAN_token_resWord)
              codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
            else
              codToken = tipoToken2codigoToken (tok);

            if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
              regex_t re;
              t_defIden *aux;
              if(identificarDefinido(nome)){
               aux = listaDefs.p_primeiro;
               flagInicio = 1;
               while(aux != NULL){
                 if(strcmp(aux->iden, nome) == 0){
                  if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                    t_registroId *regId = listaIds.p_primeiro;
                    while(regId != NULL){
                      if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                        if(listaIdenUt.p_primeiro==NULL){
                         t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                         inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                         inserirNaListaDeIdenUtilizados(novoIden);
                        }
                        else if (!idenJaFoiUtilizado(regId->lexema)){
                         t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                         inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                         inserirNaListaDeIdenUtilizados(novoIden);
                        }
                        if(umAum){
                         novoFrame = (t_frame *)malloc(sizeof(t_frame));
                         inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, (int)(tempoDec*1000), 0, mostrar);
                         inserirNaListaDeFrames(novoFrame);
                        }
                        else{
                         novoFrame = (t_frame *)malloc(sizeof(t_frame));
                         inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 1, flagInicio, (int)(tempoDec*1000), 0, mostrar);
                         inserirNaListaDeFrames(novoFrame);
                        }

                        if(flagInicio)
                         flagInicio = 0;

                      }
                      regId = regId->proximo;
                    }
                  }
                 }
                 aux = aux->proximo;
               }
             }
             aux = listaDefs.p_primeiro;
               while(aux != NULL){
                 if(strcmp(aux->iden, nome) == 0){
                  if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                    t_registroId *regId = listaIds.p_primeiro;
                    while(regId != NULL){
                      if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                        novoFrame = (t_frame *)malloc(sizeof(t_frame));
                        inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, (tempoIntPor*1000), 0, !mostrar);
                        inserirNaListaDeFrames(novoFrame);
                      }
                      regId = regId->proximo;
                   }
                 }
                }
                aux = aux->proximo;
              }
              regfree(&re);

             if(mostrar){
              if(umAum)
               printf("Mostre %s umAum apos %.2f segundos por %d segundos;\n", nome, tempoDec, tempoIntPor);
              else
               printf("Mostre %s juntos apos %.2f segundos por %d segundos;\n", nome, tempoDec, tempoIntPor);
             }
             else{
              if(umAum)
               printf("Apague %s umAum apos %.2f segundos por %d segundos;\n", nome, tempoDec, tempoIntPor);
              else
               printf("Apague %s juntos apos %.2f segundos por %d segundos;\n", nome, tempoDec, tempoIntPor);
             }
             return 1;

            }else{
              printf("ERRO! Ponto-e-virgula esperado apos \"...por %d segundos\".\n", tempoIntPor);
              return 0;
            }
          }else{
            printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %d\".\n", tempoIntPor);
            return 0;
          }

        }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
          tempoDecPor = tokVal.doubleVal;
          (void) LEXAN_get_token (
            (LEXAN_t_lexJobId)     lxJobId,
            (LEXAN_t_tokenType *) &tok,
            (LEXAN_t_tokenVal *)  &tokVal );
          if(tok == LEXAN_token_resWord)
            codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
          else
            codToken = tipoToken2codigoToken (tok);

          if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
            (void) LEXAN_get_token (
              (LEXAN_t_lexJobId)     lxJobId,
              (LEXAN_t_tokenType *) &tok,
              (LEXAN_t_tokenVal *)  &tokVal );
            if(tok == LEXAN_token_resWord)
              codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
            else
              codToken = tipoToken2codigoToken (tok);

            if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
              regex_t re;
              t_defIden *aux;
              if(identificarDefinido(nome)){
               aux = listaDefs.p_primeiro;
               flagInicio = 1;
               while(aux != NULL){
                 if(strcmp(aux->iden, nome) == 0){
                  if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                    t_registroId *regId = listaIds.p_primeiro;
                    while(regId != NULL){
                      if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                        if(listaIdenUt.p_primeiro==NULL){
                         t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                         inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                         inserirNaListaDeIdenUtilizados(novoIden);
                        }
                        else if (!idenJaFoiUtilizado(regId->lexema)){
                         t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                         inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                         inserirNaListaDeIdenUtilizados(novoIden);
                        }
                        if(umAum){
                         novoFrame = (t_frame *)malloc(sizeof(t_frame));
                         inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, (int)(tempoDec*1000), 0, mostrar);
                         inserirNaListaDeFrames(novoFrame);
                        }
                        else{
                         novoFrame = (t_frame *)malloc(sizeof(t_frame));
                         inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 1, flagInicio, (int)(tempoDec*1000), 0, mostrar);
                         inserirNaListaDeFrames(novoFrame);
                        }

                        if(flagInicio)
                         flagInicio = 0;

                      }
                      regId = regId->proximo;
                    }
                  }
                 }
                 aux = aux->proximo;
               }
             }
             aux = listaDefs.p_primeiro;
               while(aux != NULL){
                 if(strcmp(aux->iden, nome) == 0){
                  if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                    t_registroId *regId = listaIds.p_primeiro;
                    while(regId != NULL){
                      if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                        novoFrame = (t_frame *)malloc(sizeof(t_frame));
                        inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
                        inserirNaListaDeFrames(novoFrame);
                      }
                      regId = regId->proximo;
                   }
                 }
                }
                aux = aux->proximo;
              }
              regfree(&re);

             if(mostrar){
              if(umAum)
               printf("Mostre %s umAum apos %.2f segundos por %.2f segundos;\n", nome, tempoDec, tempoDecPor);
              else
               printf("Mostre %s juntos apos %.2f segundos por %.2f segundos;\n", nome, tempoDec, tempoDecPor);
             }
             else{
              if(umAum)
               printf("Apague %s umAum apos %.2f segundos por %.2f segundos;\n", nome, tempoDec, tempoDecPor);
              else
               printf("Apague %s juntos apos %.2f segundos por %.2f segundos;\n", nome, tempoDec, tempoDecPor);
             }

            }else{
              printf("ERRO! Ponto-e-virgula esperado apos \"...por %.2f segundos\".\n", tempoDecPor);
              return 0;
            }
          }else{
            printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDec);
            return 0;
          }

        }else{
          printf("ERRO! DECIMAL ou INTEIRO esperado apos \"...%.2f segundos por\".\n", tempoDec);
          return 0;
        }

       }else{
         printf("ERRO! Ponto-e-virgula ou 'por' esperado apos \"...apos %.2f segundos\".\n", tempoDec);
         return 0;
       }
     }else{
       printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...apos %.2f\".\n", tempoDec);
     }

   }else{
     if(mostrar)
      printf("ERRO! DECIMAL ou INTEIRO esperado apos \"Mostre\".\n");
     else
      printf("ERRO! DECIMAL ou INTEIRO esperado apos \"Apague\".\n");

     return 0;
   }

 }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_por") == 0 ){
   (void) LEXAN_get_token (
     (LEXAN_t_lexJobId)     lxJobId,
     (LEXAN_t_tokenType *) &tok,
     (LEXAN_t_tokenVal *)  &tokVal );
   if(tok == LEXAN_token_resWord)
     codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
   else
     codToken = tipoToken2codigoToken (tok);

   if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_INTEIRO") == 0 ){
     tempoIntPor = tokVal.longIntVal;
     (void) LEXAN_get_token (
       (LEXAN_t_lexJobId)     lxJobId,
       (LEXAN_t_tokenType *) &tok,
       (LEXAN_t_tokenVal *)  &tokVal );
     if(tok == LEXAN_token_resWord)
       codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
     else
       codToken = tipoToken2codigoToken (tok);

     if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
       (void) LEXAN_get_token (
         (LEXAN_t_lexJobId)     lxJobId,
         (LEXAN_t_tokenType *) &tok,
         (LEXAN_t_tokenVal *)  &tokVal );
       if(tok == LEXAN_token_resWord)
         codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
       else
         codToken = tipoToken2codigoToken (tok);

       if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
         regex_t re;
         t_defIden *aux;
         if(identificarDefinido(nome)){
           aux = listaDefs.p_primeiro;
           flagInicio = 1;
           while(aux != NULL){
             if(strcmp(aux->iden, nome) == 0){
              if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                t_registroId *regId = listaIds.p_primeiro;
                while(regId != NULL){
                  if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                    if(listaIdenUt.p_primeiro==NULL){
                     t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                     inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                     inserirNaListaDeIdenUtilizados(novoIden);
                    }
                    else if (!idenJaFoiUtilizado(regId->lexema)){
                     t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                     inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                     inserirNaListaDeIdenUtilizados(novoIden);
                    }
                    if(umAum){
                     novoFrame = (t_frame *)malloc(sizeof(t_frame));
                     inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, 0, 0, mostrar);
                     inserirNaListaDeFrames(novoFrame);
                    }
                    else{
                     novoFrame = (t_frame *)malloc(sizeof(t_frame));
                     inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 1, flagInicio, 0, 0, mostrar);
                     inserirNaListaDeFrames(novoFrame);
                    }
                    if(flagInicio)
                     flagInicio = 0;

                  }
                  regId = regId->proximo;
                }
              }
             }
             aux = aux->proximo;
           }
         }
         aux = listaDefs.p_primeiro;
          while(aux != NULL){
            if(strcmp(aux->iden, nome) == 0){
             if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
               t_registroId *regId = listaIds.p_primeiro;
               while(regId != NULL){
                 if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                   novoFrame = (t_frame *)malloc(sizeof(t_frame));
                   inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, tempoIntPor*1000, 0, !mostrar);
                   inserirNaListaDeFrames(novoFrame);
                 }
                 regId = regId->proximo;
              }
            }
           }
           aux = aux->proximo;
         }
         regfree(&re);

         if(mostrar){
          if(umAum)
           printf("Mostre %s umAum por %d segundos;\n", nome, tempoIntPor);
          else
           printf("Mostre %s juntos por %d segundos;\n", nome, tempoIntPor);
         }
         else{
          if(umAum)
           printf("Apague %s umAum por %d segundos;\n", nome, tempoIntPor);
          else
           printf("Apague %s juntos por %d segundos;\n", nome, tempoIntPor);
         }

         return 1;

       }else{
         printf("ERRO! Ponto-e-virgula esperado apos \"...por %d segundos\".\n", tempoIntPor);
         return 0;
       }
     }else{
       printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %d\".\n", tempoIntPor);
       return 0;
     }

   }else if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_DECIMAL") == 0 ){
     tempoDecPor = tokVal.doubleVal;
     (void) LEXAN_get_token (
       (LEXAN_t_lexJobId)     lxJobId,
       (LEXAN_t_tokenType *) &tok,
       (LEXAN_t_tokenVal *)  &tokVal );
     if(tok == LEXAN_token_resWord)
       codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
     else
       codToken = tipoToken2codigoToken (tok);

     if((strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundos") == 0 ) || (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_segundo") == 0 )){
       (void) LEXAN_get_token (
         (LEXAN_t_lexJobId)     lxJobId,
         (LEXAN_t_tokenType *) &tok,
         (LEXAN_t_tokenVal *)  &tokVal );
       if(tok == LEXAN_token_resWord)
         codToken = cadeiaToken2codigoToken (tokVal.tokenStr, caixa_Comandos == t_caixa_distinguem);
       else
         codToken = tipoToken2codigoToken (tok);

       if(strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
         regex_t re;
         t_defIden *aux;
         if(identificarDefinido(nome)){
           aux = listaDefs.p_primeiro;
           flagInicio = 1;
           while(aux != NULL){
             if(strcmp(aux->iden, nome) == 0){
              if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
                t_registroId *regId = listaIds.p_primeiro;
                while(regId != NULL){
                  if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                    if(listaIdenUt.p_primeiro==NULL){
                     t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                     inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                     inserirNaListaDeIdenUtilizados(novoIden);
                    }
                    else if (!idenJaFoiUtilizado(regId->lexema)){
                     t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
                     inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
                     inserirNaListaDeIdenUtilizados(novoIden);
                    }

                    if(umAum){
                     novoFrame = (t_frame *)malloc(sizeof(t_frame));
                     inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, 0, 0, mostrar);
                     inserirNaListaDeFrames(novoFrame);
                    }
                    else{
                     novoFrame = (t_frame *)malloc(sizeof(t_frame));
                     inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 1, flagInicio, 0, 0, mostrar);
                     inserirNaListaDeFrames(novoFrame);
                    }

                    if(flagInicio)
                     flagInicio = 0;

                  }
                  regId = regId->proximo;
                }
              }
             }
             aux = aux->proximo;
           }
          }
          aux = listaDefs.p_primeiro;
          while(aux != NULL){
            if(strcmp(aux->iden, nome) == 0){
             if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
               t_registroId *regId = listaIds.p_primeiro;
               while(regId != NULL){
                 if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
                   novoFrame = (t_frame *)malloc(sizeof(t_frame));
                   inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 2, 0, (int)(tempoDecPor*1000), 0, !mostrar);
                   inserirNaListaDeFrames(novoFrame);
                 }
                 regId = regId->proximo;
              }
            }
           }
           aux = aux->proximo;
         }
         regfree(&re);

         if(mostrar){
          if(umAum)
           printf("Mostre %s umAum por %.2f segundos;\n", nome, tempoDecPor);
          else
           printf("Mostre %s juntos por %.2f segundos;\n", nome, tempoDecPor);
         }
         else{
          if(umAum)
           printf("Apague %s umAum por %.2f segundos;\n", nome, tempoDecPor);
          else
           printf("Apague %s juntos por %.2f segundos;\n", nome, tempoDecPor);
         }

         return 1;

       }else{
         printf("ERRO! Ponto-e-virgula esperado apos \"...por %.2f segundos\".\n", tempoDecPor);
         return 0;
       }
     }else{
       printf("ERRO! 'segundo' ou 'segundos' esperado apos \"...por %.2f\".\n", tempoDecPor);
       return 0;
     }

   }else{
    if(umAum)
     printf("ERRO! DECIMAL ou INTEIRO esperado apos \"...%s umAum por\".\n", nome);
    else
     printf("ERRO! DECIMAL ou INTEIRO esperado apos \"...%s juntos por\".\n", nome);
    return 0;
   }
 }else if (strcmp(listaTokens[codToken - 1].rotuloToken, "tk_pontoVirgula") == 0 ){
   if(identificarDefinido(nome)){
    regex_t re;
    t_defIden *aux = listaDefs.p_primeiro;
    while(aux != NULL){
      if(strcmp(aux->iden, nome) == 0){
       if(!regcomp(&re, aux->cadeia, REG_EXTENDED)){
         t_registroId *regId = listaIds.p_primeiro;
         flagInicio =1;
         while(regId != NULL){
           if(regexec(&re, regId->lexema, 0, NULL, 0) == 0){
             if(listaIdenUt.p_primeiro==NULL){
              t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
              inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
              inserirNaListaDeIdenUtilizados(novoIden);
             }
             else if (!idenJaFoiUtilizado(regId->lexema)){
              t_idenUtilizado *novoIden = (t_idenUtilizado *)malloc(sizeof(t_idenUtilizado));
              inicializarRegistroListaIdenUtilizados(novoIden, regId->lexema, mostrar);
              inserirNaListaDeIdenUtilizados(novoIden);
             }
             if(umAum){
              novoFrame = (t_frame *)malloc(sizeof(t_frame));
              inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 0, flagInicio, 0, 0, mostrar);
              inserirNaListaDeFrames(novoFrame);
             }else{
              novoFrame = (t_frame *)malloc(sizeof(t_frame));
              inicializarRegistroListaDeFrames(novoFrame, regId->lexema, 1, flagInicio, 0, 0, mostrar);
              inserirNaListaDeFrames(novoFrame);
             }

             if(flagInicio)
              flagInicio = 0;
           }
           regId = regId->proximo;
         }
       }
      }
      aux = aux->proximo;
    }
    regfree(&re);
  }
  if(mostrar){
   if(umAum)
    printf("Mostre %s umAum;\n", nome);
   else
    printf("Mostre %s juntos;\n", nome);
  }
  else{
   if(umAum)
    printf("Apague %s umAum;\n", nome);
   else
    printf("Apague %s juntos;\n", nome);
  }
  return 1;

 }else{
  if(mostrar){
   if(umAum)
    printf("ERRO! Ponto-e-virgula 'apos' ou 'por' esperado apos \"Mostre %s umAum\".\n", nome);
   else
    printf("ERRO! Ponto-e-virgula ou 'por' esperado apos \"Mostre %s juntos\".\n", nome);
  }
  else{
   if(umAum)
    printf("ERRO! Ponto-e-virgula 'apos' ou 'por' esperado apos \"Apague %s umAum\".\n", nome);
   else
    printf("ERRO! Ponto-e-virgula ou 'por' esperado apos \"Apague %s juntos\".\n", nome);
  }
  return 0;
 }
 return 1;
}

void escreveSaidaHtml(const char *nomeDoArquivoFonte){
 int i=0,
     j;
 int nomes[1000]={0}; /*1=name; 2=null*/
 char *anima = NULL,
      *arrayDeFrames = NULL;
 t_frame *aux = listaFrames.p_primeiro;
 t_idenUtilizado *utils = listaIdenUt.p_primeiro;

 char *nomeDoArquivo = (char *) malloc(6 + strlen(nomeDoArquivoFonte));
 sprintf(nomeDoArquivo,"%s.html", nomeDoArquivoFonte);

 fonteSVG[strlen(fonteSVG)-1]='\n';

 escreveParaArquivo(nomeDoArquivo, htmlHeader[0]);
 for(j=1;j<46;j++)
  concatenaEmArquivo(nomeDoArquivo, htmlHeader[j]);

 while(utils != NULL){
  concatenaEmArquivo(nomeDoArquivo, "\n\t\t\ts.select(\"#\"+");
  concatenaEmArquivo(nomeDoArquivo, utils->iden);
  if(utils->acaoInicial){
   concatenaEmArquivo(nomeDoArquivo, ").animate({opacity: 0}, 100);\n");
  }
  else
   concatenaEmArquivo(nomeDoArquivo, ").animate({opacity: 1}, 100);\n");
  utils = utils->proximo;
 }

 while(aux != NULL){
  if(aux->proximo!=NULL && ((aux->tempoApos != 0 && aux->flagJuntos==0) || (aux->tempoApos!=0 && aux->flagJuntos==1 && aux->flagInicio))){
    anima = (char *) malloc(100);
    sprintf(anima, "\n\t\t\tvar anim%d = function(){\n\t\t\t\ttimeoutAnim(%d, anim%d);\n\t\t\t\tfuncIdex++;\n\t\t\t}", i, aux->tempoApos, i+1);
    concatenaEmArquivo(nomeDoArquivo, anima);
    nomes[i] = 2;
    i++;
    free(anima);
  }
  anima =  (char *) malloc(200);

  if(aux->proximo != NULL && ((aux->flagJuntos && (!aux->proximo->flagInicio && aux->proximo->flagJuntos)) || (aux->proximo->tempoApos!=0 && aux->proximo->flagInicio))){
   if(aux->acao)
    sprintf(anima,"\n\t\t\tvar anim%d = function(){\n\t\t\t\tvar name = %s;\n\t\t\t\tvar next = anim%d;\n\t\t\t\tvar time = 0;\n\t\t\t\tshowElem(name, time, next);\n\t\t\t}", i, aux->nome, i+1);
   else
    sprintf(anima,"\n\t\t\tvar anim%d = function(){\n\t\t\t\tvar name = %s;\n\t\t\t\tvar next = anim%d;\n\t\t\t\tvar time = 0;\n\t\t\t\thideElem(name, time, next);\n\t\t\t}", i, aux->nome, i+1);

  }else if(aux->proximo != NULL && (aux->proximo->tempoApos != 0 && aux->flagJuntos==0)){
   if(aux->acao)
    sprintf(anima,"\n\t\t\tvar anim%d = function(){\n\t\t\t\tvar name = %s;\n\t\t\t\tvar next = anim%d;\n\t\t\t\tvar time = 200;\n\t\t\t\tshowElem(name, time, next);\n\t\t\t}", i, aux->nome, i+1);
   else
    sprintf(anima,"\n\t\t\tvar anim%d = function(){\n\t\t\t\tvar name = %s;\n\t\t\t\tvar next = anim%d;\n\t\t\t\tvar time = 200;\n\t\t\t\thideElem(name, time, next);\n\t\t\t}", i, aux->nome, i+1);
  }else if(aux->flagJuntos){
   if(aux->acao)
    sprintf(anima,"\n\t\t\tvar anim%d = function(){\n\t\t\t\tvar name = %s;\n\t\t\t\tvar next = null;\n\t\t\t\tvar time = 0;\n\t\t\t\tshowElem(name, time, next);\n\t\t\t}", i, aux->nome);
   else
    sprintf(anima,"\n\t\t\tvar anim%d = function(){\n\t\t\t\tvar name = %s;\n\t\t\t\tvar next = null;\n\t\t\t\tvar time = 0;\n\t\t\t\thideElem(name, time, next);\n\t\t\t}", i, aux->nome);
  }else{
   if(aux->acao)
     sprintf(anima,"\n\t\t\tvar anim%d = function(){\n\t\t\t\tvar name = %s;\n\t\t\t\tvar next = null;\n\t\t\t\tvar time = 200;\n\t\t\t\tshowElem(name, time, next);\n\t\t\t}", i, aux->nome);
    else
     sprintf(anima,"\n\t\t\tvar anim%d = function(){\n\t\t\t\tvar name = %s;\n\t\t\t\tvar next = null;\n\t\t\t\tvar time = 200;\n\t\t\t\thideElem(name, time, next);\n\t\t\t}", i, aux->nome);
  }
  concatenaEmArquivo(nomeDoArquivo, anima);
  nomes[i] = 1;
  i++;

  free(anima);
  if(aux->proximo != NULL && (aux->flagJuntos!=2 && aux->proximo->flagJuntos==2)){
   anima =  (char *) malloc(100);
   sprintf(anima, "\n\t\t\tvar anim%d = function(){\n\t\t\t\ttimeoutAnim(%d, anim%d);\n\t\t\t\tfuncIdex++;\n\t\t\t}", i, aux->proximo->tempoApos, i+1);
   concatenaEmArquivo(nomeDoArquivo, anima);
   nomes[i] = 2;
   i++;
   free(anima);
  }
  aux = aux->proximo;
 }
 j=0;
 concatenaEmArquivo(nomeDoArquivo, "\n\t\t\tvar funcs = [");
 while(j<i-1){
  arrayDeFrames =  (char *) malloc(20);
  sprintf(arrayDeFrames, "anim%d, ", j);
  concatenaEmArquivo(nomeDoArquivo, arrayDeFrames);
  free(arrayDeFrames);
  j++;
 }
 arrayDeFrames =  (char *) malloc(20);
 sprintf(arrayDeFrames, "anim%d", j);
 concatenaEmArquivo(nomeDoArquivo, arrayDeFrames);
 concatenaEmArquivo(nomeDoArquivo, "];");

 concatenaEmArquivo(nomeDoArquivo, "\n\t\t\tvar names = [");
 j=0;
 aux = listaFrames.p_primeiro;
 while(nomes[j] != 0){
  if(nomes[j]==1){
   concatenaEmArquivo(nomeDoArquivo, aux->nome);
   if(aux->proximo != NULL)
    aux = aux->proximo;
  }
  else concatenaEmArquivo(nomeDoArquivo, "null");
  if(nomes[j+1] != 0){
   concatenaEmArquivo(nomeDoArquivo, ",");
  }
  j++;
 }
 concatenaEmArquivo(nomeDoArquivo, "];");

 concatenaEmArquivo(nomeDoArquivo, "\n\t\t\tvar inicios = [");
 j=0;
 aux = listaFrames.p_primeiro;
 while(nomes[j] != 0){
  if(nomes[j]==1){
   if((aux->flagInicio && aux->flagJuntos==1)||aux->flagInicio)
    concatenaEmArquivo(nomeDoArquivo, "1");
   else
    concatenaEmArquivo(nomeDoArquivo, "0");
   if(aux->proximo != NULL)
    aux = aux->proximo;
  }
  else concatenaEmArquivo(nomeDoArquivo, "0");
  if(nomes[j+1] != 0){
   concatenaEmArquivo(nomeDoArquivo, ",");
  }
  j++;
 }
 concatenaEmArquivo(nomeDoArquivo, "];");

 arrayDeFrames =  (char *) malloc(35);
 sprintf(arrayDeFrames, "\n\t\t\tvar ultimaAnim = %d;\n", i-1);

 concatenaEmArquivo(nomeDoArquivo, arrayDeFrames);

 for(j=0; j<20; j++)
  concatenaEmArquivo(nomeDoArquivo, htmlBody[j]);

 concatenaEmArquivo(nomeDoArquivo, fonteSVG);
 concatenaEmArquivo(nomeDoArquivo, htmlFooter);

 free(arrayDeFrames);
 free(nomeDoArquivo);
}

int identificarDefinido(char *iden){
  t_defIden *aux = listaDefs.p_primeiro;
  while(aux->proximo != NULL){
   if(strcmp(aux->iden, iden) == 0 )
    return 1;
   aux = aux->proximo;
  }
  if(strcmp(aux->iden, iden) == 0 )
   return 1;
  return 0;
}

int idenJaFoiUtilizado(char* iden){
  t_idenUtilizado *aux = listaIdenUt.p_primeiro;
  while(aux->proximo != NULL){
   if(strcmp(aux->iden, iden) == 0)
    return 1;
   aux = aux->proximo;
  }
  if(strcmp(aux->iden, iden) == 0 )
   return 1;
  return 0;
}

int buscarNaListaDeIds(char* iden){
 t_registroId *aux = listaIds.p_primeiro;
 while(aux->proximo != NULL){
  if(strcmp(aux->lexema, iden) == 0)
   return 1;
  aux = aux->proximo;
 }
 if(strcmp(aux->lexema, iden) == 0 )
   return 1;
  return 0;
}

void escreveParaArquivo(char *nomeDoArquivo, char *conteudo){
  FILE *f = fopen(nomeDoArquivo, "w");
  if (f == NULL)
  {
    printf("Erro na abertura do arquivo!\n");
    return;
  }

  fprintf(f, "%s", conteudo);
  fclose(f);
}


void concatenaEmArquivo(char *nomeDoArquivo, char *conteudo){
  FILE *f = fopen(nomeDoArquivo, "a");
  if (f == NULL)
  {
    printf("Erro na abertura do arquivo!\n");
    return;
  }

  fprintf(f, "%s", conteudo);
  fclose(f);
}


/*
*--------------------------------------------------------------------------------
* Fecha todos os arquivos e libera toda memória dinãmica previamente alocada
*--------------------------------------------------------------------------------
*/

void prepara_para_sair (void) {
 int
   iComando;

 if (arq_Entrada_Ortografia_Ptr)
   fclose (arq_Entrada_Ortografia_Ptr);
 if (arq_Saida_Meta_Tokens_Ptr)
   fclose (arq_Saida_Meta_Tokens_Ptr);
 if (arq_Saida_Meta_Erros_Ptr)
   fclose (arq_Saida_Meta_Erros_Ptr);
 if (arq_Entrada_Programa_Ptr)
   fclose (arq_Entrada_Programa_Ptr);
 if (arq_Saida_Erros_Ptr)
   fclose (arq_Saida_Erros_Ptr);
 if (arq_Saida_Tokens_Ptr)
   fclose (arq_Saida_Tokens_Ptr);
 if (arq_Saida_TabSimb_Ptr)
   fclose (arq_Saida_TabSimb_Ptr);

 /* Libera strings alocados dinamicamente */

 STRING_release (&lidas_regex_comentario_linha);
 STRING_release (&lidas_regex_comentario_abre);
 STRING_release (&lidas_regex_comentario_fecha);
 STRING_release (&lidas_regex_lexema_identificador);
 STRING_release (&lidas_regex_lexema_inteiro);
 STRING_release (&lidas_regex_lexema_decimal);
 STRING_release (&lidas_regex_lexema_string);
 STRING_release (&lidas_regex_TOKEN_double);
 STRING_release (&lidas_regex_TOKEN_resWord);
 STRING_release (&lidas_rotulo_lexema_identificador);
 STRING_release (&lidas_rotulo_lexema_inteiro);
 STRING_release (&lidas_rotulo_lexema_decimal);
 STRING_release (&lidas_rotulo_lexema_string);

 for (iComando = 0; iComando < NRO_MAX_COMANDOS; iComando++) {
   STRING_release (&lidas_cadeia_comando[iComando]);
   STRING_release (&lidas_regex_comando [iComando]);
   STRING_release (&lidas_rotulo_comando[iComando]);
 }

 destroi_tabSimb();
 destroi_listaIds();
 destroi_listaFrames();
 destroi_listaDefs();
 destroi_listaIdsUtils();
 free (p_tokenSpecs);
}

/*
*---------------------------------------------------------------------
* Faz a leitura do fonte SVG pegando os Ids
*---------------------------------------------------------------------
*/

void buscaIdsSVG(const char *fileSVG)
{
  int retorno = 0;
  int cursor = 0;

  estado estadosId[6] = {{0, 0},
	                        {0, 0},
	                        {0, 0},
	                        {0, 0},
	                        {0, 1},
	                        {1, 1}
	                       };

  int tabelaTransicaoId[6][12] = {{0,0,0,0,0,1,0,0,0,0,0,0},
                                  {0,0,0,0,0,0,2,0,0,0,0,0},
                                  {0,0,0,0,0,0,0,3,2,2,0,0},
                                  {0,0,0,0,0,0,0,0,3,3,4,0},
                                  {4,4,4,0,0,4,4,4,4,4,5,4},
                                  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
                                  };

  fonteSVG = lerArquivo(fileSVG);

  while( retorno != -1 ){
    retorno = pegaId(cursor, tabelaTransicaoId, estadosId);
    cursor=retorno;
  }

  retorno = 0;
  cursor = 0;
}

int pegaId(int cursor, int tabelaTransicaoId[6][12], estado estadosId[6]){

  int estadoAtualIdx = 0;
  int contLexema=0;
  char proximoChar;
  char *lexema;
  t_registroId *novoRegListId;
  e_grupoTransicaoSVG trans;

  while (!estadosId[estadoAtualIdx].estadoFinal){

    if (estadoAtualIdx == 0)
      contLexema=0;

    proximoChar = pegarProximoChar(cursor);
    trans = defineGrupoTransicao(proximoChar);
    if(trans != 12){
	    fazTransicao(&estadoAtualIdx, tabelaTransicaoId, trans);
    }
	  else{
			return -1;
	  }
	  cursor++;

   if (estadosId[estadoAtualIdx].contaCaracter)
      contLexema++;
	}

  lexema = (char *) malloc (contLexema+1);

  memcpy(lexema, &fonteSVG[cursor-contLexema], contLexema+1);

  lexema[contLexema] = '\0';

  novoRegListId = (t_registroId *)malloc(sizeof(t_registroId));
  inicializarRegistroListaDeIds(novoRegListId, lexema);
  inserirNaListaDeIds(novoRegListId);
  /*printf("%s \n", lexema);*/

  free(lexema);

  return cursor;
}

void inicializarListaDeIds(void){
  listaIds.p_primeiro = NULL;
  listaIds.p_ultimo = NULL;
}
void inicializarListaDeFrames(void){
  listaFrames.p_primeiro = NULL;
  listaFrames.p_ultimo = NULL;

  listaFrames.quantidadeDeFrames = 0;
}
void inicializarListaDeIdentificadores(void){
  listaDefs.p_primeiro = NULL;
  listaDefs.p_ultimo = NULL;
}
void inicializarListaIdenUtilizados(void){
 listaIdenUt.p_primeiro = NULL;
 listaIdenUt.p_ultimo = NULL;
}

void inicializarRegistroListaDeIds(t_registroId *registro, char *lexema){
  char *lex = (char *) malloc(strlen(lexema)+1);
  strcpy(lex, lexema);

  registro->lexema = lex;
  registro->proximo = NULL;
}
void inicializarRegistroListaDeFrames(t_frame *registro, char *nome, int flagJuntos, int flagIni, int tempoApos, int tempoPor, int acao){
  char *strNome = (char *) malloc(strlen(nome)+1);
  strcpy(strNome, nome);

  registro->nome = strNome;
  registro->flagJuntos = flagJuntos;
  registro->flagInicio = flagIni;
  registro->tempoApos = tempoApos;
  registro->tempoPor = tempoPor;
  registro->acao = acao;
  registro->proximo = NULL;
}
void inicializarRegistroListaDeIdentificadores(t_defIden *registro, char *iden, char *cadeia){
  char *strIden = (char *) malloc(strlen(iden)+1);
  char *strCadeia = (char *) malloc(strlen(cadeia));
  strcpy(strIden, iden);
  strcpy(strCadeia, cadeia);

  registro->iden = strIden;
  registro->cadeia = strCadeia;
  registro->proximo = NULL;
}
void inicializarRegistroListaIdenUtilizados(t_idenUtilizado *registro, char *iden, int acaoInicial){
 char *strIden = (char *) malloc(strlen(iden)+1);
 strcpy(strIden, iden);

 registro->iden = strIden;
 registro->acaoInicial = acaoInicial;
 registro->proximo = NULL;
}

void inserirNaListaDeIds(t_registroId *novoRegistro){
  if(listaIds.p_primeiro == NULL && listaIds.p_ultimo == NULL){
    listaIds.p_primeiro =  novoRegistro;
    listaIds.p_ultimo =  novoRegistro;
  }

  else {
    listaIds.p_ultimo->proximo = novoRegistro;
    listaIds.p_ultimo = novoRegistro;
  }
}
void inserirNaListaDeFrames(t_frame *novoRegistro){
  if(listaFrames.p_primeiro == NULL && listaFrames.p_ultimo == NULL){
    listaFrames.p_primeiro = novoRegistro;
    listaFrames.p_ultimo = novoRegistro;
  }

  else{
    listaFrames.p_ultimo->proximo = novoRegistro;
    listaFrames.p_ultimo = novoRegistro;
  }
  listaFrames.quantidadeDeFrames++;
}
void inserirNaListaDeIdentificadores(t_defIden *novoRegistro){
  if(listaDefs.p_primeiro == NULL && listaDefs.p_ultimo == NULL){
    listaDefs.p_primeiro = novoRegistro;
    listaDefs.p_ultimo = novoRegistro;
  }

  else{
    listaDefs.p_ultimo->proximo = novoRegistro;
    listaDefs.p_ultimo = novoRegistro;
  }
}
void inserirNaListaDeIdenUtilizados(t_idenUtilizado *novoRegistro){
  if(listaIdenUt.p_primeiro == NULL && listaIdenUt.p_ultimo == NULL){
   listaIdenUt.p_primeiro = novoRegistro;
   listaIdenUt.p_ultimo = novoRegistro;
  }

  else{
   listaIdenUt.p_ultimo->proximo = novoRegistro;
   listaIdenUt.p_ultimo = novoRegistro;
  }
}

void fazTransicao(int *estadoAtualIdx, int tabelaTransicaoId[6][12], e_grupoTransicaoSVG transicao){
  if(tabelaTransicaoId[*estadoAtualIdx][transicao] != -1)
    *estadoAtualIdx = tabelaTransicaoId[*estadoAtualIdx][transicao];
}

e_grupoTransicaoSVG defineGrupoTransicao(char c){
  switch (c){
    case '<':
      return menorQue;
    case '>':
      return maiorQue;
    case '=':
      return igual;
    case '_':
      return underline;
    case '\n':
      return quebraDeLinha;
    case EOF:
      return fimDeArquivo;
    case '"':
      return aspas;
    case 'i':
    	return letra_i;
    case 'd':
    	return letra_d;
    default:
      if (isspace(c))
        return branco;
      else if (isdigit(c))
        return digito;
      else if (isalpha(c))
        return letra;
      else return outros;
  }
}

char pegarProximoChar(int cursor){
  return fonteSVG[cursor];
}

char* lerArquivo(const char * nomeDoArquivo){
  char *conteudo;
  long tamanhoDaEntrada;
  FILE *arquivo = fopen(nomeDoArquivo, "rb");
  if(arquivo == NULL){
    printf("\n\nERRO : %s nao encontrado.\n\n", nomeDoArquivo);
    return NULL;
  }
  fseek(arquivo, 0, SEEK_END);
  tamanhoDaEntrada = ftell(arquivo);
  rewind(arquivo);
  conteudo = malloc(tamanhoDaEntrada * (sizeof(char)));
  if(fread(conteudo, sizeof(char), tamanhoDaEntrada, arquivo) == 0){
    printf("Arquivo vazio.");
  }
  fclose(arquivo);
  conteudo[tamanhoDaEntrada] = EOF;

  return conteudo;
}


/*
*---------------------------------------------------------------------
* Main
*---------------------------------------------------------------------
*/

int main (int argc, char *argv[])
{
 Bool
   metaTokensOK;

 inicializarListaDeIds();
 inicializarListaDeFrames();
 inicializarListaDeIdentificadores();
 inicializarListaIdenUtilizados();

 processa_linhaDeComando (argc, argv);
 prepara_arquivos_saida();
 metaTokensOK = processa_definicoesDeTokens();

 if (metaTokensOK){
   processa_fonteEmLidas();
   buscaIdsSVG(argv[5]);
   if(faz_analiseSintatica())
    escreveSaidaHtml(argv[5]);
 }
 prepara_para_sair();
 return (EXIT_SUCCESS);
}
