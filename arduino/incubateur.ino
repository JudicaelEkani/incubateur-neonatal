// BIBLIOTHÈQUES

#include <WiFi.h>
#include <DHT.h>
#include <FirebaseESP32.h>

// PARAMÈTRES WIFI ET FIREBASE

#define WIFI_SSID       "Galaxy A04e 4722"        // Nom de ton hotspot
#define WIFI_PASSWORD   "adoption"  // Mot de passe hotspot
#define FIREBASE_URL    "https://incubateur-neonatal-default-rtdb.firebaseio.com"
#define FIREBASE_SECRET "QZTmtYXS46mpe7Gtwp74x6D0uc8I5zaIxpArYY8i" // Clé dans Firebase → Paramètres → Service


// BROCHES ESP32

#define DHT_PIN       4    // DHT22
#define RELAIS1_PIN   5    // Résistance chauffante
#define RELAIS2_PIN   18   // Ventilateur 2 (refroidissement)
#define LED_VERTE_PIN 21   // LED verte (température normale)
#define LED_ROUGE_PIN 22   // LED rouge (température anormale)
#define BUZZER_PIN    25   // Buzzer (alarme sonore)

// Ventilateur 1 → branché directement sur 5V, tourne sans arrêt

// SEUILS TEMPÉRATURE BÉBÉ
#define TEMP_MIN  36.0   // En dessous → trop froid
#define TEMP_MAX  37.5   // Au dessus  → trop chaud


// OBJETS

DHT dht(DHT_PIN, DHT22);
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;


// SETUP — s'exécute une seule fois

void setup() {
  Serial.begin(115200);

  // Configurer les broches
  pinMode(RELAIS1_PIN,   OUTPUT);
  pinMode(RELAIS2_PIN,   OUTPUT);
  pinMode(LED_VERTE_PIN, OUTPUT);
  pinMode(LED_ROUGE_PIN, OUTPUT);
  pinMode(BUZZER_PIN,    OUTPUT);

  // Tout éteindre au démarrage
  digitalWrite(RELAIS1_PIN,   LOW);
  digitalWrite(RELAIS2_PIN,   LOW);
  digitalWrite(LED_VERTE_PIN, LOW);
  digitalWrite(LED_ROUGE_PIN, LOW);
  digitalWrite(BUZZER_PIN,    LOW);

  // Démarrer le capteur DHT22
  dht.begin();

  // ---- Connexion WiFi ----
  Serial.print("Connexion WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté !");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());

  // ---- Connexion Firebase ----
  config.host           = FIREBASE_URL;
  config.signer.tokens.legacy_token = FIREBASE_SECRET;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase connecté !");
}


// LOOP — s'exécute en boucle

void loop() {

  // ---- 1. LIRE LE CAPTEUR DHT22 ----
  float temperature = dht.readTemperature();
  float humidite    = dht.readHumidity();

  // Vérifier que la lecture est valide
  if (isnan(temperature) || isnan(humidite)) {
    Serial.println("Erreur : capteur DHT22 non lisible !");
    // Signaler l'erreur sur Firebase
    Firebase.setString(firebaseData, "/alarme", "Erreur capteur !");
    delay(3000);
    return;
  }

  // ---- 2. LOGIQUE DE CONTRÔLE ----
  String alarme = "Normal";

  if (temperature < TEMP_MIN) {
    // Trop froid → chauffer
    digitalWrite(RELAIS1_PIN,   HIGH); // Résistance chauffante ON
    digitalWrite(RELAIS2_PIN,   LOW);  // Ventilo refroidissement OFF
    digitalWrite(LED_VERTE_PIN, LOW);  // LED verte OFF
    digitalWrite(LED_ROUGE_PIN, HIGH); // LED rouge ON
    digitalWrite(BUZZER_PIN,    HIGH); // Buzzer ON
    alarme = "Trop froid !";

  } else if (temperature > TEMP_MAX) {
    // Trop chaud → refroidir
    digitalWrite(RELAIS1_PIN,   LOW);  // Résistance chauffante OFF
    digitalWrite(RELAIS2_PIN,   HIGH); // Ventilo refroidissement ON
    digitalWrite(LED_VERTE_PIN, LOW);  // LED verte OFF
    digitalWrite(LED_ROUGE_PIN, HIGH); // LED rouge ON
    digitalWrite(BUZZER_PIN,    HIGH); // Buzzer ON
    alarme = "Trop chaud !";

  } else {
    // Température normale
    digitalWrite(RELAIS1_PIN,   LOW);  // Résistance chauffante OFF
    digitalWrite(RELAIS2_PIN,   LOW);  // Ventilo refroidissement OFF
    digitalWrite(LED_VERTE_PIN, HIGH); // LED verte ON
    digitalWrite(LED_ROUGE_PIN, LOW);  // LED rouge OFF
    digitalWrite(BUZZER_PIN,    LOW);  // Buzzer OFF
    alarme = "Normal";
  }

  // ---- 3. AFFICHER DANS LE MONITEUR SÉRIE ----
  Serial.println("==================");
  Serial.print("Temperature : ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Humidite    : ");
  Serial.print(humidite);
  Serial.println(" %");
  Serial.print("Statut      : ");
  Serial.println(alarme);
  Serial.println("==================");

  // ---- 4. ENVOYER VERS FIREBASE ----
  if (Firebase.ready()) {
    Firebase.setFloat(firebaseData,  "/temperature", temperature);
    Firebase.setFloat(firebaseData,  "/humidite",    humidite);
    Firebase.setString(firebaseData, "/alarme",      alarme);
    Serial.println("Données envoyées à Firebase !");
  } else {
    Serial.println("Firebase non prêt, nouvelle tentative...");
  }

  // Attendre 5 secondes
  delay(5000);
}