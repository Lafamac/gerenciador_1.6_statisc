#include <18f2550.h>
#device adc=10                           //configura o compilador pra concversor A/D de 10 bits
#use delay (clock=48000000)
#fuses HSPLL, NOWDT, PROTECT, NOLVP, NODEBUG, USBDIV, PLL5, CPUDIV1, VREGEN, MCLR, PUT
#DEFINE USB_HID_DEVICE  TRUE // Si usar HID 
#define USB_EP1_TX_ENABLE  USB_ENABLE_INTERRUPT   //Activa el punto final 1 para transferencias por bloque 
#define USB_EP1_TX_SIZE    8                      //8 bytes  para envio
#define USB_EP1_RX_ENABLE  USB_ENABLE_INTERRUPT   //Activa el punto final 1 para recepcion por bloque 
#define USB_EP1_RX_SIZE    8                      //8 bytes para recepcion
#use standard_io (a)
#use standard_io (b)
#byte porta = 0xF80
#byte portb = 0xF81
#bit ADC = porta.0
#bit botao_esquerda = porta.1
#bit botao_retorna = porta.2
#bit botao_confirma = porta.3
#bit botao_direita = porta.4
#bit pino_usb_conectado = porta.5
#include <mod_lcd.c>
//#include <usb_biblioteca.h>
#include <pic18_usb.h>     								 //Funciones de bajo nivel(hardware) para la serie PIC 18Fxx5x que serviran en usb.c
#include <usb_desc_hid.h>							    //Aqui es donde van las descripciones de este dispositivo (como lo reconocera windows)
#include <usb.c>           								 //libreria para el manejo del usb
#build(reset=0x1100, interrupt=0x1108)					//É muito importante que você adicione as linhas a seguir no seu programa (firmware). Sem estas linhas, seu firmware iá sobreescrever o Bootloader, perdendo assim a funcionalidade. Estas duas linhas servem para remapear o inicio do programa e os vetores de interrupção.
#org 0x000, 0x10ff { }

int botao_pressionado;
long int valor_zero;
long int valor_lido;
long int valor_gramas;
int valor_gleba = 1;
int valor_dia;												//valores iniciais das variaveis
int valor_mes;
int valor_ano;
int valor_variedade;
int valor_esp_planta;
int valor_esp_ruas;
int valor_alt_planta;
int valor_ramos_cova;
int valor_porcentagem_cereja;
int valor_porcentagem_verde;
int valor_colhedora;
long int valor_carga_pendente;
long int valor_carga_pendente_int;
int valor_carga_pendente_dec;
long int valor_plantas_ha;
long int valor_renda;
long int valor_produtividade;
float valor_prl;
long int valor_verde_ponta [5];
long int valor_verde_meio [5];
long int valor_verde_saia [5];
long int valor_cereja_ponta_1 [5];
long int valor_cereja_ponta_2 [5];
long int valor_cereja_meio [5];
long int valor_cereja_saia_1 [5];
long int valor_cereja_saia_2 [5];
int contador = 0;
long int valor_forca_media_verde = 0;
long int valor_forca_media_cereja = 0;
signed long int valor_diferenca = 0;
int valor_diagnostico = 0;
long int valor_vibracao;
long int valor_velocidade;
int correcao_feita = 0;
float auxiliar;

union nibble						
{
   long int valor_byte;								//define uma variavel de 4 bits, que possibilita a analise de cada bit isoladamente. Sera usada para guardar o valor medio na eeprom
   int valor_nibble[2];
}variavel_nibble;

void rotina_delay ()
{
   delay_ms (250);
}

void rotina_delay_longo ()
{
   delay_ms (5000);
}

void rotina_botoes ()
{
	rotina_delay ();
	while (true)
	{
		if (botao_confirma) 
		{
			botao_pressionado = 0;
			rotina_delay ();
			return;
		}
		if (botao_retorna)
		{
			botao_pressionado = 1;
			rotina_delay ();
			return;
		}
		if (botao_esquerda) 
		{
			botao_pressionado = 2;
			rotina_delay ();
			return;
		}
		if (botao_direita) 
		{
			botao_pressionado = 3;
			rotina_delay ();
			return;
		}
	}
}

void rotina_apaga_lcd ()
{
	lcd_escreve ('\f');
}

void rotina_posiciona_lcd ()
{
	lcd_pos_xy (1,2);
}

void rotina_lcd_escreve (char mensagem)
{
	printf(lcd_escreve,"%c",mensagem);
}

void rotina_seta_direita ()
{
	lcd_pos_xy (16,2);
	output_low(lcd_rs);
   	output_bit(lcd_rs,1);
   	delay_us(100);   // aguarda 100 us
   	output_low(lcd_enable);
   
   	lcd_envia_nibble(7);
   	lcd_envia_nibble(14);	
}

void rotina_seta_esquerda ()
{
	rotina_posiciona_lcd ();
	output_low(lcd_rs);
   	output_bit(lcd_rs,1);
   	delay_us(100);   // aguarda 100 us
   	output_low(lcd_enable);
   
   	lcd_envia_nibble(7);
   	lcd_envia_nibble(15);	
}

void rotina_ler_default ()
{
	valor_dia = read_eeprom(253);													// busca na memoria EEPROM a ultima data digitada
	if (valor_dia > 31) valor_dia = 15;												//valores iniciais das variaveis
	valor_mes = read_eeprom(254);
	if (valor_mes > 12) valor_mes = 4;
	valor_ano = read_eeprom(255);
	if (valor_ano > 99) valor_ano = 16;
	valor_variedade = 0;
	valor_esp_planta = 10;
	valor_esp_ruas = 35;
	valor_alt_planta = 30;
	valor_ramos_cova = 1;
	valor_porcentagem_cereja = 25;
	valor_porcentagem_verde =  50;
	valor_renda = 480;
	valor_colhedora = 0;
	valor_carga_pendente_int = 8;
	valor_carga_pendente_dec = 0;
}
void rotina_ler_eeprom ()
{
	valor_dia = read_eeprom(((valor_gleba - 1) * 21) + 0);
	valor_mes = read_eeprom(((valor_gleba - 1) * 21) + 1);
	valor_ano = read_eeprom(((valor_gleba - 1) * 21) + 2);
	valor_variedade = read_eeprom(((valor_gleba - 1) * 21) + 3);
	valor_esp_planta = read_eeprom(((valor_gleba - 1) * 21) + 4);
	valor_esp_ruas = read_eeprom(((valor_gleba - 1) * 21) + 5);
	valor_alt_planta = read_eeprom(((valor_gleba - 1) * 21) + 6);
	valor_ramos_cova = read_eeprom(((valor_gleba - 1) * 21) + 7);
	valor_porcentagem_cereja = read_eeprom(((valor_gleba - 1) * 21) + 8);
	valor_porcentagem_verde = read_eeprom(((valor_gleba - 1) * 21) + 9);
	valor_colhedora = read_eeprom(((valor_gleba - 1) * 21) + 10);
	valor_carga_pendente_int = read_eeprom(((valor_gleba - 1) * 21) + 11);
	valor_carga_pendente_dec = read_eeprom(((valor_gleba - 1) * 21) + 12);
	valor_produtividade = read_eeprom(((valor_gleba - 1) * 21) + 13);
	variavel_nibble.valor_nibble[0] = read_eeprom(((valor_gleba - 1) * 21) + 14);
	variavel_nibble.valor_nibble[1] = read_eeprom(((valor_gleba - 1) * 21) + 15);
	valor_forca_media_verde = 	variavel_nibble.valor_byte;
	variavel_nibble.valor_nibble[0] = read_eeprom(((valor_gleba - 1) * 21) + 16);
	variavel_nibble.valor_nibble[1] = read_eeprom(((valor_gleba - 1) * 21) + 17);
	valor_forca_media_cereja = 	variavel_nibble.valor_byte;
	valor_diagnostico = read_eeprom(((valor_gleba - 1) * 21) + 18);
	valor_vibracao = read_eeprom(((valor_gleba - 1) * 21) + 19);
	valor_velocidade = read_eeprom(((valor_gleba - 1) * 21) + 20);
}

