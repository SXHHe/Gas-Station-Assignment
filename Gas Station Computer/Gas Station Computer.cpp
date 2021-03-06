// Gas Station Computer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS

#include "..\MegaGlobal.h"
#include <vector>

// Global thread declarations
UINT __stdcall PumpThread(void *args);		// Pump thread
UINT __stdcall FuelTanks(void *args);		// Fuel tank thread
UINT __stdcall RealTimeDisplay(void *args);	// Real time update thread

// Global function declarations
void pumpStatus(string status, int pump, unsigned char colour);
void displayTransaction(struct Transaction t, int num);
void windowInitialization(int i, int y, string pump);
void pumpUpdate(struct Transaction t, int pump);
void drawBox(int x, int y, int items);
const string currentDateTime();
void updateOctantDisplay();
void changeOctantPrices();
void initializeDisplay();
void setMode(int mode);
void commandLegend();

// Global mutex declarations
CMutex Window("GSC Window");	// Mutex to use when writing to consol window
CMutex Vector("TVector");		// Mutex to protect the vector

// Global structures
struct mydatapooldata {	// Pump info will be stored here to be sent to the gas station computer through datapool
	char name[8];		// Customers name
	int credit_number;	// Credit card number supplied for transactions
	int fuel_grade;		// 4 types -> 87, 89, 91, or 93
	int fuel_quantity;	// 70 liters max
	float total_cost;	// total cost for transaction
};
struct Transaction {	// A struct to store the transaction details in
	string time;		// Time of transaction
	string fuel_grade;	// Requested fuel grade
	string name;		// Name of customer
	int credit;			// Credit card number
	int fuel_quantity;	// Quantity of fuel requested
	float total_cost;	// Total cost of transaction
};
struct threaddata {
	string poolname;
	string consume;
	string produce;
	string wait;
	int pump;
};

// Global vectors
vector<struct Transaction> transaction_record;														// A vector to store transactions in

// Global arrays
int selected_grade[5];																				// Store slected fuel grade for each pump

// Global constants
const int grade_87 = 1, grade_89 = 2, grade_91 = 3, grade_93 = 4;									// Fuel Grade number
const int fuelpump1 = 1, fuelpump2 = 2, fuelpump3 = 3, fuelpump4 = 4;								// Pump number 
const unsigned char yellow = 14, red = 12, cyan = 11, green = 10, blue = 9, black = 8, grey = 7;	// Text colour values
const float fuel_low = 200.0;																		// Fuel level at which it begins to flash red and no longer accepts new fill commands

// Global floats
float price_87 = 1.00f, price_89 = 1.05f, price_91 = 1.10f, price_93 = 1.15f;						// Fuel grade prices + initial values
float T1 = 500.0, T2 = 500.0, T3 = 500.0, T4 = 500.0;												// Fuel level variables + initial values

// Global booleans
bool price_updated = false;																			// Flag giving info on whether they have been updated
bool p1dispensing = false, p2dispensing = false, p3dispensing = false, p4dispensing = false;		// Flags to determine if a pump is curerently dispensing fuel
bool pdispensing[] = { false, false, false, false };												// Flags to notify whether or not a pump is currently dispensing

// Global strings 
string current_command;																				// Stores valid command -> Can easily be accessed by any threads

