GERENCIADOR DE COLHEITA V1.5.2
COMPILACAO COM MPLAB X 6.30 E CCS PCWHD 5.008
================================================

1. INSTALACAO
-------------
Instale:
- MPLAB X IDE v6.30;
- MPLAB X IPE v6.30;
- CCS C Compiler PCWHD v5.008;
- driver do PICkit utilizado, quando aplicavel.

Reinicie o MPLAB X depois de instalar ou integrar o compilador CCS.

2. ARQUIVOS DO FIRMWARE
-----------------------
Mantenha no mesmo diretorio:
- codigo_v1_5_2.c
- 18F2550.h
- mod_lcd.c
- USB.C
- USB.H
- pic18_usb.h
- usb_desc_hid.h

Nao substitua as bibliotecas USB por versoes diferentes sem uma validacao
completa de enumeracao e compatibilidade com o software de PC.

3. CRIACAO DO PROJETO
---------------------
1. Abra o MPLAB X v6.30.
2. Selecione File > New Project.
3. Escolha Microchip Embedded > Standalone Project.
4. Selecione o dispositivo PIC18F2550.
5. Selecione a ferramenta de gravacao ou simulacao desejada.
6. Selecione o toolchain CCS C Compiler PCWHD v5.008.
7. Defina o diretorio do projeto.
8. Adicione codigo_v1_5_2.c e os includes listados acima.

Se o CCS nao aparecer:
1. Abra Tools > Options > Embedded > Build Tools.
2. Adicione o diretorio de instalacao do CCS.
3. Confirme que o executavel CCS/PCH para PIC18 foi detectado.
4. Reabra as propriedades do projeto e selecione o toolchain CCS.

4. CONFIGURACAO
---------------
- Dispositivo: PIC18F2550.
- Compilador: CCS PCH por meio do PCWHD v5.008.
- Cristal externo do hardware: 20 MHz.
- Clock declarado no firmware: 48 MHz.
- USB: HID, endpoint 1, pacotes de 8 bytes.

Os fuses HSPLL, PLL5, USBDIV e CPUDIV1 sao necessarios para o clock USB.
Nao altere pinagem, descritores USB ou mapa EEPROM.

5. BUILD
--------
1. Selecione o projeto como Main Project.
2. Execute Clean and Build Main Project.
3. Confirme que nao existem erros de compilacao.
4. Verifique no relatorio:
   - compilador PCH/PIC18;
   - dispositivo PIC18F2550;
   - vetores do programa iniciando em 0x1100.

Exemplo equivalente de compilacao CCS:

Ccsc.exe +FH +DF +LN +T +A +M +Z +Y=9 +EA codigo_v1_5_2.c

6. GERACAO DO HEX
-----------------
O build gera codigo_v1_5_2.hex no diretorio de saida configurado.

Antes de liberar:
- confirme que o HEX corresponde a codigo_v1_5_2.c;
- registre a versao 1.5.2;
- preserve o HEX anterior para retorno de campo;
- teste LCD, EEPROM, WDT e USB HID.

7. USO DO BOOTLOADER
--------------------
O firmware preserva obrigatoriamente:

#build(reset=0x1100, interrupt=0x1108)
#org 0x000, 0x10ff { }

Essas diretivas impedem que a aplicacao sobrescreva o bootloader e remapeiam
reset e interrupcoes. Para atualizacao normal pelo bootloader, use o HEX
gerado sem remover ou alterar essas linhas.

Nao grave a area 0x0000..0x10FF com o HEX da aplicacao.

8. USO COM PICKIT
-----------------
Para gravar mantendo o bootloader:
1. Leia e salve uma copia completa do PIC antes da operacao.
2. Grave o HEX do bootloader junto com o HEX da aplicacao, quando o processo
   de producao possuir ambos; ou preserve explicitamente a area do bootloader.
3. Confirme que a aplicacao permanece iniciando em 0x1100.
4. Evite a opcao de apagamento total se o bootloader nao estiver incluido.

Para gravar sem bootloader:
- somente use um projeto e HEX preparados especificamente para isso;
- remova o remapeamento apenas em uma variante controlada;
- nao use esse procedimento no firmware de campo v1.5.2.

9. VALIDACAO MINIMA
-------------------
- Tela de boot mostra Gerenciador / Colheita V1.5.2 por cerca de 2 s.
- EEPROM vazia mostra EEPROM invalida / Restaurando... por cerca de 2 s.
- Dados validos existentes nao sao apagados.
- Historico das 12 glebas continua legivel.
- USB envia 32 pacotes de 8 bytes, totalizando 256 bytes.
- O software de PC reconhece e le o equipamento normalmente.
- O equipamento nao reinicia durante menus, balanca, EEPROM ou USB.

10. OBSERVACAO DE TOOLCHAIN
---------------------------
A liberacao oficial deve ser compilada e validada no CCS PCWHD v5.008.
Compiladores de versao diferente podem ser usados apenas para verificacao
preliminar e nao substituem o build de producao.
