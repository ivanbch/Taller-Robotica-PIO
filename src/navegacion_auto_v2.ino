#include <ESP8266WiFi.h>
#define motor_der_a    D8
#define motor_der_b    D7
#define motor_izq_a    D6
#define motor_izq_b    D5
#define pinLed         D4
#define Trigger        D2
#define Echo           D1
#define pinVoltage     A0
#define tiempoAvance   5000
#define umbral         15
#define tiempoGiroAnti 1000
#define tiempoGiroHor 1100
const int v_max = 1;  // Velocidad tope (la más rápida)
const int v_min = 0.25;  // Velocidad más lenta
const float R1 = 10000.0; //  ohms
const float R2 = 5000.0;  //  ohms
bool reversa = false;
const char* nombre_wifi = "Robotin";
const char* contra = "robotin123";
long duracion;
int distancia;
bool manual = true; 
int v; //es un porcentaje de la velocidad maxima
int i;
/*

creo que lo mejor es usar el rover como Access Point

*/
IPAddress ip_rover(192,168,4,1);       
IPAddress puerta_enlace(192,168,4,1);        // Gateway (igual que la IP en modo punto de acceso)
IPAddress mascara(255,255,255,0);        // Máscara de subred


struct motor {
  int pin1;
  int pin2;
};

motor motor_izq {motor_izq_a,motor_izq_b};
motor motor_der {motor_der_a,motor_der_b};

void VoltBateria () {
  // si 1023 es 8.4 -> valor = analogWrite (A0) * 8.4/1023 
  int val = analogRead (A0);
  Serial.print("Valor digital batería: ");
  Serial.println (val);
  Serial.print("Volts batería: ");
  Serial.println(val * 0.1144); // Lee el valor analógico del pin A0
  
  /*
  float voltajePin = valorADC * (3.3 / 1023.0); // Convierte a voltaje ya que el rango es 0-1V, supuestamente la d1 mini tiene resist internas que hacen que con 3.3V llegue 1V
  float voltajeBateria = voltajePin * ((R1 + R2) / R2);
  Serial.print ("Batería: ");
  Serial.println (voltajeBateria);
  Serial.print ("Porcentaje de batería: ");
  Serial.println (voltajeBateria);
  return voltajeBateria;
  */
}
/*
void acelerar () {
  if (v < 0.9) {
    v = v + 0.20;
  }
  tiempoGiroHor = tiempoGiroHorOrig / v;
  tiempoGiroAnti = tiempoGiroAntiOrig / v;
  Serial.print("Velocidad: ");
  Serial.println (int(1023*v));
  Serial.print ("Tiempo giro horario 90 grados: ");
  Serial.println (tiempoGiroHor);
  Serial.print ("Tiempo giro antihorario 90 grados: ");
  Serial.println (tiempoGiroAnti);
}
void desacelerar () {
  if (v > 0.3) {
    v = v - 0.20;
  }
  tiempoGiroHor = tiempoGiroHorOrig / v;
  tiempoGiroAnti = tiempoGiroAntiOrig / v;
  Serial.print("Velocidad: ");
  Serial.println (int(1023*v));
  Serial.print ("Tiempo giro horario 90 grados: ");
  Serial.println (tiempoGiroHor);
  Serial.print ("Tiempo giro antihorario 90 grados: ");
  Serial.println (tiempoGiroAnti);
}
*/
//IPAddress ip_rover (192,168,253,10);        // (si quiero usar el hotspot como router)
//IPAddress ip_rover (192,168,137,10);        // (si quiero usar mi pc como router)
//IPAddress puerta_enlace (192,168,253,154); // (si quiero usar el hotspot como router)
//IPAddress puerta_enlace (192,168,137,1); // (si quiero usar mi pc como router)
//IPAddress mascara(255,255,255,0);
//R1 = 5 KOhm = 10 Ohm,
//Vout = Vin*R2/(R1+R2) -> Vout = Vin *(5/(15)) = 8,4 V * 1/3 = 2,8
//Vin lo voy a obtener multiplicando 8,4 * vout?
//recta analog a dig: y de 0 a 1024, x de 0 a 1v
// cada bat toma 4,2v y max 8,4v. 

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

// funciones de motor
void configurarMotor (motor m) {
  pinMode(m.pin1,OUTPUT);
  pinMode(m.pin2,OUTPUT);
}

void girarAdelante (motor m) {  //esto es para controlar el giro de un motor en específico hacia adelante, habría que confirmar           // que sea el correcto, sino reasigno los pines en la instancia del motor y listo
  digitalWrite(m.pin2,LOW);
  analogWrite(m.pin1,1023*v/100);
}

void girarAtras (motor m) {
  digitalWrite(m.pin1,LOW);
  analogWrite(m.pin2,1023*v/100);
}

