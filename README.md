# Pong Distribuído com ESP32

Projeto de Computação Paralela que utiliza duas placas ESP32 para controlar um jogo de Pong a partir de dados de acelerômetros MMA8452.

## Estrutura

```
pong-distribuido-esp32/
├── server/
│   └── server.js
├── esp32/
│   ├── esp32_1/
│   │   └── esp32_1.ino
│   └── esp32_2/
│       └── esp32_2.ino
└── README.md
```

## Tecnologias

- ESP32-WROOM-32
- Sensor acelerômetro MMA8452
- Arduino IDE
- C/C++ (Arduino)
- Node.js
- WebSocket
- FreeRTOS

## Funcionamento

Cada ESP32 realiza a leitura de um sensor MMA8452 e envia os dados de aceleração para um servidor WebSocket.

O processamento é dividido em duas tarefas FreeRTOS:

- **Task de aquisição:** realiza a leitura dos eixos X, Y e Z do acelerômetro.
- **Task de comunicação:** mantém a conexão WebSocket e envia os dados para o servidor.

As tarefas são executadas em cores diferentes do ESP32 e utilizam um mutex para proteger os dados compartilhados.

Os dados enviados seguem o formato:

```json
{
  "player": 1,
  "x": 123,
  "y": 456,
  "z": 789
}
```

O `player` identifica qual ESP32 enviou os dados.

## Servidor WebSocket

O servidor utiliza Node.js e a biblioteca `ws`.

### Instalação

No diretório do servidor:

```bash
npm install ws
```

### Execução

```bash
node server/server.js
```

O servidor ficará disponível na porta **8080**.

Ao receber uma conexão, o servidor exibe uma mensagem no terminal e registra os dados enviados pelas ESP32.

## Configuração das ESP32

Antes de gravar o código nas placas, configure os dados de rede nos arquivos:

- `esp32/esp32_1/esp32_1.ino`
- `esp32/esp32_2/esp32_2.ino`

Altere:

```cpp
const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA";
const char* SERVER_IP = "SEU_IP";
const uint16_t SERVER_PORT = 8080;
```

Cada placa possui um identificador diferente:

- ESP32 #1 → `PLAYER_ID = 1`
- ESP32 #2 → `PLAYER_ID = 2`

## Ligações do MMA8452

O sensor utiliza comunicação I2C.

| MMA8452 | ESP32 |
|---|---|
| SDA | GPIO 21 |
| SCL | GPIO 22 |

O endereço I2C utilizado pelo código é `0x1D`.

## Bibliotecas utilizadas nas ESP32

O código utiliza:

- `Wire.h`
- `WiFi.h`
- `WebSocketsClient.h`

A biblioteca **WebSockets** deve estar instalada na Arduino IDE.

## Fluxo do projeto

```
MMA8452
   │
   ▼
ESP32
   │
   ├── Task de aquisição
   │
   └── Task de comunicação
           │
           │ WebSocket
           ▼
     Servidor Node.js
           │
           ▼
       Dados do jogo
```

## Objetivo

O projeto demonstra conceitos de computação paralela e sistemas distribuídos utilizando múltiplos dispositivos, tarefas concorrentes, sincronização com mutex e comunicação por WebSocket.
