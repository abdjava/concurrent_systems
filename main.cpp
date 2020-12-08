//comment: abdul basit 10188967  13/11/2020
//pre-processor directives:
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <chrono>
#include <random>
#include <condition_variable>
#include <map>


using namespace std;
//global constants:
int const MAX_NUM_OF_THREADS = 6;
int const NUM_OF_SAMPLES = 50;
int const NUM_OF_LINKS = 2;
std::pair<double, double> Temperature_range{30.0 ,10.0 };   // upper lower
std::pair<double, double> Pressure_range{ 105.0 ,95.0 };   // upper lower
std::pair<double, double> Capacitive_range{ 1.0 ,5.0 };   // upper lower
//global variables:
int sensors_used[] = { 0 ,0 ,0 };	// number of times sensors used in order  temperature sensor, pressure sensor, capacitive sensor
// sensors names
string s1 = "Temperature Sensor";
string s2 = "Pressure Sensor";
string s3 = "Capacitive Sensor";

std::map<std::thread::id, int> threadIDs; //declaring map which associates thread with an integer id

std::mutex Console_mu; //mutex for console writeing
std::mutex set_id;// mutex used to set thread id


//random number generator
std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<> sensor_pick(0, 2);		// number generator used to pick sensors to sample
std::uniform_int_distribution<> random_1000( 0 ,1000);  // number generator  bewteen 0 and 1000 used to give 3 dp percsion randomness

void print_Console(std::string to_print) {
	std::lock_guard <mutex> lock(Console_mu);
	std::cout << to_print << endl;
}

int GetThreadID()
{
	std::map <std::thread::id, int>::iterator it = threadIDs.find(std::this_thread::get_id());
	if (it == threadIDs.end()) return -1; 	//thread 'id' NOT found
	else return it->second; 				//thread 'id' found, return the associated integer –note the syntax

}

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

		return (float)random_1000(gen)/1000.0 * (Temperature_range.first  -Temperature_range.second) + Temperature_range.second;
	} //end getValue
}; //end class TempSensor


class PressureSensor : public Sensor {
public:
	PressureSensor(string& s) : Sensor(s) {} //constructor
	virtual double getValue() override {
		//return a random value of ambient Pressure between 95 and 105

		return (float)random_1000(gen) / 1000.0 * (Pressure_range.first - Pressure_range.second)+ Pressure_range.second;
	} //end getValue
}; //end class PressureSensor


class CapacitiveSensor : public Sensor {
public:
	CapacitiveSensor(string& s) : Sensor(s) {} //constructor
	virtual double getValue() override {
		//return a random value of Capacitive between 1 and 5

		return (float)random_1000(gen) / 1000.0 * (Capacitive_range.first - Capacitive_range.second) + Capacitive_range.second;
	} //end getValue

}; //end class CapacitiveSensor

class SensorData { //Utility class to store sensor data
public:
	SensorData(string type) //Constructor
		: sensor_type(type) {}
	string getSensorType() { return sensor_type; }
	std::vector<double> getSensorData() { return sensor_data; }
	void addData(double newData) { sensor_data.push_back(newData); }
private:

	std::vector<double> sensor_data;
	string sensor_type;
}; //end class SensorData

class Receiver {
public:
	Receiver() { } //constructor
	//Receives a SensorData object:
	void receiveData(SensorData sd) {

		if ( !sd.getSensorType().compare(s1)) {
			save_data(temperature_data , sd.getSensorData() , &mu_temperature);
		}
		else if (!sd.getSensorType().compare(s2)) {
			save_data(pressure_data, sd.getSensorData() , &mu_pressure);
		}
		else {
			save_data(capacitive_data , sd.getSensorData() , &mu_capacitive);
		}

	}
	// print out all data for each sensor:
	void printSensorData() {
		cout << "idx  = data  for temperature  " << endl;
		print_vector(temperature_data);
		cout << "idx  = data  for pressure  " << endl;
		print_vector(pressure_data);
		cout << "idx  = data  for capacitive  " << endl;
		print_vector(capacitive_data);

	}

private:
	void print_vector(std::vector<double> vector) {
		for (int i = 0; i< vector.size(); i++) {
			cout << i <<" = " << vector.at(i) << endl;
		}
	}

