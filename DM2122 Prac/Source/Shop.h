#pragma once

#include "Player.h"
#include "CarsList.h"
#include "CCar.h"
#include "irrKlang.h"

class Shop
{
private:
	//int priceOfCar[3];

public:
	Shop();
	~Shop();

	bool buyCar(CCar* whichCar);

	///void sellCar(int whichCar);
};