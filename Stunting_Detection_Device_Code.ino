void(*mulai_reset) (void) = 0;


// Wifi Module  ===================================================================

#include "WiFiEsp.h"

char ssid[] = "your_wifi_name";                // Isi dengan nama profil Wifi
char pass[] = "your_wifi_password";            // password wifi
char host[] = "your_server.com";               // alamat web hosting


//NEXTION ==========================================================================

#include <Nextion.h>
#define nexSerial Serial2
#define DEBUG_SERIAL_ENABLE
#define dbSerial Serial


int lock;
int online;
int waktu = 0;
char buffer[100] = {0};
bool Senddata;
char Nama[40];
char Umur[40];
char Gizi[40];
char Stunting[40];
int Kelamin;
int KepalaSD;
int PahaSD;
int KakiSD;

NexText tKepala = NexText(0, 10, "tKepala");
NexText tPaha = NexText(0, 7, "tPaha");
NexText tKaki = NexText(0, 8, "tKaki");
NexText tStatus = NexText(0, 9, "tStatus");
NexText tTinggi = NexText(0, 4, "tTinggi");
NexText tBerat = NexText(0, 3, "tBerat");
NexText tNama = NexText(0, 5, "tNama");
NexText tUmur = NexText(0, 6, "tUmur");

NexDSButton bt1 = NexDSButton(0, 15, "bt1");
NexDSButton bt0 = NexDSButton(0, 11, "bPerempuan");
NexDSButton bt2 = NexDSButton(0, 12, "bLaki");

NexText tLock = NexText(0, 19, "tLock");
NexText tLaki = NexText(0, 20, "tLaki");
NexText tPerempuan = NexText(0, 21, "tPerempuan");

NexText tGizi = NexText(0, 17, "tGizi");
NexText tStunting = NexText(0, 16, "tStunting");
NexButton bHitung = NexButton(0, 18, "bHitung");
NexButton bTare = NexButton(0, 22, "bTare");
NexButton bTarePanjang = NexButton(0, 24, "bTarePanjang");
NexButton bReset = NexButton(0, 23, "bReset");
NexButton bSubmit = NexButton(0, 13, "bSubmit");
NexButton bClear = NexButton(0, 26, "bClear");
NexText tOnline = NexText(0, 25, "tOnline");


NexTouch *nex_listen_list[] = {
  &tNama,
  &tUmur,
  &bSubmit,
  &bt1,
  &bt0,
  &bt2,
  &bHitung,
  &bTare,
  &bTarePanjang,
  &bReset,
  &bClear,
  NULL
};

// Perhitungan Gizi - Stunting
#include "math.h"
float Hitungumur; // berat badan
float x; // umur calculate
float x1; // panjang badan
float x2; // berat badan

float y; //avg median panjang umur
float y1; // panjang badan median
float z; // penyesuaian panjang badan
float z1; // avg median berat panjang
float z2; // berat median
float r1; // hasil stunting
float result_2; // hasil gizi buruk

// ESP  ==========================================================================

WiFiEspClient client;
int status = WL_IDLE_STATUS;

// Sensor Berat ==================================================================
#include "HX711.h"
#define DOUT  A0
#define CLK  A1
HX711 scale(DOUT, CLK);
float calibration_factor = 24.90;
float weightgram;
float weightkg;
int weight;
float deviasi;
int konversi;

/// SENSOR JARAK  ==================================================================
float potensio_panjang = A2;
float distancecm;
float distancemm;
float distance;
float nilai_potensio;
float base_panjang;


//SD CARD   =========================================================================
#include <SPI.h>
#include <SD.h>
File myFile;



