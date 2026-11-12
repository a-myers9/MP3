// FuelLog.cpp - Anna Myers - November 9, 2026
// Description: Program reads a trip's fuel log (starting odometer, then one line per fill-up)
// and a fuel budget. Then prints a report with miles, gallons, MPG, cost, and cost per mile for each leg,
// trip totals, the cheapest and most expensive legs per mile, and whether the trip was
// under, over, or exactly on budget (To the exact penny).
// NOTE: Cost comes from the US-dollar amount her credit card charged

#include <iostream>
#include <cstdio>
#include <cmath>

using namespace std;

// Function Prototypes
double toGallons(double amount, char unit);
double fillUpCost(double amount, double pricePerUnit);
double milesPerGallon(double miles, double gallons);
double costPerMile(double cost, double miles);
double sumOf(double values[], int count);
int indexOfSmallest(double values[], int count);
int indexOfLargest(double values[], int count);

int main(void) {
	// Define all constants: max number of fill ups and the unit conversions
	const int MAX_FILLUPS = 20; // no trip has more than 20 fill-ups
	const int CENTS_PER_DOLLAR = 100; // number of cents in one dollar
	const int WARNING_DIFFERENCE = 50; // cents of gap between amount x price and pump total before warning

	// Define arrays that hold the log (note: one array per column)
	// Same index means the same fill-up
	// Index 0 of the odometery array is the starting reading
	double odometer[MAX_FILLUPS + 1]; // odometer reading at the start and at each fill-up
	double amountPumped[MAX_FILLUPS + 1]; // amount pumped at each fill-up
	char unitLetter[MAX_FILLUPS + 1]; // G for gallons or L for liters
	double pricePerUnit[MAX_FILLUPS + 1]; // price per unit as on the pump
	double pumpTotal[MAX_FILLUPS + 1]; // total sale (as on the pump)
	
	double usdCharged[MAX_FILLUPS + 1]; // amount card charged in US dolalrs

	// Define arrays that hold computed values (per leg)
	double legMiles[MAX_FILLUPS]; // miles driven on each leg
	double legGallons[MAX_FILLUPS]; // gallons burned on each leg
	double legMpg[MAX_FILLUPS]; // miles per gallon on each leg
	double legCost[MAX_FILLUPS]; // US dollar cost of each leg
	double legCostPerMile[MAX_FILLUPS]; // cost per mile of each leg

	// Obtain the starting odometer reading through User Input
	cout << "Enter starting odometer reading: ";
	cin >> odometer[0];

	// Obtain the number of fill-ups. Continue asking until it is from 1 to MAX_FILLUPS
	int count; // number of fillups in this trip

	do {
		cout << "Enter number of fill-ups: ";
		cin >> count;

		if (count < 1 || count > MAX_FILLUPS) {
			cout << "Error: number of fill-ups must be from 1 to " << MAX_FILLUPS << "." << endl;
		}
	} while (count < 1 || count > MAX_FILLUPS);
	// Read entire log before computing
	// Each fill-up is entered on one line and is re-entered until it passes each check
	for (int i = 1; i <= count; i++) {
		bool ok = false; // true once this fill up has passed each check

		while (!ok) {
			double od, amt, price, total, usd; // values typed for this fill-up
			char u; // unit letter typed

			printf("Fill-up %d (odometer, amount, G or L, price, total): ", i);
			cin >> od >> amt >> u >> price >> total;

			// Check unit letter first, then odomoter order
			if (u != 'G' && u != 'L') {
				cout << "Error: unit must be G or L. Enter this fill-up again." << endl;
			}
			else if (od <= odometer[i - 1]) {
				cout << "Error: odometer must be greater than the previous reading. Enter this fill-up again." << endl;
			}
			else {
				// Added input for the client: what her card actually charged in US dollars
				// For a US stop this is the pump total. For canadian stop its converted to US dollars
				// US- dollar amount shown on her credit card statement
				cout << "US dollars charged to your card for this fill-up: ";
				cin >> usd;



				// Fill-ups pass every check, so store in arrays
				odometer[i] = od;
				amountPumped[i] = amt;
				unitLetter[i] = u;
				pricePerUnit[i] = price;
				pumpTotal[i] = total;
				usdCharged[i] = usd;
				ok = true;

				// Check verification for rounding: if large gap, then a warning but small gap is normal.
				// Compared in whole cents (ints) to avoid comparing doubles
				int calculatedCents = (int)(fillUpCost(amt, price) * CENTS_PER_DOLLAR + 0.5); // amount x price in cents
				int totalCentsOnPump = (int)(total * CENTS_PER_DOLLAR + 0.5); // pump total in cents
				int gapCents = calculatedCents - totalCentsOnPump; // difference in cents

				if (gapCents < 0) {
					gapCents = -gapCents;
				}
				if (gapCents > WARNING_DIFFERENCE) {
					printf("Warning: amount x price differs from the pump total by $%.2f. Please double check.\n",
						(double)gapCents / CENTS_PER_DOLLAR);

				}
			}
		}
	}

	//Obtain the fuel budget through User input
	double budget; // fuel budget for trip, in US dollars

	cout << "Enter fuel budget for the trip: ";
	cin >> budget;

	// compute each leg now that the whole log has been read.
	for (int i = 1; i <= count; i++) {
		int leg = i - 1; // array index for leg

		legMiles[leg] = odometer[i] - odometer[i - 1];
		legGallons[leg] = toGallons(amountPumped[i], unitLetter[i]);
		legMpg[leg] = milesPerGallon(legMiles[leg], legGallons[leg]);

		// The leg cost is what the card was charged in US dollars
		// Rounded to whole cents the way the bank report does
		// Specifically, rounding each leg keeps trip total matching statement to the penny.
		int legCents = (int)(usdCharged[i] * CENTS_PER_DOLLAR + 0.5); // this leg's cost in whole cents
		legCost[leg] = (double)legCents / CENTS_PER_DOLLAR;
		legCostPerMile[leg] = costPerMile(legCost[leg], legMiles[leg]);
	}

	// Calculate the trip totals from the totals
	double totalMiles = sumOf(legMiles, count);
	double totalGallons = sumOf(legGallons, count);
	double totalCost = sumOf(legCost, count);
	double overallMpg = milesPerGallon(totalMiles, totalGallons);
	double overallCostPerMile = costPerMile(totalCost, totalMiles);

	// Find cheapest and most expensive legs pr mile
	// If a tie, it goes to the earlier leg
	int cheapest = indexOfSmallest(legCostPerMile, count);
	int mostExpensive = indexOfLargest(legCostPerMile, count);

	// Print the table with one row per leg
	printf("\n");
	printf("%-4s %8s %9s %7s %10s %11s\n", "Leg", "Miles", "Gallons", "MPG", "Cost", "Cost/mile");
	for (int leg = 0; leg < count; leg++) {
		printf("%-4d %8.1f %9.2f %7.1f $%9.2f $%10.3f\n",
			leg + 1, legMiles[leg], legGallons[leg], legMpg[leg], legCost[leg], legCostPerMile[leg]);
	}

	// Print the trip totals and cheapest and most expensive legs
	printf("\n");
	printf("Total miles: %.1f\n", totalMiles);
	printf("Total gallons: %.2f\n", totalGallons);
	printf("Overall MPG: %.1f\n", overallMpg);
	printf("Total fuel cost: $%.2f\n", totalCost);
	printf("Overall cost per mile: $%.3f\n", overallCostPerMile);
	printf("Cheapest leg per mile: Leg %d\n", cheapest + 1);
	printf("Most expensive leg per mile: Leg %d\n", mostExpensive + 1);

	// Compare the budget and total in whole cents
	// Exactly on budget means to the penny
	int budgetCents = (int)(budget * CENTS_PER_DOLLAR + 0.5); // budget in whole cents
	int totalCents = (int)(totalCost * CENTS_PER_DOLLAR + 0.5); // total fuel cost in whole cents

	// Report whether trip was under, over, or exactly on budget and by how much
	if (totalCents < budgetCents) {
		printf("Under budget by $%.2f\n", (double)(budgetCents - totalCents) / CENTS_PER_DOLLAR);
	}
	else if (totalCents > budgetCents) {
		printf("Over budget by $%.2f\n", (double)(totalCents - budgetCents) / CENTS_PER_DOLLAR);
	}
	else {
		printf("Exactly on budget ($%.2f)\n", budget);
	}
	return 0;

}

