/*
************** usb_biblioteca.h **************
Esta é uma biblioteca para transmissão de dados através do protocolo de
USB de comunicação. Contém funções básicas de transmissão e recepção de
dados. É implementada para simular uma PORTA COM VIRTUAL.
Modelo do microprocessador: PIC 18F4550
Compilador: CCS 4.057
TCC 2009.1 - DSC / UPE
Aluno: Leonardo de Sá Leal Santos
Orientador: Sergio Campello
** Instruções **
* Inclusão de um descritor CDC diferente
A inclusão de descritores é feita no arquivo usb_cdc.h que está localizado na
pasta "\...\PICC\Drivers", a deve-se trocar o arquivo usb_desc_cdc.h pelo nome
do novo descritor caso deseje trocá-lo.
* Criação de um novo arquivo descritor usb
Caso deseje efetuar alguma modificação no descritor, o mesmo se encontra
na pasta "\...\PICC\Drivers" com o nome usb_desc_cdc.h. Faça as modificações
e salve na mesma pasta onde se encontra o projeto, não esquecendo de
adicioná-lo nos arquivos de configuração.
*/

#include <usb_cdc.h>
/* ** usb_cdc_putString **
Envia uma string
char *p -> ponteiro que represente a string
obs: A string deve estar declarada para ser enviada, por exemplo:
char temp[] = {"valor da string"};
usb_cdc_putString(temp);
Há uma forma alternativa para o envio de string e com ela não é necessário
a declaração da string anteriormente.
printf(usb_cdc_putc, "valor da string"); */

void usb_cdc_putString(char *p)
{
	char i = 0;	
	while (*(p+i) != '\0')
	{
		usb_cdc_putc(*(p+i));
		i ++;
	}
}
/* ** usb_cdc_putEEPROM **
Envia um bloco da memória de dados
int posicaoI -> Posição inicial do bloco de memória
int tamanho -> Comprimento do bloco de memória
Obs: Pode enviar os valores em hex ou em caracteres ASCII (código comentado)*/

void usb_cdc_putEEPROM (int posicaoI, int tamanho)
{
	int i;
	for(i=0; i<=tamanho; i++)
	{
		printf(usb_cdc_putc, "%2x ", read_eeprom(i + posicaoI) );
		//usb_cdc_putc(read_eeprom(i + posicaoI));
	}
}
/* ** usb_cdc_putFLASH **
Envia um bloco da memória de programa
int posicaoI -> Posição inicial do bloco de memória
int tamanho -> Comprimento do bloco de memória
Obs: Pode enviar os valores em hex ou em caracteres ASCII (código comentado)*/

void usb_cdc_putFLASH (char posicaoI, char tamanho)
{
	char i;
	int16 dado;
	for(i=0; i<=tamanho; i++)
	{
		dado = read_program_eeprom(i + posicaoI);
		usb_cdc_putc(dado);
	}
}
/* ** usb_cdc_conectar **
Inicializa, configura e enumera o dispositivo USB
Obs: Deve ser usado quando for conectado o dispositivo USB no
computador para efetuar a transmissão */

void usb_cdc_conectar()
{
	//usb_cdc_init();
	usb_init_cs();// não trava na configuração e é necessário chamar usb_task
	usb_init();
	usb_task();
	//while(!usb_cdc_connected()) {}
}

/* ***** Funções nativas *****
** usb_enumerated() **
Verifica se o dispositivo está pronto para a comunicação
Retorna True ou False
Obs: Maiores informações sobre estas funções e mais funções
estão no arquivo usb.c em \...\PICC\Drivers
** usb_detach() **
Desconecta o dispositivo, usado antes de sua remoção física do computador*
** usb_attach() **
Re-conecta o dispositivo, usado para re-conectá-lo
quando o dispositivo foi desconectado mas ainda não removido literalmente
Obs: Maiores informações sobre estas funções e mais funções
estão no arquivo pic18_usb.h em \...\PICC\Drivers
** usb_cdc_putc(char c) **
Envia um caracter para a transmissão
** usb_cdc_kbhit() **
Verifica se existe algum dado no buffer de recepção
Retorna True ou False
** usb_cdc_getc() **
Recebe um caracter do buffer de recepção, deve-se usar o usb_cdc_kbhit()
anteriormente para verificar se existe dados
** get_float_usb() **
Recebe um numero ponto flutuante
** get_long_usb() **
Recebe um numero longo
** get_int_usb() **
Recebe um inteiro
** get_string_usb(char *s, int max) **
Recebe uma String
** gethex_usb() **
Recebe um Hexadecimal
obs: Maiores informações sobre estas funções e mais funções
estão no arquivo usb_cdc.h em \...\PICC\Drivers */