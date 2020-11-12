//comment: author's name, ID, and date.
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
int const NUM_OF_SAMPLES = 5;
//global variables:
int sensors_used[] = { 0 ,0 ,0 };
//random number genrator 
std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<> sensor_pick(0, 2);
std::uniform_int_distribution<> sensor_value_random( 0 ,1000);
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
		
		return (float)sensor_value_random(gen)/1000.0 * 20.0 + 10.0;
	} //end getValue
}; //end class TempSensor
class PressureSensor : public Sensor {
public:
	PressureSensor(string& s) : Sensor(s) {} //constructor
	virtual double getValue() override {
		//return a random value of ambient Pressure between 95 and 105

		return (float)sensor_value_random(gen) / 1000.0 * 10 + 95;
	} //end getValue

}; //end class PressureSensor
class CapacitiveSensor : public Sensor {
public:
	CapacitiveSensor(string& s) : Sensor(s) {} //constructor
	virtual double getValue() override {
		//return a random value of Capacitive between 1 and 5

		return (float)sensor_value_random(gen) / 1000.0 * 4 + 1;
	} //end getValue

}; //end class CapacitiveSensor
class BC {
public:
	//constructor: initialises a vector of Sensor pointers that are
	//passed in by reference:
	BC(std::vector<Sensor*>& sensors) : theSensors(sensors) {}
	void requestBC() {
		BC_mu.lock();
		lock = true;
	}
	bool get_lock() { return lock; }
	double getSensorValue(int selector) {
		return (*theSensors[selector]).getValue();
	}
	string getSensorType(int selector) {
		return theSensors[selector]->getType();
	}
	void releaseBC() {
		if (lock) {
			BC_mu.unlock();
			lock = false;
		}
	}
private:
	bool lock = false; //'false' means that the BC is not locked
	std::vector<Sensor*>& theSensors; //reference to vector of Sensor pointers
	std::mutex BC_mu; //mutex
	
}; //end class BC
//run function – executed by each thread:
void run(BC& theBC ,int idx) { //run(BC& theBC, int idx)
	// array to store how many times each sensors is used 
	
		
		for (int i = 0; i < NUM_OF_SAMPLES; i++) { // NUM_OF_SAMPLES = 50 (initially)
			// request use of the BC:
			//if (theBC.get_lock()) { cout << "Bus is locked " << idx << " suspended " << endl; }
			theBC.requestBC();

			cout << "BusController is locked by " << idx << endl;
			// generate a random value between 0 and 2, and use it to
			int s = sensor_pick(gen);
			
			// select a sensor and obtain a value and the sensor's type:
			double value = theBC.getSensorValue(s);
			string type = theBC.getSensorType(s);
			
			cout << "thread " << idx << " sampled " <<  type << "= " << value << endl; ;
			// increment counter for sensor chosen (to keep count of how many times each was used)
			sensors_used[s] ++;
				// release the BC:
			cout << "BusController is unlocked by " << idx << endl;
			theBC.releaseBC();

				// delay for random period between 0.001s – 0.01s:  // tbd make this a float somehow 
			//std::this_thread::sleep_for(std::chrono::milliseconds(rand() %10 +1 ));
		} // end of for
		
} // end of run

mutex test;
void number(int idx) {

	for (int i = 0; i < 3; i++) {
		test.lock();
		cout << sensor_pick(gen) << " idx = " << idx << endl;
		test.unlock();
	}

}
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
		//the_threads[i] = thread(number ,i);

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