#define BLYNK_TEMPLATE_ID   "Txxxxxxx"
#define BLYNK_TEMPLATE_NAME "Incubateur Neonatal"
#define BLYNK_AUTH_TOKEN    "votre_aut"

#include <WiFi.h>
#include <DHT.h>
#include <BlynkSimpleEsp32.h>

#define WIFI_SSID     "votre ssid"
#define WIFI_PASSWORD "mot de passe"

#define DHT_PIN       4
#define RELAIS1_PIN   5
#define RELAIS2_PIN   6
#define LED_VERTE_PIN 7
#define LED_ROUGE_PIN 8
#define BUZZER_PIN    9

#define TEMP_MIN 36.0
#define TEMP_MAX 37.5

DHT dht(DHT_PIN, DHT22);
BlynkTimer timer;

void envoyerDonnees() {
  float temperature = dht.readTemperature();
  float humidite    = dht.readHumidity();

  if (isnan(temperature) || isnan(humidite)) {
    Blynk.virtualWrite(V2, "Erreur capteur !");
    return;
  }

  String alarme = "Normal";

  if (temperature < TEMP_MIN) {
    digitalWrite(RELAIS1_PIN,   HIGH);
    digitalWrite(RELAIS2_PIN,   LOW);
    digitalWrite(LED_VERTE_PIN, LOW);
    digitalWrite(LED_ROUGE_PIN, HIGH);
    digitalWrite(BUZZER_PIN,    HIGH);
    alarme = "ALERTE : Trop froid !";
    Blynk.logEvent("trop_froid");

  } else if (temperature > TEMP_MAX) {
    digitalWrite(RELAIS1_PIN,   LOW);
    digitalWrite(RELAIS2_PIN,   HIGH);
    digitalWrite(LED_VERTE_PIN, LOW);
    digitalWrite(LED_ROUGE_PIN, HIGH);
    digitalWrite(BUZZER_PIN,    HIGH);
    alarme = "ALERTE : Trop chaud !";
    Blynk.logEvent("trop_chaud");

  } else {
    digitalWrite(RELAIS1_PIN,   LOW);
    digitalWrite(RELAIS2_PIN,   LOW);
    digitalWrite(LED_VERTE_PIN, HIGH);
    digitalWrite(LED_ROUGE_PIN, LOW);
    digitalWrite(BUZZER_PIN,    LOW);
    alarme = "Normal";
  }

  Serial.println("==================");
  Serial.print("Temperature : "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Humidite    : "); Serial.print(humidite);    Serial.println(" %");
  Serial.print("Statut      : "); Serial.println(alarme);
  Serial.println("==================");

  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidite);
  Blynk.virtualWrite(V2, alarme);
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAIS1_PIN,   OUTPUT);
  pinMode(RELAIS2_PIN,   OUTPUT);
  pinMode(LED_VERTE_PIN, OUTPUT);
  pinMode(LED_ROUGE_PIN, OUTPUT);
  pinMode(BUZZER_PIN,    OUTPUT);

  digitalWrite(RELAIS1_PIN,   LOW);
  digitalWrite(RELAIS2_PIN,   LOW);
  digitalWrite(LED_VERTE_PIN, LOW);
  digitalWrite(LED_ROUGE_PIN, LOW);
  digitalWrite(BUZZER_PIN,    LOW);

  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  timer.setInterval(5000L, envoyerDonnees);
}

void loop() {
  Blynk.run();
  timer.run();
}