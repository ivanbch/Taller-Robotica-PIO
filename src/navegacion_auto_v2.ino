#include <ESP8266WiFi.h>
#define motor_izq_a   D8
#define motor_izq_b   D7
#define motor_der_a   D6
#define motor_der_b   D5
#define pinLed        D4
#define Trigger       D2
#define Echo          D1
#define tiempoGiro    500
#define tiempoAvance  1500
#define umbral        15
bool reversa = false;
const char* nombre_wifi = "Robotin";
const char* contra = "robotin123";
long duracion;
int distancia;
bool manual = true; 
/*

creo que lo mejor es usar el rover como Access Point

*/
IPAddress ip_rover(192,168,4,1);       
IPAddress puerta_enlace(192,168,4,1);        // Gateway (igual que la IP en modo punto de acceso)
IPAddress mascara(255,255,255,0);        // Máscara de subred

//IPAddress ip_rover (192,168,253,10);        // (si quiero usar el hotspot como router)
//IPAddress ip_rover (192,168,137,10);        // (si quiero usar mi pc como router)
//IPAddress puerta_enlace (192,168,253,154); // (si quiero usar el hotspot como router)
//IPAddress puerta_enlace (192,168,137,1); // (si quiero usar mi pc como router)
//IPAddress mascara(255,255,255,0);

struct motor {
  int pin1;
  int pin2;
};

motor motor_izq {motor_izq_a,motor_izq_b};
motor motor_der {motor_der_a,motor_der_b};

WiFiServer server(80);
int estadoLed = LOW;
const long intervalo = 200;
unsigned long millisAnterior = 0;

// funciones de motor
void configurarMotor (motor m) {
  pinMode(m.pin1,OUTPUT);
  pinMode(m.pin2,OUTPUT);
}

void girarAdelante (motor m) {  //esto es para controlar el giro de un motor en específico hacia adelante, habría que confirmar
  digitalWrite(m.pin1,HIGH);           // que sea el correcto, sino reasigno los pines en la instancia del motor y listo
  digitalWrite(m.pin2,LOW);
}

void girarAtras (motor m) {
  digitalWrite(m.pin1,LOW);
  digitalWrite(m.pin2,HIGH);
}

void frenar (motor m) { //frena UN motor solo
  digitalWrite(m.pin1,LOW);
  digitalWrite(m.pin2,LOW);
}

void prenderLed () {
  digitalWrite(pinLed,LOW);
}

void apagarLed () {
  digitalWrite(pinLed,HIGH);
}

int calcularDistancia() {
 digitalWrite(Trigger, LOW);
 delayMicroseconds(2);
//Configurar trigPin en alto por 10 microsegundos
 digitalWrite(Trigger, HIGH);
 delayMicroseconds(10);
 digitalWrite(Trigger, LOW);
// Leer el pin echo
 duracion = pulseIn(Echo, HIGH);
 distancia = duracion*0.0343/2; //ojo con las unidades
 Serial.println("distancia a obstáculo: " + String(distancia));
 return distancia;
}


int promedioDistancia () {  //como el sensor es muy sensible, voy a hacer un promedio de 3 mediciones antes de compararlas
  int sum = 0;                                                                                              //con el umbral
  for(int p = 0; p < 3;p++) {
    sum = sum += calcularDistancia();
    delay(60);  //según recomendaciones tecnicas
  }
  int prom = sum / 3;
  return prom;
}

/*
Funciones de navegación básica
*/
void girarIzq () { //rover gira hacia la izquierda; revisar sentido
  frenar (motor_izq);
  girarAdelante (motor_der);
  prenderLed ();
  Serial.println("gira hacia la izquierda");
}

void girarDer () { //lo mismo pero hacia la derecha
  frenar (motor_der);
  girarAdelante (motor_izq);
  prenderLed ();
  Serial.println("gira a la derecha");
}

void avanzar () {
  girarAdelante (motor_izq);
  girarAdelante (motor_der);
  prenderLed ();
  Serial.println("avanza");
} 

void detenerse () {
  frenar (motor_izq);
  frenar (motor_der);
  apagarLed ();
  Serial.println("se detiene");
} 

