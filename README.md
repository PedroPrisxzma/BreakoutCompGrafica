# BreakoutCompGrafica
Breakout game implementation done for Computer Graphics course at UFMG


# Dependências
- OpenGL (usamos o 3.3)
- GLFW - https://www.glfw.org/
- Grad - https://github.com/Dav1dde/glad ou https://glad.dav1d.de/ **
- STB - Especificamente STB_image.h - https://github.com/nothings/stb **
- Freetype2

**: Devem ter os arquivos incluidos manualmente na /usr/include

# Executando
Uma vez com todas as dependências instaladas, note que alguns pacotes vão pedir subdependências ao instalá-las, a execução da
compilação é feita simplesmente rodando "make" na pasta que contém o Makefile. A execução do jogo, é então feita rodando 
"./main" no terminal na pasta que contém o main (arquivo compilado).

# Observações
No Makefile, temos a flag -lglfw3, ela deve ser alterada para a versão correspondente do GLFW instalada, por exemplo, -lglfw2 ou -lglfw.


# Decisões de implementação

Implementação em C/C++ devido a familiaridade com a linguagem e os recursos de OpenGL disponibilizados na linguagem.
Escolhemos uma estética de Space para o jogo.

No jogo em si, implementamos:

- Menu
  - O jogo se inicia nessa tela, nela tem-se a opção de selecionar dentre os níveis disponíveis utilizando as teclas A e D.
  - Pode-se iniciar o jogo apertando a barra de espaço, SPACE.
  
- Controles básicos:
  - O mouse controla o paddle, mas de forma indireta, colocando o mouse na esquerda ou direita da tela, dividida exatamente no meio, faz com que o paddle se movimente naquela direção, entretanto sua velocidade depende da distâncida que o mouse está do centro, chegando na velocidade máxima na borda da janela de jogo.

  - A tecla R serve de reinicialização imediata do jogo.

  - A tecla Q serve de finalização imediata do jogo.

  - A butão esquerdo do mouse pausa.

  - O butão direito exibe informação sobre o posicionamento de todos os elementos na tela e pausa o jogo.

- A bola deixa uma trilha de partículas que vão sumindo rapidamente por onde ela passa.

- Blocos
  - Blocos indestrutíveis 
    - Ao se acertar um bloco indestrutível, a tela treme rapidamente.
  - Blocos destrutíveis
    - Quebram e refletem a bola, com uma chance de fazer um PowerUp aparecer.

- PowerUps  
  - Bons, aparecem com uma chance de 1/75 ao se qubrar um bloco 
    - SPEED, aumenta a velocidade do paddle.
    - STICKY, a bola gruda no paddle ao bater nele, podendo ser solta ao se apertar SPACE.
    - BIGGER, o paddle aumenta de tamanho.
    - PHANTOM, a bola não é refletida ao bater nos blocos.
  - Ruins, aparecem com uma chance de 1/15 ao se quebrar um bloco
    - CHAOS, Distorce o espaço, ciclando e movendo pela janela, além de modificar a aparência em si da tela. 
    - INVERT, Inverte a tela, cima é baixo, direita é esquerda, as cores também são invertidas.