void frenar (motor m) { //frena UN motor solo
  digitalWrite(m.pin1,LOW);
  digitalWrite(m.pin2,LOW);
}

// funciones auxiliares

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

void girarDerMarchaAtras () { //rover gira hacia la derecha marcha atrás; revisar sentido
  girarAtras (motor_izq);
  frenar (motor_der);
  prenderLed ();
  Serial.println("gira marcha atrás hacia la derecha");
}

// funciones autónomas
/*
bool evadirIzq () {
  Serial.println ("Intenta evadir por izquierda.");
  int i = 0;
  detenerse();
  delay(500);
  girarAntiHorario ();
  delay(tiempoGiroHor);
  detenerse ();
  delay(500);
  int distancia = calcularDistancia();
  bool frenteLibre = false;
  if (distancia > umbral) {
    while (distancia > umbral && i < tiempoAvance && !frenteLibre) {  
      if (i%1000 == 0 && i > 0) {    //explicado en la función de abajo
        girarHorario ();
        delay(tiempoGiroHor);
        detenerse ();
        delay(500);
        distancia = calcularDistancia ();
        if (distancia > umbral) {
          frenteLibre = true;
        } else {
          girarAntiHorario ();
          delay (tiempoGiroAnti);
          detenerse();
          delay(500);
        }
      }
      if (!frenteLibre) {
        avanzar ();
        delay (50);
        i += 50;
        distancia = calcularDistancia ();
      }
      
    }
    if (!frenteLibre) {   
      int tAvanz= i;       //avanzó una vez que dobló pero era muy poco (depende del ancho del objeto)
      i = 0;
      detenerse ();
      delay(500);
      marchaAtras ();
      delay(tAvanz);
      detenerse ();
      delay(500);
      i = 0;
      girarHorario ();   
      delay(tiempoGiroHor);
      detenerse();
    } 
  } else {        
      detenerse ();
      delay(500);
      i = 0;
      girarHorario ();                 //deshace el giro (no llegó a avanzar)
      delay(tiempoGiroHor);
      detenerse();
    }
  if (frenteLibre) {
    Serial.println("giró bien izq 90");
  } else {
    Serial.println("NO giró izq 90");
  }
  return frenteLibre;
}

bool evadirDer () {
  Serial.println ("Intenta evadir por derecha.");
  int i = 0;
  detenerse();
  delay (500);
  girarHorario ();
  delay(tiempoGiroHor);
  detenerse();
  delay (500);
  int distancia = calcularDistancia ();
  bool frenteLibre = false;
  if (distancia > umbral) {
    while (distancia > umbral && i < tiempoAvance && !frenteLibre) {  
      if (i%1000 == 0 && i > 0) {   //como tiene un radar en el frente nada más, cada medio segundo gira 90 grados para quedar en la dirección original,
        girarAntiHorario ();   //analiza si el frente está libre, si no lo está gira 90 grados para el otro lado, y sigue avanzando
        delay(tiempoGiroAnti);              //paralelo al obstáculo hasta que cumpla el tiempoAvance máximo. 
        detenerse();      
        delay(500);            
        distancia = calcularDistancia ();
        if (distancia > umbral) {
          frenteLibre = true;
        } else {
          girarHorario ();
          delay (tiempoGiroHor);
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
    if (!frenteLibre) {      //retrocede porque llegó a avanzar hacia el costado
      int tAvanz= i;    //tiempo que avanzó
      i = 0;
      detenerse();
      delay (500);
      marchaAtras ();
      delay (tAvanz);
      detenerse();
      delay (500);
      girarAntiHorario ();   
      delay(tiempoGiroAnti);
      detenerse();
    } 
  } else {        
      i = 0;
      detenerse();
      delay (500);      
      girarAntiHorario ();           //deshace el giro (no llegó a avanzar)
      delay(tiempoGiroAnti);
      detenerse();
    }
  if (frenteLibre) {
    Serial.println("giró bien der 90");
  } else {
    Serial.println("NO giró der 90");
  }
  return frenteLibre;
}
*/

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
    detenerse(); //importante!!!! que frene antes de girar
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

//conexión a wifi

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
    //desacelerar ();
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
  for (int c = 0;c<4;c++) { //para que no arranque tan rápido el serial y todo eso
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
  WiFiClient cliente = server.accept();  //supuestamente se usa accept ahora y no available
  if (cliente) {
    if (clienteNuevo(cliente)) {  //si es un cliente nuevo imprime su ip
      Serial.print("nuevo cliente, su ip es: ");
      Serial.println(cliente.remoteIP());
    }

    manejarCliente (cliente);
  }
  //VoltBateria();
  if (!manual) {
    navegacionAutomatica ();
  }
  
}