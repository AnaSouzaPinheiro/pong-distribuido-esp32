const WebSocket = require("ws");

const PORT = 8080;

const wss = new WebSocket.Server({
  port: PORT,
});

console.log(`Servidor WebSocket rodando na porta ${PORT}`);

wss.on("connection", (ws, request) => {
  console.log("Nova conexão recebida!");

  ws.on("message", (message) => {
    try {
      const data = JSON.parse(message.toString());

      console.log("Dados recebidos:", data);
    } catch (error) {
      console.log("Mensagem inválida:", message.toString());
    }
  });

  ws.on("close", () => {
    console.log("Conexão encerrada.");
  });

  ws.on("error", (error) => {
    console.log("Erro WebSocket:", error.message);
  });
});