void setup() {

  Serial.begin(9600);

//Nextion Setup ================================================================
  dbSerial.begin(9600);
  nexSerial.begin(9600);
  nexInit();
  bSubmit.attachPop(bSubmitPopCallback, &bSubmit);
  bHitung.attachPop(bHitungPopCallback, &bHitung);
  bTare.attachPop(bTarePopCallback, &bTare);
  bTarePanjang.attachPop(bTarePanjangPopCallback, &bTarePanjang);
  bReset.attachPop(bResetPopCallback, &bReset);
  bt1.attachPush(bt1PushCallback, &bt1);
  bt0.attachPush(bt0PushCallback, &bt0);
  bt2.attachPush(bt2PushCallback, &bt2);
  bClear.attachPop(bClearPopCallback, &bClear);


//SDcard Setup ================================================================
  
  Serial.print("Initializing SD card...");
  tStatus.setText("Initializing SD card...");
  
  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    tStatus.setText("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  tStatus.setText("SD initialization done!");
   
   
  //  //ESP Setup ================================================================

     Serial1.begin(115200);
     WiFi.init(&Serial1);
      // check for the presence of the shield
      if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("WiFi shield not present");
        // don't continue
        while (true);
      }
     
      // attempt to connect to WiFi network
//      while ( status != WL_CONNECTED) {
//        Serial.print("Attempting to connect to WPA SSID: ");
//        tStatus.setText("Connecting ");
//        Serial.println(ssid);
//        // Connect to WPA/WPA2 network
//        status = WiFi.begin(ssid, pass);
//      }
//      // you're connected now, so print out the data
//      Serial.println("You're connected to the network");
//      tStatus.setText("Connected");
//    //  printWifiStatus();

    // ====================================================
      do {
        Serial.print("Attempting to connect to WPA SSID: ");
        tStatus.setText("Connecting ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network
        status = WiFi.begin(ssid, pass);

        waktu ++;
        delay(100);
        Serial.println(waktu);
        
      } while (waktu < 2);
    
      if (status != WL_CONNECTED) {
        delay(100);
        tOnline.setText("OFFLINE MODE");
        delay(100);
        tOnline.Set_background_color_bco(1055);
      }
      else {
        delay(100);
        tOnline.setText("ONLINE MODE");
        delay(100);
        tOnline.Set_background_color_bco(2016);
      }


  //Sensor Berat Setup ================================================================
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);

  //Sensor Jarak Setup ================================================================

  base_panjang = analogRead(potensio_panjang);

  

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  

  //Body Detector Setup ================================================================

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);


}

void loop() {

  //Nextion
  nexLoop(nex_listen_list);


  //sensor berat loop --------------------------------------------------------
  if (lock == 0) {
    scale.set_scale(calibration_factor); //Adjust to this calibration factor

    weight = scale.get_units(5);
    weightgram = abs(weight);
    weightkg = weightgram/1000;
    //weightkg =(weightgram-deviasi)/1000 ;
    //Serial.print(scale.get_units(), 2);
    // Serial.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
    //weightkg = ((-0.0003 * (pow (deviasi, 2))) + (0,9866* deviasi) -  0.0233);//orde2
    
    Serial.print(weightgram);
    Serial.print("beratKG ");
    //Serial.print(weightkg);
    Serial.print(weightkg);
    Serial.print(" Kg");
    Serial.println();

    static char Berat[6];
    dtostrf(weightkg, 6, 2, Berat );
    tBerat.setText(Berat);
  }

  //sensor tinggi loop --------------------------------------------------------


  nilai_potensio = analogRead(potensio_panjang);
  //sensorVal = analogRead(sensorPin);
  Serial.print(nilai_potensio); Serial.print("  ");
  distance = map(nilai_potensio, base_panjang, base_panjang - 438, 0, 4275);
  //Serial.println((panjang / 100) + 55);


  distancecm = (distance / 100) + 53.5 ;
  distancemm = distancecm * 10 ;

  Serial.print("Tinggi: ");
  //Serial.print(distancecm);
  Serial.print(nilai_potensio
  );
  Serial.print("mm");
  delay(100);
  Serial.println();
  static char Tinggi[6];
  dtostrf(distancecm, 6, 2, Tinggi);
  tTinggi.setText(Tinggi);
  Serial.print(lock);

  //sensor sentuh loop --------------------------------------------------------

  int kepala = digitalRead(4);
  int paha = digitalRead(3);
  int kaki = digitalRead(2);


  if (kepala == LOW) {
    //tKepala.setText("detect");
    tKepala.Set_background_color_bco(2032);
    KepalaSD = 1;
    Serial.println("Kepala OK");
  }
  else {
    //tKepala.setText("no-detect");
    tKepala.Set_background_color_bco(63488);
    KepalaSD = 0;
    Serial.println("Kepala tidak OK");
  } 

  if (paha == HIGH) {
    //tPaha.setText("detect");
    tPaha.Set_background_color_bco(2032);
    PahaSD = 1;
    Serial.println("paha OK");
  }
  else {
    //tPaha.setText("no-detect");
    tPaha.Set_background_color_bco(63488);
    PahaSD = 0;
    Serial.println("paha tidak OK");
  }

  if (kaki == LOW) {
    //tKaki.setText("detect");
    tKaki.Set_background_color_bco(2032);
    PahaSD = 1;
    Serial.println("kaki  OK");
  }
  else {
    //tKaki.setText("no-detect");
    tKaki.Set_background_color_bco(63488);
    PahaSD = 0;

    Serial.println("kaki tidak OK");
  }



  if (kepala == LOW && paha == HIGH && kaki == LOW) {
    tStatus.setText("OK");
  }
  else {
    tStatus.setText("READY");
  }
  delay(100);


  if (status != WL_CONNECTED) {
        tOnline.setText("OFFLINE MODE");
        delay(100);
        tOnline.Set_background_color_bco(1055);
      }
      else {
        tOnline.setText("ONLINE MODE");
        delay(100);
        tOnline.Set_background_color_bco(2016);
      }
  //  Serial.println();
  //  Serial.println("closing connection");
  delay(1000);
}