void rotina_conversao ()
{
	valor_gramas = (valor_lido - valor_zero)*3;								// sensibilidade: 3 gramas/bit
	//valor_gramas = (valor_lido - valor_zero)*25/8;						//essa parte do programa eh extremamente importante para um correto funcionamento. Para que essa conversao seja exata deve-se calibrar o potenciometro do amplificador de instrumentacao de modo que ele gere um diferenca de potencial de 3,128V entre uma peesagem de 0Kg e outra de 2Kg, isto é se para 0Kg temos por exemplo 0,91V sera necessario que para 2Kg tenhamos 4,038V. Logo se idealmente temos que 0Kg corresponde a 0V, esta balanca mede até 3,196Kg
	//valor_gramas = (valor_lido - valor_zero)*50/18 + 200;					//essa parte do programa eh extremamente importante para um correto funcionamento. Para que essa conversao seja exata deve-se calibrar o potenciometro do amplificador de instrumentacao de modo que ele gere um diferenca de potencial de 3,4V entre uma peesagem de 0.3Kg e outra de 2Kg, isto é se para 0.3Kg temos por exemplo 0,75V sera necessario que para 2Kg tenhamos 4,15V. Logo se idealmente temos que 0Kg corresponde a 0V, esta balanca mede até 2,941Kg
}

long int rotina_sensor (long int valor_maior[5], int contador)
{
	valor_maior[contador] = 0;
	while (botao_confirma == 0 && botao_retorna == 0)									//espera apertar o botao ENTER ou RETORNA para sair da rotina
	{
		delay_ms (100);
		valor_lido = read_adc ();
		if (valor_lido < valor_zero) valor_gramas = 0;									//anula valores menores de o zero da escala
		else rotina_conversao ();
		if (valor_maior[contador] < valor_gramas) valor_maior[contador] = valor_gramas;			//guarda o maior valor de forca supondo que o utilizador esta puxando um fruto
		lcd_pos_xy(11,2);
		auxiliar = valor_maior[contador];
		auxiliar /= 100;
		printf(lcd_escreve,"%2.1f N   ",auxiliar);
	}
	if (botao_retorna) botao_pressionado = 1;
	rotina_delay ();
	return valor_maior[contador];
}

void rotina_display_recomendacao ()
{
	rotina_apaga_lcd ();                                     
	printf(lcd_escreve,"Vibracao %lurpm ", valor_vibracao);
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"  Vel. %lum/h   ", valor_velocidade);
}

void rotina_display_diagnostico ()
{
   	rotina_apaga_lcd ();                                     
	rotina_lcd_escreve ("  Diagnostico:  ");
	rotina_posiciona_lcd ();
	if (valor_diagnostico == 1) rotina_lcd_escreve ("Plena - 21 dias ");
	else if (valor_diagnostico == 2) rotina_lcd_escreve ("Plena - 14 dias ");
	else if (valor_diagnostico == 3) rotina_lcd_escreve ("Plena - 7 dias  ");
	else if (valor_diagnostico == 4) rotina_lcd_escreve ("Plena - imediata");
	else if (valor_diagnostico == 5) rotina_lcd_escreve ("Selet - 21 dias "); 
	else if (valor_diagnostico == 6) rotina_lcd_escreve ("Selet - 14 dias ");
	else if (valor_diagnostico == 7) rotina_lcd_escreve ("Selet - 7 dias  ");
	else if (valor_diagnostico == 8) rotina_lcd_escreve ("Selet - imediata");
	else rotina_lcd_escreve ("Medicao invalida");
}

void rotina_display_forca_media_cereja ()
{
	rotina_apaga_lcd ();   
	rotina_lcd_escreve ("  Forca Media:  ");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"  Cereja %2.1f N    ",auxiliar);
}

void rotina_display_forca_media_verde ()
{
	rotina_apaga_lcd ();   
	rotina_lcd_escreve ("  Forca Media:  ");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"  Verde: %2.1f N    ",auxiliar);
}

void rotina_display_produtividade ()
{
	rotina_apaga_lcd ();   
	rotina_lcd_escreve (" Produtividade: ");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"    %lu sc/ha     ", valor_produtividade);
}

void rotina_display_carga_pendente ()
{
	rotina_apaga_lcd ();   
	rotina_lcd_escreve (" Carga Pendente:");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"  %02lu.%u L/planta ", valor_carga_pendente_int, valor_carga_pendente_dec);
}


void rotina_display_colhedora () 	
{
	rotina_apaga_lcd ();                                     
	rotina_lcd_escreve ("   Colhedora:   ");
	rotina_posiciona_lcd ();
	if (valor_colhedora == 0) rotina_lcd_escreve ("KTR             ");
	if (valor_colhedora == 1) rotina_lcd_escreve ("Matao           ");
	if (valor_colhedora == 2) rotina_lcd_escreve ("Matao Tracionada");
	if (valor_colhedora == 3) rotina_lcd_escreve ("Korvan          ");
	if (valor_colhedora == 4) rotina_lcd_escreve ("CASE            ");
	if (valor_colhedora == 5) rotina_lcd_escreve ("CASE Tracionada ");
	if (valor_colhedora == 6) rotina_lcd_escreve ("K3              ");
	if (valor_colhedora == 7) rotina_lcd_escreve ("TDI             ");
	if (valor_colhedora == 8) rotina_lcd_escreve ("TDI Tracionada  ");
	if (valor_colhedora == 9) rotina_lcd_escreve ("Vetor           ");
}

void rotina_display_porcentagem ()
{
	lcd_pos_xy (15,2);
	output_low(lcd_rs);
   	output_bit(lcd_rs,1);
   	delay_us(100);   // aguarda 100 us
   	output_low(lcd_enable);
   	lcd_envia_nibble(2);
   	lcd_envia_nibble(5);
}

void rotina_display_renda ()
{
	rotina_apaga_lcd ();
   	rotina_lcd_escreve ("     Renda:     ");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"   %lu L/saca   ", valor_renda);
}

void rotina_display_porcentagem_verde ()
{
	rotina_apaga_lcd ();    
    rotina_lcd_escreve (" Porcent. Verde:");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"  Avaliada: %02u   ",valor_porcentagem_verde);
	rotina_display_porcentagem ();
}

void rotina_display_porcentagem_cereja ()
{
	rotina_apaga_lcd ();    
   	rotina_lcd_escreve ("Porcent. Cereja:");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"  Avaliada: %02u   ",valor_porcentagem_cereja);
	rotina_display_porcentagem ();
}

void rotina_display_ramos_cova ()
{
	rotina_apaga_lcd ();
   	rotina_lcd_escreve ("  Ramos/planta: ");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"        %u       ", valor_ramos_cova);
}

void rotina_display_alt_planta ()
{
	rotina_apaga_lcd (); 
   	rotina_lcd_escreve ("Altura da Planta");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"   %01.1f metros   ", auxiliar);
}

void rotina_display_esp_ruas ()
{
	rotina_apaga_lcd (); 
   	rotina_lcd_escreve ("Espaco entre Rua");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"   %01.1f metros   ", auxiliar);
}

void rotina_display_esp_planta ()
{
	rotina_apaga_lcd (); 
   	rotina_lcd_escreve ("Espaco entre Pes");
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"   %01.1f metros   ", auxiliar);
}

void rotina_display_variedade ()
{
	rotina_apaga_lcd (); 
   	rotina_lcd_escreve ("   Variedade:   ");
	rotina_posiciona_lcd ();
	if (valor_variedade == 0) rotina_lcd_escreve ("   Mundo Novo   ");
	if (valor_variedade == 1) rotina_lcd_escreve ("     Catuai     ");
	if (valor_variedade == 2) rotina_lcd_escreve ("    Catucai     ");
	if (valor_variedade == 3) rotina_lcd_escreve ("     Acaia      ");
	if (valor_variedade == 4) rotina_lcd_escreve ("     Icatu      ");
	if (valor_variedade == 5) rotina_lcd_escreve ("    Topazio     ");
	if (valor_variedade == 6) rotina_lcd_escreve ("      Rubi      ");
	if (valor_variedade == 7) rotina_lcd_escreve ("     Obata      ");
	if (valor_variedade == 8) rotina_lcd_escreve ("     Outros     ");
}

void rotina_display_data ()
{
	rotina_apaga_lcd ();                                     
	printf(lcd_escreve," Data: %02u/%02u/%02u", valor_dia, valor_mes, valor_ano);
}

