/* CONTROLLER BOARD */

#include "ee.h"
#include "cpu/pic30/inc/ee_irqstub.h"
#include "setup.h"
#include "uart_dma.h"
#include "math.h"

_FOSCSEL(FNOSC_PRIPLL);	// Primary (XT, HS, EC) Oscillator with PLL
_FOSC(OSCIOFNC_ON & POSCMD_XT); // OSC2 Pin Function: OSC2 is Clock Output
								// Primary Oscillator Mode: XT Crystal
_FWDT(FWDTEN_OFF);		// Watchdog Timer Enabled/disabled by user software
_FGS(GCP_OFF);			// Disable Code Protection

/*	Variables	*/

static float r=-1;

static float v_max=3.3;  //dsPIC voltage reference
static float offset=0.0;//0.06;//OPAMP offset

//////////////////////////////////////
//Two thirds
static float A[2][2] = {{0,-23.8095},{0,0}};
static float B[2][1] = {{0},{-23.8095}};
static float L[1][2] = {{0.1449, -0.5575}};
static float x[2][1] = {{0},{0}};
static float alpha = 0.66;
static float eta = 0.0277;
static float tau_max = 38e-3;
static float tau_min = 15e-3;
static float tau_k = 15e-3;
static float u = 0;
static float gain1[2] = {-0.7995,    0.1430};
static float gain2[2] = {1.7644,   -0.5550};
static float L_k[1][2] = {{0, 0}};

//Observer
static float Ad[2][2] = {{1,-0.3571},{0,1}};
static float Bd[2][1] = {{0.0638},{-0.3571}};

static float gainObs1[2] = {0, 2};
static float gainObs2[2] = {68.4053, -3.5213};

static float x_hat[2][1] = {{0},{0}};
static float Lobs_k[2][1] = {{0},{0}};

#define lengthBuffer 450
static unsigned char bufferTau1[lengthBuffer];
static unsigned char bufferTau2[lengthBuffer];

#define buffer1 1
#define buffer2 2
#define experiment 1
#define tauAcquisition 2
#define none 0
static unsigned char whichBuffer = buffer1;
unsigned char processState = none; //by default, system turned-off
static unsigned int countBufferTau = 0;
static unsigned int countReadBuffer = 0;

static unsigned long sys_time=0;

/* Change the reference value between -0.5 and 0.5V*/
TASK(TaskReferenceChange)
{
	if (r == -1)
	{
		r = 1;
		//LATBbits.LATB14 = 1;//Orange led switch on
	}
	else
	{
		r = -1;
		//LATBbits.LATB14 = 0;//Orange led switch off
	}
}

/* Read RCRC circuit output voltages */
void Read_State(void)
{
	AD1CHS0 = 22; 		   // Input ADC channel selection
	AD1CON1bits.SAMP = 1;  // Start conversion
	while(!IFS0bits.AD1IF);// Wait till the EOC
	IFS0bits.AD1IF = 0;    // Reset ADC interrupt flag
	x[0][0] = (ADC1BUF0*v_max/4096)-v_max/2-offset;// Acquire data and scale

	AD1CHS0 = 23; 		   // Input ADC channel selection
	AD1CON1bits.SAMP = 1;  // Start conversion
	while(!IFS0bits.AD1IF);// Wait till the EOC
	IFS0bits.AD1IF = 0;    // Reset ADC interrupt flag
	x[1][0] = (ADC1BUF0*v_max/4096)-v_max/2-offset;// Acquire data and scale
}


void Actuate(void)
{
	PDC1 = (u/v_max)*0x7fff+0x3FFF;
}
/* Controller Task */