void bTarePanjangPopCallback(void *ptr)
{
  base_panjang = analogRead(potensio_panjang);
}


void bTarePopCallback(void *ptr)
{
  scale.tare();
}

void bResetPopCallback(void *ptr)
{
  mulai_reset();
}

void bHitungPopCallback(void *ptr) {
  tUmur.getText(Umur, 40);
  Hitungumur = atof (Umur);
  // Hitungumur = 1 ;// umur asli
  x = Hitungumur + 1; // umur perhitungan
  x1 = distancecm; // panjang badan
  x2 = weightkg; // berat badan
  //Kelamin = 1;

  // perempuan ===============================================================================
  if (Kelamin == 0) {
    y = 1.8617 + (5.49123 *  pow(10, -2) * (x)); // avg median panjang umur

    // penentuan panjang median anak perempuan
    y1 = (-2.286 *  pow(10, -4) * (pow (x, 4))) + (1.454 *  pow(10, -2) * (pow (x, 3))) - (3.444 *  pow(10, -1) * (pow (x, 2))) + (4.7282 *  x) + 45.238;



    // hasil stunting anak perempuan

    r1 = (x1 - y1) / y ;


    // berat berdasarkan panjang anak perempuan

    z = (x1 - 45 + (5 *  pow(10, -1))) / (5 *  pow(10, -1)); //penyesuaian panjang badan

    z1 = (5 *  pow(10, -9) * (pow (z, 4))) - (6.426 *  pow(10, -7) * (pow (z, 3))) - (1.4013 *  pow(10, -6) * (pow (z, 2))) + (0.01206 *  z) + 0.193;

    z2 = (3 *  pow(10, -8) * (pow (z, 4))) - (4.41 *  pow(10, -6) * (pow (z, 3))) - (3.48 *  pow(10, -5) * (pow (z, 2))) + (0.1222 *  z) + 2.2142;

    result_2 = (x2 - z2) / z1 ;
    //r2 = -1.5;


    Serial.println("perempuan");
    Serial.println(z, 6);
    Serial.println(z1, 6);
    Serial.println(z2, 6);
    Serial.println(r1, 6);
    Serial.println(result_2, 6);

  }

  // laki-laki ===============================================================================
  if (Kelamin == 1) {
    y = (-5 *  pow(10, -6) * (pow (x, 4))) + (3 *  pow(10, -4) * (pow (x, 3))) - (4.1 *  pow(10, -3) * (pow (x, 2))) + (5.79 *  pow(10, -2) *  x) + 1.8522; // avg median panjang umur

    // penentuan panjang median anak laki
    y1 = (-2.2655 *  pow(10, -4) * (pow (x, 4))) + (1.7 *  pow(10, -2) * (pow (x, 3))) - (4.02 *  pow(10, -1) * (pow (x, 2))) + (5.262 *  x) + 45.476;



    // hasil stunting anak laki

    r1 = (x1 - y1) / y ;


    // berat berdasarkan panjang anak laki

    z = (x1 - 45 + (5 *  pow(10, -1))) / (5 *  pow(10, -1)); //penyesuaian panjang badan

    z1 = (1 *  pow(10, -8) * (pow (z, 4))) - (2.7 *  pow(10, -6) * (pow (z, 3))) + (1.738 *  pow(10, -4) * (pow (z, 2))) + (0.00607 *  z) + 0.224; // penentuan avg selisih berat

    z2 = (6 *  pow(10, -8) * (pow (z, 4))) - (1.1 *  pow(10, -5) * (pow (z, 3))) + (5.33 *  pow(10, -4) * (pow (z, 2))) + (0.1168 *  z) + 2.077;

    result_2 = (x2 - z2) / z1 ;
    //r2 = -1.5;


    Serial.println("laki");
    Serial.println(z, 6);
    Serial.println(z1, 6);
    Serial.println(z2, 6);
    Serial.println(r1, 6);
    Serial.println(result_2, 6);

  }

  // penentuan stunting
  if ( r1 <= -3) {
    Serial.println("Sangat pendek (severely stunted)");
    tStunting.setText("Sangat Stunting");
    tStunting.Set_background_color_bco(63488);
  }
  else if (r1 <= -2) {
    Serial.println("Pendek (stunted)");
    tStunting.setText("Pendek (Stunted)");
    tStunting.Set_background_color_bco(64520);
  }
  else if (r1 <= 3) {
    Serial.println("Tinggi normal");
    tStunting.setText("Tinggi normal");
    tStunting.Set_background_color_bco(2032);
  }
  else if (3 <= r1) {
    Serial.println("Tinggi");
    tStunting.setText("Tinggi");
    tStunting.Set_background_color_bco(2032);
  }
  // penentuan gizi
  if ( result_2 <= -3) {
    Serial.println("Gizi Buruk - Severely Wasted");
    tGizi.setText("Gizi Buruk");
    tGizi.Set_background_color_bco(63488);
  }
  else if (result_2 <= -2) {
    Serial.println("Gizi Kurang -  Wasted");
    tGizi.setText("Gizi Kurang");
    tGizi.Set_background_color_bco(64520);
  }
  else if (result_2 <= 1) {
    Serial.println("Normal");
    tGizi.setText("Berat Normal");
    tGizi.Set_background_color_bco(2032);
  }
  else if (result_2 <= 2) {
    Serial.println("Beresiko gizi berlebih");
    tGizi.setText("Risk gizi berlebih");
    tGizi.Set_background_color_bco(64520);
  }
  else if (result_2 <= 3) {
    Serial.println("Gizi lebih (Overweight) ");
    tGizi.setText("Gizi lebih");
    tGizi.Set_background_color_bco(64520);
  }
  if (3 <= result_2 ) {
    Serial.println("Obesitas");
    tGizi.setText("Obesitas");
    tGizi.Set_background_color_bco(63488);
  }

  //delay (2000);
}