void marchaAtras () {
  girarAtras (motor_izq);
  girarAtras (motor_der);
  prenderLed ();
  Serial.println("marcha atrás");
}

void girarIzqMarchaAtras () { //rover gira hacia la izquierda marcha atrás; revisar sentido
  girarAtras (motor_der);
  frenar (motor_izq);
  prenderLed ();
  Serial.println("gira marcha atrás hacia la izquierda");
}

void girarHorario () { //rover gira hacia la izquierda marcha atrás; revisar sentido
  girarAtras (motor_der);
  girarAdelante (motor_izq);
  prenderLed ();
  Serial.println("gira en sentido horario");
}

void girarAntiHorario () { //rover gira hacia la izquierda marcha atrás; revisar sentido
  girarAtras (motor_izq);
  girarAdelante (motor_der);
  prenderLed ();
  Serial.println("gira en sentido antihorario");
}

void girarDerMarchaAtras () { //rover gira hacia la derecha marcha atrás; revisar sentido
  girarAtras (motor_izq);
  frenar (motor_der);
  prenderLed ();
  Serial.println("gira marcha atrás hacia la derecha");
}

// funciones autónomas

bool girar90Izq () {
  int i = 0;
  detenerse();
  delay(500);
  girarAntiHorario ();
  while (i < tiempoGiro) {       //tiempo para que gire 90 grados en ms, fijarse
    delay (100);
    i += 100;
  }
  i = 0;
  detenerse ();
  delay(500);
  int distancia = promedioDistancia ();
  bool frenteLibre = false;
  if (distancia > umbral) {
    while (distancia > umbral && i < tiempoAvance && frenteLibre == false) {  
      if (i%500 == 0) {    //explicado en la función de abajo
        girarHorario ();
        delay(tiempoGiro);
        detenerse ();
        delay(500);
        distancia = promedioDistancia ();
        if (distancia > umbral) {
          frenteLibre = true;
        } else {
          girarAntiHorario ();
          delay (tiempoGiro);
          detenerse();
          delay(500);
        }
      }
      if (frenteLibre == false) {
        avanzar ();
        delay (100);
        i += 100;
        distancia = calcularDistancia ();
      }
      
    }
    if (frenteLibre == false) {   
      int tAvanz= i;       //avanzó una vez que dobló pero era muy poco (depende del ancho del objeto)
      i = 0;
      detenerse ();
      delay(500);
      marchaAtras ();
      while (i < tAvanz) {    
        delay (100);
        i += 100;
      }
      detenerse ();
      delay(500);
      i = 0;
      girarHorario ();   
      while (i < tiempoGiro) {
        delay (100);
        i += 100;
      }
      detenerse();
    } 
  } else {        
      detenerse ();
      delay(500);
      i = 0;
      girarHorario ();                 //deshace el giro (no llegó a avanzar)
      while (i < tiempoGiro) {  
        delay (100);
        i += 100;
      }
      detenerse();
    }
  if (frenteLibre == true) {
    Serial.println("giró bien izq 90");
  } else {
    Serial.println("NO giró izq 90");
  }
  return frenteLibre;
}