void rotina_salvar_eeprom ()
{
	write_eeprom (((valor_gleba - 1) * 21) + 0,valor_dia);
	write_eeprom (((valor_gleba - 1) * 21) + 1,valor_mes);
	write_eeprom (((valor_gleba - 1) * 21) + 2,valor_ano);
	write_eeprom (((valor_gleba - 1) * 21) + 3,valor_variedade);
	write_eeprom (((valor_gleba - 1) * 21) + 4,valor_esp_planta);
	write_eeprom (((valor_gleba - 1) * 21) + 5,valor_esp_ruas);
	write_eeprom (((valor_gleba - 1) * 21) + 6,valor_alt_planta);
	write_eeprom (((valor_gleba - 1) * 21) + 7,valor_ramos_cova);
	write_eeprom (((valor_gleba - 1) * 21) + 8,valor_porcentagem_cereja);
	write_eeprom (((valor_gleba - 1) * 21) + 9,valor_porcentagem_verde);
	write_eeprom (((valor_gleba - 1) * 21) + 10,valor_colhedora);
	write_eeprom (((valor_gleba - 1) * 21) + 11,valor_carga_pendente_int);
	write_eeprom (((valor_gleba - 1) * 21) + 12,valor_carga_pendente_dec);
	write_eeprom (((valor_gleba - 1) * 21) + 13,valor_produtividade);
	variavel_nibble.valor_byte = valor_forca_media_verde;
	write_eeprom (((valor_gleba - 1) * 21) + 14,variavel_nibble.valor_nibble[0]);
	write_eeprom (((valor_gleba - 1) * 21) + 15,variavel_nibble.valor_nibble[1]);
	variavel_nibble.valor_byte = valor_forca_media_cereja;
	write_eeprom (((valor_gleba - 1) * 21) + 16,variavel_nibble.valor_nibble[0]);
	write_eeprom (((valor_gleba - 1) * 21) + 17,variavel_nibble.valor_nibble[1]);
	write_eeprom (((valor_gleba - 1) * 21) + 18,valor_diagnostico);
	write_eeprom (((valor_gleba - 1) * 21) + 19,valor_vibracao);
	write_eeprom (((valor_gleba - 1) * 21) + 20,valor_velocidade);
	rotina_apaga_lcd ();                                     
	printf(lcd_escreve,"Avaliacao salva!");
	rotina_posiciona_lcd ();
	printf(lcd_escreve," Vide historico ");
	rotina_delay_longo ();
}

void rotina_recomendacao ()
{
	if (valor_diagnostico == 4)							// colheita plena
	{
		if (valor_forca_media_cereja < 450 && valor_diferenca < 300) valor_vibracao = 850;
		else if (valor_forca_media_cereja < 450 && valor_diferenca >= 300) valor_vibracao = 900;
		else if (valor_forca_media_cereja >= 450 && valor_forca_media_cereja <= 650 && valor_diferenca < 300) valor_vibracao = 950;
		else if (valor_forca_media_cereja > 650 && valor_diferenca >= 300) valor_vibracao = 1000;
		else valor_vibracao = 900;						// caso nenhuma das condicoes seja satisfeita

		if ((valor_colhedora == 0 && valor_alt_planta >= 37 && valor_alt_planta <= 45) || (valor_colhedora == 2 && valor_alt_planta >= 35 && valor_alt_planta <= 40) || ((valor_colhedora == 5 || valor_colhedora == 8) && valor_alt_planta >= 30 && valor_alt_planta < 35)) valor_velocidade = 800;
		else if ((valor_colhedora == 0 && valor_alt_planta < 37) || ((valor_colhedora == 1 || valor_colhedora == 6) && valor_alt_planta >= 35 && valor_alt_planta <= 40) || (valor_colhedora == 2 && valor_alt_planta < 35) || ((valor_colhedora == 3 || valor_colhedora == 4 || valor_colhedora == 7 || valor_colhedora == 9) && valor_alt_planta >= 30 && valor_alt_planta <= 37) || ((valor_colhedora == 5 || valor_colhedora == 8) && valor_alt_planta < 30)) valor_velocidade = 900;
		else if (((valor_colhedora == 1 || valor_colhedora == 6) && valor_alt_planta < 35) || ((valor_colhedora == 3 || valor_colhedora == 4 || valor_colhedora == 7 || valor_colhedora == 9) && valor_alt_planta <30)) valor_velocidade = 1000;
		else valor_velocidade = 700;					// caso nenhuma das condicoes seja satisfeita
	}
	else												// colheita seletiva
	{
		if (((valor_variedade == 0 || valor_variedade == 5) && valor_forca_media_cereja < 480) || ((valor_variedade == 1 || valor_variedade == 3) && valor_forca_media_cereja < 500 && valor_diferenca < 300)) valor_vibracao = 700;
		else if (((valor_variedade == 0 || valor_variedade == 5) && valor_forca_media_cereja >= 480 && valor_forca_media_cereja < 530 && valor_diferenca < 300) || ((valor_variedade == 1 || valor_variedade == 3) && valor_forca_media_cereja < 500 && valor_diferenca >= 300) || ((valor_variedade == 6 || valor_variedade == 2 || valor_variedade == 8) && valor_forca_media_cereja < 560) || ((valor_variedade == 4 || valor_variedade == 7) && valor_forca_media_cereja < 580) && valor_diferenca < 300) valor_vibracao = 750;
		else if (((valor_variedade == 0 || valor_variedade == 5) && valor_forca_media_cereja >= 480 && valor_forca_media_cereja < 530 && valor_diferenca >= 300) || ((valor_variedade == 1 || valor_variedade == 3) && valor_forca_media_cereja >= 500 && valor_forca_media_cereja < 550 && valor_diferenca < 300) || ((valor_variedade == 6 || valor_variedade == 2 || valor_variedade == 8) && valor_forca_media_cereja >= 560 && valor_forca_media_cereja < 600 && valor_diferenca < 300)  || ((valor_variedade == 4 || valor_variedade == 7) && valor_forca_media_cereja < 580 && valor_diferenca >= 300)) valor_vibracao = 800;
		else if (((valor_variedade == 0 || valor_variedade == 5) && valor_forca_media_cereja >= 530 && valor_forca_media_cereja < 580 && valor_diferenca < 300) || ((valor_variedade == 1 || valor_variedade == 3) && valor_forca_media_cereja >= 500 && valor_forca_media_cereja < 550 && valor_diferenca >= 300) || ((valor_variedade == 6 || valor_variedade == 2 || valor_variedade == 8) && valor_forca_media_cereja >= 560 && valor_forca_media_cereja < 600 && valor_diferenca >= 300)  || ((valor_variedade == 4 || valor_variedade == 7) && valor_forca_media_cereja >= 580 && valor_forca_media_cereja < 620 && valor_diferenca < 300)) valor_vibracao = 850;
		else if ((valor_variedade == 4 || valor_variedade == 7) && valor_forca_media_cereja >= 620 && valor_forca_media_cereja < 680 && valor_diferenca >= 300) valor_vibracao = 1000;
		else if (((valor_variedade == 1 || valor_variedade == 3) && valor_forca_media_cereja >= 550 && valor_forca_media_cereja < 600 && valor_diferenca >= 300) || ((valor_variedade == 6 || valor_variedade == 2 || valor_variedade == 8) && valor_forca_media_cereja >= 600 && valor_forca_media_cereja < 650 && valor_diferenca >= 300)  || ((valor_variedade == 4 || valor_variedade == 7) && valor_forca_media_cereja >= 620 && valor_forca_media_cereja < 680 && valor_diferenca < 300)) valor_vibracao = 950;
		else valor_vibracao = 900;						// caso nenhuma das condicoes seja satisfeita

		if (((valor_colhedora == 0 && valor_alt_planta < 37) || ((valor_colhedora == 1 || valor_colhedora == 6) && valor_alt_planta < 35) || ((valor_colhedora == 3 || valor_colhedora == 9) && valor_alt_planta < 30) || ((valor_colhedora == 4 || valor_colhedora == 7) && valor_alt_planta < 30)) && valor_prl >= 5 && valor_prl <= 13 && valor_ramos_cova < 3) valor_velocidade = 1600;
		else if ((((valor_colhedora == 0 && valor_alt_planta < 37) || ((valor_colhedora == 1 || valor_colhedora == 6) && valor_alt_planta < 35) || ((valor_colhedora == 3 || valor_colhedora == 9) && valor_alt_planta < 30) || ((valor_colhedora == 4 || valor_colhedora == 7) && valor_alt_planta < 30)) && (valor_prl > 13 || valor_ramos_cova == 3)) || (((valor_colhedora == 0 && valor_alt_planta >= 37 && valor_alt_planta <= 45) || ((valor_colhedora == 1 || valor_colhedora == 6) && valor_alt_planta >= 35 && valor_alt_planta <= 40) || ((valor_colhedora == 3 || valor_colhedora == 9) && valor_alt_planta >= 30 && valor_alt_planta <= 37) || ((valor_colhedora == 4 || valor_colhedora == 7) && valor_alt_planta >= 30 && valor_alt_planta <= 35)) && valor_prl >= 5 && valor_prl <= 13 && valor_ramos_cova < 3)) valor_velocidade = 1300;
		else if (((valor_colhedora == 2 && valor_alt_planta < 35) || ((valor_colhedora == 5 || valor_colhedora == 8) && valor_alt_planta  < 30)) && valor_prl >=5 && valor_prl <= 13 && valor_ramos_cova < 3) valor_velocidade = 1200;
		//else if ((((valor_colhedora == 2 && valor_alt_planta < 35) || ((valor_colhedora == 5 || valor_colhedora == 8) && valor_alt_planta < 30)) && (valor_prl > 13 || valor_ramos_cova == 3)) || ((((valor_colhedora == 2 && valor_alt_planta >= 35 && valor_alt_planta <= 40) || ((valor_colhedora == 5 || valor_colhedora == 8) && valor_alt_planta >= 30 && valor_alt_planta <= 35)) && valor_prl >= 5 && valor_prl <= 13 && valor_ramos_cova < 3) || ((((valor_colhedora == 1 || valor_colhedora == 6) && valor_alt_planta >= 35 && valor_alt_planta <= 40) || ((valor_colhedora == 3 || valor_colhedora == 9) && valor_alt_planta >= 30 && valor_alt_planta <= 35) || ((valor_colhedora == 4 || valor_colhedora == 7) && valor_alt_planta >= 30 && valor_alt_planta <= 35)) && (valor_prl > 13 || valor_ramos_cova == 3)))) valor_velocidade = 1000;
		else if ((valor_prl > 13 || valor_ramos_cova == 3) && ((valor_colhedora == 0 && valor_alt_planta >= 37 && valor_alt_planta <= 45) || (valor_colhedora == 2 && valor_alt_planta >= 35 && valor_alt_planta <= 40) || ((valor_colhedora == 5 || valor_colhedora == 8) && valor_alt_planta >= 30 && valor_alt_planta <= 35))) valor_velocidade = 800;
		else valor_velocidade = 1000;					// caso nenhuma das condicoes seja satisfeita
	}

	label_recomendacao:
	rotina_display_recomendacao ();
	//rotina_seta_esquerda ();
	//rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 0)
	{
		valor_vibracao /= 10;
		valor_velocidade /= 10;
		return;
	}
	if (botao_pressionado == 1) return;
	goto label_recomendacao;
}

