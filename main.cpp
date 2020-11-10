//comment: author's name, ID, and date.
//pre-processor directives:
#include <iostream>
#include <thread>
#include<vector>
#include<string>
#include <mutex>

using namespace std;
//global constants:
int const MAX_NUM_OF_THREADS = 6;

//global variables:

//function prototypes: (as required)


class Sensor { //abstract base class that models a sensor
public:
	Sensor(string& type) : sensorType(type) {} //constructor
	//Declare a virtual method to be overridden by derived classes:
	virtual double getValue() = 0;
	string sensorType;
	//Declare non-virtual method:
	string getType() {
		//returns the type of Sensor that this is:
		return sensorType;
	}
	//Declare any instance variable(s):
	

}; //end abstract class Sensor


class TempSensor : public Sensor { //syntax declares a derived class
public:
	TempSensor(string& s) : Sensor(s) {} //constructor
	virtual double getValue  () override {
		//return a random value of ambient temperature between 10 and 30
		
		return (float)rand()/ (float)RAND_MAX * 20 + 10;
	} //end getValue
}; //end class TempSensor
class PressureSensor : public Sensor {
	
	PressureSensor(string& s) : Sensor(s) {} //constructor
	virtual double getValue() override {
		//return a random value of ambient Pressure between 95 and 105

		return (float)rand()/ (float)RAND_MAX * 10 + 95;
	} //end getValue

}; //end class PressureSensor
class CapacitiveSensor : public Sensor {

	CapacitiveSensor(string& s) : Sensor(s) {} //constructor
	virtual double getValue() override {
		//return a random value of Capacitive between 1 and 5

		return  (float)rand()/(float)RAND_MAX * 4 + 1;
	} //end getValue

}; //end class CapacitiveSensor
class BC {
public:
	//constructor: initialises a vector of Sensor pointers that are
	//passed in by reference:
	BC(std::vector<Sensor*>& sensors) : theSensors(sensors) {}
	void requestBC() {
		....
	}
	double getSensorValue(int selector) {
		return (*theSensors[selector]).getValue();
	}
	string getSensorType(int selector) {
		....
	}
	void releaseBC() {
		....
	}
private:
	bool lock = false; //'false' means that the BC is not locked
	std::vector<Sensor*>& theSensors; //reference to vector of Sensor pointers
	std::mutex BC_mu; //mutex
	....
}; //end class BC
//run function – executed by each thread:
void run(BC& theBC, int idx) {
	
		....
		for (i = 0; i < NUM_OF_SAMPLES; i++) { // NUM_OF_SAMPLES = 50 (initially)
		// request use of the BC:
			....
				// generate a random value between 0 and 2, and use it to
				// select a sensor and obtain a value and the sensor's type:
				....
				// increment counter for sensor chosen (to keep count of
				// how many times each was used)
				// release the BC:
				....
				// delay for random period between 0.001s – 0.01s:
				....
		} // end of for
} // end of run
int main() {
	//declare a vector of Sensor pointers:
	std::vector<Sensor*> sensors;
	//initialise each sensor and insert into the vector:
	string s = "temperature sensor";
	sensors.push_back(new TempSensor(s)); //push_back is a vector method.
	....
		// Instantiate the BC:
		BC theBC(std::ref(sensors));
	//instantiate and start the threads:
	std::thread the_threads[MAX_NUM_OF_THREADS]; //array of threads
	for (int i = 0; i < MAX_NUM_OF_THREADS; i++) {
		//launch the threads:
		....
	}
	//wait for the threads to finish:
	....
		cout << "All threads terminated" << endl;
	//print out the number of times each sensor was accessed:
	....
		return 0;
} // end of main