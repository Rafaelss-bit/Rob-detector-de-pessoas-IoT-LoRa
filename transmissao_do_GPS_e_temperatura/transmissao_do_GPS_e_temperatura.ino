
//Referencias:
//https://github.com/OtaviodaCruz  
//https://www.dragino.com


// Biblioteca
#include <TinyGPS.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <SoftwareSerial.h>
//https://github.com/adafruit/Adafruit-MLX90614-Library
#include <Adafruit_MLX90614.h>
// Cria um objeto gps
TinyGPS gps;
// Arduino RX, TX to conenct
SoftwareSerial ss(3, 4);
// Declara um objeto mlx da biblioteca Adafruit_MLX90614, que foi utilizado para usar-se de funções
prontas para o sensor de temperatura
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
static void smartdelay(unsigned long ms);
// Para contagem
unsigned int count = 0;
// VariÃ¡vel que armazenara a temperatura do objeto
float temp_obj;
// VariÃ¡veis:
float longitude,latitude;
float flat,flon,falt;
// Vetor de dados. Número inteiro de 8 bits
//Parece que a posiÃ§~]ao 1 e dois nunca são alteradas
static uint8_t mydata[14]
={0x03,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00};
// LoRaWAN NwkSKey, network session key
// This is the default Semtech key, which is used by the early prototype TTN network.
static const PROGMEM u1_t NWKSKEY[16] ={ 0x2A, 0x1B, 0x80, 0x8F, 0x0D, 0x51, 0x37, 0x60,
0xD1, 0x70, 0x4B, 0xC5, 0xC8, 0x0D, 0xFD, 0x13 };
// LoRaWAN AppSKey, application session key
// This is the default Semtech key, which is used by the early prototype TTN network.
static const u1_t PROGMEM APPSKEY[16] ={ 0x78, 0xE5, 0xE4, 0xFB, 0x55, 0x45, 0xAC, 0x3F,
0x0B, 0x63, 0xDC, 0x74, 0x82, 0x47, 0x45, 0xAF };
// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = 0x26021E1D; // <-- Change this address for every node!
// Esses retornos de chamada sÃ£o usados apenas na ativaÃ§Ã£o aÃ©rea, portanto, eles sÃ£o
// deixado vazio aqui (nÃ£o podemos deixÃ¡-los de fora completamente, a menos que
// DISABLE_JOIN seja definido em config.h, caso contrÃ¡rio, o vinculador reclamarÃ¡).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }
// osjob_t Ã© um tipo de variável criada na biblioteca
static osjob_t sendjob;
// Programe TX a cada TX_INTERVAL segundo (pode ficar mais longo devido a limitações do ciclo de
serviÃ§o).
const unsigned TX_INTERVAL = 0;
// Pin mapping, precisa definir
const lmic_pinmap lmic_pins = {
 .nss = 10,
 .rxtx = LMIC_UNUSED_PIN,
 .rst = 9,
 .dio = {2, 6, 7},
};
// Testa eventos
void onEvent (ev_t ev) {
 Serial.print(os_getTime());
 Serial.print(": ");
 switch(ev) {
 // EV_JOINING: O nÃ³ começou a ingressar na rede.
 case EV_JOINING:
 Serial.println(F("EV_JOINING"));
 break;
 // EV_JOINED: O nÃ³ ingressou na rede com sucesso e agora está¡ pronto para trocas de dados.
 case EV_JOINED:
 Serial.println(F("EV_JOINED"));
 break;
 // EV_JOIN_FAILED: O nÃ³ não pede ingressar na rede (apÃ³s tentar novamente).
 case EV_JOIN_FAILED:
 Serial.println(F("EV_JOIN_FAILED"));
 break;
 // EV_REJOIN_FAILED: O nÃ³ não ingressou em uma nova rede, mas ainda está¡ conectado
Ã rede antiga.
 case EV_REJOIN_FAILED:
 Serial.println(F("EV_REJOIN_FAILED"));
 break;
 // EV_TXCOMPLETE: Os dados preparados via LMIC_setTxData () foram enviados e,
eventualmente, os dados posteriores
 // foram recebidos em troca. Se a confirmação foi solicitada, a confirmação foi recebida.
 //Esse evemto Ã© o que geralmente cai
 case EV_TXCOMPLETE:
 Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
 //Se recebber algum ack
 if (LMIC.txrxFlags & TXRX_ACK)
 Serial.println(F("Received ack"));
 //Se receber alguma mensagem nesse tempo de espera
 if (LMIC.dataLen) {
 Serial.println(F("Received "));
 Serial.println(LMIC.dataLen);
 Serial.println(F(" bytes of payload"));
 }
 // next transmission
 os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
 break;
 // EV_RXCOMPLETE: Dados a jusante foram recebidos.
 case EV_RXCOMPLETE:
 // data received in ping slot
 Serial.println(F("EV_RXCOMPLETE"));
 break;
 default:
 Serial.println(F("Unknown event"));
 break;
 }
}
// World Geodetic System ==> Mars Geodetic System
// World Geodetic: Ã© uma norma usada em cartografia de origem geocÃªntrica utilizado pelo GNSS do
DoD,
// e pelo Sistema de Posicionamento Global - (GPS).
double transformLat(double x, double y)
{
 double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));
 ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;
 ret += (20.0 * sin(y * M_PI) + 40.0 * sin(y / 3.0 * M_PI)) * 2.0 / 3.0;
 ret += (160.0 * sin(y / 12.0 * M_PI) + 320 * sin(y * M_PI / 30.0)) * 2.0 / 3.0;
 return ret;
}
double transformLon(double x, double y)
{
 double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));
 ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;
 ret += (20.0 * sin(x * M_PI) + 40.0 * sin(x / 3.0 * M_PI)) * 2.0 / 3.0;
 ret += (150.0 * sin(x / 12.0 * M_PI) + 300.0 * sin(x / 30.0 * M_PI)) * 2.0 / 3.0;
 return ret;
}
// Transforme a coordenada entre a terra (WGS-84) e marte na china (GCJ-02).
// A coordenada GCJ-02 Ã© usada pelo Google Maps, Autonavi Map e outros serviÃ§os de mapas da
China. (O Baidu Map tem um deslocamento extra com base no GCJ-02)
// Essa funÃ§Ã£o passa de WGS para GCJ, pode ser retirada jÃ¡ que nÃ£o estou na china
// Ã‰ um dado geodÃ©sico formulado pelo EscritÃ³rio Estadual de InspeÃ§Ã£o e Mapeamento (
chinÃªs : Y æµ‹ å±€ ; pinyin : guÃ³-cÃ¨-jÃº ) e com base no WGS-84 .
// Utiliza um algoritmo de ofuscaÃ§Ã£o que adiciona compensaÃ§Ãµes aparentemente aleatÃ³rias
Ã latitude e longitude, com o suposto objetivo de melhorar a seguranÃ§a nacional.
// Existe uma taxa de licenÃ§a associada ao uso desse algoritmo obrigatÃ³rio na China.
void WGS2GCJTransform(float wgLon, float wgLat, float &mgLon, float &mgLat)
{
 const double a = 6378245.0;
 const double ee = 0.00669342162296594323;
 double dLat = transformLat(wgLon - 105.0, wgLat - 35.0);
 double dLon = transformLon(wgLon - 105.0, wgLat - 35.0);
 double radLat = wgLat / 180.0 * M_PI;
 double magic = sin(radLat);
 magic = 1 - ee * magic * magic;
 double sqrtMagic = sqrt(magic);
 dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * M_PI);
 dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * M_PI);
 mgLat = wgLat + dLat;
 mgLon = wgLon + dLon;
}
void TempRead()
{
 //Pega a temperatura do ambiente em graus celsius
 //temp_amb = mlx.readAmbientTempC();

 //Pega a temperatura do objeto em graus celsius
 temp_obj = mlx.readObjectTempC();
 //Confere se a temperatura adquirida pelo sensor está entre a faixa de temperatura do corpo humano
 if(temp_obj>32 && temp_obj<40)
 {
 // Caso esteja envia "1", para ficar picando no "cayenne"
 mydata[13] = 0x01;
 }
 else
 {
 // Caso nÃ£o esteja envia "0", nÃ£o ficar picando no "cayenne"
 mydata[13] = 0x00;
 }
}
void GPSRead()
{
unsigned long age;
//Pega a latitude e longitude
gps.f_get_position(&flat, &flon, &age);
//get altitude
falt=gps.f_altitude();
//save six decimal places longitude
flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;
 //save six decimal places latitude
flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
//save two decimal places altitude
falt == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : falt, 2;
if((flon < 72.004 || flon > 137.8347)&&(flat < 0.8293 || flat >55.8271)) //out of China
{
 longitude=flon;
 latitude=flat;
Serial.println("Out of China");
}
else
{
 WGS2GCJTransform(flon,flat,longitude,latitude);
 Serial.println("In China");
}
// Acredito que seja para ficar um numero inteiro
// 32 bits
int32_t lat = latitude * 10000;
int32_t lon = longitude * 10000;
int32_t alt = falt * 100;
// Exemplo de como funciona:
 //int a = 5; // binÃ¡rio: 0000000000000101
 //int b = a << 3; // binÃ¡rio: 0000000000101000 ou 40 em decimal, perde os da esquerda
 //int c = b >> 3; // binÃ¡rio: 0000000000000101 ou volta para 5 como comeÃ§amos com

 // As prÃ³ximas 3 linhas sÃ£o usadas para ignorar os 8 primeiros bits de lat. lembrando que zero a
esquerda "nÃ£o Ã© nada"
 // e que uint8_t pega oito bits, ao que parece pega da direita para esquerda


 Serial.print(lat); delay(10); Serial.println();
mydata[2] = lat >> 16;
mydata[3] = lat >> 8;
mydata[4] = lat;
// for (int i = 0; i < 5; i++){
//
// Serial.print(mydata[i]); delay(10); Serial.println();
//}
mydata[5] = lon >> 16;
mydata[6] = lon >> 8;
mydata[7] = lon;
mydata[8] = alt >> 16;
mydata[9] = alt >> 8;
mydata[10] = alt;
}
// Imprime resultados
void printdata(){
 Serial.print(F("########### "));
 Serial.print(F("NO."));
 Serial.print(count);
 Serial.println(F(" ###########"));
 //Successfully positioning
 //if(flon!=1000.000000)
//{
 Serial.println(F("A longtitude, latitude e altitude sÃ£o:"));
 Serial.print(F("["));
 Serial.print(longitude,4);
 Serial.print(F(","));
 Serial.print(latitude,4);
 Serial.print(F(","));
 Serial.print(falt);
 Serial.print(F("]"));
 Serial.println(F(""));
 Serial.println(F("Temperatura do objeto:"));
 Serial.println(temp_obj);

 count++;
//}
//else
// {
// Serial.println(F("Fail positioning"));
// }
}
// Onde ele define esse ms?
static void smartdelay(unsigned long ms)
{
Serial.println(ms);
//millis() retorna quantos segundos se passaram desde que a placa arduino comerão a executar o
//programa
unsigned long start = millis();
do
{
 // ss.available() retorna o número de bytes disponíveis para leitura
 while (ss.available())
 {
 // ss.read() retorna true se houver uma sentenÃ§a vÃ¡lida
 gps.encode(ss.read());
 }
// VÃª se jÃ¡ passou o tempo definido para uma nova leitura
} while (millis() - start < ms);
}
//Send
void do_send(osjob_t* j){
 // Check if there is not a current TX/RX job running
 if (LMIC.opmode & OP_TXRXPEND) {
 Serial.println(F("OP_TXRXPEND, not sending"));
 } else {
 //Define o tempo para o prÃ³ximo valor vÃ¡lido para o GPS
 smartdelay(1000);
 GPSRead();
 TempRead();
 printdata();
 // Prepare upstream data transmission at the next possible time.
 //LMIC_setTxData2 (u1_t port, xref2u1_t data, u1_t dlen, u1_t confirmed)
 //A porta u1_t Ã© o FPort usado para a transmissão. O padrÃ£o Ã© 1.
 //VocÃª pode enviar diferentes tipos de dados usando FPorts diferentes, para que o decodificador de
carga Ãºtil possa extrair as informações, dependendo da porta usada.
 //xref2u1_t data Ã© sua carga útil, então os dados que você deseja enviar
 //u1_t dlen Ã© o tamanho da sua carga Ãºtil
 //u1_t confirmado permitirá¡ que você ative um ACK para cada pacote enviado. 0 significa sem
ACK, 1 significa com ACK. Cada ACK custa um downlink!
 LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
 Serial.println(F("Pacote na fila"));
 }
 // Next TX is scheduled after TX_COMPLETE event.
}
void setup() {
 // O cÃ³digo LMIC Ã© bastante portÃ¡til.
 // O Ãºnico problema que tive foi ao usar um relÃ³gio menos preciso.
 // VocÃª pode tentar adicionar
 LMIC_setClockError(MAX_CLOCK_ERROR * 1/100);
 //Inicia o Serial monitor em 9600
 Serial.begin(9600);
 Serial.println(F("Starting"));
 //Inicia o SoftwareSerial em 9600
 ss.begin(9600);
 // NÃ£o entra, tem a ver com lmic_pinmap
 // ifdef se estiver definido
 #ifdef VCC_ENABLE
 pinMode(VCC_ENABLE, OUTPUT);
 digitalWrite(VCC_ENABLE, HIGH);
 delay(1000);
 Serial.println(F("tata"));
 #endif
 //Inicializa o MLX90614
 mlx.begin();
 // LMIC init
 os_init();
 // Redefina o estado do MAC. As transferências de dados pendentes e de sessão serão descartadas.
 LMIC_reset();
 // PROGMEM: Armazena dados na memÃ³ria flash (memÃ³ria de programa) em vez da SRAM. Uma
descriÃ§Ã£o dos vÃ¡rios tipos de memÃ³ria
 // das placas Arduino
 // parece que tem a ver com as variÃ¡veis DEVADDR, nwkskey, appskey
 #ifdef PROGMEM
 // On AVR, these values are stored in flash and only copied to RAM
 // once. Copy them to a temporary buffer here, LMIC_setSession will
 // copy them into a buffer of its own again.
 uint8_t appskey[sizeof(APPSKEY)];
 uint8_t nwkskey[sizeof(NWKSKEY)];
 memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
 memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
 // Inicia uma sessÃ£o LMIC
 LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
 #else
 // If not running an AVR with PROGMEM, just use the arrays directly
 // primeiro argumento netid
 LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
 #endif
 #if defined(CFG_eu868)
 // Set up the channels used by the Things Network, which corresponds
 // to the defaults of most gateways. Without this, only three base
 // channels from the LoRaWAN specification are used, which certainly
 // works, so it is good for debugging, but can overload those
 // frequencies, so be sure to configure the full frequency range of
 // your network here (unless your network autoconfigures them).
 // Setting up channels should happen after LMIC_setSession, as that
 // configures the minimal channel set.
 // NA-US channels 0-71 are configured automatically
 LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); //