bool girar90Der () {
  int i = 0;
  detenerse();
  delay (500);
  girarHorario ();
  while (i < tiempoGiro) {       //tiempo para que gire 90 grados en ms, fijarse
    delay (100);
    i += 100;
  }
  i = 0;
  detenerse();
  delay (500);
  int distancia = promedioDistancia ();
  bool frenteLibre = false;
  if (distancia > umbral) {
    while (distancia > umbral && i < tiempoAvance && frenteLibre == false) {  
      if (i%500 == 0) {   //como tiene un radar en el frente nada más, cada medio segundo gira 90 grados para quedar en la dirección original,
        girarAntiHorario ();   //analiza si el frente está libre, si no lo está gira 90 grados para el otro lado, y sigue avanzando
        delay(tiempoGiro);              //paralelo al obstáculo hasta que cumpla el tiempoAvance máximo. 
        detenerse();      
        delay(500);            
        distancia = promedioDistancia ();
        if (distancia > umbral) {
          frenteLibre = true;
        } else {
          girarHorario ();
          delay (tiempoGiro);
          detenerse();
          delay(500);
        }
      }
      if (frenteLibre  == false) {
        avanzar ();
        delay (100);
        i += 100;
        distancia = calcularDistancia ();
      }
    }
    if (frenteLibre == false) {      //retrocede porque llegó a avanzar hacia el costado
      int tAvanz= i;    //tiempo que avanzó
      i = 0;
      detenerse();
      delay (500);
      marchaAtras ();
      while (i < tAvanz) {    
        delay (100);
        i += 100;
      }
      i = 0;
      detenerse();
      delay (500);
      girarAntiHorario ();   
      while (i < tiempoGiro) {
        delay (100);
        i += 100;
      }
      detenerse();
    } 
  } else {        
      i = 0;
      detenerse();
      delay (500);      
      girarAntiHorario ();           //deshace el giro (no llegó a avanzar)
      while (i < tiempoGiro) {
        delay (100);
        i += 100;
      }
      detenerse();
    }
  if (frenteLibre == true) {
    Serial.println("giró bien der 90");
  } else {
    Serial.println("NO giró der 90");
  }
  return frenteLibre;
}
/*
bool esquivarXIzq () {
  bool pudo = true;
  int i = 0;
  int distancia = promedioDistancia ();
  detenerse();
  delay (500);
  bool giro90 = girar90Izq ();
  if (giro90) {
    while (i < tiempoGiro) {
      girarHorario ();
      delay (100);
      i += 100;
    }
    distancia = promedioDistancia();
    if (distancia < umbral) {
      pudo = false;
      i = 0;
      detenerse();
      delay (500);
      while (i < tiempoGiro) {            // creo que haría la L invertida
        marchaAtras ();
        delay (100);
        i += 100;
      }
      detenerse();
      delay (500);
      i = 0;
      while (i < tiempoGiro) {
        girarAntiHorario ();
        delay (100);
        i += 100;
      }
      detenerse();
      delay (500);
      i = 0;
      while (i < tiempoGiro) {
        marchaAtras ();
        delay (100);
        i += 100;
      }
      detenerse();
      delay (500);
      i = 0;
      while (i < tiempoGiro) {
        girarHorario ();
        delay (100);
        i += 100;
      }
      detenerse();
      delay (800);
    }
  } else {
    pudo = false;
  }
  if (pudo == true) {
    Serial.println("esquivó bien x izq");
  } else {
    Serial.println("NO esquivó x izq");
  } 
 return pudo; 
} 

bool esquivarXDer () {
  bool pudo = true;
  int i = 0;
  int distancia = promedioDistancia ();
  detenerse();
  delay (500);
  bool giro90 = girar90Der ();
  if (giro90) {
    while (i < tiempoGiro) {
      girarAntiHorario ();
      delay (100);
      i += 100;
    }
    distancia = promedioDistancia ();
    if (distancia < umbral) {
      pudo = false;
      detenerse();
      delay (500);
      i = 0;
      while (i < tiempoGiro) {            // creo que haría la L invertida
        marchaAtras ();
        delay (100);
        i += 100;
      }
      detenerse();
      delay (500);
      i = 0;
      while (i < tiempoGiro) {
        girarHorario ();
        delay (100);
        i += 100;
      }
      detenerse();
      delay (500);
      i = 0;
      while (i < tiempoGiro) {
        marchaAtras ();
        delay (100);
        i += 100;
      }
      detenerse();
      delay (500);
      i = 0;
      while (i < tiempoGiro) {
        girarAntiHorario ();
        delay (100);
        i += 100;
      }
      detenerse();
      delay (800);
    }
  } else {
    pudo = false;
  } 
  if (pudo == true) {
    Serial.println("esquivó bien x der");
  } else {
    Serial.println("NO esquivó x der");
  }
 return pudo; 
}

*/
void giro180 () {
  int i = 0;
  detenerse();
  delay(500);
  girarHorario ();
  while (i < 2*tiempoGiro) { //tiempo giro 180 grados, calcular
    delay (100);
    i += 100;
  }
  Serial.println("gira 180");
}

