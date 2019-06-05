#ifndef __Pump__
#define __Pump__

#include "..\MegaGlobal.h"

class Pump :
	public ActiveClass
{
private:
	struct info {											// Customer information will be stored in this struct
		char name[8];										// Customers name
		int credit_number;									// Credit card number supplied for transactions
		int fuel_grade;										// 4 types -> 87, 89, 91, or 93
		int fuel_quantity;									// 70 liters max
	} info;

	struct mydatapooldata {	// Pump info will be stored here to be sent to the gas station computer
		char name[8];										// Customers name
		int credit_number;									// Credit card number supplied for transactions
		int fuel_grade;										// 4 types -> 87, 89, 91, or 93
		int fuel_quantity;									// 70 liters max
		float total_cost;									// total cost for transaction
	};

	// Instance identifiers
	string pipe_name;										// Name of the pipeline instance
	string pool_name;										// Name of the datapool instance
	string consumer_name;									// Name of the consumer instance
	string producer_name;									// Name of the producer instance

	// Private variables
	float total;											// Total Cost for transaction
	float counter;											// Keep track of amount of fuel dispensed
	int Customer_count = 0;										// Keep track of how many customers served
	
	// Private Functions //

	// Active class needs a main
	int main(void);

	// Sets up initial display for the Gas Station -> Creates a quadrant border
	void windowInitialization(int i, int y, float total, string pump);

	// Converts integer fuel grade into strings
	string fuel_grade_display(int grade);

	// creates inital box for GUI
	void drawBox(int x, int y, int items);

public:

	int served_customers();

	// Constructor & Destrcutor
	Pump(string pipe, string pool, string producaphore, string consumaphore);	// Constructor with instance names passed
	~Pump();
};

#endif