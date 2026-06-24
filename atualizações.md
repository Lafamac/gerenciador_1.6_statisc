# Atualizações realizadas no firmware (Gerenciador de Colheita)

## Versão 1.6.0 (Atual)

### Autonomia e Standalone
- **Independência do Celular/Android**: O firmware foi modificado para atuar de maneira 100% autônoma, sem necessidade de enviar dados ao app Android. O próprio PIC realiza todos os cálculos e exibe as estatísticas e recomendações finais no LCD.
- **Isolação e Macro de USB**: Implementação da diretiva `#ifdef USAR_USB` para compilação condicional da pilha USB HID. Por padrão (`USAR_USB` desativada), os drivers USB e a rotina `rotina_upload` não são compilados, liberando espaço valioso de ROM (reduzido de 77% para ~68%) e eliminando avisos de reentrância ou redefinindo o fluxo de carregamento.
- **Estruturação do Novo Código**: Arquivo fonte criado como `codigo_v1_6_estatisticas.c` a partir da base `codigo_v1_5_3.c`.

### Navegação e Interface
- **Menu Principal Reformulado**: Transição de menus carrossel acoplados a um menu principal unificado em loop baseado no display LCD:
  1. `1. Nova Avaliacao`
  2. `2. Historico`
  3. `3. Calibrar` (Exposta a rotina de calibração)
  4. `4. Limpar Memoria`
- **Proteção Watchdog (WDT)**: Inclusão de `restart_wdt()` na máquina de estados principal e loops internos de botões para evitar resets acidentais em caso de operador demorar a interagir.
- **Melhoria nos Botões**: Debounce aprimorado e prevenção de bounce de retorno nos botões CONFIRMA e RETORNA ao voltar dos submenus de Calibração e Balança.

### Balança e EEPROM
- **Zero Inicial com Alta Definição**: A tara automática do ADC feita no boot foi ampliada para tirar a média de 64 amostras (usando `ler_adc_filtrado_amostras(64)`), garantindo um ponto de zero muito mais estável.
- **Limpeza Segura de EEPROM**: Reestruturação da limpeza física da EEPROM no endereço `0..255` via rotina dedicada `rotina_apagar_eeprom_toda()`.
- **Inicialização Automática**: Se a EEPROM for detectada como vazia (primeira inicialização do chip com todos os bytes em `0xFF`), o firmware autodetecta e grava os valores padrão e inicializa o Checksum, impedindo estados de erro indefinidos no boot. Se corrompida, exibe `"Memoria verificar"` mas não bloqueia a utilização do aparelho no campo.

---

## Versão 1.5 (Legado)

### Compatibilidade Preservada
- MCU PIC18F2550 e compilador CCS C mantidos.
- Vetores do bootloader em `0x1100` e `0x1108` preservados.
- Registros das 12 glebas continuam usando 21 bytes cada.
- Datas continuam nos endereços 253, 254 e 255.
- USB HID continua usando endpoint 1, 32 pacotes de 8 bytes.
- Arquivos antigos da pilha USB não foram editados.

### Robustez
- WDT ativado com `WDT1024`.
- Loops demorados e atrasos longos alimentam o WDT.
- Botões receberam debounce de 30 ms e espera de soltura.
- Divisões variáveis agora verificam denominador zero.
- Dados fora das faixas básicas da EEPROM são normalizados em RAM.

### Balança e ADC
- ADC configurado com `ADC_CLOCK_DIV_64` para clock de CPU de 48 MHz.
- Pesagens usam média de 32 amostras separadas por 200 us.
- A conversão usa diferença com sinal e zona morta de 2 bits.
- Leituras abaixo do zero não geram underflow.
- O zero inicial é filtrado e continua armazenado somente em RAM.
- O modo de calibração mostra o valor filtrado; CONFIRMA atualiza o zero.

### EEPROM
- Gravações repetidas são evitadas por `eeprom_write_if_changed`.
- A limpeza percorre os endereços 0 a 255.
- O byte 252 guarda XOR dos bytes 0 a 251.
- Salvar avaliação e limpar memória atualizam o checksum.
- Checksum inválido mostra `EEPROM invalida`, mas não bloqueia o uso.

### USB
- Corrigido o bug sem efeito `pacote == 0`.
- Cada nova resposta inicia em `pacote = 0`.
- Removido o salto `goto` do envio.
- Buffers de entrada e saída iniciam zerados.
- A rotina alimenta o WDT junto de `usb_task()`.
- Desconexão chama `usb_detach()` e encerra a rotina.
- Ao concluir, o LCD mostra `Dados enviados`.