// Returns the amount in Gallons
// Unit is G (Gallons) or L (liters)
double toGallons(double amount, char unit) {
	const double LITERS_PER_GALLON = 3.78541; // Liters in one US Gallon

	// Liters are divided by the conversion factor and gallons are in gallons already
	if (unit == 'L') {
		return amount / LITERS_PER_GALLON;
	}
	return amount;
}

// Returns the cost of one fill-up: the amount pumped times the price per unit
double fillUpCost(double amount, double pricePerUnit) {
	return amount * pricePerUnit;
}

// Returns miles per gallon for the given miles and gallons.
double milesPerGallon(double miles, double gallons) {
	return miles / gallons;
}

// Returns the cost per mile for the given cost and miles.
double costPerMile(double cost, double miles) {
	return cost / miles;
}

// Returns the sum of the first count elements of the array
double sumOf(double values[], int count) {
	double sum = 0.0;

	for (int i = 0; i < count; i++) {
		sum += values[i];
	}
	return sum;
}

// Returns the index of the smallest of first count elements
int indexOfSmallest(double values[], int count) {
	int smallest = 0; // index of the smallest element found 

	// Start at 1 because 0 is the starting guess
	for (int i = 1; i < count; i++) {
		if (values[i] < values[smallest]) {
			smallest = i;
		}
	}
	return smallest;
}

// Returns the index of the largest of the first count elements
int indexOfLargest(double values[], int count) {
	int largest = 0; // index of the largest elembt found 

	// Start at 1 because index 0 is the starting guess
	for (int i = 1; i < count; i++) {
		if (values[i] > values[largest]) {
			largest = i;
		}
	}
	return largest;
}