#ifndef __Customer__
#define __Customer__

#include "..\MegaGlobal.h"

using namespace std;

class Customer :
	public ActiveClass
{
private:

	string agent[26] = {
	"Agent A",	"Agent B", 	"Agent C", 	"Agent D", 	"Agent E",	"Agent F",	"Agent G",	"Agent H", 
	"Agent I",	"Agent J",	"Agent K",	"Agent L", 	"Agent M",	"Agent N",	"Agent O",	"Agent P", 
	"Agent Q",	"Agent R",	"Agent S",	"Agent T", 	"Agent U",	"Agent V",	"Agent W",	"Agent X", 
	"Agent Y",	"Agent Z" };

	struct customer_info {									// Customer information will be stored in this struct									
		char name[8];										// Customers name
		int credit_number;									// Credit card number supplied for transactions
		int fuel_grade;										// 4 types -> 87, 89, 91, or 93
		int fuel_quantity;									// 70 liters max

	} customer_info;

	// Instance identifiers
	string pipe_name;
	string mutex_name;

	// Private Functions -> Active class needs a main
	int main(void);

public:
	Customer(string pipe, string mutex);					// Constructor with instance names passed
	~Customer();
};

#endif