//SD CARD loop --------------------------------------------------------
void bSubmitPopCallback(void *ptr) {
  tNama.getText(Nama, 40);
  tUmur.getText(Umur, 40);
  tGizi.getText(Gizi, 40);
  tStunting.getText(Stunting, 40);
  Serial.println(Nama);
  Serial.println(Umur);

  myFile = SD.open("BABY.txt", FILE_WRITE);
  tStatus.setText("Sending..");
  myFile.print("Nama= ");
  myFile.println(Nama);
  Serial.print("Nama= ");
  Serial.println(Nama);

  myFile.print("Umur= ");
  myFile.print(Umur);
  myFile.println(" bulan");
  Serial.print("Umur= ");
  Serial.print(Umur);
  Serial.println(" bulan");

  myFile.print("Berat= ");
  myFile.print(weightkg);
  myFile.println(" Kg");
  Serial.print("Berat= ");
  Serial.print(weightkg);
  Serial.println(" Kg");

  myFile.print("Panjang= ");
  myFile.print(distancemm);
  myFile.println(" cm");
  Serial.print("Panjang= ");
  Serial.print(distancemm);
  Serial.println(" cm");

  myFile.print("Hasil Panjang Badan= ");
  myFile.print(Stunting);
  myFile.println(" cm");
  Serial.print("Hasil Panjang Badan== ");
  Serial.print(Stunting);
  Serial.println(" cm");

  myFile.print("Hasil Berat Badan= ");
  myFile.print(Gizi);
  myFile.println(" Kg");
  Serial.print("Hasil Berat Badan== ");
  Serial.print(Gizi);
  Serial.println(" Kg");

  myFile.print("Kepala= ");
  myFile.print(KepalaSD);
  Serial.print("Kepala= ");
  Serial.print(KepalaSD);

  myFile.print("Paha= ");
  myFile.print(PahaSD);
  Serial.print("Paha= ");
  Serial.print(PahaSD);

  myFile.print("Kaki= ");
  myFile.print(KakiSD);
  myFile.println("");
  myFile.println("-----------------------");
  Serial.print("Kaki= ");
  Serial.print(KakiSD);



  myFile.close();


  // esp wifi loop --------------------------------------------------------

  Serial.print("connecting to ");
  Serial.println(host);

  WiFiEspClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/add.php?";
  url += "Nama=";
  url += Nama;
  url += "-";
  url += "Umur=";
  url += Umur;
  url += "-";
  url += "Berat=";
  url += weightgram;
  url += "-";
  url += "Tinggi=";
  url += distancemm;
  url += "-";
  url += "Gender=";
  url += Kelamin;
  url += "-";
  url += "Kepala=";
  url += KepalaSD;
  url += "-";
  url += "Betis=";
  url += PahaSD;
  url += "-";
  url += "Kaki=";
  url += KakiSD;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line);

    if (line.indexOf("sukses gaes") != -1) {
      Serial.println();
      Serial.println("Yes, data masuk");
      delay(100);
      tStatus.setText("Data Sent");
    } else if (line.indexOf("gagal gaes") != -1) {
      Serial.println();
      Serial.println("Maaf, data gagal masuk");
      delay(100);
      tStatus.setText("Data Failed");
      //digitalWrite(alarmPin, HIGH);
    }
  }

  Serial.println("terkirim");

  delay (1000);

}

