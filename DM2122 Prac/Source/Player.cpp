#include "Player.h" 

Player::Player()
{
	bool check = false;

	check = hasFile();
	
	if (check == false)
	{
		createFile();

		initMoney();

	}
	else
	{
		initMoney();

		initOwnedCars();
	}
}

Player::~Player()
{
	rewriteFile();
}

bool Player::hasFile()
{
	ifstream runTime(File);

	if (runTime.fail())
	{
		std::cout << "Unable to open file!" << std::endl;

		return false;
	}
	else
	{
		return true;
	}
}

void Player::createFile()
{
	ofstream runTime(File);

	runTime << "50c464d33fd802fc1bd247499d037340\n";
	runTime << "Player 1\n";
	runTime << "fb1217a508f8bf839aba8b87afaef694\n";
	runTime << to_string(startingBalance) + "\n";
	runTime << "b799f7f2affa11d74804bd574f04de8a\n";
	runTime << "OWNED\n";
	runTime << "UNOWNED\n";
	runTime << "UNOWNED\n";
	runTime << "UNOWNED\n";
	runTime << "UNOWNED\n";
}

void Player::initMoney()
{
	ifstream runTime(File);

	string line;

	int lineNumber = 1;

	if (runTime.is_open())
	{
		while (getline(runTime, line))
		{
			if (lineNumber == 4)
			{
				money = stoi(line);
			}

			lineNumber++;

			if (lineNumber == 11) // Break out of look such that doesnt check other lines.
			{
				break;
			}
		}
	}
}

void Player::initOwnedCars()
{
	ifstream runTime(File);

	string line;

	int lineNumber = 1;

	if (runTime.is_open())
	{
		while (getline(runTime, line))
		{
			if (lineNumber == 6)
			{
				if (line == "OWNED")
				{
					carsOwned[0] = line;
				}
				else
				{
					carsOwned[0] = line;
				}
			}

			if (lineNumber == 7)
			{
				if (line == "OWNED")
				{
					carsOwned[1] = line;
				}
				else
				{
					carsOwned[1] = line;
				}
			}

			if (lineNumber == 8)
			{
				if (line == "OWNED")
				{
					carsOwned[2] = line;
				}
				else
				{
					carsOwned[2] = line;
				}
			}

			if (lineNumber == 9)
			{
				if (line == "OWNED")
				{
					carsOwned[3] = line;
				}
				else
				{
					carsOwned[3] = line;
				}
			}

			if (lineNumber == 10)
			{
				if (line == "OWNED")
				{
					carsOwned[4] = line;
				}
				else
				{
					carsOwned[4] = line;
				}
			}

			lineNumber++;

			if (lineNumber == 11) // Break out of look such that doesnt check other lines.
			{
				break;
			}
		}
	}
}

void Player::removeMoney(int amountToBeRemoved) // Function to remove money.
{
	int tempBalance = getMoney() - amountToBeRemoved;

	bool isItNegativeBalance = false;

	if (tempBalance < 0) // Checks if Player's Balance is negative.
	{
		isItNegativeBalance = true;
	}

	if (isItNegativeBalance == true) // If Player's Balance is negative, then money wont be removed.
	{
		std::cout << "Not enough money!" << std::endl;
	}
	else
	{
		money = getMoney() - amountToBeRemoved;
	}
}

void Player::addMoney(int amountToBeAdded) // Function to add money.
{
	money = getMoney() + amountToBeAdded;
}

void Player::removeCar(int whichCar)
{
	carsOwned[whichCar] = "UNOWNED";
}

void Player::addCar(int whichCar)
{


	carsOwned[whichCar] = "OWNED";
}

int Player::getMoney()
{
	return money;
}

void Player::PrintOwnedCars()
{
	std::cout << "Car 1: " << carsOwned[0] << std::endl;
	std::cout << "Car 2: " << carsOwned[1] << std::endl;
	std::cout << "Car 3: " << carsOwned[2] << std::endl;
	std::cout << "Car 4: " << carsOwned[3] << std::endl;
	std::cout << "Car 5: " << carsOwned[4] << std::endl;
}

bool Player::hasCar(int whichCar) // If returns TRUE, Player own the car.
{
	if (carsOwned[whichCar] ==  "OWNED")
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Player::rewriteFile()
{
	ifstream readTest(File);
	string line;

	int lineNumber = 1;

	if (readTest.is_open())
	{
		while (getline(readTest, line))
		{
			if (lineNumber == 1)
			{
				line1 = line;
			}

			if (lineNumber == 2)
			{
				line2 = line;
			}

			if (lineNumber == 3)
			{
				line3 = line;
			}

			if (lineNumber == 4)
			{
				line4 = to_string(getMoney());
			}

			lineNumber++;

			if (lineNumber == 11) // Break out of look such that doesnt check other lines.
			{
				break;
			}
		}
	}

	if (remove("player.txt") != 0)
	{
		perror("Error deleting file");
	}
	else
	{
		puts("File successfully deleted");
	}

	fstream write(File);

	write << line1 + "\n";
	write << line2 + "\n";
	write << line3 + "\n";
	write << line4 + "\n";
}