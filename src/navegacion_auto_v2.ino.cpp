# 1 "C:\\Users\\ivanb\\AppData\\Local\\Temp\\tmpib4nn7w3"
#include <Arduino.h>
# 1 "C:/Users/ivanb/Documents/PlatformIO/Projects/Taller-Robotica-PIO/src/navegacion_auto_v2.ino"
#include <ESP8266WiFi.h>
#define motor_der_a D8
#define motor_der_b D7
#define motor_izq_a D6
#define motor_izq_b D5
#define pinLed D4
#define Trigger D2
#define Echo D1
#define pinVoltage A0
#define tiempoAvance 5000
#define umbral 15
#define tiempoGiroAnti 1200
#define tiempoGiroHor 1100
const int v_max = 1;
const int v_min = 0.25;
const float R1 = 10000.0;
const float R2 = 5000.0;
bool reversa = false;
const char* nombre_wifi = "Robotin";
const char* contra = "robotin123";
long duracion;
int distancia;
bool manual = true;
int v;
int i;





IPAddress ip_rover(192,168,4,1);
IPAddress puerta_enlace(192,168,4,1);
IPAddress mascara(255,255,255,0);


struct motor {
  int pin1;
  int pin2;
};

motor motor_izq {motor_izq_a,motor_izq_b};
motor motor_der {motor_der_a,motor_der_b};
void VoltBateria ();
void setearVel (int pos);
int vActual ();
void configurarMotor (motor m);
void girarAdelante (motor m);
void girarAtras (motor m);
void frenar (motor m);
void prenderLed ();
void apagarLed ();
int calcularDistancia();
void girarIzq ();
void girarDer ();
void avanzar ();
void detenerse ();
void marchaAtras ();
void girarIzqMarchaAtras ();
void girarHorario ();
void girarAntiHorario ();
void girarDerMarchaAtras ();
bool evadirIzq ();
bool evadirDer ();
void giro180 ();
void navegacionAutomatica();
void manejarCliente(WiFiClient &cliente);
bool clienteNuevo (WiFiClient &cliente);
void setup();
void loop();
#line 44 "C:/Users/ivanb/Documents/PlatformIO/Projects/Taller-Robotica-PIO/src/navegacion_auto_v2.ino"
void VoltBateria () {

  int val = analogRead (A0);
  Serial.print("Valor digital batería: ");
  Serial.println (val);
  Serial.print("Volts batería: ");
  Serial.println(val * 0.1144);
# 61 "C:/Users/ivanb/Documents/PlatformIO/Projects/Taller-Robotica-PIO/src/navegacion_auto_v2.ino"
}
# 101 "C:/Users/ivanb/Documents/PlatformIO/Projects/Taller-Robotica-PIO/src/navegacion_auto_v2.ino"
int velocidades[5] = {10,15,30,50,100};
WiFiServer server(80);
int estadoLed = LOW;
const long intervalo = 200;
unsigned long millisAnterior = 0;

void setearVel (int pos) {
  v = velocidades[pos];
  Serial.print ("Velocidad: ");
  Serial.println (1023*v/100);

}

int vActual () {
  return v;
}


void configurarMotor (motor m) {
  pinMode(m.pin1,OUTPUT);
  pinMode(m.pin2,OUTPUT);
}

void girarAdelante (motor m) {
  digitalWrite(m.pin2,LOW);
  analogWrite(m.pin1,1023*v/100);
}

void girarAtras (motor m) {
  digitalWrite(m.pin1,LOW);
  analogWrite(m.pin2,1023*v/100);
}

void frenar (motor m) {
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

 digitalWrite(Trigger, HIGH);
 delayMicroseconds(10);
 digitalWrite(Trigger, LOW);

 duracion = pulseIn(Echo, HIGH);
 distancia = duracion*0.0343/2;
 Serial.println("distancia a obstáculo: " + String(distancia));
 return distancia;
}




void girarIzq () {
  frenar (motor_izq);
  girarAdelante (motor_der);
  prenderLed ();
  Serial.println("gira hacia la izquierda");
}