	void save_data(std::vector <double>& save_to , std::vector <double> save_from  ,std::mutex* mu) {
		lock_guard<mutex> lock(*mu);
		for (int i = 0; i < save_from.size(); i++) {
			save_to.push_back(save_from.at(i));
		}
	}
	//mutex:
	std::mutex  mu_temperature;
	std::mutex  mu_pressure;
	std::mutex  mu_capacitive;
	//vectors to store sensor numeric data received from threads:
	std::vector<double> temperature_data;
	std::vector<double> pressure_data;
	std::vector<double> capacitive_data;

}; //end class Receiver


class Link {
public:
	Link(Receiver& r, int linkNum) //Constructor
		: inUse(false), myReceiver(r), linkId(linkNum)
	{}
	//check if the link is currently in use
	bool isInUse() {
		return inUse;
	}
	//set the link status to busy
	void setInUse() {
		inUse = true;
	}
	//set the link status to idle
	void setIdle() {
		inUse = false;
	}
	//write data to the receiver
	void writeToDataLink( std::vector<SensorData> sd) {
		for (int i =0 ; i <sd.size() ;i++)
			myReceiver.receiveData(sd.at(i));
	}
	void writeToDataLink(SensorData sd) {

			myReceiver.receiveData(sd);
	}
	//returns the link Id
	int getLinkId() {
		return linkId;
	}
private:
	bool inUse;
	Receiver& myReceiver; //Receiver reference
	int linkId;
}; //end class Link


class LinkAccessController {
public:
	LinkAccessController(Receiver& r) //Constructor
		: myReceiver(r), numOfAvailableLinks(NUM_OF_LINKS)
	{
		for (int i = 0; i < NUM_OF_LINKS; i++) {
			commsLinks.push_back(Link(myReceiver, i));
		}
	}
	//Request a comm's link: returns a reference to an available Link.
	//If none are available, the calling thread is suspended.
	Link& requestLink() {
		unique_lock <mutex> lock(LAC_mu);
		int i = 0;
		for (; i < NUM_OF_LINKS; i++) {
			if (!commsLinks.at(i).isInUse() ) {
				numOfAvailableLinks--;
				commsLinks[i].setInUse();
				break;
			}
			else if (i == NUM_OF_LINKS - 1) {
				i = -1;
				print_Console(string("Thread " + std::to_string(GetThreadID()) + string("  is suspended waiting for link") ));
				while (numOfAvailableLinks == 0) {
					avaiable_links.wait(lock );
				}

			}
		}
		print_Console(string("Thread ") + std::to_string(GetThreadID()) + string(" has locked link ") +std::to_string(i) );
			return commsLinks.at(i);
	}
	//Release a comms link:
	void releaseLink(Link& releasedLink) {
		unique_lock <mutex> lock(LAC_mu);
		releasedLink.setIdle();
		print_Console(string("Thread ") + to_string(GetThreadID()) + string(" has unlocked link ") + to_string(releasedLink.getLinkId()));
		numOfAvailableLinks++;
		avaiable_links.notify_one();
	}
	//continued

private:
	Receiver & myReceiver; //Receiver reference
	int numOfAvailableLinks;
	std::vector<Link> commsLinks;
	std::mutex LAC_mu; //mutex
	std::condition_variable avaiable_links;

}; //end class LinkAccessController




class BC {
public:
	//constructor: initialises a vector of Sensor pointers that are
	//passed in by reference:
	BC(std::vector<Sensor*>& sensors) : theSensors(sensors) {}


