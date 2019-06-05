#include "Pump.h"



int Pump::served_customers()
{
	return Customer_count;
}

Pump::Pump(string pipe, string pool, string producaphore, string consumaphore)
{
	pipe_name = pipe;				// Name of the pipeline instance
	pool_name = pool;				// Name of the datapool instance
	consumer_name = consumaphore;	// Name of the consumer instance
	producer_name = producaphore;	// Name of the producer instance
}


Pump::~Pump()
{
}

int Pump::main(void)
{
	// Need a rendezvous object to start all threads/processes in sync
	CRendezvous program_sync("MyRendezvous", quantity);											// Instance name + number of threads/processes to sync

	// Producer and consumer semaphore for the datapool shared with the pump and the gas station computer
	CSemaphore CS(consumer_name, 1, 1);
	CSemaphore PS(producer_name, 0, 1);

	// Instantiation of a datapool to share pump status with the Gas Station Computer
	CDataPool pump(pool_name, sizeof(struct mydatapooldata));								// Use name given on class instantiation to create a pool specific to one class instance

	// Struct that exists in the datapool -> holds information regarding the pump status
	struct mydatapooldata  *MyDataPool = (struct mydatapooldata *)(pump.LinkDataPool());

	// Pipe used to recieve data from the customer
	CPipe	pipe(pipe_name, 1024);															// Use name given on class instantiation to create a pipe specific to one class instance

	// Mutex to use when writing to consol window
	CMutex	Window("Customers Window");

	// Monitor for 4 tanks
	TankMonitor Tank("FTank");

	// Draw initial box. Location depends on the pump#
	if (pipe_name == "Pipe1") {
		// Initialize the display for pump 1 -> Upper left quadrant
		Window.Wait();
		drawBox(0, 1, 9);
		Window.Signal();
	}
	else if (pipe_name == "Pipe2") {
		// Initialize the display for pump 2 -> Upper right quadrant
		Window.Wait();
		drawBox(51, 1, 9);
		Window.Signal();
	}
	else if (pipe_name == "Pipe3") {
		// Initialize the display for pump 3 -> Lower left quadrant
		Window.Wait();
		drawBox(0, 10, 9);
		Window.Signal();
	}
	else if (pipe_name == "Pipe4") {
		// Initialize the display for pump 4 -> Lower right quadrant
		Window.Wait();
		drawBox(51, 10, 9);
		Window.Signal();
	}

	// Wait here until setup is complete for all threads/processes
	program_sync.Wait();

	// Need to forever loop

	while (TRUE) {
		// Read item form datapool
		pipe.Read(&info, sizeof(info));

		// Claculate the total cost for the transaction
		total = (Tank.grade_cost_L(info.fuel_grade))*(info.fuel_quantity);

		// To keep track of how much more fuel need to be dispensed
		counter = float(info.fuel_quantity);

		// Add count to transactions
		Customer_count ++;

		// Display customer info to window. Location depends on the pump#
		if (pipe_name == "Pipe1") {
			// Initialize the display for pump 1 -> Upper left quadrant
			Window.Wait();
			drawBox(0, 1, 9);
			windowInitialization(2, 2, total, "Pump 1");
			Window.Signal();
		}
		else if (pipe_name == "Pipe2") {
			// Initialize the display for pump 2 -> Upper right quadrant
			Window.Wait();
			drawBox(51, 1, 9);
			windowInitialization(53, 2, total, "Pump 2");
			Window.Signal();
		}
		else if (pipe_name == "Pipe3") {
			// Initialize the display for pump 3 -> Lower left quadrant
			Window.Wait();
			drawBox(0, 10, 9);
			windowInitialization(2, 11, total, "Pump 3");
			Window.Signal();
		}
		else if (pipe_name == "Pipe4") {
			// Initialize the display for pump 4 -> Lower right quadrant
			Window.Wait();
			drawBox(51, 10, 9);
			windowInitialization(53, 11, total, "Pump 4");
			Window.Signal();
		}

		// produce customer info data to the GSC
		CS.Wait();
		
		strcpy(MyDataPool->name , info.name);
		MyDataPool->credit_number = info.credit_number;
		MyDataPool->fuel_grade = info.fuel_grade;
		MyDataPool->fuel_quantity = info.fuel_quantity;
		MyDataPool->total_cost = total;

		PS.Signal();

		// After GSC signals we dispense fuel

		CS.Wait();

		// loop for dispensing fuel
		while (counter != 0) {
			if (Tank.tank_value(info.fuel_grade) <= 5) {				// dont dispense fuel if less than 5
				Sleep(1000);
			}
			else {													
				Tank.dispense(info.fuel_grade);
				counter = counter - 0.5;
				// Display customer info to window. Location depends on the pump#
				if (pipe_name == "Pipe1") {
					// Initialize the display for pump 1 -> Upper left quadrant
					Window.Wait();
					// Display progress of fuel dispension
					MOVE_CURSOR(2, 8);
					printf("Dispensing Process  :  %.1fL / %2iL", (info.fuel_quantity - counter), info.fuel_quantity);
					Window.Signal();
				}
				else if (pipe_name == "Pipe2") {
					// Initialize the display for pump 2 -> Upper right quadrant
					Window.Wait();
					// Display progress of fuel dispension
					MOVE_CURSOR(53, 8);
					printf("Dispensing Process  :  %.1fL / %2iL", (info.fuel_quantity - counter), info.fuel_quantity);
					Window.Signal();
				}
				else if (pipe_name == "Pipe3") {
					// Initialize the display for pump 3 -> Lower left quadrant
					Window.Wait();
					// Display progress of fuel dispension
					MOVE_CURSOR(2, 17);
					printf("Dispensing Process  :  %.1fL / %2iL", (info.fuel_quantity - counter), info.fuel_quantity);
					Window.Signal();
				}
				else if (pipe_name == "Pipe4") {
					// Initialize the display for pump 4 -> Lower right quadrant
					Window.Wait();
					// Display progress of fuel dispension
					MOVE_CURSOR(53, 17);
					printf("Dispensing Process  :  %.1fL / %2iL", (info.fuel_quantity - counter), info.fuel_quantity);
					Window.Signal();
				}
				Sleep(1000);
			}
		}

		// Clear box of customer
		// Draw initial box. Location depends on the pump#
		if (pipe_name == "Pipe1") {
			// Initialize the display for pump 1 -> Upper left quadrant
			Window.Wait();
			drawBox(0, 1, 9);
			Window.Signal();
		}
		else if (pipe_name == "Pipe2") {
			// Initialize the display for pump 2 -> Upper right quadrant
			Window.Wait();
			drawBox(51, 1, 9);
			Window.Signal();
		}
		else if (pipe_name == "Pipe3") {
			// Initialize the display for pump 3 -> Lower left quadrant
			Window.Wait();
			drawBox(0, 10, 9);
			Window.Signal();
		}
		else if (pipe_name == "Pipe4") {
			// Initialize the display for pump 4 -> Lower right quadrant
			Window.Wait();
			drawBox(51, 10, 9);
			Window.Signal();
		}

		PS.Signal();

	}

	Sleep(5000);

	return 0;
}


