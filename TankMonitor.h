#ifndef __TankMonitor__
#define __TankMonitor__

#include "rt.h"
#include <stdio.h>
#include <string.h>

class TankMonitor
{
private:
	struct tankData {
		// Floats to store liters remaining in each fuel tank	
		float tank_fuel[5];

		// Fuel octant prices
		float price_87;
		float price_89;
		float price_91;
		float price_93;
	};

	// Pointer to data
	tankData *T_ptr;

	// Pointer to a hidden mutex protecting the variables
	CMutex *tankMutex;

	// datapool to contain the fuel quantities
	CDataPool *tankDataPool;

public:

	// insert price for fuel grades, 87, 89, 91, 93 respectively
	void fuel_price_change(float a, float b, float c, float d);

	// funtion to return amount of fuel left in tank
	float tank_value(int i);

	// funtion to return cost/L of requested fuel grade
	float grade_cost_L(int grade);

	// funtion to refill tank i to 2000
	void refuel(int i);

	// funtion to dispence amount of fuel from tank i
	void dispense(int i);


	// constructor, deconstructor
	TankMonitor(string Name);
	TankMonitor(string Name, float fuel1, float fuel2, float fuel3, float fuel4, float fuelp1, float fuelp2, float fuelp3, float fuelp4 );
	~TankMonitor();
};

#endif