void rotina_diagnostico ()
{
	label_diagnostico:
	if (valor_prl <= 5 && valor_porcentagem_verde > 45)
	{
		valor_diagnostico = 1;				//colheita plena aguarde 20 dias
	}	
	else if (valor_prl <= 5 && valor_porcentagem_verde >= 35 && valor_porcentagem_verde <= 45)
	{
		valor_diagnostico = 2;				// colheita plena aguarde 15 dias
	}
	else if (valor_prl <= 5 && valor_porcentagem_verde >= 30 && valor_porcentagem_verde < 35)
	{
		valor_diagnostico = 3;				// colheita plena aguarde 7 dias
	}
	else if ((valor_prl <= 5 && valor_porcentagem_verde < 30) || (valor_prl > 5 && valor_porcentagem_cereja <= 35 && valor_porcentagem_verde < 30))
	{
		valor_diagnostico = 4;				// colheita plena
	}
	else if (valor_prl > 5 && valor_porcentagem_cereja <= 15)
	{
		valor_diagnostico = 5;				// colheita seletiva aguarde 20 dias
	}
	else if ((valor_prl > 5 && valor_porcentagem_cereja > 15 && valor_porcentagem_cereja <= 30) || ((valor_prl > 5 && valor_porcentagem_cereja > 35) && (((valor_variedade == 0 || valor_variedade == 5) && valor_forca_media_cereja >= 590 && valor_diferenca <= 300) || ((valor_variedade == 1 || valor_variedade == 3) && valor_forca_media_cereja >= 610 && valor_diferenca <= 300) || ((valor_variedade == 6 || valor_variedade == 2 || valor_variedade == 8) && valor_forca_media_cereja >= 660 && valor_diferenca <= 300) || ((valor_variedade == 4 || valor_variedade == 7) && valor_forca_media_cereja >= 690 && valor_diferenca <= 300)))) 
	{
		valor_diagnostico = 6;				// colheita seletiva aguarde 15 dias
	}
	else if ((valor_prl > 5 && valor_porcentagem_cereja > 30 && valor_porcentagem_cereja <= 35) || ((valor_prl > 5 && valor_porcentagem_cereja > 35) && (((valor_variedade == 0 || valor_variedade == 5) && valor_forca_media_cereja >= 590 && valor_diferenca > 300) || ((valor_variedade == 1 || valor_variedade == 3) && valor_forca_media_cereja >= 610 && valor_diferenca > 300) || ((valor_variedade == 6 || valor_variedade == 2 || valor_variedade == 8) && valor_forca_media_cereja >= 660 && valor_diferenca > 300) || ((valor_variedade == 4 || valor_variedade == 7) && valor_forca_media_cereja >= 690 && valor_diferenca > 300))))
	{
		valor_diagnostico = 7;				// colheita seletiva aguarde 7 dias
	}
	else if (valor_prl > 5 && valor_porcentagem_cereja > 35)
	{
		valor_diagnostico = 8;				// colheita seletiva
	}
	else valor_diagnostico = 9;
	rotina_display_diagnostico();
	/*
	lcd_pos_xy (1,1);												//insere uma setinha a esquerda no lcd
	output_low(lcd_rs);
   	output_bit(lcd_rs,1);
   	delay_us(100);   // aguarda 100 us
   	output_low(lcd_enable);
   	lcd_envia_nibble(7);
   	lcd_envia_nibble(15);
	lcd_pos_xy (16,1);											//insere uma setinha a direita no lcd uma vez que a medicao seja valida
	output_low(lcd_rs);
   	output_bit(lcd_rs,1);
   	delay_us(100);   // aguarda 100 us
   	output_low(lcd_enable);
   	lcd_envia_nibble(7);
   	lcd_envia_nibble(14);
	lcd_pos_xy (17,1);
	*/
	rotina_botoes ();
	if (botao_pressionado == 0 || botao_pressionado == 1) return;
	goto label_diagnostico;
}

void rotina_forca_media ()
{
	long int valor_soma = 0;
	valor_forca_media_verde = 0;
	for (contador = 0; contador < 5; contador++)
	{
		valor_soma = (valor_verde_ponta[contador] + valor_verde_meio[contador] + valor_verde_saia[contador])/3;
		valor_forca_media_verde += valor_soma;
	}
	valor_forca_media_verde /= 5;
	
	valor_soma = 0;
	valor_forca_media_cereja = 0;
	for (contador = 0; contador < 5; contador++)
	{
		valor_soma = (valor_cereja_ponta_1[contador] + valor_cereja_ponta_2[contador] + valor_cereja_meio[contador] + valor_cereja_saia_1[contador] + valor_cereja_saia_2[contador])/5;
		valor_forca_media_cereja += valor_soma;
	}
	valor_forca_media_cereja /= 5;

	valor_diferenca = valor_forca_media_verde - valor_forca_media_cereja;

	label_forca_media_verde:
	auxiliar = valor_forca_media_verde;
	auxiliar /= 100;
	rotina_display_forca_media_verde ();
	//rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 0) goto label_forca_media_cereja;
	goto label_forca_media_verde;

	label_forca_media_cereja:
	auxiliar = valor_forca_media_cereja;
	auxiliar /= 100;
	rotina_display_forca_media_cereja ();
	//rotina_seta_esquerda ();
	//rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) goto label_forca_media_verde;
	if (botao_pressionado == 0) return;
	goto label_forca_media_cereja;
}