// Sets up initial display for the Gas Station -> Creates a quadrant border
void Pump::windowInitialization(int i, int y, float total, string pump)
{
	// Give Pump #
	MOVE_CURSOR(i, y);
	printf("%s", pump.c_str());
	// Display Name of customer
	MOVE_CURSOR(i, y + 2);
	printf("Name                :  %s ", info.name);
	// Display Credit card number
	MOVE_CURSOR(i, y + 3);
	printf("Credit Card #       :  %06i",info.credit_number);
	// Display grade of fuel to be delivered to customer
	MOVE_CURSOR(i, y + 4);
	printf("Selected Fuel Grade :  %8s",fuel_grade_display(info.fuel_grade).c_str());
	// Display quantity of fuel
	MOVE_CURSOR(i, y + 5);
	printf("Requested Quantitiy :  %2iL",info.fuel_quantity);
	// Display progress of fuel dispension
	MOVE_CURSOR(i, y + 6);
	printf("Dispensing Process  :  %.1fL / %2iL", (info.fuel_quantity - counter),info.fuel_quantity);
	// Display Total Cost
	MOVE_CURSOR(i, y + 7);
	printf("Total Cost          :  $%.2f ", total);
	fflush(stdout);
}

// creates inital box for GUI
void Pump::drawBox(int x, int y, int items)
{
	MOVE_CURSOR(x + 1, y);
	printf("__________________________________________________");

	for (int i = 1; i < items + 1; i++) {
		MOVE_CURSOR(x, y + i);
		printf("|                                                  |");
	}

	MOVE_CURSOR(x + 1, y + items);
	printf("__________________________________________________");
	fflush(stdout);
}

// Converts integer fuel grade into strings
string Pump::fuel_grade_display(int grade)
{
	if (grade == 1) {
		return string("Grade 87");
	}
	else if (grade == 2) {
		return string("Grade 89");
	}
	else if (grade == 3) {
		return string("Grade 91");
	}
	else if (grade == 4) {
		return string("Grade 93");
	}
	else {
		return string("N/A");
	}
}
