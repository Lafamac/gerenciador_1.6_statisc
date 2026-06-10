Você é um engenheiro sênior de firmware embarcado C para PIC18F2550 usando CCS C Compiler PCWHD v5.008, com projeto aberto no MPLAB X v6.30. Trabalhe no projeto legado "Gerenciador de Colheita v1.4" sem trocar o microcontrolador e sem migrar o código para XC8 neste momento. Preserve a lógica funcional existente, LCD 16x2, USB HID e mapa de EEPROM, mas melhore robustez, calibração, leitura ADC, USB, EEPROM e manutenção.

Contexto técnico:
- MCU: PIC18F2550.
- Compilador principal: CCS C Compiler PCWHD v5.008 integrado ao MPLAB X v6.30.
- Clock esperado: 48 MHz interno via PLL USB usando cristal externo de 20 MHz.
- Fuses atuais no codigo.c: #fuses HSPLL, NOWDT, PROTECT, NOLVP, NODEBUG, USBDIV, PLL5, CPUDIV1, VREGEN, MCLR, PUT
- ADC: AN0/RA0, 10 bits, usado para célula de carga/amplificador.
- Botões: RA1 esquerda, RA2 retorna, RA3 confirma, RA4 direita.
- RA5 detecta USB conectado.
- USB: HID endpoint 1, pacotes de 8 bytes, envia 256 bytes da EEPROM em 32 pacotes.
- EEPROM interna: 256 bytes. Cada gleba usa 21 bytes. Glebas 1..12 usam 252 bytes. Bytes 253,254,255 guardam última data. Byte 252 está livre.
- Bootloader: manter #build(reset=0x1100, interrupt=0x1108) e #org 0x000, 0x10ff { }.

Objetivos obrigatórios:
1. Não alterar o PIC18F2550.
2. Manter o CCS C como compilador do firmware existente.
3. Preservar compatibilidade com o software de PC atual sempre que possível.
4. Não mudar o layout dos 21 bytes de cada gleba, salvo se for criado modo de compatibilidade.
5. Corrigir bugs reais sem alterar comportamento esperado da interface.
6. Melhorar filtragem ADC e calibração mantendo entrada AN0.
7. Melhorar proteção contra travamentos com Watchdog, sem quebrar USB.
8. Melhorar gravação/leitura EEPROM com validação simples.
9. Reduzir risco de estouro, leituras instáveis e loops presos.
10. Gerar código limpo, comentado e compatível com CCS v5.008.

Alterações específicas a implementar:

A) Criar seção de constantes no início do codigo.c:
#define FW_VERSION_MAJOR 1
#define FW_VERSION_MINOR 5
#define ADC_AMOSTRAS 32
#define ADC_DELAY_US 200
#define ADC_DEADBAND 2
#define EEPROM_REG_SIZE 21
#define EEPROM_MAX_GLEBAS 12
#define EEPROM_ADDR_CHECKSUM 252
#define EEPROM_ADDR_DIA 253
#define EEPROM_ADDR_MES 254
#define EEPROM_ADDR_ANO 255
#define USB_PACKET_SIZE 8
#define USB_TOTAL_PACKETS 32
#define CAL_PADRAO_GRAMAS_POR_BIT 3

B) Revisar fuses:
- Para versão de campo, considerar trocar NOWDT por WDT.
- Manter HSPLL, USBDIV, PLL5, CPUDIV1, VREGEN, MCLR, PUT, NOLVP.
- Se WDT for ativado, inserir restart_wdt() em todos os loops longos: rotina_botoes, rotina_sensor, rotina_upload, rotina_balanca, rotina_calibracao e menus principais.
- Atenção: durante USB, chamar restart_wdt() dentro do while junto com usb_task().

C) Implementar leitura ADC filtrada:
Criar função:
long int ler_adc_filtrado(void)
{
   int i;
   long int soma = 0;
   for(i = 0; i < ADC_AMOSTRAS; i++) {
      soma += read_adc();
      delay_us(ADC_DELAY_US);
      restart_wdt();
   }
   return soma / ADC_AMOSTRAS;
}
Substituir leituras diretas read_adc() usadas para peso por ler_adc_filtrado(). Manter read_adc() apenas se for leitura bruta de diagnóstico.

D) Melhorar rotina_conversao:
- Evitar underflow quando valor_lido < valor_zero.
- Aplicar zona morta de ADC_DEADBAND.
- Usar signed long para diferença.
Exemplo:
void rotina_conversao(void)
{
   signed long diferenca;
   diferenca = (signed long)valor_lido - (signed long)valor_zero;
   if(diferenca <= ADC_DEADBAND) {
      valor_gramas = 0;
   } else {
      valor_gramas = (long int)(diferenca * CAL_PADRAO_GRAMAS_POR_BIT);
   }
}

E) Melhorar zero automático:
- No main(), substituir valor_zero = read_adc(); por valor_zero = ler_adc_filtrado();
- Mostrar no LCD algo como "Zerando balanca" antes da leitura.
- Opcional: coletar 64 amostras para zero inicial.