	// Locks the bus
	void requestBC() {
		std::unique_lock <std::mutex> lock_mu(BC_mu);
		while (locked)
		{
			print_Console(std::string("Bus is locked ") + std::to_string(GetThreadID()) + std::string(" suspended "));
			BC_avaible.wait(lock_mu);
		}

		locked = true;
		print_Console(std::string("BusController is locked by ") + std::to_string(GetThreadID()));
	}
	// Releases the bus if locked
	void releaseBC() {
		std::unique_lock <std::mutex> lock_mu(BC_mu);
		if (locked) {
			locked = false;
			print_Console(std::string("BusController is unlocked by ") + std::to_string(GetThreadID()));
			BC_avaible.notify_one();
		}
	}
	// to let you check if the bus is locked
	bool get_lock() { return locked; }
	// sample sensor
	double getSensorValue(int selector) {
		return (*theSensors[selector]).getValue();
	}
	// get sensor type
	string getSensorType(int selector) {
		return theSensors[selector]->getType();
	}

private:
	bool locked = false; //'false' means that the BC is not locked
	std::vector<Sensor*>& theSensors; //reference to vector of Sensor pointers
	std::mutex BC_mu; //mutex fpr bus controller
	std::condition_variable BC_avaible ;
}; //end class BC


//run function – executed by each thread:
void run(BC& theBC , LinkAccessController& LAC , int idx ) { //run(BC& theBC, int idx)
	/// main run funcation
	set_id.lock();
	threadIDs.insert(std::make_pair(std::this_thread::get_id() , idx));
	set_id.unlock();

	// data stroage
	std::vector <SensorData> data;
	data.push_back(SensorData(s1));
	data.push_back(SensorData(s2));
	data.push_back(SensorData(s3));

		for (int i = 0; i < NUM_OF_SAMPLES; i++) { // NUM_OF_SAMPLES = 50 (initially)


			// request use of the BC:
			theBC.requestBC();

			// generate a random value between 0 and 2, and use it to
			int s = sensor_pick(gen);

			// select a sensor and obtain a value and the sensor's type:
			double value = theBC.getSensorValue(s);
			string type = theBC.getSensorType(s);
			sensors_used[s] ++;// increment counter for sensor chosen (to keep count of how many times each was used)
			// print the results from sensors
			print_Console(std::string("thread ") + std::to_string(idx) + std::string(" sampled ") + type +std::string(" ") +std::to_string(value));

			data.at(s).addData(value);

			// release the BC:

			theBC.releaseBC();


			// delay for random period between 0.001s – 0.01s:  3 significant figures
			std::this_thread::sleep_for(std::chrono::microseconds(random_1000(gen) *9 +1000 ));
		} // end of for
		// transmit data
		for (auto data_item : data) {
			Link& tramistion_link = LAC.requestLink();
			print_Console(string("Thread ") +std::to_string(GetThreadID()) +string(" transmitting data via link ") +std::to_string(tramistion_link.getLinkId()));
			tramistion_link.writeToDataLink(data_item);
			LAC.releaseLink(tramistion_link);
			std::this_thread::sleep_for(std::chrono::microseconds(random_1000(gen) * 9 + 1000));
		}


} // end of run


int main() {
	//declare a vector of Sensor pointers:
	std::vector<Sensor*> sensors;

	//initialise each sensor and insert into the vector:
	sensors.push_back(new TempSensor(s1)); //push_back is a vector method.
	sensors.push_back(new PressureSensor (s2));
	sensors.push_back(new CapacitiveSensor(s3));

	Receiver theRC; // creating the Receiver
	LinkAccessController LAC( std::ref(theRC));
	// Instantiate the BC:
	BC theBC(std::ref(sensors));


	//instantiate and start the threads:
	std::thread the_threads[MAX_NUM_OF_THREADS]; //array of threads
	for (int i = 0; i < MAX_NUM_OF_THREADS; i++) {
		//launch the threads:
		the_threads[i] = thread(run , std::ref( theBC) , std::ref(LAC) , i );

	}
	//wait for the threads to finish:
	for (int i = 0; i < MAX_NUM_OF_THREADS; i++) {
		the_threads[i].join();
	}

	cout << "All threads terminated" << endl;
	theRC.printSensorData();
	//print out the number of times each sensor was accessed:
	std::cout << " Temperature Sensor used " << sensors_used[0] << " times "
		<< " Pressure Sensor used " << sensors_used[1] << " times "
		<< " Capacitive Sensor used " << sensors_used[2] << " times " << endl;
	return 0;
} // end of main