void rotina_medicao_forca ()
{
	for (contador = 0; contador < 5; contador++)
	{
		label_verde_ponta:
		rotina_apaga_lcd ();                                     
		printf(lcd_escreve,"Forca Verde (%u) ", contador+1);
		rotina_posiciona_lcd ();
		rotina_lcd_escreve ("Ponta:          ");
		valor_verde_ponta[contador] = rotina_sensor (valor_verde_ponta, contador);
		if (botao_pressionado == 1) 		
		{
			//if (contador == 0) return;				// desta maneira a cada medicao errada, o progama volta nela msm. Se usarmos o pedaco do codigo que esta comentado, o programa volta a medicao passada. Vide sugestoes Jarbas
			//else 
			//{
				//contador--;
				botao_pressionado = 0;
				goto label_verde_ponta;
			//}
		}	
		
		label_verde_meio:
		rotina_apaga_lcd ();                                     
		printf(lcd_escreve,"Forca Verde (%u) ", contador+1);
		rotina_posiciona_lcd ();
		rotina_lcd_escreve ("Meio:           ");
		valor_verde_meio[contador] = rotina_sensor (valor_verde_meio, contador);
		if (botao_pressionado == 1)
		{
			botao_pressionado = 0;
			goto label_verde_meio;
		}
		
		label_verde_saia:
		rotina_apaga_lcd ();                                     
		printf(lcd_escreve,"Forca Verde (%u) ", contador+1);
		rotina_posiciona_lcd ();
		rotina_lcd_escreve ("Saia:           ");
		valor_verde_saia[contador] = rotina_sensor (valor_verde_saia, contador);
		if (botao_pressionado == 1)
		{
			botao_pressionado = 0;
			goto label_verde_saia;
		}
		
		label_cereja_ponta_1:
		rotina_apaga_lcd ();                                     
		printf(lcd_escreve,"Forca Cereja (%u)", contador+1);
		rotina_posiciona_lcd ();
		rotina_lcd_escreve ("Ponta 1:        ");
		valor_cereja_ponta_1[contador] = rotina_sensor (valor_cereja_ponta_1, contador);
		if (botao_pressionado == 1)
		{
			botao_pressionado = 0;
			goto label_cereja_ponta_1;
		}
		
		label_cereja_ponta_2:
		rotina_apaga_lcd ();                                     
		printf(lcd_escreve,"Forca Cereja (%u)", contador+1);
		rotina_posiciona_lcd ();
		rotina_lcd_escreve ("Ponta 2:        ");
		valor_cereja_ponta_2[contador] = rotina_sensor (valor_cereja_ponta_2, contador);
		if (botao_pressionado == 1)
		{
			botao_pressionado = 0;
			goto label_cereja_ponta_2;
		}
		
		label_cereja_meio:
		rotina_apaga_lcd ();                                     
		printf(lcd_escreve,"Forca Cereja (%u)", contador+1);
		rotina_posiciona_lcd ();
		rotina_lcd_escreve ("Meio:           ");
		valor_cereja_meio[contador] = rotina_sensor (valor_cereja_meio, contador);
		if (botao_pressionado == 1)
		{
			botao_pressionado = 0;
			goto label_cereja_meio;
		}
	
		label_cereja_saia_1:
		rotina_apaga_lcd ();                                    
		printf(lcd_escreve,"Forca Cereja (%u)", contador+1);
		rotina_posiciona_lcd ();
		rotina_lcd_escreve ("Saia 1:         ");
		valor_cereja_saia_1[contador] = rotina_sensor (valor_cereja_saia_1, contador);
		if (botao_pressionado == 1)
		{
			botao_pressionado = 0;
			goto label_cereja_saia_1;
		}
		
		label_cereja_saia_2:
		rotina_apaga_lcd ();                                     
		printf(lcd_escreve,"Forca Cereja (%u)", contador+1);
		rotina_posiciona_lcd ();
		rotina_lcd_escreve ("Saia 2:         ");
		valor_cereja_saia_2[contador] = rotina_sensor (valor_cereja_saia_2, contador);
		if (botao_pressionado == 1)
		{
			botao_pressionado = 0;
			goto label_cereja_saia_2;
		}
	}
}

void rotina_prl ()
{
	valor_prl = ((float) valor_carga_pendente) / valor_esp_planta;					//podemos tornar valor_prl INT ao inves de FLOAT
}

void rotina_plantacao_produtividade ()
{
	label_plantacao:
	rotina_apaga_lcd (); 
	rotina_lcd_escreve ("   Plantacao:   ");
	rotina_posiciona_lcd (); 
	if (correcao_feita == 0)													//se ainda nao houve uma correcao desse valor procede-se com o calculo
  	{
		valor_plantas_ha = 30000 / valor_esp_planta;							//calcula a plantacao de uma forma a otimizar o resultado
		valor_plantas_ha *= 10;
		valor_plantas_ha /= valor_esp_ruas;
		valor_plantas_ha *= 10;
		valor_plantas_ha /= 3;
	}
	printf(lcd_escreve,"  %lu pes/ha   ", valor_plantas_ha);
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_produtividade;
	goto label_plantacao;

	label_produtividade:
	if (correcao_feita == 0) 
	{
		valor_produtividade = ((valor_plantas_ha * 5)/ valor_renda) * valor_carga_pendente / 50;			//se ainda nao houve uma correcao desse valor procede-se com o calculo
		if (valor_produtividade > 255) valor_produtividade = 255;												//esse calculo acima realizado permite obter valores de produtividade de ate 1149. Como nossa variavel é um INT limitei aqui seu valor a 255 uma vez que nao teremos na pratica valores superiores a 255
	}
	rotina_display_produtividade ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 2) goto label_plantacao;
	if (botao_pressionado == 3) goto label_confirmar_valores;
	goto label_produtividade;

	label_confirmar_valores:
	rotina_apaga_lcd ();
	rotina_lcd_escreve ("    Confirmar   ");
	rotina_posiciona_lcd ();
	rotina_lcd_escreve ("    Valores?    ");
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 0 || botao_pressionado == 1) return;
	if (botao_pressionado == 2) goto label_produtividade;
	if (botao_pressionado == 3) goto label_corrigir_valores;
	goto label_confirmar_valores;

	label_corrigir_valores:
	rotina_apaga_lcd ();
	rotina_lcd_escreve ("    Corrigir    ");
	rotina_posiciona_lcd ();
	rotina_lcd_escreve ("    Valores?    ");
	rotina_seta_esquerda ();
	lcd_pos_xy (17,2);
	rotina_botoes ();
	if (botao_pressionado == 0) goto label_correcao_feita;
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 2) goto label_confirmar_valores;
	goto label_corrigir_valores;

	label_correcao_feita:
 	correcao_feita = 1;
	
	label_plantacao_corrigida:
	rotina_apaga_lcd (); 
	rotina_lcd_escreve ("   Plantacao:   ");
	rotina_posiciona_lcd ();                                    
	printf(lcd_escreve,"  %lu pes/ha   ", valor_plantas_ha);
	if (valor_plantas_ha < 10000) lcd_pos_xy (6,2);
	else lcd_pos_xy (7,2);
	rotina_botoes ();
	if (botao_pressionado == 1) goto label_confirmar_valores;	
	if (botao_pressionado == 3 && valor_plantas_ha < 11111) valor_plantas_ha += 1;
	if (botao_pressionado == 2 && valor_plantas_ha > 1000) valor_plantas_ha -= 1;
	if (botao_pressionado == 0)	goto label_produtividade_corrigida;
	goto label_plantacao_corrigida;
	
	label_produtividade_corrigida:
	rotina_display_produtividade ();
	if (valor_produtividade >= 1000) lcd_pos_xy (8,2);
	else if (valor_produtividade >= 100 && valor_produtividade < 1000) lcd_pos_xy (7,2);
	else if (valor_produtividade >= 10 && valor_produtividade < 100) lcd_pos_xy (6,2);
	else lcd_pos_xy (5,2);
	rotina_botoes ();
	if (botao_pressionado == 1) goto label_plantacao_corrigida;
	if (botao_pressionado == 3 && valor_produtividade < 255) valor_produtividade += 1;
	if (botao_pressionado == 2 && valor_produtividade > 1) valor_produtividade -= 1;
	if (botao_pressionado == 0)	
	{
		auxiliar = (float) valor_produtividade * valor_renda / valor_plantas_ha;					//recalcula a carga pendente com base na correcao dos valores de plantas-ha e produtividade
		valor_carga_pendente_int = auxiliar;
		auxiliar -= valor_carga_pendente_int;
		valor_carga_pendente_dec = auxiliar*10;
		valor_carga_pendente = (valor_carga_pendente_int * 10) + valor_carga_pendente_dec;	
		goto label_confirmar_valores;
	}
	goto label_produtividade_corrigida;
}