F) Implementar menu simples de calibração sem alterar fluxo principal:
- A rotina_calibracao atual mostra ADC bruto. Mantê-la, mas melhorar:
  1. Mostra ADC filtrado.
  2. Confirma salva valor_zero atual.
  3. Retorna sai.
- Como EEPROM está cheia, não salvar zero nos bytes 253-255. Se for necessário salvar calibração permanente, usar EEPROM externa ou criar novo protocolo. Neste momento, manter zero em RAM para não quebrar histórico/data.

G) Corrigir bug em rotina_upload:
No final do envio USB trocar:
pacote == 0;
por:
pacote = 0;
Além disso, antes de iniciar nova resposta, garantir pacote = 0;.
Evitar goto label_enviar_usb, usando for ou while se possível.
Manter protocolo: se in_data[0] == 0, desconecta; caso contrário envia 32 pacotes de 8 bytes.

H) Melhorar rotina_upload:
- Inicializar arrays in_data e out_data com zero.
- Chamar restart_wdt() no while.
- Usar USB_TOTAL_PACKETS e USB_PACKET_SIZE.
- Após enviar tudo, mostrar "Dados enviados".
- Se USB desconectar, chamar usb_detach() e retornar.

I) Melhorar EEPROM:
- Criar função eeprom_write_if_changed(endereco, valor), que só grava se read_eeprom(endereco) != valor. Isso reduz desgaste.
- Substituir write_eeprom na rotina_salvar_eeprom por essa função.
- Corrigir limpeza de memória: hoje for(contador = 0; contador < 255; contador++) não apaga o byte 255. Usar contador <= 255, mas contador precisa ser long/int16 para não estourar int8 do CCS. Exemplo:
void rotina_apagar_eeprom_toda(void)
{
   long int addr;
   for(addr = 0; addr < 256; addr++) {
      eeprom_write_if_changed(addr, 255);
      restart_wdt();
   }
}
- Cuidado: no CCS int pode ser 8 bits. Use long int para endereços 0..255.

J) Checksum simples:
- Como byte 252 está livre, criar checksum global dos bytes 0..251 e salvar em 252.
- Função:
int eeprom_calc_checksum(void) { int chk=0; long int addr; for(addr=0; addr<252; addr++) chk ^= read_eeprom(addr); return chk; }
- Após salvar avaliação ou limpar memória, atualizar byte 252.
- Ao iniciar, calcular e comparar; se inválido, mostrar aviso "Memoria verificar" mas não bloquear equipamento para preservar compatibilidade.

K) Melhorar botões:
- rotina_botoes hoje fica em while(true) sem restart_wdt(). Inserir restart_wdt().
- Implementar debounce simples: confirmar que o botão continua pressionado após 30 ms.
- Depois de detectar, aguardar soltar ou delay curto para evitar múltiplos cliques.
- Manter os códigos: confirma=0, retorna=1, esquerda=2, direita=3.

L) Proteger limites matemáticos:
- Antes de divisões, garantir denominador != 0, principalmente PRL e cálculos de espaçamento.
- Garantir que valores lidos da EEPROM fora da faixa sejam normalizados.

M) Preservar bootloader:
- Não remover #build(reset=0x1100, interrupt=0x1108) e #org 0x000, 0x10ff { }.
- Criar comentário claro: remover apenas se for gravar via programador sem bootloader.

N) Organização MPLAB X 6.30:
- Criar README_COMPILACAO.txt com instruções:
  1. Instalar CCS PCWHD v5.008.
  2. Integrar CCS ao MPLAB X, ou compilar pelo CCS e importar HEX no MPLAB X/IPE.
  3. Manter includes no mesmo diretório: 18F2550.h, mod_lcd.c, USB.C, USB.H, pic18_usb.h, usb_desc_hid.h.
  4. Cristal no hardware/simulação deve ser 20 MHz.
  5. Configuração USB depende de PLL 48 MHz.

O) Entregáveis:
- codigo_v1_5.c com as alterações.
- diff/patch contra codigo.c original.
- README_COMPILACAO.txt.
- CHANGELOG_v1_5.txt com lista de mudanças.
- Não alterar arquivos USB antigos a menos que seja estritamente necessário.

Critérios de aceite:
- Compilar no CCS C Compiler v5.008.
- Não quebrar a interface LCD e navegação.
- USB HID continua enviando 256 bytes em 32 pacotes de 8 bytes.
- Dados antigos na EEPROM continuam legíveis.
- Leitura da balança fica mais estável por média de ADC.
- Não há underflow quando ADC fica abaixo do zero.
- Limpar memória apaga todos os 256 bytes.
- Bug pacote == 0 corrigido.
- WDT, se ativado, não reseta durante uso normal nem durante USB.

P) Criar o arquivo atualizações.md com todas as alterações que foram realizadas

Q) Criar um arquivo simulação.md com os testes que deverão ser realizados
