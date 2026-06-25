# Plano de Simulação e Testes (Gerenciador de Colheita v1.6.0)

Este documento orienta os testes funcionais do firmware `codigo_v1_6_estatisticas.c` em bancada ou simulação (ex: Proteus).

## 1. Preparação
1. Compilar `codigo_v1_6_estatisticas.c` usando o CCS C Compiler v5.008 (selecionando PCH/PIC18F2550).
2. Carregar o arquivo `codigo_v1_6_estatisticas.hex` no microcontrolador (ou no circuito simulado).
3. Se for utilizar gravação direta ICSP (sem o bootloader), lembre-se de comentar as diretivas `#build` e `#org` no topo do código.

---

## 2. Roteiro de Testes

### Teste 1: Inicialização e Calibração Automática de Zero (Tara)
- **Ação**: Ligue o equipamento.
- **Resultado esperado**:
  1. O LCD exibe o splash screen: `Gerenciador` / `Colheita V1.6.0`.
  2. Logo após, a tela exibe `Zerando balanca` / `Nao toque peso`.
  3. O ADC realiza 64 amostras filtradas de forma estável sem reinicialização de Watchdog.
  4. O menu principal é mostrado.

### Teste 2: Atalhos do Teclado no Boot
- **Ação**: Ligue o circuito mantendo pressionado o botão **Esquerda** (RA1).
- **Resultado esperado**: O PIC entra diretamente no **Modo calibracao** exibindo o ADC filtrado. O Watchdog não deve resetar.
- **Ação**: Ligue o circuito mantendo pressionado o botão **Direita** (RA4).
- **Resultado esperado**: O PIC entra diretamente no **Modo Balanca** exibindo a pesagem direta em gramas.
- **Ação**: Em ambos os modos, pressione **Retorna** (RA2).
- **Resultado esperado**: O firmware limpa a tela e avança para a inicialização normal sem re-acionar cliques indesejados.

### Teste 3: Navegação do Menu Principal
- **Ação**: Com o menu principal ativo, use os botões **Esquerda** (RA1) e **Direita** (RA4) para navegar pelas opções.
- **Resultado esperado**:
  - Opção 1: `1. Nova Avaliacao`
  - Opção 2: `2. Historico`
  - Opção 3: `3. Calibrar`
  - Opção 4: `4. Limpar Memoria`
- **Ação**: Pressione **Retorna** na tela do menu principal.
- **Resultado esperado**: O menu não deve executar nenhuma ação inválida e deve continuar exibindo a opção atual.

### Teste 4: Fluxo de Avaliação Completo (Modo Standalone)
- **Ação**: Entre na opção `1. Nova Avaliacao` pressionando **Confirma** (RA3).
- **Resultado esperado**: Seguir a sequência de parametrização:
  1. **Gleba**: Ajustável de 1 a 12. Pressione Confirma.
  2. **Data**: DD/MM/AA. Pressione Confirma em cada campo.
  3. **Variedade**: Ex: `Mundo Novo`. Navegue com Esquerda/Direita e confirme.
  4. **Espaço entre Pés (Planta)** e **Espaço entre Ruas**: Ajuste com Esquerda/Direita (exibe em metros) e confirme.
  5. **Altura da Planta** e **Ramos/planta**: Ajuste e confirme.
  6. **Porcentagem de Cereja** e **Porcentagem de Verde**: Ajuste e confirme.
  7. **Renda**: Ajuste e confirme. (A tela de Colhedora foi comentada e removida do fluxo).
  8. **Carga Pendente**: Ajuste o valor inteiro e depois o decimal (ex: `02.5 L/planta`) e confirme.
  9. **Plantacao/Produtividade**: Exibe o resultado calculado em sacas/ha (ex: `38 sc/ha`). Pressione Confirma para aceitar os valores calculados de produtividade ou Direita para entrar no modo de correção manual.
  10. **Medição de Força**:
      - Realizar as 5 séries de medições de força Verde (Ponta, Meio, Saia).
      - Realizar as 5 séries de medições de força Cereja (Ponta 1, Ponta 2, Meio, Saia 1, Saia 2).
      - Verifique se as forças médias são calculadas corretamente e mostradas no LCD.
  11. **Diagnóstico**: O LCD deve mostrar o diagnóstico de colheita correspondente (ex: `Selet - 7 dias`).
  12. **Recomendação**: O LCD deve exibir a rotação de vibração e velocidade recomendadas da máquina (ex: `Vibracao 850rpm` / `Vel. 1300m/h`).
  13. **Salvar**: Ao confirmar no final, o display exibe `Avaliacao salva! / Vide historico` e retorna ao menu principal.

### Teste 5: Consulta e Navegação de Histórico
- **Ação**: Entre na opção `2. Historico`.
- **Resultado esperado**:
  1. Permite selecionar a Gleba (1 a 12).
  2. Pressione Confirma.
  3. Exibe sequencialmente as telas de histórico (Data, Variedade, Espaçamento, Porcentagens, Produtividade, Forças, Diagnóstico e Recomendações) usando Esquerda/Direita.
  4. Pressione **Retorna** para voltar à seleção de gleba.

### Teste 6: Limpeza de Memória e Checksum
- **Ação**: Entre na opção `4. Limpar Memoria` e selecione **Confirma**.
- **Resultado esperado**:
  1. Exibe `Memoria apagada!` e retorna ao menu.
  2. Ao abrir o Histórico de qualquer gleba, deve exibir `Gleba nao cadastrada`.
  3. O checksum global no endereço `252` é recalculado e reescrito na EEPROM com o valor correto.

---

## 3. Teste de Margem e Robustez
- **Denominador Zero**: Configure os valores de espaçamento para zero. Certifique-se de que o sistema não congela ou reinicia durante o cálculo da produtividade (a proteção contra divisão por zero do PIC deve atuar, definindo o resultado final como zero).
- **Sobretara / Sobrecarga**: Aplique uma leitura ADC acima de `valor_zero`. Confirme que a conversão calcula o peso proporcional.
- **Subtara**: Desconecte o ADC ou force uma leitura inferior ao zero tara. Confirme que o display exibe `0000 gramas` ao invés de estourar para valores altos (proteção de underflow).
- **Queda de Energia**: Desligue a alimentação após salvar uma avaliação. Ao ligar novamente, certifique-se de que todos os dados persistem idênticos na EEPROM no menu Histórico.