int main()
{
	// Need a rendezvous object to start all threads/processes in sync
	CRendezvous program_sync("MyRendezvous", quantity);	// Instance name + number of threads/processes to sync

	// String array
	struct threaddata pump1; pump1.poolname = "Pool1"; pump1.consume = "CS1"; pump1.produce = "PS1"; pump1.pump = 1; pump1.wait = "wait1";
	struct threaddata pump2; pump2.poolname = "Pool2"; pump2.consume = "CS2"; pump2.produce = "PS2"; pump2.pump = 2; pump2.wait = "wait2";
	struct threaddata pump3; pump3.poolname = "Pool3"; pump3.consume = "CS3"; pump3.produce = "PS3"; pump3.pump = 3; pump3.wait = "wait3";
	struct threaddata pump4; pump4.poolname = "Pool4"; pump4.consume = "CS4"; pump4.produce = "PS4"; pump4.pump = 4; pump4.wait = "wait4";

	// Create 4 threads -> One for communicating with each pump
	CThread PUMP1(PumpThread, ACTIVE, &pump1);
	CThread PUMP2(PumpThread, ACTIVE, &pump2);
	CThread PUMP3(PumpThread, ACTIVE, &pump3);
	CThread PUMP4(PumpThread, ACTIVE, &pump4);

	// Create a thread for fuel tank interactions
	CThread FTANK(FuelTanks, ACTIVE, NULL);

	// Create a thread to update the time displayed on the window
	CThread RTIME(RealTimeDisplay, ACTIVE, NULL);

	// Semaphores to communicate commands to pumps
	CSemaphore wait1("wait1", 0, 1);					// Signaled when user has allowed pump 1 to dispense fuel
	CSemaphore wait2("wait2", 0, 1);					// Signaled when user has allowed pump 2 to dispense fuel
	CSemaphore wait3("wait3", 0, 1);					// Signaled when user has allowed pump 3 to dispense fuel
	CSemaphore wait4("wait4", 0, 1);					// Signaled when user has allowed pump 4 to dispense fuel

	// Variables
	char command[10];									// Stores user entered command
	string user_command = "";							// Command in string form
	vector<struct Transaction>::iterator vptr;			// Iterator for a transaction vector
	vector<struct Transaction> present_transactions;	// Vector to store all transaction up to date and iterate through
	int tran_num = 1;									// current transaction number being viewed

	Window.Wait();
	CURSOR_OFF();										// Turn the cursor off
	initializeDisplay();								// Set up the display
	Window.Signal();

	// Wait here until setup is complete for all threads/processes
	program_sync.Wait();

	while (true) {														// Main thread loop
		if (TEST_FOR_KEYBOARD() != 0) {									// Will only enter this loop if keyboard activity is detected
			Window.Wait();
			setMode(0);													// Set mode to command
			
			MOVE_CURSOR(16, 25); printf("%-50s", " "); fflush(stdout);	// Clear command input area
			MOVE_CURSOR(0, 26);  printf("%-65s", " "); fflush(stdout);	// Clear command input area
			MOVE_CURSOR(0, 27);  printf("%-65s", " "); fflush(stdout);	// Clear command input area
			MOVE_CURSOR(0, 28);  printf("%-65s", " "); fflush(stdout);	// Clear command input area
			
			MOVE_CURSOR(16, 25); TEXT_COLOUR(green);
			scanf("%10s", command); TEXT_COLOUR(grey);					// Get user input -> Entered text will be green
			user_command = string(command);								// Store entered characters into a string

			if (user_command.size() == 2) {
				if (user_command == "F1" || user_command == "F2" || user_command == "F3" || user_command == "F4") {			// If command is to send fuel to a pump

					if (user_command == "F1" && !pdispensing[0]) {															// Command is for pump 1 and pump 1 isn't dispensing fuel
						switch (selected_grade[fuelpump1]) {																// Need to make sure more than 200L in requested tank
							case grade_87: { if (T1 > fuel_low) { pdispensing[0] = true; wait1.Signal(); 					// Pump 1 will now be dispensing fuel from tank 1
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 1"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_89: { if (T2 > fuel_low) { pdispensing[0] = true; wait1.Signal(); 					// Pump 1 will now be dispensing fuel from tank 2
											MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 2"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_91: { if (T3 > fuel_low) { pdispensing[0] = true; wait1.Signal(); 					// Pump 1 will now be dispensing fuel from tank 3
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 3"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_93: { if (T4 > fuel_low) { pdispensing[0] = true; wait1.Signal(); 					// Pump 1 will now be dispensing fuel from tank 4
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 4"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }	
						}
					}
					else if (user_command == "F2" && !pdispensing[1]) {														// Command is for pump 2 and pump 2 isn't dispensing fuel
						switch (selected_grade[fuelpump2]) {																// Need to make sure more than 200L in requested tank
							case grade_87: { if (T1 > fuel_low) { pdispensing[1] = true; wait2.Signal(); 					// Pump 2 will now be dispensing fuel from tank 1
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 1"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_89: { if (T2 > fuel_low) { pdispensing[1] = true; wait2.Signal(); 					// Pump 2 will now be dispensing fuel from tank 2
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 2"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_91: { if (T3 > fuel_low) { pdispensing[1] = true; wait2.Signal(); 					// Pump 2 will now be dispensing fuel from tank 3
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 3"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_93: { if (T4 > fuel_low) { pdispensing[1] = true; wait2.Signal(); 					// Pump 2 will now be dispensing fuel from tank 4
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 4"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
						}
					}
					else if (user_command == "F3" && !pdispensing[2]) {														// Command is for pump 3 and pump 3 isn't dispensing fuel
						switch (selected_grade[fuelpump3]) {																// Need to make sure more than 200L in requested tank
							case grade_87: { if (T1 > fuel_low) { pdispensing[2] = true; wait3.Signal(); 					// Pump 3 will now be dispensing fuel from tank 1
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 1"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_89: { if (T2 > fuel_low) { pdispensing[2] = true; wait3.Signal(); 					// Pump 3 will now be dispensing fuel from tank 2
												 MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 2"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_91: { if (T3 > fuel_low) { pdispensing[2] = true; wait3.Signal(); 					// Pump 3 will now be dispensing fuel from tank 3
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 3"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_93: { if (T4 > fuel_low) { pdispensing[2] = true; wait3.Signal(); 					// Pump 3 will now be dispensing fuel from tank 4
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 4"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
						}
					}
					else if (user_command == "F4" && !pdispensing[3]) {														// Command is for pump 4 and pump 4 isn't dispensing fuel
						switch (selected_grade[fuelpump4]) {																// Need to make sure more than 200L in requested tank
							case grade_87: { if (T1 > fuel_low) { pdispensing[3] = true; wait4.Signal(); 					// Pump 4 will now be dispensing fuel from tank 1
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 1"); fflush(stdout); } 	// Tank needs to be refilled -> notify user
										   break; }
							case grade_89: { if (T2 > fuel_low) { pdispensing[3] = true; wait4.Signal(); 					// Pump 4 will now be dispensing fuel from tank 2
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 2"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_91: { if (T3 > fuel_low) { pdispensing[3] = true; wait4.Signal(); 					// Pump 4 will now be dispensing fuel from tank 3
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 3"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
							case grade_93: { if (T4 > fuel_low) { pdispensing[3] = true; wait4.Signal(); 					// Pump 4 will now be dispensing fuel from tank 4
												MOVE_CURSOR(16, 25); printf("Command accepted"); fflush(stdout); }
										   else { MOVE_CURSOR(16, 25); printf("Please refill tank 4"); fflush(stdout); }	// Tank needs to be refilled -> notify user
										   break; }
						}
					}
					else {																									// The pump is already in use
						MOVE_CURSOR(16, 25); printf("Pump is currently delivering fuel"); fflush(stdout);					// Display message stating usage
					}
				}

				// Refill a fuel tank
				else if (user_command == "R1" || user_command == "R2" || user_command == "R3" || user_command == "R4")		// If command is for refilling a tank
					current_command = user_command;																			// Set global command to entered command for fuel tank thread
					
				// Fuel grade price change
				else if (user_command == "OP") {																			// If user wants to change fuel grade price
					setMode(2);																								// Set mode to octant entry
					MOVE_CURSOR(16, 25); printf("Please change fuel grade prices below"); fflush(stdout);
					changeOctantPrices();																					// Let user change fuel grade prices
					price_updated = true;																					// Set flag to true since prices were changed
				}

				// Enter transaction mode where user can view all transactions
				else if (user_command == "TR") {						// Get copy of all transactions
					if (!transaction_record.empty()) {					// As long as transaction record is not empty store a copy
						present_transactions = transaction_record;
						vptr = present_transactions.begin();			// Set iterator to first transaction
						tran_num = 1;
						displayTransaction(*vptr, tran_num);			// Print transaction to the screen
					}
				}

				// Display previous transaction
				else if (user_command == "PT") {
					if (!present_transactions.empty()) {
						if (vptr == present_transactions.begin()) {		// If currently on first transaction go to last one
							vptr = present_transactions.end() - 1;
							tran_num = present_transactions.size();		// Set transaction number to size of vector -> final transaction
						}
						else {
							vptr--;										// Decrement iterator
							tran_num--;
						}
						displayTransaction(*vptr, tran_num);			// Print transaction to the screen
					}
					else {
						MOVE_CURSOR(16, 25); printf("The command TR must be entered first"); fflush(stdout);
					}
				}

				// Display next transaction
				else if (user_command == "NT") {
					if (!present_transactions.empty()) {
						if (vptr == present_transactions.end() - 1) {	// If currently on last transaction go to first one
							vptr = present_transactions.begin();
							tran_num = 1;								// Set transaction number to 1
						}
						else {
							vptr++;										// Increment iterator
							tran_num++;
						}
						displayTransaction(*vptr, tran_num);			// Print transaction to the screen
					}
					else {
						MOVE_CURSOR(16, 25); printf("The command TR must be entered first"); fflush(stdout);
					}
				}

				// User has entered an invalid command -> notify the user
				else {
					MOVE_CURSOR(16, 25); printf("Invalid Command"); fflush(stdout); 
				}
			}
			
			// User has entered an invalid command -> notify the user
			else {
				MOVE_CURSOR(16, 25); printf("Invalid Command"); fflush(stdout);
			}

			user_command.clear();	// Clear the command string
			setMode(1);				// Set mode to monitor
			Window.Signal();
		}
	}

	PUMP1.WaitForThread();
	PUMP2.WaitForThread();
	PUMP3.WaitForThread();
	PUMP4.WaitForThread();
	FTANK.WaitForThread();
	RTIME.WaitForThread();

	return 0;
}

// Thread for the computer to communicate with Pump 1
UINT __stdcall PumpThread(void *args) {	
	// Need a rendezvous object to start all threads/processes in sync
	CRendezvous program_sync("MyRendezvous", quantity);	// Instance name + number of threads/processes to sync

	// Structure that recieves the arguments passed to the thread
	struct threaddata pumpdata = *(struct threaddata*)(args);

	// Instantiation of a datapool to share pump 1 status with the Gas Station Computer
	CDataPool pump(pumpdata.poolname, sizeof(struct mydatapooldata));	// Give the name pool 1 to declare it communicates with pump 1
	// Struct that exists in the datapool -> holds information regarding the pump 1 status
	struct mydatapooldata  *MyDataPool = (struct mydatapooldata *)(pump.LinkDataPool());

	// Transaction structure
	struct Transaction transaction;

	// Producer and consumer semaphores for the datapool shared with the pump
	CSemaphore CS(pumpdata.consume, 1, 1);
	CSemaphore PS(pumpdata.produce, 0, 1);

	// Wait for command from user
	CSemaphore wait(pumpdata.wait, 0, 1);

	// Wait here until setup is complete for all threads/processes
	program_sync.Wait();

	// Main thread loop
	while (true) {
		PS.Wait();	// Wait until transaction occurs at pump

		transaction.name = string (MyDataPool->name);						// Add customer name from datapool to transaction record 
		transaction.time = currentDateTime();								// Add time of transaction from datapool to transaction record
		transaction.credit = MyDataPool->credit_number;						// Add credit card number from datapool to transaction record
		transaction.fuel_quantity = MyDataPool->fuel_quantity;				// Add fuel quantity from datapool to transaction record
		transaction.total_cost = MyDataPool->total_cost;					// Add total cost from datapool to transaction record

		selected_grade[pumpdata.pump] = MyDataPool->fuel_grade;				// Store selected grade into identifier variable

		// Convert integer fuel grade to a string and put in transaction record
		if (MyDataPool->fuel_grade == grade_87)
			transaction.fuel_grade = "Grade 87";							// Grade is 87 so store 87 as a string
		else if (MyDataPool->fuel_grade == grade_89)
			transaction.fuel_grade = "Grade 89";							// Grade is 89 so store 89 as a string
		else if (MyDataPool->fuel_grade == grade_91)
			transaction.fuel_grade = "Grade 91";							// Grade is 91 so store 91 as a string
		else if (MyDataPool->fuel_grade == grade_93)
			transaction.fuel_grade = "Grade 93";							// Grade is 93 so store 93 as a string
		
		pumpUpdate(transaction, pumpdata.pump);								// Update pump details shown on the window to regard the current customer
		pumpStatus("Waiting", pumpdata.pump, red);							// Set status to waiting in red to notify user to allow fuel pumping

		wait.Wait();														// Wait here until the user has allowed gas to be dispensed
		CS.Signal();														// Pump will start to take fuel from the tank
		pumpStatus("In Progress", pumpdata.pump, yellow);					// Update status to in progress to show that fuel is being pumped

		Vector.Wait();														// Protect vector as it is accessed by multiple threads
		transaction_record.push_back(transaction);							// Place the transaction into a vector
		Vector.Signal();

		PS.Wait();															// Wait until pump finishes delivering fuel

		pumpStatus("Complete", pumpdata.pump, green);						// Set status to complete
		pdispensing[pumpdata.pump - 1] = false;								// Pump is no longer dispensing fuel
		CS.Signal();														// Allow pump to begin speaking to next customer
	}

	return 0;	// terminate child thread
}

// Interacts with the monitor class to send and recieve information regarding the fuel tanks
UINT __stdcall FuelTanks(void *args) {
	// Need a rendezvous object to start all threads/processes in sync
	CRendezvous program_sync("MyRendezvous", quantity);	// Instance name + number of threads/processes to sync

	// Colour variable used to make tank level flash red
	unsigned char colour = grey;

	// Create a tank monitor object
	TankMonitor Tank("FTank", T1, T2, T3, T4, price_87, price_89, price_91, price_93);	// Pass initial tank values and fuel grade prices

	// Initialize the display for all fuel tanks
	Window.Wait();
	
	Window.Signal();

	// Wait here until setup is complete for all threads/processes
	program_sync.Wait();

	// Main thread loop
	while (true) {

		if (current_command == "R1") {			// Notify user that tank has been refilled
			current_command = "";
			Window.Wait(); MOVE_CURSOR(16, 25); printf("Tank 1 refilled                                   "); fflush(stdout); Window.Signal();
			Tank.refuel(grade_87);				// Tell monitor to refill tank 1
		}
		else if (current_command == "R2") {		// Notify user that tank has been refilled
			current_command = "";
			Window.Wait(); MOVE_CURSOR(16, 25); printf("Tank 2 refilled                                   "); fflush(stdout); Window.Signal();
			Tank.refuel(grade_89);				// Tell monitor to refill tank 2
		}
		else if (current_command == "R3") {		// Notify user that tank has been refilled
			current_command = "";
			Window.Wait(); MOVE_CURSOR(16, 25); printf("Tank 3 refilled                                   "); fflush(stdout); Window.Signal();
			Tank.refuel(grade_91);				// Tell monitor to refill tank 3
		}
		else if (current_command == "R4") {		// Notify user that tank has been refilled
			current_command = "";
			Window.Wait(); MOVE_CURSOR(16, 25); printf("Tank 4 refilled                                   "); fflush(stdout); Window.Signal();
			Tank.refuel(grade_93);				// Tell monitor to refill tank 4
		}

		// If the fuel grade prices have changed update the info in the fuel tank monitor
		if (price_updated) {
			price_updated = false;				// Update has gone through so reset this value to false
			Tank.fuel_price_change(price_87, price_89, price_91, price_93);
		}

		T1 = Tank.tank_value(grade_87);			// Assign T1 the fuel level of tank 1
		T2 = Tank.tank_value(grade_89);			// Assign T2 the fuel level of tank 2
		T3 = Tank.tank_value(grade_91);			// Assign T3 the fuel level of tank 3
		T4 = Tank.tank_value(grade_93);			// Assign T4 the fuel level of tank 4

		// Set next colour of fuel level -> either red or grey
		if (colour == 7)
			colour = red;
		else
			colour = grey;

		Sleep(250);								// Sleep .25s so that flashing is reasonably paced

		Window.Wait();

		// If a tank is below 5L notify user immediately
		if (T1 <= 5.0) {
			MOVE_CURSOR(16, 25); TEXT_COLOUR(red); printf("Tank 1 below 5L -> Dispense halted until refill"); fflush(stdout);
		}
		else if (T2 <= 5.0) {
			MOVE_CURSOR(16, 25); TEXT_COLOUR(red); printf("Tank 2 below 5L -> Dispense halted until refill"); fflush(stdout);
		}
		else if (T3 <= 5.0) {
			MOVE_CURSOR(16, 25); TEXT_COLOUR(red); printf("Tank 3 below 5L -> Dispense halted until refill"); fflush(stdout);
		}
		else if (T4 <= 5.0) {
			MOVE_CURSOR(16, 25); TEXT_COLOUR(red); printf("Tank 4 below 5L -> Dispense halted until refill"); fflush(stdout);
		}

		// Check the tank 1 level -> If <= 200 flash red otherwise print in grey
		MOVE_CURSOR(9, 20);
		if (T1 <= fuel_low) {
			TEXT_COLOUR(colour); printf("%-5.1fL", T1); fflush(stdout);		
		}
		else {
			TEXT_COLOUR(grey); printf("%-5.1fL", T1); fflush(stdout);
		}

		// Check the tank 2 level -> If <= 200 flash red otherwise print in grey
		MOVE_CURSOR(9, 21);
		if (T2 <= fuel_low) {
			TEXT_COLOUR(colour); printf("%-5.1fL", T2); fflush(stdout);
		}
		else {
			TEXT_COLOUR(grey); printf("%-5.1fL", T2); fflush(stdout);
		}

		// Check the tank 3 level -> If <= 200 flash red otherwise print in grey
		MOVE_CURSOR(9, 22);
		if (T3 <= fuel_low) {
			TEXT_COLOUR(colour); printf("%-5.1fL", T3); fflush(stdout);
		}
		else {
			TEXT_COLOUR(grey); printf("%-5.1fL", T3); fflush(stdout);
		}

		// Check the tank 4 level -> If <= 200 flash red otherwise print in grey
		MOVE_CURSOR(9, 23);
		if (T4 <= fuel_low) {
			TEXT_COLOUR(colour); printf("%-5.1fL", T4); fflush(stdout);
		}
		else {
			TEXT_COLOUR(grey); printf("%-5.1fL", T4); fflush(stdout);
		}
		TEXT_COLOUR(grey);	// Set colour back to default for when other threads use the window
		Window.Signal();
	}

	return 0;	// terminate child thread
}

// Thread to update the time on the GSC window
UINT __stdcall RealTimeDisplay(void *args)
{
	while (true) {
		Window.Wait();
		MOVE_CURSOR(0, 0);
		TEXT_COLOUR(green);	
		printf("Current time: %s\n", currentDateTime().c_str());	// Display current time in green
		fflush(stdout);
		TEXT_COLOUR(grey);											// Set text colour back to default
		Window.Signal();
		Sleep(200);
	}

	return 0;		// terminate child thread
}

// Function that returns the current date and time as a string
const string currentDateTime()
{
	time_t now = time(0);											// Current time
	struct tm timestruct;
	char buf[80];													// Character buffer to store datetime string
	timestruct = *localtime(&now);									// Put local time into a structure

	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timestruct);	// Return properly formated string

	return buf;
}

// Print transactions
void displayTransaction(struct Transaction t, int num)
{
	MOVE_CURSOR(1, 30);
	printf("Transaction: %i   ", num);							// Print transaction number
	fflush(stdout);
	MOVE_CURSOR(1, 31);
	printf("Time:        %-40s", t.time.c_str());				// Print time of transaction
	fflush(stdout);
	MOVE_CURSOR(1, 32);
	printf("Name:        %-40s  ", t.name.c_str());				// Print customer name
	fflush(stdout);
	MOVE_CURSOR(1, 33);
	printf("Credit #:    %-40i  ", t.credit);					// Print credit info
	fflush(stdout);
	MOVE_CURSOR(1, 34);
	printf("Fuel Grade:  %-40s  ", t.fuel_grade.c_str());		// Print the fuel grade selected
	fflush(stdout);
	MOVE_CURSOR(1, 35);
	printf("Quantity:    %iL       ", t.fuel_quantity);			// Print selected quantity
	fflush(stdout);
	MOVE_CURSOR(1, 36);
	printf("Cost:        $%-40.2f  ", t.total_cost);			// Print total cost of transaction
	fflush(stdout);
}

// Place pump information on window
void pumpUpdate(struct Transaction t, int pump) 
{
	int x, y;											// x and y mouse cursor coordinate variables

	// Set initial x and y coordinates based on which pump called the function
	if (pump == fuelpump1) { x = 23; y = 4; }
	else if (pump == fuelpump2) { x = 74; y = 4; }
	else if (pump == fuelpump3) { x = 23; y = 12; }
	else if (pump == fuelpump4) { x = 74; y = 12; }
	else return;										// If not 1, 2, 3 or 4 then return since pump doesn't exist

	Window.Wait();
	MOVE_CURSOR(x, y);
	printf("%-27s", t.time.c_str());					// Print transaction time
	fflush(stdout);
	MOVE_CURSOR(x, y + 1);
	printf("%-27s", t.fuel_grade.c_str());				// Print selected fuel grade
	fflush(stdout); 
	MOVE_CURSOR(x, y + 2);
	printf("%iL  ", t.fuel_quantity);					// Print selected quantity
	fflush(stdout);
	Window.Signal();
}

// Update the pumps current status
void pumpStatus(string status, int pump, unsigned char colour)
{
	int x, y;											// x and y mouse cursor coordinate variables

	// Set initial x and y coordinates based on which pump called the function
	if (pump == fuelpump1) { x = 23; y = 8; }
	else if (pump == fuelpump2) { x = 74; y = 8; }
	else if (pump == fuelpump3) { x = 23; y = 16; }
	else if (pump == fuelpump4) { x = 74; y = 16; }
	else return;

	Window.Wait();
	MOVE_CURSOR(x, y); TEXT_COLOUR(colour);
	printf("%-27s", status.c_str());					// Print pump status in desired colour
	fflush(stdout); TEXT_COLOUR(grey);
	Window.Signal();
}

// Sets up initial display for the Gas Station Computer -> Creates a quadrant border
void windowInitialization(int i, int y, string pump)
{
	// Give Pump #
	MOVE_CURSOR(i, y);
	printf("%s", pump.c_str());
	fflush(stdout);
	MOVE_CURSOR(i, y + 2);
	printf("Time of purchase:    N/A");			// Display time of purchase
	fflush(stdout);
	MOVE_CURSOR(i, y + 3);
	printf("Selected Fuel Grade: N/A");			// Display selected fuel grade
	fflush(stdout);
	MOVE_CURSOR(i, y + 4);
	printf("Requested Quantitiy: N/A");			// Display quantity of fuel to be delivered to customer
	fflush(stdout);
	MOVE_CURSOR(i, y + 6);
	printf("Status:              Not in use");	// Display initial pump status -> Not in use
	fflush(stdout);
}

// Set the display mode of the window
void setMode(int mode)
{
	MOVE_CURSOR(51, 0);

	if (mode == 1) {
		TEXT_COLOUR(blue);		// Set text colour to blue
		printf("Mode: Monitor      ");
	}
	else if (mode == 2) {
		TEXT_COLOUR(green);
		printf("Mode: Fuel Grade   ");
	}
	else {
		TEXT_COLOUR(cyan);	// Set text colour to cyan
		printf("Mode: Command      ");
	}

	fflush(stdout);
	TEXT_COLOUR(7);			// Set text colour back to default
}

// Place a command legend on the window -> command displayed in green and description in grey
void commandLegend()
{
	MOVE_CURSOR(70, 19);
	printf("Command Legend"); fflush(stdout);
	// Pump commands
	MOVE_CURSOR(70, 20); TEXT_COLOUR(green); printf("F1"); TEXT_COLOUR(grey); printf(": Send fuel to pump 1"); fflush(stdout);
	MOVE_CURSOR(70, 21); TEXT_COLOUR(green); printf("F2"); TEXT_COLOUR(grey); printf(": Send fuel to pump 2"); fflush(stdout);
	MOVE_CURSOR(70, 22); TEXT_COLOUR(green); printf("F3"); TEXT_COLOUR(grey); printf(": Send fuel to pump 3"); fflush(stdout);
	MOVE_CURSOR(70, 23); TEXT_COLOUR(green); printf("F4"); TEXT_COLOUR(grey); printf(": Send fuel to pump 4"); fflush(stdout);
	// Fuel tank commands
	MOVE_CURSOR(70, 24); TEXT_COLOUR(green); printf("R1"); TEXT_COLOUR(grey); printf(": Refill fuel tank 1"); fflush(stdout);
	MOVE_CURSOR(70, 25); TEXT_COLOUR(green); printf("R2"); TEXT_COLOUR(grey); printf(": Refill fuel tank 2"); fflush(stdout);
	MOVE_CURSOR(70, 26); TEXT_COLOUR(green); printf("R3"); TEXT_COLOUR(grey); printf(": Refill fuel tank 3"); fflush(stdout);
	MOVE_CURSOR(70, 27); TEXT_COLOUR(green); printf("R4"); TEXT_COLOUR(grey); printf(": Refill fuel tank 4"); fflush(stdout);
	// Change octant prices
	MOVE_CURSOR(70, 28); TEXT_COLOUR(green); printf("OP"); TEXT_COLOUR(grey); printf(": Change octant prices"); fflush(stdout);
	// Enter Record mode
	MOVE_CURSOR(70, 29); TEXT_COLOUR(green); printf("TR"); TEXT_COLOUR(grey); printf(": Refresh transaction list"); fflush(stdout);
	MOVE_CURSOR(70, 30); TEXT_COLOUR(green); printf("NT"); TEXT_COLOUR(grey); printf(": Next Transaction"); fflush(stdout);
	MOVE_CURSOR(70, 31); TEXT_COLOUR(green); printf("PT"); TEXT_COLOUR(grey); printf(": Previous Transaction"); fflush(stdout);
}

// Set up everything for the display on boot up
void initializeDisplay()
{
	// Initialize the display for pumps
	drawBox(0, 1, 8);
	windowInitialization(2, 2, "Pump 1");
	drawBox(51, 1, 8);
	windowInitialization(53, 2, "Pump 2");
	drawBox(0, 9, 8);
	windowInitialization(2, 10, "Pump 3");
	drawBox(51, 9, 8);
	windowInitialization(53, 10, "Pump 4");

	// Initialize mode display
	setMode(1);

	// Initialize command entry location
	MOVE_CURSOR(1, 25);
	printf("Command Entry:");
	fflush(stdout);

	// Initialize command legend
	commandLegend();

	// Initialize octant prices
	updateOctantDisplay();

	// Initialize fuel tank level
	MOVE_CURSOR(0, 19);
	printf(" Tank Fuel Levels:\n Tank 1: \n Tank 2: \n Tank 3: \n Tank 4: ");
}

// Reprint the fuel grade prices with the up to date values	
void updateOctantDisplay()
{
	MOVE_CURSOR(25, 19);
	printf("Fuel Grade Prices: ");
	fflush(stdout);
	MOVE_CURSOR(25, 20);
	printf("87: $%.2f   ", price_87);	// Print the 87 grade
	fflush(stdout);
	MOVE_CURSOR(25, 21);
	printf("89: $%.2f   ", price_89);	// Print the 89 grade
	fflush(stdout);
	MOVE_CURSOR(25, 22);
	printf("91: $%.2f   ", price_91);	// Print the 91 grade
	fflush(stdout);
	MOVE_CURSOR(25, 23);
	printf("93: $%.2f   ", price_93);	// Print the 93 grade
	fflush(stdout);
	TEXT_COLOUR(grey);					// Set text colour back to default
}

// Let user input new prices for each fuel grade and if the value entered is valid update it
void changeOctantPrices()
{
	char p87[80], p89[80], p91[80], p93[80];

	CURSOR_ON();

	MOVE_CURSOR(1, 26); printf("New 87 grade price: "); fflush(stdout);
	scanf("%9s", p87);
	// Update price if first value entered in numerical
	if (p87[0] == '0' || p87[0] == '1' || p87[0] == '2' || p87[0] == '3' || p87[0] == '4' || p87[0] == '5' || p87[0] == '6' || p87[0] == '7' || p87[0] == '8' || p87[0] == '9')
		price_87 = stof(string(p87));	// Convert user input string to a float

	MOVE_CURSOR(1, 27); printf("New 89 grade price: "); fflush(stdout);
	scanf("%9s", p89);
	// Update price if first value entered in numerical
	if (p89[0] == '0' || p89[0] == '1' || p89[0] == '2' || p89[0] == '3' || p89[0] == '4' || p89[0] == '5' || p89[0] == '6' || p89[0] == '7' || p89[0] == '8' || p89[0] == '9')
		price_89 = stof(string(p89));	// Convert user input string to a float

	MOVE_CURSOR(1, 28); printf("New 91 grade price: "); fflush(stdout);
	scanf("%9s", p91);
	// Update price if first value entered in numerical
	if (p91[0] == '0' || p91[0] == '1' || p91[0] == '2' || p91[0] == '3' || p91[0] == '4' || p91[0] == '5' || p91[0] == '6' || p91[0] == '7' || p91[0] == '8' || p91[0] == '9')
		price_91 = stof(string(p91));	// Convert user input string to a float

	MOVE_CURSOR(1, 29); printf("New 93 grade price: "); fflush(stdout);
	scanf("%9s", p93);
	// Update price if first value entered in numerical
	if (p93[0] == '0' || p93[0] == '1' || p93[0] == '2' || p93[0] == '3' || p93[0] == '4' || p93[0] == '5' || p93[0] == '6' || p93[0] == '7' || p93[0] == '8' || p93[0] == '9')
		price_93 = stof(string(p93));	// Convert user input string to a float
	
	CURSOR_OFF();

	MOVE_CURSOR(16, 25); printf("%-50s", " "); fflush(stdout);	// Clear command input area
	MOVE_CURSOR(1, 26); printf("%50s", " "); fflush(stdout);
	MOVE_CURSOR(1, 27); printf("%50s", " "); fflush(stdout);
	MOVE_CURSOR(1, 28); printf("%50s", " "); fflush(stdout);
	MOVE_CURSOR(1, 29); printf("%50s", " "); fflush(stdout);

	updateOctantDisplay();

}

// Draw the box around the chosen pump specify the upper left corner and how many items should be in it
void drawBox(int x, int y, int items)
{
	MOVE_CURSOR(x + 1, y);
	printf("__________________________________________________");
	fflush(stdout);

	for (int i = 1; i < items + 1; i++) {
		MOVE_CURSOR(x, y + i);
		printf("|                                                  |");
		fflush(stdout);
	}

	MOVE_CURSOR(x + 1, y + items);
	printf("__________________________________________________");
	fflush(stdout);
}