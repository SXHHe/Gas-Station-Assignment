#include "TankMonitor.h"



TankMonitor::TankMonitor(string Name)
{
	tankMutex = new CMutex(string("__Mutex__") + string(Name));
	tankDataPool = new CDataPool(string("__DataPool__") + string(Name), sizeof(struct tankData));
	T_ptr = (struct tankData *)(tankDataPool->LinkDataPool());
}

TankMonitor::TankMonitor(string Name, float ifuel1, float ifuel2, float ifuel3, float ifuel4, float ifuelp1, float ifuelp2, float ifuelp3, float ifuelp4)
{
	tankMutex = new CMutex(string("__Mutex__") + string(Name));
	tankDataPool = new CDataPool(string("__DataPool__") + string(Name), sizeof(struct tankData));
	T_ptr = (struct tankData *)(tankDataPool->LinkDataPool());

	T_ptr->tank_fuel[1] = ifuel1;
	T_ptr->tank_fuel[2] = ifuel2;
	T_ptr->tank_fuel[3] = ifuel3;
	T_ptr->tank_fuel[4] = ifuel4;

	T_ptr->price_87 = ifuelp1;
	T_ptr->price_89 = ifuelp2;
	T_ptr->price_91 = ifuelp3;
	T_ptr->price_93 = ifuelp4;
}


TankMonitor::~TankMonitor()
{
}

// insert price for fuel grades, 87, 89, 91, 93 respectively
void TankMonitor::fuel_price_change(float a, float b, float c, float d)
{
	tankMutex->Wait();
	T_ptr->price_87 = a;
	T_ptr->price_89 = b;
	T_ptr->price_91 = c;
	T_ptr->price_93 = d;
	tankMutex->Signal();
}

// funtion to return amount of fuel left in tank
float TankMonitor::tank_value(int i)
{
	//tankMutex->Wait();
	return T_ptr->tank_fuel[i];
	//tankMutex->Signal();
}

// Returns the cost of the different fuel grades
float TankMonitor::grade_cost_L(int grade)
{
	//tankMutex->Wait();
	if (grade == 1) {
		return T_ptr->price_87;
	}
	else if (grade == 2) {
		return T_ptr->price_89;
	}
	else if (grade == 3) {
		return T_ptr->price_91;
	}
	else if (grade == 4) {
		return T_ptr->price_93;
	}
	else {
		return 0;
	}
	//tankMutex->Signal();
}


// funtion to refill tank i to 500L
void TankMonitor::refuel(int i)
{
	tankMutex->Wait();
	T_ptr->tank_fuel[i] = 500;
	tankMutex->Signal();
}


// funtion to dispence amount of fuel from tank i at a rate of 0.5L/s
// incomplete, needs to incorporate rate and allow customers to finish fueling even if the tank drops below 200L
void TankMonitor::dispense(int i)
{
	tankMutex->Wait();													// dispense fuel
	T_ptr->tank_fuel[i] = T_ptr->tank_fuel[i] - 0.5;
	tankMutex->Signal();

}