void girarDer () {
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

void girarIzqMarchaAtras () {
  girarAtras (motor_der);
  frenar (motor_izq);
  prenderLed ();
  Serial.println("gira marcha atrás hacia la izquierda");
}

void girarHorario () {
  int a = v;
  Serial.println(v*1023/100);
  setearVel (0);
  girarAtras (motor_der);
  girarAdelante (motor_izq);
  prenderLed ();
  v = a;
  Serial.println(v*1023/100);
  Serial.println("gira en sentido horario");
}

void girarAntiHorario () {
  int a = v;
  Serial.println(a*1023/100);
  setearVel (0);
  girarAtras (motor_izq);
  girarAdelante (motor_der);
  prenderLed ();
  v = a;
  Serial.println(v*1023/100);
  Serial.println("gira en sentido antihorario");
}

void girarDerMarchaAtras () {
  girarAtras (motor_izq);
  frenar (motor_der);
  prenderLed ();
  Serial.println("gira marcha atrás hacia la derecha");
}
# 372 "C:/Users/ivanb/Documents/PlatformIO/Projects/Taller-Robotica-PIO/src/navegacion_auto_v2.ino"
bool evadirIzq () {
  bool frenteLibre;
  girarAntiHorario ();
  delay (tiempoGiroAnti);
  detenerse();
  delay(100);
  if (calcularDistancia() < umbral) {
    frenteLibre = false;
    girarHorario ();
    delay (tiempoGiroHor);
    detenerse ();
    delay(100);
  } else {
    frenteLibre = true;
  }
  return frenteLibre;
}

bool evadirDer () {
  bool frenteLibre;
  girarHorario ();
  delay (tiempoGiroHor);
  detenerse();
  delay(100);
  if (calcularDistancia() < umbral) {
    frenteLibre = false;
    girarAntiHorario ();
    delay (tiempoGiroAnti);
    detenerse ();
    delay(100);
  } else {
    frenteLibre = true;
  }
  return frenteLibre;
}

void giro180 () {
  detenerse();
  delay(100);
  girarHorario ();
  delay(2*tiempoGiroHor);
  detenerse ();
  Serial.println("gira 180");
}

void navegacionAutomatica() {
  if (calcularDistancia() > umbral) {
    avanzar();
    delay(10);
  }
  else {
    detenerse();
    delay (100);
    marchaAtras ();
    delay (300);
    detenerse();
    i = random(0,2);
    if (i == 0) {
      if (!evadirIzq()) {
        detenerse();
        delay(500);
        if (!evadirDer ()) {
          detenerse();
          delay(500);
          marchaAtras();
          delay(300);
          giro180();
        }
      }
    } else {
      if (!evadirDer()) {
        detenerse();
        delay(500);
        if (!evadirIzq()) {
          detenerse();
          delay(500);
          marchaAtras();
          delay(300);
          giro180();
        }
      }

    }
  }
}



void manejarCliente(WiFiClient &cliente) {
  while (!cliente.available()) {
    delay(1);
  }
  String request = cliente.readStringUntil('\n');
  if (request.indexOf("/v1") != -1) {
    Serial.println("velocidad 1");
    cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>velocidad 1</body></html>\n");
    setearVel (0);
  }
  if (request.indexOf("/v2") != -1) {
    Serial.println("velocidad 2");
    cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>velocidad 2</body></html>\n");
    setearVel (1);
  }
  if (request.indexOf("/v3") != -1) {
    Serial.println("velocidad 3");
    cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>velocidad 3</body></html>\n");
    setearVel (2);
  }
  if (request.indexOf("/v4") != -1) {
    Serial.println("velocidad 4");
    cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>velocidad 4</body></html>\n");
    setearVel (3);
  }
  if (request.indexOf("/v5") != -1) {
    Serial.println("velocidad 5");
    cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>velocidad 5</body></html>\n");
    setearVel (4);
  }
  if (request.indexOf("/giro90hor") != -1) {
    Serial.println("gira 90 grados en sentido horario");
    cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>gira 90 grados en sentido horario</body></html>\n");
    girarHorario();
    delay(tiempoGiroHor);
    detenerse();
  }

  if (request.indexOf("/giro90anti") != -1) {
    Serial.println("gira 90 grados en sentido antihorario");
    cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>gira 90 grados en sentido antihorario</body></html>\n");
    girarAntiHorario();
    delay(tiempoGiroAnti);
    detenerse();
  }

  if (request.indexOf("/desacelerar") != -1) {
    Serial.println("desacelera");
    cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>desacelera</body></html>\n");

  }
  if (request.indexOf("/cambiodemodo") != -1) {
    reversa = false;
    manual = !manual;
    Serial.println("cambio de modo");
    cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>cambio de modo</body></html>\n");
    detenerse();
  }
    if (request.indexOf("/manual") != -1) {
      reversa = false;
      manual = !manual;
      Serial.println("modo manual");
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>modo manual</body></html>\n");
      detenerse();
    }
    if (request.indexOf("/automatico") != -1) {
      reversa = false;
      manual = !manual;
      Serial.println("modo automatico");
      cliente.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n\n<html><body>modo automático</body></html>\n");
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
  detenerse ();
  v = velocidades[2];
  configurarMotor (motor_izq);
  configurarMotor (motor_der);
  WiFi.softAPConfig(ip_rover, puerta_enlace, mascara);
  WiFi.softAP(nombre_wifi, contra);
  server.begin();
  pinMode(pinLed,OUTPUT);
  pinMode(Trigger, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(A0,OUTPUT);
  detenerse();
  manual = true;
  for (int c = 0;c<4;c++) {
    prenderLed ();
    delay (150);
    apagarLed();
    delay (150);
  }
  Serial.begin(115200);
  Serial.println("\n\n\n===== NUEVA LECTURA =====\n\n\n");
  Serial.println("punto de acceso iniciado");
  Serial.print("ip: ");
  Serial.println(WiFi.softAPIP());
  detenerse ();
}
void loop() {
  WiFiClient cliente = server.accept();
  if (cliente) {
    if (clienteNuevo(cliente)) {
      Serial.print("nuevo cliente, su ip es: ");
      Serial.println(cliente.remoteIP());
    }

    manejarCliente (cliente);
  }

  if (!manual) {
    navegacionAutomatica ();
  }

}