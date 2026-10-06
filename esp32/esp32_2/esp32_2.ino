#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsClient.h>

#define MMA8452_ADDR 0x1D

const int PLAYER_ID = 2;

// ============================================================
// WI-FI
// ============================================================

const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA";

// IP DO NOTEBOOK
const char* SERVER_IP = "SEU_IP";
const uint16_t SERVER_PORT = 8080;


// ============================================================
// WEBSOCKET
// ============================================================

WebSocketsClient webSocket;


// ============================================================
// DADOS COMPARTILHADOS
// ============================================================

int16_t sensorX = 0;
int16_t sensorY = 0;
int16_t sensorZ = 0;

SemaphoreHandle_t sensorMutex;


// ============================================================
// LEITURA DO MMA8452
// ============================================================

void lerAcelerometro(int16_t &x, int16_t &y, int16_t &z) {

  Wire.beginTransmission(MMA8452_ADDR);
  Wire.write(0x01);
  Wire.endTransmission(false);

  Wire.requestFrom(MMA8452_ADDR, 6);

  if (Wire.available() == 6) {

    x = (Wire.read() << 8) | Wire.read();
    y = (Wire.read() << 8) | Wire.read();
    z = (Wire.read() << 8) | Wire.read();

    x >>= 4;
    y >>= 4;
    z >>= 4;
  }
}


// ============================================================
// TASK 1 — AQUISIÇÃO
// CORE 0
// ============================================================

void taskAquisicao(void *parameter) {

  Serial.print("Task aquisicao - Core ");
  Serial.println(xPortGetCoreID());

  while (true) {

    int16_t x;
    int16_t y;
    int16_t z;

    lerAcelerometro(x, y, z);

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {

      sensorX = x;
      sensorY = y;
      sensorZ = z;

      xSemaphoreGive(sensorMutex);
    }

    Serial.print("[AQUISICAO] Core ");
    Serial.print(xPortGetCoreID());

    Serial.print(" | X: ");
    Serial.print(x);

    Serial.print(" | Y: ");
    Serial.print(y);

    Serial.print(" | Z: ");
    Serial.println(z);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


// ============================================================
// TASK 2 — COMUNICAÇÃO
// CORE 1
// ============================================================

void taskComunicacao(void *parameter) {

  Serial.print("Task comunicacao - Core ");
  Serial.println(xPortGetCoreID());

  while (true) {

    int16_t x;
    int16_t y;
    int16_t z;

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {

      x = sensorX;
      y = sensorY;
      z = sensorZ;

      xSemaphoreGive(sensorMutex);
    }

    // Mantém o WebSocket funcionando
    webSocket.loop();

    // Envia os dados para o servidor
    if (webSocket.isConnected()) {

      String mensagem = "{";
      mensagem += "\"player\":" + String(PLAYER_ID) + ",";
      mensagem += "\"x\":" + String(x) + ",";
      mensagem += "\"y\":" + String(y) + ",";
      mensagem += "\"z\":" + String(z);
      mensagem += "}";

      webSocket.sendTXT(mensagem);

      Serial.print("[COMUNICACAO] Core ");
      Serial.print(xPortGetCoreID());
      Serial.print(" | Enviando: ");
      Serial.println(mensagem);

    } else {

      Serial.println("[COMUNICACAO] WebSocket desconectado.");
    }

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println(" ESP32 #2 - PONG DISTRIBUIDO");
  Serial.println("================================");


  // ==========================================================
  // I2C
  // ==========================================================

  Wire.begin(21, 22);


  // ==========================================================
  // MMA8452
  // ==========================================================

  Wire.beginTransmission(MMA8452_ADDR);
  Wire.write(0x2A);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.beginTransmission(MMA8452_ADDR);
  Wire.write(0x0E);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.beginTransmission(MMA8452_ADDR);
  Wire.write(0x2A);
  Wire.write(0x01);
  Wire.endTransmission();

  Serial.println("MMA8452 iniciado.");


  // ==========================================================
  // MUTEX
  // ==========================================================

  sensorMutex = xSemaphoreCreateMutex();

  if (sensorMutex == NULL) {

    Serial.println("ERRO: nao foi possivel criar o mutex!");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("Mutex criado.");


  // ==========================================================
  // WI-FI
  // ==========================================================

  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");

  Serial.print("IP da ESP32: ");
  Serial.println(WiFi.localIP());


  // ==========================================================
  // WEBSOCKET
  // ==========================================================

  Serial.println("Conectando ao servidor WebSocket...");

  webSocket.begin(
    SERVER_IP,
    SERVER_PORT,
    "/"
  );

  webSocket.setReconnectInterval(5000);


  // ==========================================================
  // TASKS
  // ==========================================================

  xTaskCreatePinnedToCore(
    taskAquisicao,
    "TaskAquisicao",
    4096,
    NULL,
    1,
    NULL,
    0
  );

  xTaskCreatePinnedToCore(
    taskComunicacao,
    "TaskComunicacao",
    4096,
    NULL,
    1,
    NULL,
    1
  );
}


// ============================================================
// LOOP
// ============================================================

void loop() {
}