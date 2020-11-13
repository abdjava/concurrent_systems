//comment: abdul basit 10188967  13/11/2020
//pre-processor directives:
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <chrono>
#include <random>

using namespace std;
//global constants:
int const MAX_NUM_OF_THREADS = 6;
int const NUM_OF_SAMPLES = 50;
//global variables:
int sensors_used[] = { 0 ,0 ,0 };	// number of times sensors used in order  temperature sensor, pressure sensor, capacitive sensor
//random number generator
std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<> sensor_pick(0, 2);		// number generator used to pick sensors to sample 
std::uniform_int_distribution<> random_1000( 0 ,1000);  // number generator  bewteen 0 and 1000 used to give 3 dp percsion randomness 
//function prototypes: (as required)


class Sensor { //abstract base class that models a sensor
public:
	Sensor(string& type) : sensorType(type) {} //constructor
	//Declare a virtual method to be overridden by derived classes:
	virtual double getValue() = 0;
	//Declare non-virtual method:
	string getType() {
		//returns the type of Sensor that this is:
		return sensorType;
	}

private :
	//Declare instance variable(s):
	string sensorType;
}; //end abstract class Sensor


class TempSensor : public Sensor { //syntax declares a derived class
public:
	TempSensor(string& s) : Sensor(s) {} //constructor
	virtual double getValue  () override {
		//return a random value of ambient temperature between 10 and 30
		
		return (float)random_1000(gen)/1000.0 * 20.0 + 10.0;
	} //end getValue
}; //end class TempSensor


class PressureSensor : public Sensor {
public:
	PressureSensor(string& s) : Sensor(s) {} //constructor
	virtual double getValue() override {
		//return a random value of ambient Pressure between 95 and 105

		return (float)random_1000(gen) / 1000.0 * 10 + 95;
	} //end getValue
}; //end class PressureSensor


class CapacitiveSensor : public Sensor {
public:
	CapacitiveSensor(string& s) : Sensor(s) {} //constructor
	virtual double getValue() override {
		//return a random value of Capacitive between 1 and 5

		return (float)random_1000(gen) / 1000.0 * 4 + 1;
	} //end getValue

}; //end class CapacitiveSensor


class BC {
public:
	//constructor: initialises a vector of Sensor pointers that are
	//passed in by reference:
	BC(std::vector<Sensor*>& sensors) : theSensors(sensors) {}

	// locks access to output console 
	void request_Console() {
		Console_mu.lock();
	}
	//realses access to output Console
	void release_Console() {
		Console_mu.unlock();
	}
	// Locks the bus
	void requestBC() {
		BC_mu.lock();
		lock = true;
	}
	// Releases the bus if locked
	void releaseBC() {
		if (lock) {
			lock = false;
			BC_mu.unlock();
		}
	}
	// to let you check if the bus is locked 
	bool get_lock() { return lock; }
	// sameple sensor 
	double getSensorValue(int selector) {
		return (*theSensors[selector]).getValue();
	}
	// get sensor type 
	string getSensorType(int selector) {
		return theSensors[selector]->getType();
	}

private:
	bool lock = false; //'false' means that the BC is not locked
	std::vector<Sensor*>& theSensors; //reference to vector of Sensor pointers
	std::mutex BC_mu; //mutex fpr bus controller
	std::mutex Console_mu; //mutex for consoule writeing 
	
}; //end class BC


//run function – executed by each thread:
void run(BC& theBC ,int idx) { //run(BC& theBC, int idx)
	/// todo need to clean up this code a bit more 
	
		
		for (int i = 0; i < NUM_OF_SAMPLES; i++) { // NUM_OF_SAMPLES = 50 (initially)
			// checks if bus is free if not locks the output console then prints message 
			if (theBC.get_lock()) {
				theBC.request_Console();
				cout << "Bus is locked " << idx << " suspended " << endl;
				theBC.release_Console();
			}
			// request use of the BC:
			theBC.requestBC();

			// lets you know its been locked 
			theBC.request_Console();
			cout << "BusController is locked by " << idx << endl;
			theBC.release_Console();
			// generate a random value between 0 and 2, and use it to
			int s = sensor_pick(gen);
			
			// select a sensor and obtain a value and the sensor's type:
			double value = theBC.getSensorValue(s);
			string type = theBC.getSensorType(s);
			
			// print the results from sensors 
			theBC.request_Console();
			cout << "thread " << idx << " sampled " <<  type << "= " << value << endl; 
			theBC.release_Console();
			// increment counter for sensor chosen (to keep count of how many times each was used)
			sensors_used[s] ++;
				// release the BC:
			theBC.request_Console();
			cout << "BusController is unlocked by " << idx << endl;
			theBC.release_Console();
			theBC.releaseBC();

				// delay for random period between 0.001s – 0.01s:  3 significant figures 
			std::this_thread::sleep_for(std::chrono::microseconds(random_1000(gen) *9 +1000 ));
		} // end of for
		
} // end of run


int main() {
	//declare a vector of Sensor pointers:
	std::vector<Sensor*> sensors;
	//initialise each sensor and insert into the vector:
	string s = "Temperature Sensor";
	sensors.push_back(new TempSensor( s)); //push_back is a vector method.
	s = "Pressure Sensor";
	sensors.push_back(new PressureSensor (s));
	s = "Capacitive Sensor";
	sensors.push_back(new CapacitiveSensor(s));
	
	// Instantiate the BC:
	BC theBC(std::ref(sensors));
	//instantiate and start the threads:
	std::thread the_threads[MAX_NUM_OF_THREADS]; //array of threads
	for (int i = 0; i < MAX_NUM_OF_THREADS; i++) {
		//launch the threads:
		the_threads[i] = thread(run , std::ref( theBC) ,i );

	}
	//wait for the threads to finish:
	for (int i = 0; i < MAX_NUM_OF_THREADS; i++) {
		the_threads[i].join();
	}

	cout << "All threads terminated" << endl;
	//print out the number of times each sensor was accessed:
	cout << " Temperature Sensor used " << sensors_used[0] << " times "
		<< " Pressure Sensor used " << sensors_used[1] << " times "
		<< " Capacitive Sensor used " << sensors_used[2] << " times " << endl;
		return 0;
} // end of main