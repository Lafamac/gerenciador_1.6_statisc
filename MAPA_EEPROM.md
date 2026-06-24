# Mapa de Endereçamento da EEPROM (Gerenciador de Colheita v1.6)

A EEPROM interna do PIC18F2550 possui **256 bytes** (endereços de `0x00` a `0xFF`). Este documento descreve como os dados das glebas e metadados estão organizados para garantir a compatibilidade e a persistência histórica.

## Divisão de Espaço

- **Espaço para Glebas**: Endereços `0` a `251` (252 bytes totais).
- **Checksum Global**: Endereço `252` (1 byte).
- **Última Data Registrada**: Endereços `253` a `255` (3 bytes).

```
+-------------------------------------------------------------------+
| Endereço (Dec) | Descrição                                        |
+-------------------------------------------------------------------+
| 0 .. 20        | Registro da Gleba 1 (21 bytes)                   |
| 21 .. 41       | Registro da Gleba 2 (21 bytes)                   |
| 42 .. 62       | Registro da Gleba 3 (21 bytes)                   |
| 63 .. 83       | Registro da Gleba 4 (21 bytes)                   |
| 84 .. 104      | Registro da Gleba 5 (21 bytes)                   |
| 105 .. 125     | Registro da Gleba 6 (21 bytes)                   |
| 126 .. 146     | Registro da Gleba 7 (21 bytes)                   |
| 147 .. 167     | Registro da Gleba 8 (21 bytes)                   |
| 168 .. 188     | Registro da Gleba 9 (21 bytes)                   |
| 189 .. 209     | Registro da Gleba 10 (21 bytes)                  |
| 210 .. 230     | Registro da Gleba 11 (21 bytes)                  |
| 231 .. 251     | Registro da Gleba 12 (21 bytes)                  |
| 252            | Checksum XOR global (0..251)                     |
| 253            | Dia da última avaliação                          |
| 254            | Mês da última avaliação                          |
| 255            | Ano da última avaliação                          |
+-------------------------------------------------------------------+
```

## Estrutura do Registro de uma Gleba (21 bytes)

Cada gleba ocupa exatamente **21 bytes** (`EEPROM_REG_SIZE`). O layout dos bytes é descrito a seguir:

> [!NOTE]
> Para compatibilidade futura e integridade das leituras, se um campo numérico ler `0xFF` (255) da EEPROM nova/apagada, ele deve ser tratado/normalizado no código para `0` ou valor padrão seguro.

```
+------------------------------------------------------------------------+
| Offset | Variável no Firmware         | Tipo    | Descrição            |
+------------------------------------------------------------------------+
| 0      | data_dia                     | uint8   | Dia do registro      |
| 1      | data_mes                     | uint8   | Mês do registro      |
| 2      | data_ano                     | uint8   | Ano do registro      |
| 3      | variedade                    | uint8   | ID da variedade      |
| 4      | esp_planta_int               | uint8   | Espac. Planta (Int)  |
| 5      | esp_planta_dec               | uint8   | Espac. Planta (Dec)  |
| 6      | esp_rua_int                  | uint8   | Espac. Rua (Int)     |
| 7      | esp_rua_dec                  | uint8   | Espac. Rua (Dec)     |
| 8      | altura_planta                | uint8   | Altura da planta     |
| 9      | ramos_cova                   | uint8   | Qtd de ramos/cova    |
| 10     | renda                        | uint8   | Renda esperada       |
| 11     | colhedora                    | uint8   | ID da colhedora      |
| 12     | porc_verde                   | uint8   | % Frutos Verdes      |
| 13     | porc_cereja                  | uint8   | % Frutos Cereja      |
| 14     | carga_pendente_int           | uint8   | Carga Pendente (Int) |
| 15     | carga_pendente_dec           | uint8   | Carga Pendente (Dec) |
| 16     | produtividade                | uint8   | Produtividade        |
| 17     | forca_media_verde            | uint8   | Força Média Verde    |
| 18     | forca_media_cereja           | uint8   | Força Média Cereja   |
| 19     | recomendacao_vibracao        | uint8   | Recomend. Vibração   |
| 20     | recomendacao_velocidade      | uint8   | Recomend. Velocidade |
+------------------------------------------------------------------------+
```

## Checksum Global (XOR)

O byte `252` é usado para armazenar o checksum das 12 glebas (bytes `0` a `251`).
O cálculo é feito aplicando-se a operação lógica `XOR` sequencialmente em todos esses bytes.

Se o checksum lido da EEPROM diferir do checksum calculado durante a inicialização, o display exibirá a mensagem de aviso `"Memoria verificar"`, permitindo ao operador utilizar o sistema mesmo em caso de falha de integridade, para evitar o travamento do equipamento no campo.