void rotina_carga_pendente ()
{
	correcao_feita = 0;

	label_carga_pendente_int:
	rotina_display_carga_pendente ();
	lcd_pos_xy (4,2);																	//posiciona cursor
	rotina_botoes ();
	if (botao_pressionado == 0) goto label_carga_pendente_dec;
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3 && valor_carga_pendente_int <= 44) valor_carga_pendente_int += 1;
	if (botao_pressionado == 2 && valor_carga_pendente_int >= 2) valor_carga_pendente_int -= 1;
	goto label_carga_pendente_int;
	
	label_carga_pendente_dec:
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"  %02lu.%u L/planta ", valor_carga_pendente_int, valor_carga_pendente_dec);
	lcd_pos_xy (6,2);																	//posiciona cursor
	rotina_botoes ();
	if (botao_pressionado == 0) 
	{
		valor_carga_pendente = (valor_carga_pendente_int * 10) + valor_carga_pendente_dec;	
		return;
	}
	if (botao_pressionado == 1) goto label_carga_pendente_int;
	if (botao_pressionado == 3 && valor_carga_pendente_dec <= 8) valor_carga_pendente_dec += 1;
	if (botao_pressionado == 2 && valor_carga_pendente_dec >= 1) valor_carga_pendente_dec -= 1;
	goto label_carga_pendente_dec;
}

void rotina_colhedora ()			// colhedora 
{
	label_colhedora:
	rotina_display_colhedora ();
	rotina_botoes ();
	if (botao_pressionado == 0 || botao_pressionado == 1) return;
	if (botao_pressionado == 3 && valor_colhedora < 9 ) valor_colhedora += 1;
	if (botao_pressionado == 2 && valor_colhedora > 0) valor_colhedora -= 1;
	goto label_colhedora;
}

void rotina_renda ()			// renda
{
	label_renda:
	rotina_display_renda ();
	lcd_pos_xy(6,2);
	rotina_botoes ();
	if (botao_pressionado == 0 || botao_pressionado == 1) return;
	if (botao_pressionado == 3 && valor_renda < 520) valor_renda += 10;
	if (botao_pressionado == 2 && valor_renda > 430) valor_renda -= 10;
	goto label_renda;
}

void rotina_porcentagem_verde ()			// porcentagem verde
{
	label_porcentagem_verde:
	rotina_display_porcentagem_verde ();
	lcd_pos_xy (14,2);
	rotina_botoes ();
	if (botao_pressionado == 0 || botao_pressionado == 1) return;
	if (botao_pressionado == 3 && valor_porcentagem_verde < (100 - valor_porcentagem_cereja)) valor_porcentagem_verde += 1;
	if (botao_pressionado == 2 && valor_porcentagem_verde > 5) valor_porcentagem_verde -= 1;
	goto label_porcentagem_verde;
}

void rotina_porcentagem_cereja ()			// porcentagem cereja
{
	label_porcentagem_cereja:
	rotina_display_porcentagem_cereja ();
	lcd_pos_xy (14,2);
	rotina_botoes ();
	if (botao_pressionado == 0 || botao_pressionado == 1) return;
	if (botao_pressionado == 3 && valor_porcentagem_cereja < 90) valor_porcentagem_cereja += 1;
	if (botao_pressionado == 2 && valor_porcentagem_cereja > 5) valor_porcentagem_cereja -= 1;
	goto label_porcentagem_cereja;
}

void rotina_ramos_cova ()			// plantas-ramos por cova
{
	label_ramos_cova:
	rotina_display_ramos_cova ();
	lcd_pos_xy(9,2);
	rotina_botoes ();
	if (botao_pressionado == 0 || botao_pressionado == 1) return;
	if (botao_pressionado == 3 && valor_ramos_cova < 3) valor_ramos_cova += 1;
	if (botao_pressionado == 2 && valor_ramos_cova > 1) valor_ramos_cova -= 1;
	goto label_ramos_cova;
}

void rotina_alt_planta ()			// altura da planta
{
	auxiliar = (float) valor_alt_planta / 10;

	label_alt_planta:
	rotina_display_alt_planta ();						
	lcd_pos_xy (6,2);
	rotina_botoes ();
	if (botao_pressionado == 0)
	{
		auxiliar *= 10;
		valor_alt_planta = auxiliar;
		return;	
	}
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3 && auxiliar <= 5.0) auxiliar += 0.1;
	if (botao_pressionado == 2 && auxiliar >= 1.1) auxiliar -= 0.1;
	goto label_alt_planta;
}

void rotina_esp_ruas ()			// espacamento entre ruas
{
	auxiliar = (float) valor_esp_ruas / 10;

	label_esp_ruas:
	rotina_display_esp_ruas ();
	lcd_pos_xy (6,2);
	rotina_botoes ();
	if (botao_pressionado == 0)
	{
		auxiliar *= 10;
		valor_esp_ruas = auxiliar;
		return;	
	}
	if (botao_pressionado == 1) return; 
	if (botao_pressionado == 3 && auxiliar <= 5.0) auxiliar += 0.1;
	if (botao_pressionado == 2 && auxiliar >= 1.9) auxiliar -= 0.1;
	goto label_esp_ruas;
}

void rotina_esp_plantas ()			// espacamento entre plantas
{
	auxiliar = (float) valor_esp_planta / 10;

	label_esp_plantas:
	rotina_display_esp_planta ();
	lcd_pos_xy (6,2);
	rotina_botoes ();
	if (botao_pressionado == 0)
	{
		auxiliar *= 10;
		valor_esp_planta = auxiliar;
		return;	
	}
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3 && auxiliar <= 2.5) auxiliar += 0.1;
	if (botao_pressionado == 2 && auxiliar >= 0.6) auxiliar -= 0.1;
	goto label_esp_plantas;
}

void rotina_variedade ()			// variedade
{
    label_variedade:
	rotina_display_variedade ();
	rotina_botoes ();
	if (botao_pressionado == 0 || botao_pressionado == 1) return;
	if (botao_pressionado == 3 && valor_variedade < 8 ) valor_variedade += 1;
	if (botao_pressionado == 2 && valor_variedade > 0) valor_variedade -= 1;
	goto label_variedade;
}

void rotina_data ()			// data
{
	label_dia:
	rotina_display_data ();
	lcd_pos_xy (9,1);											//posiciona o cursor sob o dia
	rotina_botoes ();
	if (botao_pressionado == 0) 
	{
		write_eeprom (253,valor_dia);							// salva na eeprom o ultimo dia registrado pelo usuario
		goto label_mes;
	}
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3 && valor_dia <= 30) valor_dia += 1;
	if (botao_pressionado == 2 && valor_dia >= 2) valor_dia -= 1;
	goto label_dia;
	
	label_mes:
	rotina_display_data ();
	lcd_pos_xy (12,1);											//posiciona o cursor sob o mes
	rotina_botoes ();
	if (botao_pressionado == 0) 
	{
		/*
		if (valor_mes == 4) valor_renda = 520;										//atribui os valores de renda de acordo com as datas do ano da medicao
		else if (valor_mes == 5 && valor_dia <=15 ) valor_renda = 522;  
		else if (valor_mes == 5 && valor_dia > 15) valor_renda = 518;
		else if (valor_mes == 6 && valor_dia <= 15) valor_renda = 505;
		else if (valor_mes == 6 && valor_dia > 15) valor_renda = 495; 
		else if (valor_mes == 7 && valor_dia <= 15) valor_renda = 480;
		else if (valor_mes == 7 && valor_dia > 15) valor_renda = 460;
		else valor_renda = 435;
		*/													//considerei 435 para qualquer valor entre agosto e abril
		write_eeprom (254,valor_mes);							// salva na eeprom o ultimo mes registrado pelo usuario
		goto label_ano;
	}
	if (botao_pressionado == 1) goto label_dia;
	if (botao_pressionado == 3 && valor_mes <= 11) valor_mes += 1;
	if (botao_pressionado == 2 && valor_mes >= 2) valor_mes -= 1;
	goto label_mes;
	
	label_ano:
	rotina_display_data ();
	lcd_pos_xy (15,1);											//posiciona o cursor sob o ano
	rotina_botoes ();
	if (botao_pressionado == 0) 
	{
		write_eeprom (255,valor_ano);							// salva na eeprom o ultimo ano registrado pelo usuario
		return;
	}
	if (botao_pressionado == 1) goto label_mes;
	if (botao_pressionado == 3 && valor_ano <= 98) valor_ano += 1;
	if (botao_pressionado == 2 && valor_ano >= 1) valor_ano -= 1;
	goto label_ano;
}