TASK(TaskController)
{
	float accum1 = 0, accum2 = 0;
	static unsigned char *p_r = (unsigned char *)&r;

////////////////
	//static unsigned char *p_r = (unsigned char *)&L_k[0][0];
	//static unsigned char *p_x0 = (unsigned char *)&L_k[0][1];
/////////////////
	//static unsigned char *p_x0 = (unsigned char *)&x_hat[0][0];
	static unsigned char *p_x0 = (unsigned char *)&x[0][0];
	//static unsigned char *p_x1 = (unsigned char *)&x_hat[1][0];
	static unsigned char *p_x1 = (unsigned char *)&x[1][0];
	static unsigned char *p_u = (unsigned char *)&u;


	LATBbits.LATB14 = 1;//Turn on orange led

	sys_time=GetTime();

	Read_State();

	//L*(A+B*L)*x =
	// =(x00-r)*(L00*(A00 + B00*L00) + L01*(A10 + B10*L00)) +
	//  x10*(L00*(A01 + B00*L01) + L01*(A11 + B10*L01))

	accum1  = L[0][0]*(A[0][0] + B[0][0]*L[0][0]);
	accum1 += L[0][1]*(A[1][0] + B[1][0]*L[0][0]);
	//standard controller
	accum1 *= (x[0][0]-r);
	//controller with observer
	//accum1 *= (x_hat[0][0]-r);

	accum2  = L[0][0]*(A[0][1] + B[0][0]*L[0][1]);
	accum2 += L[0][1]*(A[1][1] + B[1][0]*L[0][1]);
	//standard controller
	accum2 *= x[1][0];
	//controller with observer
	//accum2 *= x_hat[1][0];

	accum1 += accum2;

	//abs(L*(A+B*L)*x)
	if (accum1<0)
		accum1 = -accum1;

	//(abs(L*(A+B*L)*x))^alpha
	accum2 = pow(accum1, alpha);

	//[(tau_max/eta)*(abs(L*(A+B*L)*x))^alpha]+1
	accum1 = (tau_max/eta)*accum2 + 1;

	//tau_k
	tau_k = tau_max/accum1; //value in milliseconds

	if (tau_k > tau_max)
		tau_k = tau_max;
	else if (tau_k < tau_min)
		tau_k = tau_min;

	//fix the value to be used to trigger the next process
	accum1 = tau_k*1000;
	////////////////
	//AQUI APLICAR ROUND
	////////////////
	SetRelAlarm(AlarmController,(unsigned int)(accum1),0);

	if (whichBuffer == buffer1)
	{
		bufferTau1[countBufferTau] = (unsigned char)(accum1);
		countBufferTau++;
		if (countBufferTau > lengthBuffer)
		{
			countBufferTau = 0;
			whichBuffer = buffer2;
		}
	}
	else if (whichBuffer == buffer2)
	{
		bufferTau2[countBufferTau] = (unsigned char)(accum1);
		countBufferTau++;
		if (countBufferTau > lengthBuffer)
		{
			countBufferTau = 0;
			whichBuffer = none; //buffers out, more information will not be saved
		}
	}
	else if (whichBuffer == none)
	{

	}

	//L_k is calculated with two polynomial taken from polyfits made
	//with the set of L_k computed off-line using matlab
	L_k[0][0] = gain1[0]*tau_k + gain1[1];
	L_k[0][1] = gain2[0]*tau_k + gain2[1];

	//standard controller
	u = -L_k[0][0]*(x[0][0]-r) - L_k[0][1]*x[1][0];

	//controller with observer
	//u = -L_k[0][0]*(x_hat[0][0]-r) - L_k[0][1]*x_hat[1][0];

	//Check for saturation
	if (u > v_max/2) u = v_max/2;
	if (u < -v_max/2) u = -v_max/2;

	//Lobs_k is calculated with two polynomial taken from polyfits made
	//with the set of Lobs_k computed off-line using matlab
	Lobs_k[0][0] = gainObs1[0]*tau_k + gainObs1[1];
	Lobs_k[1][0] = gainObs2[0]*tau_k + gainObs2[1];

	//Discretize A and B using the current sampling and the approximations shown
	//in Computer Controlled Systems by Astrom

	Ad[0][1] = -23.8095*tau_k;
	Bd[0][0] = 283.4461*tau_k*tau_k;
	Bd[1][0] = Ad[0][1];

	//Using accumulators because both calculations must be done at the same time
	accum1 =  Ad[0][0]*x_hat[0][0] + Ad[0][1]*x_hat[1][0];
	accum1 += Lobs_k[0][0]*(x[0][0] - x_hat[0][0]) + Bd[0][0]*u;

	accum2 =  Ad[1][0]*x_hat[0][0] + Ad[1][1]*x_hat[1][0];
	accum2 += Lobs_k[1][0]*(x[0][0] - x_hat[0][0]) + Bd[1][0]*u;

	x_hat[0][0] = accum1;
	x_hat[1][0] = accum2;
	Actuate();

	//LATBbits.LATB10 = 1; //To get time with the oscilloscope

	//sys_time=GetTime();  //Get system time (EDF Scheduler)

	OutBuffer[0]=0x01;//header;
	OutBuffer[1]=(unsigned char)(sys_time>>24);//4th byte of unsigned long
	OutBuffer[2]=(unsigned char)(sys_time>>16);//3rd byte of unsigned long
	OutBuffer[3]=(unsigned char)(sys_time>>8); //2nd byte of unsigned long
	OutBuffer[4]=(unsigned char)sys_time;      //1st byte of unsigned long
	OutBuffer[5]=*p_r;    //4th byte of float (32bits)
	OutBuffer[6]=*(p_r+1);//3rd byte of float (32bits)
	OutBuffer[7]=*(p_r+2);//2nd byte of float (32bits)
	OutBuffer[8]=*(p_r+3);//1st byte of float (32bits)
	OutBuffer[9]=*p_x0;
	OutBuffer[10]=*(p_x0+1);
	OutBuffer[11]=*(p_x0+2);
	OutBuffer[12]=*(p_x0+3);
	OutBuffer[13]=*p_x1;
	OutBuffer[14]=*(p_x1+1);
	OutBuffer[15]=*(p_x1+2);
	OutBuffer[16]=*(p_x1+3);
	OutBuffer[17]=*p_u;
	OutBuffer[18]=*(p_u+1);
	OutBuffer[19]=*(p_u+2);
	OutBuffer[20]=*(p_u+3);

	//Force sending data
	DMA4CONbits.CHEN  = 1;			// Re-enable DMA4 Channel
	DMA4REQbits.FORCE = 1;			// Manual mode: Kick-start the first transfer

	//LATBbits.LATB10 = 0; //To get time with the oscilloscope

	LATBbits.LATB14 = 0;//Turn off orange led

}

