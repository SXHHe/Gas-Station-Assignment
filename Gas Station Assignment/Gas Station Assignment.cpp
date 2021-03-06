// Gas Station Assignment.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS


#include "..\MegaGlobal.h"
#include "Pump.h"
#include "Customer.h"

// Function declarations
const string currentDateTime();
void updateOctantDisplay(float a, float b, float c, float d);
void updateQueDisplay(int a, int b, int c, int d);

// Thread declarations
UINT __stdcall Incoming_Customer(void *args);

// Global Variables
int Total_customers[5];

int main()
{
	// Need a rendezvous object to start all threads/processes in sync
	CRendezvous program_sync("MyRendezvous", quantity);							// Instance name + number of threads/processes to sync

	// Creates the gas station computer child process
	CProcess GSC("..\\Debug\\Gas Station Computer.exe",						// pathlist to child program executable				
		NORMAL_PRIORITY_CLASS,												// priority
		OWN_WINDOW,															// process has its own window					
		ACTIVE																// process is active immediately
	);

	// Mutex to use when writing to consol window
	CMutex Window("Customers Window");
	
	// Pump objects need an instance name for their datapool and pipeline
	// Defined strings for each
	string pipe1_name = "Pipe1";	string pool1_name = "Pool1";
	string pipe2_name = "Pipe2";	string pool2_name = "Pool2";
	string pipe3_name = "Pipe3";	string pool3_name = "Pool3";
	string pipe4_name = "Pipe4";	string pool4_name = "Pool4";

	// Create instances of the required pumps
	Pump pump1(pipe1_name, pool1_name, "PS1", "CS1");
	Pump pump2(pipe2_name, pool2_name, "PS2", "CS2");
	Pump pump3(pipe3_name, pool3_name, "PS3", "CS3");
	Pump pump4(pipe4_name, pool4_name, "PS4", "CS4");

	// Create instance of tank
	TankMonitor Tank("FTank");

	// Active classes begin suspended so resume them
	pump1.Resume();
	pump2.Resume();
	pump3.Resume();
	pump4.Resume();

	// Initialize global variable
	for (int i = 1; i < 5; i++) {
		Total_customers[i] = 0;
	}

	// Turn the cursor off
	CURSOR_OFF();

	// Wait here until setup is complete for all threads/processes
	program_sync.Wait();

	// Thread to randomly create customers
	CThread randCust(Incoming_Customer, ACTIVE, NULL);

	while (true) {
		Window.Wait();
		MOVE_CURSOR(0, 0);
		TEXT_COLOUR(10);		// Set text colour to green
		printf("Current time: %s\n", currentDateTime().c_str());
		fflush(stdout);
		TEXT_COLOUR(7);			// Set text colour back to default
		updateOctantDisplay(Tank.grade_cost_L(1), Tank.grade_cost_L(2), Tank.grade_cost_L(3), Tank.grade_cost_L(4));
		updateQueDisplay(Total_customers[1]-pump1.served_customers(), Total_customers[2] - pump2.served_customers(), Total_customers[3] - pump3.served_customers(), Total_customers[4] - pump4.served_customers());
		Window.Signal();
		Sleep(1000);
	}

	Sleep(1000);

	GSC.WaitForProcess();
	pump1.WaitForThread();
	pump2.WaitForThread();
	pump3.WaitForThread();
	pump4.WaitForThread();

	getchar();


	return 0;
}


//Thread to generate customers and their arrival pump 
UINT __stdcall Incoming_Customer(void *args) {

	// Semaphore to limit the amount of customers to not overexert cpu
	CSemaphore limit("limit", 16, 16);

	while (TRUE) {
		// randomly generate which pump customer goes for fuel
		int arrival_pump = rand() % 4 + 1;

		Customer *the_customer;												//create customer pointers

		Sleep(rand() % 10000+1000);

		limit.Wait();

		if (arrival_pump == 1) {											// customer arrives at pump 1
			the_customer = new Customer("Pipe1", "Mutex1");
			Total_customers[1]++;
			the_customer->Resume();
		}
		else if (arrival_pump == 2) {										// customer arrives at pump 2
			the_customer = new Customer("Pipe2", "Mutex2");
			Total_customers[2]++;
			the_customer->Resume();
		}
		else if (arrival_pump == 3) {										// customer arrives at pump 3
			the_customer = new Customer("Pipe3", "Mutex3");
			Total_customers[3]++;
			the_customer->Resume();
		}
		else if (arrival_pump == 4) {										// customer arrives at pump 4
			the_customer = new Customer("Pipe4", "Mutex4");
			Total_customers[4]++;
			the_customer->Resume();
		}

	}

	return 0;
}


// Function that returns the current date and time as a string
const string currentDateTime()
{
	time_t now = time(0);													// Current time
	struct tm timestruct;
	char buf[80];															// Character buffer to store datetime string
	timestruct = *localtime(&now);											// Put local time into a structure

	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timestruct);			// Return properly formated string

	return buf;
}


// Displays prices per L
void updateOctantDisplay(float a, float b, float c, float d)
{
	MOVE_CURSOR(25, 21);
	printf("Fuel Grade Prices: ");
	fflush(stdout);
	MOVE_CURSOR(25, 22);
	printf("87: $%.2f   ", a);
	fflush(stdout);
	MOVE_CURSOR(25, 23);
	printf("89: $%.2f   ", b);
	fflush(stdout);
	MOVE_CURSOR(25, 24);
	printf("91: $%.2f   ", c);
	fflush(stdout);
	MOVE_CURSOR(25, 25);
	printf("93: $%.2f   ", d);
	fflush(stdout);
	TEXT_COLOUR(7);			// Set text colour back to default
}

// Displays Amount of customers that are waiting in the que
void updateQueDisplay(int a, int b, int c, int d)
{
	MOVE_CURSOR(0, 21);
	printf("Current Queue: ");
	fflush(stdout);
	MOVE_CURSOR(0, 22);
	printf("Pump 1: %i   ", a);
	fflush(stdout);
	MOVE_CURSOR(0, 23);
	printf("Pump 2: %i   ", b);
	fflush(stdout);
	MOVE_CURSOR(0, 24);
	printf("Pump 3: %i   ", c);
	fflush(stdout);
	MOVE_CURSOR(0, 25);
	printf("Pump 4: %i   ", d);
	fflush(stdout);
	TEXT_COLOUR(7);			// Set text colour back to default
}
