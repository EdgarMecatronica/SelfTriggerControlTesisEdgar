//declaracion de variables
volatile int order_angle=0;
char incomingByte;
volatile long  count=0;
int error;
volatile int actual_angle;
int pwm;
int flag=2;
int x=5,i=0,j=0,k=0;
int y=6;
int salidal;
volatile char S;
long timep, time, etime, f=1000000;
volatile boolean A,B;
volatile byte state, statep;
volatile int analogInPin=A0;
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


time = micros();
etime=time-timep;

 if (time>= f)
 {
  f=3000000+time;
  if(i=1)
  {
    order_angle=360;
    i=2;
    }else if (i=2)
          {
             order_angle=0;
             i=1;
          }
    
   
  }

actual_angle=map(count,0,231,0,360);//transformamos lo acomulado 
// inplementacion de bucle cerrado
//error = order_angle-actual_angle; // calculo del error
//pwm=map(error,0,360,0,255);//transformamos lo acomulado 
//analogWrite(11,pwm);//Se imprime el valor en la  saslida  pwm
if(order_angle==actual_angle) 
{
  digitalWrite(y,LOW);
  digitalWrite(x,LOW);
   flag=2;
}else if (order_angle<actual_angle)
 {
   turnLeft();//Motor girando a la izquierda
     flag=0;
  }else if (order_angle>actual_angle)
    {
        turnRigth();
        flag=1;
      }
envia_datos(order_angle,actual_angle,etime);
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
//  countp=count;
  timep=time;
  }
void Achange()//Fauncion que detecta el cabio de flanco en el canal 
{
  A=digitalRead(2);//Lectura del canal A
  B=digitalRead(3);//Lectura del canal B  
S='A';
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
S='B';
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
    digitalWrite(x,HIGH);
    digitalWrite(y,LOW);
    }
void turnLeft()//Ordear el giro en CCW
  {
    digitalWrite(11,HIGH);
     digitalWrite(x,LOW);
    digitalWrite(y,HIGH);
    }
 
