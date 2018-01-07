//declaracion de variables
volatile int order_angle=0;
char incomingByte;
volatile long  count=0,ecount=0,countp=0;
volatile int actual_angle;
volatile boolean A,B;
volatile float pwm,rpm;
int x=5;
int y=6;
long timep, time, etime;
long RPM=100;
volatile byte state, statep;
volatile int analogInPin = A0;
volatile float voltaje=0,volReal=0;

void setup() {
  
Serial.begin(9600);// Inicializa ola comunicacionn serial
pinMode(2,INPUT);//Canal A encoder
pinMode(3,INPUT);//Canal B encoder
pinMode(x,OUTPUT);//In1 giro dle motor   
pinMode(y,OUTPUT);//In2 giro del motor  
pinMode(11,OUTPUT);//Ena Activa pwm drive 

attachInterrupt(0,Achange,CHANGE);//denera una interrupcion al detectar un cambio en el encoder 
attachInterrupt(1,Bchange,CHANGE);//denera una interrupcion al detectar un cambio en el encoder

timep=micros();
A= digitalRead(2);
B= digitalRead(3);

//Declaracion de los estados posibles entre la lectura de los canales 
if((A==HIGH)&&(B==LOW))statep=1;
if((A==HIGH)&&(B==HIGH))statep=2;
if((A==LOW)&&(B==HIGH))statep=3;
if((A==LOW)&&(B==LOW))statep=4;

digitalWrite(x,LOW);
digitalWrite(y,LOW);
 interrupts();

}

void loop() {
time = millis();
etime=time-timep;
ecount=count-countp;

voltaje=analogRead(analogInPin);
pwm=map(voltaje, 0, 1023, 0, 255);
volReal=map(pwm, 0, 255, 0,5);
turnRigth();
envia_datos(volReal,count,etime);
}
void envia_datos(float x,float y,float z)
{
  
  Serial.print(x);
  Serial.flush();
   Serial.print(" ");
  Serial.flush();
  Serial.print(y);
  Serial.flush();
  Serial.print(" ");
  Serial.flush();
  Serial.println(z);
  Serial.flush();
  countp=count;
  timep=time;
  }
void Achange()//Fauncion que detecta el cabio de flanco en el canal 
{
  A=digitalRead(2);//Lectura del canal A
  B=digitalRead(3);//Lectura del canal B  

  //Declaracion de estados entre las lecturas de los canaes 
  if((A==HIGH)&&(B==LOW))state=1;
  if((A==HIGH)&&(B==HIGH))state=2;
  if((A==LOW)&&(B==HIGH))state=3;
  if((A==LOW)&&(B==LOW))state=4;
  
  switch(state)
  {
    case 1: ;//Si elo estedo actual es 1 
    {
      //Compparacion con el estado pasado 
      
      if(statep == 2) count--;//Si el esttado pasado es 2 decrementar 
      if(statep == 4) count++;//Si el esttado pasado es 4 incrementar 
      break;
      }
      case 2:// si el estado actual es 2
      {
          //Compparacion con el estado pasado 
      
      if(statep == 1) count++;//Si el esttado pasado es 2 decrementar 
      if(statep == 3) count--;//Si el esttado pasado es 4 incrementar 
      break;
        }
          case 3:// si el estado actual es 3
      {
          //Compparacion con el estado pasado 
      
      if(statep == 2) count++;//Si el esttado pasado es 2 decrementar 
      if(statep == 4) count--;//Si el esttado pasado es 4 incrementar 
      break;
        }
        default:
        {
          //Compparacion con el estado pasado 
      
      if(statep == 1) count--;//Si el esttado pasado es 1 decrementar 
      if(statep == 3) count++;//Si el esttado pasado es 3 incrementar 
        }
    
    }
    statep = state; //Actualizamos la variable statep con el valor de state
}
void Bchange()//Fauncion que detecta el cabio de flanco en el canal 
{
 A=digitalRead(2);//Lectura del canal A
B=digitalRead(3);//Lectura del canal B   

  //Declaracion de estados entre las lecturas de los canaes 
  if((A==HIGH)&&(B==LOW))state=1;
  if((A==HIGH)&&(B==HIGH))state=2;
  if((A==LOW)&&(B==HIGH))state=3;
  if((A==LOW)&&(B==LOW))state=4;
  switch(state)
  {
    case 1: ;//Si elo estedo actual es 1 
    {
      //Compparacion con el estado pasado 
      
      if(statep == 2) count--;//Si el esttado pasado es 2 decrementar 
      if(statep == 4) count++;//Si el esttado pasado es 4 incrementar 
      break;
      }
      case 2:// si el estado actual es 2
      {
          //Compparacion con el estado pasado 
      
      if(statep == 1) count++;//Si el esttado pasado es 2 decrementar 
      if(statep == 3) count--;//Si el esttado pasado es 4 incrementar 
      break;
        }
          case 3:// si el estado actual es 3
      {
          //Compparacion con el estado pasado 
      
      if(statep == 2) count++;//Si el esttado pasado es 2 decrementar 
      if(statep == 4) count--;//Si el esttado pasado es 4 incrementar 
      break;
        }
        default:
        {
          //Compparacion con el estado pasado 
      
      if(statep == 1) count--;//Si el esttado pasado es 1 decrementar 
      if(statep == 3) count++;//Si el esttado pasado es 3 incrementar 
        }
    
    }
    statep = state; //Actualizamos la variable statep con el valor de state
 }
void turnRigth()//Ordear el giro en CW
  {
    
    digitalWrite(11,HIGH);
    analogWrite(x,pwm);
    digitalWrite(y,LOW);
    
    
    }
void turnLeft()//Ordear el giro en CCW
  {
     // analogWrite(11,255);
    digitalWrite(y,pwm);
    digitalWrite(x,LOW);
    }
 