void bt0PushCallback(void *ptr) {

  uint32_t puan_state;
  NexDSButton *btn = (NexDSButton *)ptr;
  Serial.print("bt Puan Jalan");
  Serial.print("ptr=");
  Serial.print((uint32_t)ptr);
  memset(buffer, 0, sizeof(buffer));

  /* Get the state value of dual state button component . */
  bt0.getValue(&puan_state);
  if (puan_state)
  {
    Kelamin = 0;
    delay(100);
    tLaki.Set_background_color_bco(63488);
    delay(100);
    tPerempuan.Set_background_color_bco(2032);
  }
      else
      {
          tPerempuan.Set_background_color_bco(63488);
          delay (100);
          Kelamin = 1;
      }

}


void bt1PushCallback(void *ptr)
{
  uint32_t lock_state;
  NexDSButton *btn = (NexDSButton *)ptr;
  Serial.print("bt Lock Jalan");
  Serial.print("ptr=");
  Serial.print((uint32_t)ptr);
  memset(buffer, 0, sizeof(buffer));

  /* Get the state value of dual state button component . */
  bt1.getValue(&lock_state);
  if (lock_state)
  {
    tLock.Set_background_color_bco(2032);
    delay (100);
    lock = 1;
  }
  else
  {
    tLock.Set_background_color_bco(63488);
    delay (100);
    lock = 0;
  }
}


void bt2PushCallback(void *ptr) {
  uint32_t laki_state;
  NexDSButton *btn = (NexDSButton *)ptr;
  Serial.print("bt Laki Jalan");
  Serial.print("ptr=");
  Serial.print((uint32_t)ptr);
  memset(buffer, 0, sizeof(buffer));

  /* Get the state value of dual state button component . */
  bt2.getValue(&laki_state);
  if (laki_state)
  {
    tLaki.Set_background_color_bco(2032);
    delay(100);
    tPerempuan.Set_background_color_bco(63488);
    delay(100);
    Kelamin = 1;

  }
      else
      {
          Kelamin = 0;
          delay (100);
          tLaki.Set_background_color_bco(63488);
          delay (100);
          tPerempuan.Set_background_color_bco(2032);
      }

}


void bClearPopCallback(void *ptr) {
  

  tNama.setText("");
  delay (100);
  tUmur.setText("");
  delay (100);
  tStunting.setText("");
  delay (100);
  tGizi.setText("");
  delay (100);
  tGizi.Set_background_color_bco(65535);
  delay (100);
  tStunting.Set_background_color_bco(65535);
  //mulai_reset();


}
