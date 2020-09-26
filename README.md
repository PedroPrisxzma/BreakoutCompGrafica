# BreakoutCompGrafica
Breakout game implementation done for Computer Graphics course at UFMG


# Dependências
- GLFW - https://www.glfw.org/
- Glad - https://github.com/Dav1dde/glad - https://glad.dav1d.de/
- STB - Especificamente STB_image.h - https://github.com/nothings/stb
- Freetype2

# Executando
Uma vez com todas as dependências instaladas, note que alguns pacotes vão pedir subdependências ao instalá-las, a execução da
compilação é feita simplesmente rodando "make" na pasta que contém o Makefile. A execução do jogo, é então feita rodando 
"./main" no terminal na pasta que contém o main (arquivo compilado).

# Observações
No Makefile, temos a flag -lglfw3, ela deve ser alterada para a versão correspondente do GLFW instalada, por exemplo, -lglfw2 ou -lglfw.