g-band
 LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B),
BAND_CENTI); // g-band
 LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); //
g-band
 LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); //
g-band
 LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); //
g-band
 LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); //
g-band
 LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); //
g-band
 LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); //
g-band
 LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI); //
g2-band
 // TTN defines an additional channel at 869.525Mhz using SF9 for class B
 // devices' ping slots. LMIC does not have an easy way to define set this
 // frequency and support for class B is spotty and untested, so this
 // frequency is not configured here.
 #elif defined(CFG_us915)
 Serial.println(F("CFG_us915"));
 // Os canais NA-US 0-71 sÃ£o configurados automaticamente mas apenas um grupo de 8 deve (uma
sub-banda) deve estar ativo. TTN recomenda a segunda sub-banda, 1 em uma contagem baseada em zero.
 // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
 // NÃ£o parece precisar dessa funÃ§Ã£o
 LMIC_selectSubBand(1);
 #endif
 // Disable link check validation
 LMIC_setLinkCheckMode(0);
 // TTN uses SF9 for its RX2 window.
 LMIC.dn2Dr = DR_SF9;
 // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
 LMIC_setDrTxpow(DR_SF7,14);
 // Start job
 do_send(&sendjob);
}
void loop() {
 os_runloop_once();
}
 Código do gateway, que 