void navegacionAutomatica() {
  distancia = calcularDistancia();
  if (distancia > umbral) {   
    avanzar();
    delay(50);
  } else {
    detenerse();
    delay(500); //importante!!!! que frene antes de girar 
    bool evadirDer= girar90Der();
    if (evadirDer == false) {
      detenerse();
      delay(500);
      bool evadirIzq = girar90Izq();
      if (evadirIzq == false) {
        detenerse();
        delay(500);
        marchaAtras();
        delay(500);
        giro180();
      }
    }
  }
}


//conexión a wifi

void manejarCliente(WiFiClient &cliente) {
  while (!cliente.available()) {   
    delay(1);
  }
  String request = cliente.readStringUntil('\n');
  if (request.indexOf("/cambiodemodo") != -1) {
    reversa = false;
    manual = !manual;
    Serial.println("cambio de modo");
    cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>cambio de modo</body></html>\n");
    detenerse();
  }

  if (manual) {
    if (request.indexOf("/avanzar") != -1) {
      reversa = false;
      avanzar();
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>avanza</body></html>\n");
    }
    if (request.indexOf("/doblarder") != -1) {
      reversa = false;
      girarDer();
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>gira a la derecha</body></html>\n");
    }
    if (request.indexOf("/doblarizq") != -1) {
      reversa = false;
      girarIzq();
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>gira a la izquierda</body></html>\n");
    }
    if (request.indexOf("/detener") != -1) {
      reversa = false;
      detenerse();
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>se detiene</body></html>\n");
    }
    if (request.indexOf("/reversa") != -1) {
      reversa = true;
      marchaAtras();
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>marcha atras</body></html>\n");
    }
    if (request.indexOf("/doblardratras") != -1) {
      reversa = true;
      girarDerMarchaAtras();
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>dobla a la derecha marcha atras</body></html>\n");
    }
    if (request.indexOf("/doblariatras") != -1) {
      reversa = true;
      girarIzqMarchaAtras();
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>dobla a la izquierda marcha atras</body></html>\n");
    }
    if (request.indexOf("/girohor") != -1) {
      reversa = false;
      girarHorario();
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>gira en sentido horario</body></html>\n");
    }
    if (request.indexOf("/giroantihor") != -1) {
      reversa = false;
      girarAntiHorario();
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>gira en sentido antihorario</body></html>\n");
    }
  }
  cliente.flush();
}
byte listaIp[5] = {0,0,0,0,0};
// creo una lista del ultimo octeto de la ip de CADA cliente, o sea ipcliente[3], para guardar los clientes y mostrar cuando uno nuevo se conecte;
 //se manejan en  bloques de bytes las ip, son como un array[4] de bytes, guardo SOLO el ultimo octeto
bool clienteNuevo (WiFiClient &cliente) {
  bool esNuevo = true;
  int i = 0;
  while (i < 5) {
    if (cliente.remoteIP()[3] == listaIp[i]) {
      esNuevo = false;
    } 
    i++;
  }
  if (esNuevo) {
    int x = 0;
    while (listaIp[x] != 0) {
      x++;
    }
    listaIp[x] = cliente.remoteIP()[3];
  }
    return esNuevo;
}
  
void setup() {
  configurarMotor (motor_izq);
  configurarMotor (motor_der);
  detenerse ();
  Serial.begin(115200);
  WiFi.softAPConfig(ip_rover, puerta_enlace, mascara);
  WiFi.softAP(nombre_wifi, contra);
  server.begin();
  pinMode(pinLed,OUTPUT);
  pinMode(Trigger, OUTPUT);
  pinMode(Echo, INPUT);
  manual = true;
  for (int c = 0;c<4;c++) { //para que no arranque tan rápido el serial y todo eso
    prenderLed ();
    delay (800);
    apagarLed();
    delay(800);
  }
  Serial.println("\n\n\n===== NUEVA LECTURA =====\n\n\n");
  Serial.println("punto de acceso iniciado");
  Serial.print("ip: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  WiFiClient cliente = server.accept();  //supuestamente se usa accept ahora y no available
  if (cliente) {
    if (clienteNuevo(cliente)) {  //si es un cliente nuevo imprime su ip
      Serial.print("nuevo cliente, su ip es: ");
      Serial.println(cliente.remoteIP());
    }
    
    manejarCliente (cliente);
  }    
  if (!manual) {
    navegacionAutomatica ();
  } 
}