void rotina_aviso ()								// rotina pra indicar quando gleba ja tiver sido cadastrada
{
	rotina_ler_default ();				// seta valores iniciais default
	rotina_ler_eeprom ();
	if (valor_diagnostico > 9) 						// gleba ainda nao cadastrada
	{
		rotina_ler_default ();				// seta valores iniciais default
		return;	
	}	
	label_rotina_aviso:
	rotina_apaga_lcd ();   
	rotina_lcd_escreve ("Gleba cadastrada");
	rotina_posiciona_lcd ();
	rotina_lcd_escreve ("Importar dados? ");
	rotina_botoes ();
	if (botao_pressionado == 0 || botao_pressionado == 1) return;
	goto label_rotina_aviso;
}

void rotina_gleba ()			// gleba
{
	label_gleba:
	rotina_apaga_lcd ();                                     
	printf(lcd_escreve,"    Gleba: %u    ", valor_gleba);
	if (valor_gleba > 9) lcd_pos_xy (13,1);
	else lcd_pos_xy (12,1);														//posiciona o cursor piscante
	rotina_botoes ();
	if (botao_pressionado == 0 || botao_pressionado == 1) return;
	if (botao_pressionado == 2 && valor_gleba > 1) valor_gleba -= 1;
	if (botao_pressionado == 3 && valor_gleba < 12) valor_gleba += 1;													///////mudei
	goto label_gleba;
}

void rotina_limpar_memoria ()
{
	label_rotina_limpar_memoria:
	rotina_apaga_lcd (); 
	rotina_lcd_escreve (" Confirma limpar");
	rotina_posiciona_lcd ();
	rotina_lcd_escreve ("    memoria?    ");
	rotina_botoes ();
	if (botao_pressionado == 0) 
	{
		for (contador = 0; contador < 255; contador++) write_eeprom (contador,255);					// apaga memorias
		rotina_apaga_lcd (); 
		rotina_lcd_escreve ("    Memoria     ");
		rotina_posiciona_lcd ();
		rotina_lcd_escreve ("    apagada!    ");
		rotina_delay_longo ();
		return;
	}
	if (botao_pressionado == 1) return;
	goto label_rotina_limpar_memoria;

}
void rotina_upload ()
{
	int contador = 0;
	int pacote = 0;
	int in_data [8];	
	int out_data [8];
				
	label_rotina_upload:
	rotina_apaga_lcd (); 
	rotina_lcd_escreve (" Conecte o USB  ");
    rotina_posiciona_lcd ();
	rotina_lcd_escreve (" ao computador! ");	
	
	label_usb_conectado:
	if (botao_retorna) return;
	if (pino_usb_conectado == 0) goto label_usb_conectado;
	usb_init ();
	usb_attach ();
	rotina_apaga_lcd (); 
	rotina_lcd_escreve (" USB conectado! ");
	while (botao_retorna == 0 && pino_usb_conectado == 1)
	{
		usb_task ();
		if (usb_enumerated()) 
		{
			if (usb_kbhit(1)) 
			{
				usb_get_packet(1,in_data,8);
				if (in_data[0] == 0) goto label_desconectar;
				else
				{ 
					label_enviar_usb:
					for (contador = 0; contador < 8; contador++) out_data[contador] = read_eeprom((pacote * 8) + contador);				//preenche o vetor de saida com 8 bytes
					usb_put_packet(1,out_data,8,USB_DTS_TOGGLE);
					delay_ms(50);													//com 10ms nao funcionou
					pacote++;
					if (pacote < 32) goto label_enviar_usb;							//sao enviados 32 pacotes de 8 bytes cada. Isso corresponde aos 256 bytes da eeprom (256 = 32 * 8)
					pacote == 0;
				}
			}
		}
	}
	label_desconectar:
	//printf(usb_cdc_putc, "\n\r\n\rVocê já pode desconectar o dispositivo com segurança!");
	rotina_delay ();
	usb_detach();
	return;
}

void rotina_historico ()
{
	label_historico_gleba:
	rotina_gleba ();	
	if (botao_pressionado == 1) return;
	rotina_ler_eeprom ();
	if (valor_diagnostico > 9) goto label_historico_fim;
	valor_vibracao *= 10;
	valor_velocidade *= 10;

	label_historico_data:
	rotina_display_data ();
	rotina_seta_direita();
	rotina_botoes ();
	if (botao_pressionado == 1) goto label_historico_gleba;
	if (botao_pressionado == 3) goto label_historico_variedade;
	goto label_historico_data;

	label_historico_variedade:
	rotina_display_variedade ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_esp_planta;
	if (botao_pressionado == 2) goto label_historico_data;
	goto label_historico_variedade;

	label_historico_esp_planta:
	auxiliar = (float) valor_esp_planta / 10;
	rotina_display_esp_planta ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_esp_ruas;
	if (botao_pressionado == 2) goto label_historico_variedade;
	goto label_historico_esp_planta;

	label_historico_esp_ruas:
	auxiliar = (float) valor_esp_ruas / 10;
	rotina_display_esp_ruas ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_alt_planta;
	if (botao_pressionado == 2) goto label_historico_esp_planta;
	goto label_historico_esp_ruas;

	label_historico_alt_planta:
	auxiliar = (float) valor_alt_planta / 10;
	rotina_display_alt_planta ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_ramos_cova;
	if (botao_pressionado == 2) goto label_historico_esp_ruas;
	goto label_historico_alt_planta;

	label_historico_ramos_cova:
	rotina_display_ramos_cova ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_porcentagem_cereja;
	if (botao_pressionado == 2) goto label_historico_alt_planta;
	goto label_historico_ramos_cova;

	label_historico_porcentagem_cereja:
	rotina_display_porcentagem_cereja ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_porcentagem_verde;
	if (botao_pressionado == 2) goto label_historico_ramos_cova;
	goto label_historico_porcentagem_cereja;

	label_historico_porcentagem_verde:
	rotina_display_porcentagem_verde ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_colhedora;
	if (botao_pressionado == 2) goto label_historico_porcentagem_cereja;
	goto label_historico_porcentagem_verde;

	label_historico_colhedora:
	rotina_display_colhedora();
	lcd_pos_xy (1,1);												//insere uma setinha a esquerda no lcd
	output_low(lcd_rs);
   	output_bit(lcd_rs,1);
   	delay_us(100);   // aguarda 100 us
   	output_low(lcd_enable);
   	lcd_envia_nibble(7);
   	lcd_envia_nibble(15);
	lcd_pos_xy (16,1);											//insere uma setinha a direita no lcd 
	output_low(lcd_rs);
   	output_bit(lcd_rs,1);
   	delay_us(100);   // aguarda 100 us
   	output_low(lcd_enable);
   	lcd_envia_nibble(7);
   	lcd_envia_nibble(14);
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_carga_pendente;
	if (botao_pressionado == 2) goto label_historico_porcentagem_verde;
	goto label_historico_colhedora;

	label_historico_carga_pendente:
	rotina_display_carga_pendente ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_produtividade;
	if (botao_pressionado == 2) goto label_historico_colhedora;
	goto label_historico_carga_pendente;

	label_historico_produtividade:
	rotina_display_produtividade ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_forca_verde;
	if (botao_pressionado == 2) goto label_historico_carga_pendente;
	goto label_historico_produtividade;

	label_historico_forca_verde:
	auxiliar = valor_forca_media_verde;
	auxiliar /= 100;
	rotina_display_forca_media_verde ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_forca_cereja;
	if (botao_pressionado == 2) goto label_historico_produtividade;
	goto label_historico_forca_verde;

	label_historico_forca_cereja:
	auxiliar = valor_forca_media_cereja;
	auxiliar /= 100;
	rotina_display_forca_media_cereja ();
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3) goto label_historico_diagnostico;
	if (botao_pressionado == 2) goto label_historico_forca_verde;
	goto label_historico_forca_cereja;

	label_historico_diagnostico:
	rotina_display_diagnostico();
	lcd_pos_xy (1,1);												//insere uma setinha a esquerda no lcd
	output_low(lcd_rs);
   	output_bit(lcd_rs,1);
   	delay_us(100);   // aguarda 100 us
   	output_low(lcd_enable);
   	lcd_envia_nibble(7);
   	lcd_envia_nibble(15);
	if (valor_diagnostico == 4 || valor_diagnostico == 8)
	{
		lcd_pos_xy (16,1);											//insere uma setinha a direita no lcd uma vez que a medicao seja valida
		output_low(lcd_rs);
   		output_bit(lcd_rs,1);
   		delay_us(100);   // aguarda 100 us
   		output_low(lcd_enable);
   		lcd_envia_nibble(7);
   		lcd_envia_nibble(14);
	}	
	lcd_pos_xy (17,1);
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 3 && (valor_diagnostico == 4 || valor_diagnostico == 8)) goto label_historico_recomendacao;
	if (botao_pressionado == 2) goto label_historico_forca_cereja;
	goto label_historico_diagnostico;

	label_historico_recomendacao:
	rotina_display_recomendacao ();
	rotina_seta_esquerda ();
	lcd_pos_xy(17,2);
   	rotina_botoes ();
	if (botao_pressionado == 1) return;
	if (botao_pressionado == 2) goto label_historico_diagnostico;
	goto label_historico_recomendacao;

	label_historico_fim:
   	rotina_apaga_lcd ();   
	rotina_lcd_escreve ("   Gleba nao    ");
	rotina_posiciona_lcd ();
	rotina_lcd_escreve ("   cadastrada   ");
	rotina_botoes ();
	if (botao_pressionado == 1) return;
	goto label_historico_fim;
}

