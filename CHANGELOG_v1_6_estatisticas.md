# Changelog - Gerenciador de Colheita v1.6

Todas as alterações notáveis feitas no firmware nesta versão v1.6 estão registradas abaixo.

## [v1.6] - 2026-06-24

### Alterações Principais
- **Operação Standalone**: Firmware reformulado para funcionar de forma 100% autônoma, dispensando qualquer dependência do aplicativo móvel Android.
- **Estruturação do Novo Código**: Arquivo fonte criado como `codigo_v1_6_estatisticas.c` a partir da base estável `codigo_v1_5_3.c`.
- **Isolação do Recurso USB**: Implementação da diretiva `#define USAR_USB 0` para isolar código legado de comunicação USB HID, simplificando o fluxo principal mas preservando a lógica original para manutenção futura.

### Melhorias no LCD e Navegação por Botões
- **Menu Principal Simplificado**: Criação de uma interface direta com 4 opções principais: `1 Nova Avaliacao`, `2 Historico`, `3 Calibrar`, `4 Apagar Memoria`.
- **Prevenção de Travamentos (WDT)**: Inclusão de `restart_wdt()` em loops de espera de botões e loops de calibração para evitar reinicializações indesejadas pelo Watchdog Timer.
- **Debounce de Botões**: Implementação de debounce por software para evitar cliques repetidos acidentais nos botões de navegação e confirmação.
- **Remoção das Telas de Colhedora**: Comentadas as funções `rotina_colhedora()` e `rotina_display_colhedora()` e suas chamadas nas rotinas de Nova Avaliação e Histórico, simplificando o fluxo no campo.


### Correções e Melhorias no ADC e Balança
- **Filtro de Leitura ADC**: Substituição das chamadas diretas de `read_adc()` pela função `ler_adc_filtrado()`, realizando uma média aritmética de amostras para maior estabilidade na medição de força e peso.
- **Tratamento de Underflow e Zona Morta**: Implementada zona morta (`ADC_DEADBAND`) e controle de sinal na rotina de conversão para evitar valores negativos de força/peso (underflow) quando o valor lido estiver abaixo do ponto de tara.
- **Função de Zero Inicial**: O processo de tara inicial passou a usar a média filtrada do ADC, exibindo feedback no LCD ("Zerando balanca") durante a inicialização.

### Melhorias na EEPROM e Checksum
- **Prevenção de Desgaste**: Substituição de gravações diretas por `eeprom_write_if_changed()`, gravando apenas se o valor for diferente do já armazenado no endereço.
- **Checksum de Dados**: Implementação de um checksum XOR dos bytes 0 a 251, salvo no byte 252. O checksum é validado no boot com exibição de aviso "Memoria verificar" em caso de erro, sem bloquear o sistema.
- **Correção da Rotina de Apagar**: Corrigido o loop de formatação de memória para garantir que o byte 255 (último byte de ano) seja adequadamente formatado, prevenindo estouros no contador de endereço.

### Correções Matemáticas e Proteções
- **Correção da Média de Cereja**: Corrigida a divisão dupla inadequada por 100 em `rotina_forca_media` para os frutos cereja.
- **Proteção contra Divisão por Zero**: Inseridas verificações de denominador em todos os cálculos matemáticos críticos que envolvem divisões (espaçamento, cálculo de plantas por hectare, cálculo de porcentagem de frutos, etc.).
