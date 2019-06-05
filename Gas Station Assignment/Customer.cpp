#include "Customer.h"



Customer::Customer(string pipe, string mutex)
{
	pipe_name = pipe;																	// Pipe instance name
	mutex_name = mutex;																	// Mutex instance name
	
	srand((unsigned)time(0));															// initialize random seed

	int j = rand() % 26;

	for (int i = 0; i < 8; i++) {														// Customers name
		customer_info.name[i] = agent[j][i];
	}
	customer_info.credit_number = (rand()%10000 + rand()%10000 * 10000);				// Credit card number supplied for transactions
	customer_info.fuel_grade = (rand()%4)+1;											// 4 types -> (from 1 - 4) 87, 89, 91, or 93
	customer_info.fuel_quantity = (rand()%70) + 1;										// 70 liters max

	// Need to randomly create data to be stored within the customer info struct
}


Customer::~Customer()
{
}


int Customer::main(void)
{


	// Pipe used to send data to the pump
	CPipe	pipe(pipe_name, 1024);								// Use name given on class instantiation to create a pipe specific to one class instance

	// Mutex creation so that only 1 customer will access a pipe at a time
	CMutex	PipeM(mutex_name);									// Given instance name passed during class instantiation

	// Semaphore to limit the amount of customers to not overexert cpu
	CSemaphore limit("limit", 16, 16);


	// Code to write to into the pipeline communicating with pump
	PipeM.Wait();												// gain access to the pipe
	pipe.Write(&customer_info, sizeof(customer_info));			// write information to the pipe
	PipeM.Signal();												// release access to the pipe

	limit.Signal();												// allow new customer to be generated if limit was reached

	return 0;
}