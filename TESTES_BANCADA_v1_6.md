# Plano de Testes de Bancada (Gerenciador de Colheita v1.6)

Este documento descreve os testes que devem ser realizados para validar o funcionamento do firmware `codigo_v1_6_estatisticas.c` em bancada ou simulação.

## Teste 1: Inicialização e Verificação de Boot
1. Ligue o circuito.
2. Certifique-se de que o LCD inicializa e exibe a mensagem de splash:
   - Linha 1: `Gerenciador`
   - Linha 2: `Colheita V1.6` (ou similar conforme configurado).
3. Verifique se o microcontrolador não reinicia indefinidamente (Watchdog alimentado corretamente).

## Teste 2: Calibração de Zero Inicial
1. Logo após a inicialização, o display deve mostrar:
   - Linha 1: `Zerando balanca`
   - Linha 2: `Aguarde...`
2. Certifique-se de que o firmware realiza a leitura estável do zero (utilizando `ler_adc_filtrado()`).
3. O firmware não deve travar neste loop se a célula de carga estiver desconectada (deve apenas reportar um valor flutuante ou estável).

## Teste 3: Navegação do Menu Principal
1. Após zerar a balança, o menu principal deve ser exibido.
2. Navegue usando os botões físicos (RA1/Esquerda, RA4/Direita):
   - Opções esperadas:
     1. `1 Nova Avaliacao`
     2. `2 Historico`
     3. `3 Calibrar`
     4. `4 Apagar Memoria`
3. Pressione Confirma (RA3) para entrar em uma opção e Retorna (RA2) para voltar ao menu.
4. Verifique o debounce dos botões: não deve ocorrer disparos múltiplos com apenas um toque rápido.

## Teste 4: Fluxo de Nova Avaliação (Entrada de Dados)
1. Selecione a opção `1 Nova Avaliacao`.
2. Siga as telas e verifique se é possível configurar:
   - **Gleba**: 01 a 12 (Direita/Esquerda para incrementar/decrementar).
   - **Data**: DD/MM/AA.
   - **Variedade**: Mundo Novo, Catuaí, etc.
   - **Espaçamento**: Espaço plantas, Espaço ruas, Altura, Ramos/cova.
   - **Renda e Colhedora**.
3. Em caso de denominador igual a zero (ex: espaçamento plantas = 0), certifique-se de que o PIC não crashe ao calcular plantas por hectare (defesas de divisão por zero ativas).

## Teste 5: Medição de Frutos e Porcentagens
1. Insira dados de contagem de frutos:
   - **Verde**: 40
   - **Cereja**: 60
2. O sistema deve calcular internamente e exibir:
   - **% Verde**: 40%
   - **% Cereja**: 60%
3. Se o total de frutos digitado for zero, o percentual de frutos verdes/cereja deve ser normalizado para 0% (evitando crash de divisão por zero).

## Teste 6: Medição de Força de Desprendimento
1. Simule a medição de forças de desprendimento para frutos verdes e cerejas.
2. Certifique-se de que as leituras são feitas sequencialmente por região (Ponta, Meio, Saia).
3. Verifique se o cálculo final de força média exibe valores coerentes no LCD (ex: dividindo a soma total de leituras pela quantidade de amostras correspondentes).
4. Verifique se a escala no LCD bate com a leitura real (ex: Newton).

## Teste 7: Gravação e Leitura da EEPROM (Persistência)
1. Conclua uma avaliação completa e confirme o salvamento dos dados (`Salvar avaliacao? -> Confirma`).
2. Desligue o equipamento da fonte de alimentação.
3. Ligue novamente, aguarde a inicialização e acesse o menu `2 Historico`.
4. Selecione a gleba onde salvou e verifique se as informações salvas (Data, Variedade, %, Forças, Carga, Produtividade, Recomendações) estão idênticas às gravadas.
5. Verifique o comportamento do Checksum: se algum byte for corrompido forçadamente na EEPROM, a mensagem de aviso `"Memoria verificar"` deve aparecer no boot, mas sem impedir o uso.

## Teste 8: Limpeza Completa da EEPROM
1. Acesse a opção `4 Apagar Memoria` no menu principal.
2. Confirme a exclusão dos dados.
3. Verifique se a limpeza de fato apagou os endereços de 0 a 255 (especialmente o byte 255).
4. Acesse o menu `2 Historico` e certifique-se de que a mensagem de "Gleba vazia" é mostrada para todos os slots.