/* Send data using the UART port 1 via RS-232 to the PC
 * Note: This Task takes less than 0.0002 seconds */

TASK(TaskSupervision)
{

	static unsigned int i;
	static unsigned char noThisBuffer = 0;


    if (processState == tauAcquisition)
	{
		if (whichBuffer == buffer1)
		{
			OutBuffer[0] = 0xff; //header

			noThisBuffer = 0;

			for (i=1; i<=22; i++)
			{
				if (noThisBuffer == 0)
				{
					OutBuffer[i] = bufferTau1[countReadBuffer];
					countReadBuffer++;
					if (countReadBuffer >= lengthBuffer)
					{
						countReadBuffer = 0;
						noThisBuffer = 1;
						whichBuffer = buffer2;
					}
				}
				else
				{
					OutBuffer[i] = bufferTau2[countReadBuffer];
					countReadBuffer++;
				}
			}
			//Force sending data
			DMA4CONbits.CHEN  = 1; //Re-enable DMA4 Channel
			DMA4REQbits.FORCE = 1; //Manual mode: Kick-start the first transfer

			//LATBbits.LATB10 = 0; //To get time with the oscilloscope

		}
		else if (whichBuffer == buffer2)
		{
			OutBuffer[0] = 0xff; //header

			noThisBuffer = 0;

			for (i=1; i<=22; i++)
			{
				if (noThisBuffer == 0)
				{
					OutBuffer[i] = bufferTau2[countReadBuffer];
					countReadBuffer++;
					if (countReadBuffer >= lengthBuffer)
					{
						countReadBuffer = 0;
						noThisBuffer = 1;
						whichBuffer = none;
					}
				}
				else
				{
					OutBuffer[i] = 0;
				}
			}
			//Force sending data
			DMA4CONbits.CHEN  = 1; //Re-enable DMA4 Channel
			DMA4REQbits.FORCE = 1; //Manual mode: Kick-start the first transfer
			//LATBbits.LATB10 = 0; //To get time with the oscilloscope
		}
		else //auto-stop supervision
		{
			processState = none;
			countBufferTau = 0;
			countReadBuffer = 0;
			CancelAlarm(AlarmSupervision);

			for(i=0; i<=lengthBuffer; i++)
			{
				bufferTau1[i] = 0;
				bufferTau2[i] = 0;
			}
		}
	}
}

ISR2(_DMA5Interrupt) //UART RX interruption
{
	//LATBbits.LATB14 ^= 1; //To get time with the oscilloscope

	switch (InBufferA[0])
	{
		case 0x30: //stop
			processState = none;
			//reset values
			r = -1;
			u = 0;
			x_hat[0][0] = 0;
			x_hat[1][0] = 0;
			x[0][0] = 0;
			x[1][0] = 0;
			CancelAlarm(AlarmReferenceChange);
			CancelAlarm(AlarmController);
			CancelAlarm(AlarmSupervision);

			break;

		case 0x31: //start running experiment
			processState = experiment;
			whichBuffer = buffer1;
			countBufferTau = 0;
			SetRelAlarm(AlarmReferenceChange,50,1500); //ref changes every 1500ms
			SetRelAlarm(AlarmController,50,0);
			break;

		case 0x32: //start reading buffer - it is auto-stopped
			processState = tauAcquisition;
			whichBuffer = buffer1;
			countReadBuffer = 0;
			SetRelAlarm(AlarmSupervision, 1, 5);//Data is sent to computer every 5ms
			break;

		default:
			processState = none;
			break;
	}

	IFS3bits.DMA5IF = 0; // Clear the DMA1 Interrupt Flag
}

int main(void)
{
	Sys_init();//Initialize clock, devices and peripherals

	//SetRelAlarm(AlarmReferenceChange,10,1000); //ref changes every 1000ms
	//SetRelAlarm(AlarmController,(unsigned int)(tau_min*1000),0);
	//SetRelAlarm(AlarmSupervision, 1, 5);//Data is sent to computer every 5ms

	/* Forever loop: background activities (if any) should go here */
	for (;;);

	return 0;
}
