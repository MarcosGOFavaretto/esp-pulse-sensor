#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Monitor de Saúde em Tempo Real</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background-color: #e8f5e9;
            /* Fundo em tom de verde claro para remeter à saúde */
        }

        .container {
            text-align: center;
            padding: 30px;
            background-color: white;
            border-radius: 20px;
            box-shadow: 0 0 30px rgba(0, 0, 0, 0.1);
            width: 300px;
            position: relative;
        }

        h1 {
            font-size: 2rem;
            color: #4caf50;
            /* Verde escuro para dar um toque de saúde e medicina */
        }

        .heart-icon,
        .oxygen-icon {
            font-size: 3rem;
            color: #f44336;
        }

        #heart-rate,
        #oxygen-level {
            font-size: 3rem;
            margin: 10px 0;
        }

        #heart-rate {
            color: #f44336;
            /* Vermelho para o valor de bpm */
            animation: heartbeat 1s infinite;
        }

        #oxygen-level {
            color: #03a9f4;
            /* Azul para representar oxigenação */
        }

        @keyframes heartbeat {

            0%,
            100% {
                transform: scale(1);
            }

            50% {
                transform: scale(1.2);
            }
        }

        .status {
            font-size: 1.2rem;
            color: #4caf50;
            margin-top: 10px;
        }

        .disconnected {
            color: #f44336;
        }

        /* Estilo para as bolhas de oxigênio */
        .bubble-container {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 60px;
            margin: 0 auto;
        }

        .bubble {
            position: absolute;
            background-color: rgba(3, 169, 244, 0.7);
            /* Cor azul translúcida */
            border-radius: 50%;
            animation: float 4s ease-in-out infinite;
        }

        .bubble:nth-child(1) {
            width: 20px;
            height: 20px;
            bottom: 0;
            left: 5px;
            animation-duration: 3s;
            animation-delay: 0s;
        }

        .bubble:nth-child(2) {
            width: 15px;
            height: 15px;
            bottom: 10px;
            left: 25px;
            animation-duration: 4s;
            animation-delay: 1s;
        }

        .bubble:nth-child(3) {
            width: 10px;
            height: 10px;
            bottom: 20px;
            left: 45px;
            animation-duration: 5s;
            animation-delay: 2s;
        }

        @keyframes float {
            0% {
                transform: translateY(0);
                opacity: 1;
            }

            100% {
                transform: translateY(-60px);
                opacity: 0;
            }
        }
    </style>
</head>

<body>

    <div class="container">
        <h1>Monitor de Saúde</h1>

        <!-- Batimento Cardíaco -->
        <div class="heart-icon">❤️</div>
        <div id="heart-rate">--</div>

    <script>
        async function fetchData() {
            try {
                const response = await fetch('/beat');
                if (!response.ok) {
                    throw new Error('Erro na requisição: ' + response.status);
                }
                const data = await response.text();
                const valorRecebido = parseFloat(data);

                let div = document.getElementById('heart-rate');
                div.innerHTML = valorRecebido;

                return valorRecebido;
            } catch (error) {
                console.error('Erro ao buscar dados:', error);
            }
        }

        setInterval(fetchData, 100);
    </script>

</body>

</html>
)rawliteral";

MAX30105 particleSensor;

const byte RATE_SIZE = 4;  //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];     //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;  //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  WiFi.begin("ssid", "passw");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/beat", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", String(beatAvg));
  });

  server.begin();

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))  //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1)
      ;
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup();                     //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A);  //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);   //Turn off Green LED
}

void loop() {
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;  //Store this reading in the array
      rateSpot %= RATE_SIZE;                     //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
}
