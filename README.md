# geraLR
Geração e animação automáticas de autômatos finitos

# Resumo
Diagramas, imagens e animações são instrumentos didáticos poderosos, especialmente em programas curriculares com muitos conteúdos envolvendo processos de natureza variada. Em cursos de computação há muitos exemplos: construção de estruturas de dados, funcionamento de máquinas de estados, realização de análise léxica e sintática, processamento de datagramas em redes de computadores, etc.  Uma ferramenta didática valiosa nas disciplinas de Teoria da Computação e Compiladores seria um programa capaz de automatizar a construção e exibição de autômatos a partir de uma gramática livre de contexto. Esse é o objeto deste projeto. Nossa ferramenta permitirá controlar aspectos visuais dos autômatos e exigirá o mínimo de esforço de desenvolvimento, na medida em que reutilizará outros programas de código aberto. A funcionalidade proposta será construída em etapas:  construção de autômatos; sua representação como grafos e exportação como imagens vetoriais; animação das imagens. Ao final, será aferido o valor didático da ferramenta nas disciplinas citadas.

# Problema
Uma tarefa recorrente nas disciplinas de Teoria da Computação e Compiladores é a construção de autômatos finitos determinísticos e não determinísticos a partir de gramáticas livres de contexto. Por ser um componente do conteúdo programático de ambas as disciplinas, tal construção é introduzida passo-a-passo, de maneira didática, a fim de demonstrar as conexões entre os vários conceitos e métodos envolvidos. Logo em sequência, essa construção de autômatos volta a ser empregada como recurso didático, em exemplos cada vez mais complexos, e de avaliação, em questões de provas. Vê-se, assim, o valor de se automatizar todo o processo de construção e exibição dos autômatos a partir de uma gramática, com a vantagem adicional de oferecer uma ferramenta didática para uso extraclasse pelos alunos.
Apesar da construção desses autômatos ser tarefa relativamente simples, sua automação é melhor realizada em etapas: 1) construção, a partir da gramática, dos autômatos; 2) representação dos autômatos como grafos direcionados; diagramação de seus nós e arestas (disposição no plano dos estados e transições dos autômatos); e geração de uma representação visual do grafo; 3) animação da imagem do autômato demonstrando o passo-a-passo da construção do autômato.
A duas etapas iniciais estão parcialmente solucionadas. Na 1ª etapa, o programa GeraLR toma como entrada uma gramática livre de contexto e produz uma descrição textual dos autômatos finitos correspondentes. Contudo, é necessário transformar essa descrição textual em um formato aceitável pelo Graphviz (software de layout de grafos). Na 2ª etapa, o Graphviz monta o layout de um grafo a partir de uma descrição, em linguagem própria, de seus nós e arestas e gera imagens correspondentes, em diversos formatos. Na última etapa será usada LiDAS, uma linguagem de definição de animações, e seu respectivo compilador, que transforma comandos de animação em código JavaScript e os exporta, juntamente com a imagem original, para um documento HTML. Assim, a animação pode ser visualizada em qualquer navegador web moderno.

# Objetivos Gerais
1.	Gerar imagens vetoriais estáticas de autômatos produzidos a partir de gramáticas livres de contexto.
2.	Animar essas imagens estáticas a fim de demonstrar o passo-a-passo da construção dos autômatos.