void rotina_avaliacao ()
{
	label_avaliacao_gleba:
	rotina_gleba ();
	if (botao_pressionado == 1) return;
	rotina_aviso ();
	if (botao_pressionado == 1) goto label_avaliacao_gleba;
	label_avaliacao_data:
	rotina_data ();
	if (botao_pressionado == 1) goto label_avaliacao_gleba;
	label_avaliacao_variedade:
	rotina_variedade ();
	if (botao_pressionado == 1) goto label_avaliacao_data;
	label_avaliacao_esp_plantas:
	rotina_esp_plantas ();
	if (botao_pressionado == 1) goto label_avaliacao_variedade;
	label_avaliacao_esp_ruas:
	rotina_esp_ruas ();
	if (botao_pressionado == 1) goto label_avaliacao_esp_plantas;
	label_avaliacao_alt_planta:
	rotina_alt_planta ();
	if (botao_pressionado == 1) goto label_avaliacao_esp_ruas;
	label_avaliacao_ramos_cova:
	rotina_ramos_cova ();
	if (botao_pressionado == 1) goto label_avaliacao_alt_planta;
	label_avaliacao_porcentagem_cereja:
	rotina_porcentagem_cereja ();
	if (botao_pressionado == 1) goto label_avaliacao_ramos_cova;
	label_avaliacao_porcentagem_verde:
	rotina_porcentagem_verde ();
	if (botao_pressionado == 1) goto label_avaliacao_porcentagem_cereja;
	label_avaliacao_renda:
	rotina_renda ();
	if (botao_pressionado == 1) goto label_avaliacao_porcentagem_verde;
	label_avaliacao_colhedora:
	rotina_colhedora ();
	if (botao_pressionado == 1) goto label_avaliacao_renda;
	label_avaliacao_carga_pendente:
	rotina_carga_pendente ();
	if (botao_pressionado == 1) goto label_avaliacao_colhedora;
	label_avaliacao_plantacao_produtividade:
	rotina_plantacao_produtividade ();
	if (botao_pressionado == 1) goto label_avaliacao_carga_pendente;
	label_avaliacao_prl:
	rotina_prl ();
	if (botao_pressionado == 1) goto label_avaliacao_plantacao_produtividade;
	label_avaliacao_medicao_forca:
	rotina_medicao_forca ();
	if (botao_pressionado == 1) goto label_avaliacao_prl;
	label_avaliacao_forca_media:
	rotina_forca_media ();
	label_avaliacao_diagnostico:
	rotina_diagnostico ();
	if (botao_pressionado == 1) goto label_avaliacao_forca_media;
	if (valor_diagnostico != 4 && valor_diagnostico != 8) goto label_avaliacao_salvar_eeprom;
	label_avaliacao_recomendacao:
	rotina_recomendacao ();
	if (botao_pressionado == 1) goto label_avaliacao_diagnostico;
	label_avaliacao_salvar_eeprom:
	rotina_salvar_eeprom ();
}

void rotina_balanca ()
{
	rotina_apaga_lcd ();                                    
	rotina_lcd_escreve ("  Modo Balanca  ");
    label_balanca:
	rotina_delay();
	valor_lido = read_adc ();
	if (valor_lido < valor_zero) valor_gramas = 0;									//anula valores menores de o zero da escala
	else rotina_conversao ();
	rotina_posiciona_lcd ();
	printf(lcd_escreve,"   %04lu gramas  ",valor_gramas);
	if (botao_retorna) return;
	goto label_balanca;
}

void rotina_calibracao ()
{
	rotina_apaga_lcd ();                                    
	rotina_lcd_escreve ("Modo calibracao ");
	label_calibracao:
	rotina_delay ();
	valor_lido = read_adc ();
	rotina_posiciona_lcd ();												//posiciona o display na primeira coluna da segunda linha
	printf(lcd_escreve,"      %04lu      ",valor_lido);
	if (botao_retorna) return;
	goto label_calibracao;
}

void main()
{	
	setup_adc (ADC_CLOCK_INTERNAL);									//define clock interno
	setup_adc_ports (AN0_analog);									//define port a0 como entrada analogica
	set_adc_channel (0);											//define port a0 como conversao A/D
	lcd_ini();														//inicializa o display
	rotina_apaga_lcd ();                                     
	rotina_lcd_escreve ("    Inovacao    ");
	rotina_posiciona_lcd ();												//posiciona o display na primeira coluna da segunda linha
	rotina_lcd_escreve ("   CEIFA Ltda.  ");
	rotina_delay_longo();   
	if (botao_esquerda) rotina_calibracao();
	valor_zero = read_adc ();											// o programa atual seta o ponto de valor_zero da celula a cada nova inicializacao do aparelho e nao mais salva na memoria o seu valor
	//valor_zero = read_eeprom(255);									//le o valor guardado no eeprom correspondente ao ponto de zero da balanca
	if (botao_direita) rotina_balanca();                                    
	rotina_apaga_lcd ();                                     
	rotina_lcd_escreve (" Gerenciador de ");
	rotina_posiciona_lcd ();
	rotina_lcd_escreve ("Colheita - v.1.4");
	rotina_delay_longo();
    
	label_avaliacao:  
	rotina_apaga_lcd (); 
	rotina_lcd_escreve ("    Efetuar     ");
	rotina_posiciona_lcd ();
	rotina_lcd_escreve ("   Avaliacao    ");
	rotina_seta_direita ();
    rotina_botoes ();
	if (botao_pressionado == 0) rotina_avaliacao ();
	if (botao_pressionado == 3) goto label_historico;
	goto label_avaliacao;
	
	label_historico:
	rotina_apaga_lcd ();
	rotina_lcd_escreve ("   Visualizar   ");
	rotina_posiciona_lcd ();
	rotina_lcd_escreve ("   Historico    ");
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 0) rotina_historico ();
	if (botao_pressionado == 2) goto label_avaliacao;  
	if (botao_pressionado == 3) goto label_upload;
	goto label_historico;

	label_upload:
	rotina_apaga_lcd (); 
	rotina_lcd_escreve ("  Descarregar   ");
	rotina_posiciona_lcd ();
	rotina_lcd_escreve ("     Dados      ");
	rotina_seta_esquerda ();
	rotina_seta_direita ();
	rotina_botoes ();
	if (botao_pressionado == 0) rotina_upload ();
	if (botao_pressionado == 2) goto label_historico;
	if (botao_pressionado == 3) goto label_limpar_memoria;
	goto label_upload; 

	label_limpar_memoria:
	rotina_apaga_lcd (); 
	rotina_lcd_escreve ("     Limpar     ");
	rotina_posiciona_lcd ();
	rotina_lcd_escreve ("    Memoria     ");
	rotina_seta_esquerda ();
	lcd_pos_xy (17,2);
	rotina_botoes ();
	if (botao_pressionado == 0) rotina_limpar_memoria ();
	if (botao_pressionado == 2) goto label_upload;
	goto label_limpar_memoria;
}



