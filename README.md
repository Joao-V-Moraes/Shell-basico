# Shell-basico
Projeto desenvolvido em linguagem C, para a matéria de "Projetos de Sistemas Operacionais"
Instruções para uso:

cd <path> - Altera o diretorio atual para o <path> inserido; 
Ex: cd /home

path <dir> [<dir>...] - Define caminho(s) para busca de executáveis; 
Ex: path /home/<user>

dir - Lista todos os diretorios possiveis a partir do atual;

cat <file> - Exibe o conteúdo do <file>;
Ex: cat /home/<user>/teste/teste.txt
./teste

ls [-l] [-a] - Lista o conteúdo do diretório atual com opções;
Ex: ls -a
    ls -l

help - Lista todas as funcionalidades disponíveis;

exit - Sair do Shell;

Você pode executar programas externos caso seja passado todo o caminho.
Ex: /home/<user>/teste/teste.exe
