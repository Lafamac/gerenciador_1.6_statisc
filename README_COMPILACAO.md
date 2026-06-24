# README - Instruções de Compilação (Gerenciador de Colheita v1.6)

Este documento descreve como configurar o ambiente e compilar o firmware `codigo_v1_6_estatisticas.c` para o microcontrolador PIC18F2550.

## Requisitos de Software

1. **Compilador**: CCS C Compiler PCWHD v5.008 (ou versão compatível).
2. **IDE**: MPLAB X IDE v6.30 (ou compilar diretamente pelo CCS PCWHD e importar o arquivo `.hex` resultante no MPLAB IPE v6.30 para gravação).

## Arquivos Necessários no mesmo Diretório

Certifique-se de que os seguintes arquivos de cabeçalho e bibliotecas estejam presentes no mesmo diretório do arquivo fonte principal (`codigo_v1_6_estatisticas.c`):

- `18F2550.h` (Definições de registradores do PIC)
- `mod_lcd.c` (Driver de controle do LCD 16x2)
- `usb.c` / `usb.h` (Pilha USB do CCS C)
- `pic18_usb.h` (Driver USB específico para PIC18)
- `usb_desc_hid.h` (Descritores USB HID)

## Configurações do Hardware

- **Cristal Oscilador**: 20 MHz externo.
- **Frequência do Processador (CPU Clock)**: 48 MHz via PLL interno.
- **Microcontrolador**: PIC18F2550.
- **Configuração de Fuses**:
  - `HSPLL` (High-Speed Crystal com PLL ativo)
  - `PLL5` (Divisor do PLL por 5, entrada de 20MHz gera 4MHz)
  - `CPUDIV1` (Sem divisão pós-PLL, CPU roda a 48MHz)
  - `USBDIV` (Clock do USB derivado do PLL/2, 48MHz)
  - `VREGEN` (Regulador interno de 3.3V do USB ativo)
  - `MCLR` (Master Clear ativo)
  - `PUT` (Power-up Timer ativo)
  - `NOLVP` (Low Voltage Programming desativado)
  - `NOWDT` / `WDT` (Watchdog Timer conforme seleção de compilação)
  - `PROTECT` (Proteção de código)

## Passos para Compilação no CCS C Compiler

1. Abra o CCS C Compiler PCWHD.
2. Crie ou abra o projeto com o arquivo fonte `codigo_v1_6_estatisticas.c`.
3. Verifique se o compilador reconhece o arquivo de definições `18F2550.h`.
4. Compile o projeto (Pressione `F9` ou clique em **Build**).
5. Certifique-se de que nenhum erro foi gerado. O compilador gerará o arquivo `codigo_v1_6_estatisticas.hex`.

## Importando e Gravando via MPLAB X / IPE

1. Abra o **MPLAB X IPE** (Integrated Programming Environment).
2. Selecione a Família: **Advanced 8-bit MCUs (PIC18)** e o Dispositivo: **PIC18F2550**.
3. Selecione a ferramenta de gravação (ex: PICkit 3, PICkit 4 ou ICD 4).
4. Clique em **Connect** para conectar ao gravador e alimentar o alvo.
5. Em **Source**, clique em **Browse** e selecione o arquivo `codigo_v1_6_estatisticas.hex`.
6. Clique no botão **Program** para gravar o chip.

> [!NOTE]
> O firmware original prevê o uso de bootloader com as diretivas `#build(reset=0x1100, interrupt=0x1108)` e `#org 0x000, 0x10ff { }`. Se for realizar a gravação direta por gravador externo (ICSP) sem bootloader, essas diretivas devem ser comentadas no arquivo fonte antes de compilar.
