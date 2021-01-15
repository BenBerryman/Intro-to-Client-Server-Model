/*	COSC 3360 - Fundamentals of Operating Systems
 * 	Assignment 2 - Benjamin Berryman
 * 	server.cpp
 *
 *  This is the server side of my client-server model.
 *
 *  As soon as the server is started, it reads in contents from "weather.txt" and puts the data
 *  in a table. It then asks the user for a port number to use, and creates a socket with it. It then
 *  listens for A connection (it can only take one, as per the professor's instructions). When it receives
 *  one, it will then listen for a message containing the city name that the client wants the weather data
 *  for. It converts it to lower case to avoid case sensitivity, grabs the associated data, puts it in a message,
 *  and sends it back to the client. It then waits for another reply from the same client infinitely. As per the
 *  professor's instructions, the server only shuts down from a Ctrl-C interrupt. THERE IS NO FORMAL CLOSE.
 */

#include <errno.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <cstring>
#include <stdexcept>


class input_table
{
	struct entry
	{
		std::string city, sky_conditions;
		int max_temp;
		entry(std::string &c, int &t, std::string &sky) : city(c), sky_conditions(sky),  max_temp(t){};
	};

	std::vector<entry> inputs;

public:
	void add_entry(std::string &in)
	{
		std::string city = in.substr(0,in.find(","));
		in = in.substr(in.find(",")+1);

		int max_temp = stoi(in.substr(0,in.find(",")));
		in = in.substr(in.find(",")+1);

		std::string sky_cond = in;

		entry *temp = new entry(city, max_temp, sky_cond); //Allocates memory for and creates a new entry.

		/* NOTE: I know this LOOKS like a memory leak. I do not believe it is, simply because I'm returning and storing the reference to this memory, there's still something pointing to it.
		 * Plus, in this application, the way we are supposed to exit the client/server interaction is through an interrupt, so the data is never actually going out of scope, and
		 * all of the memory the program uses is freed up when it exits anyways. Any effort to reallocate this memory myself would be redundant.
		 */

		inputs.push_back(*temp);
	}

	std::string get_info(std::string &city_in)
	{
		std::string output;
		bool found = false;
		for (int i = 0; i < inputs.size(); i++)
		{
			std::string lower_case;
			for (int j = 0; j < inputs.at(i).city.length(); j++)  //For loop to convert to lowercase
			{
				lower_case += tolower(inputs.at(i).city.at(j));
			}

			if (lower_case.compare(city_in) == 0)
			{
				std::cout << "Weather report for " << inputs.at(i).city << std::endl;
				output += "Tomorrow's maximum temperature is " + std::to_string(inputs.at(i).max_temp) + " F.\n"; //Notice I was able to avoid issues with network byte order for
																											 //integers, as converting it to a std::string solidifies it anyways.
				output += "Tomorrow's sky condition is " + inputs.at(i).sky_conditions + ".\n";
				found = true;
			}
		}
		if (!found)
			output += "No data.\n";
		return output;
	}
};

int establish(int portnum)
{
	struct sockaddr_in s_address; //Socket info
	int s; //Used for the creation of the actual socket
	struct hostent *hp; //Host info
	char myname[1024]; //Host name
	int opt = 1;

	memset(&s_address, 0, sizeof(struct sockaddr_in)); //Clears address space
	gethostname(myname, 1023);
	hp = gethostbyname(myname); //Get host address info

	if (hp == NULL)
		return -1;

	s_address.sin_family=hp->h_addrtype;
	s_address.sin_port=htons(portnum); //htons() switches from little endian to network byte order, which basically swaps bytes

	if ((s=socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (-1);

	if (bind(s,(struct sockaddr *)&s_address, sizeof(s_address)) < 0)
	{
		close(s);
		return -1;
	}

	setsockopt(s, SOL_SOCKET, SO_REUSEADDR | SO_REUSEADDR, &opt, sizeof(opt));
	listen(s,5);
	return s;
}

int get_connection(int s)
{
	int t;

	if ((t=accept(s, NULL, NULL)) < 0)
		return(-1);
	std::cout << "Connection received." << std::endl;
	std::cout << "NOTE: To exit the program, press Ctrl-C on your keyboard." << std::endl << std::endl;
	return(t);
}

int read_data(int s, //Socket descriptor
			  char *buf, //Pointer to buffer for data
			  int n) //Number of bytes we want
{
	int bytes_read = 0;
	int bytes_in_pass = 0;
	while (bytes_read < n)
	{
		if ((bytes_in_pass = read(s, buf, n-bytes_read)) > 0)
		{
			bytes_read += bytes_in_pass;
			buf += bytes_in_pass;
		}
		else if (bytes_in_pass < 0)
			return -1;
	}
	return bytes_read;
}

int write_data(int s, //Socket descriptor
			  char *buf, //Pointer to buffer for data
			  int n) //Number of bytes we want
{
	int bytes_written = 0;
	int bytes_in_pass = 0;
	while (bytes_written < n)
	{
		if ((bytes_in_pass = write(s, buf, n-bytes_written)) > 0)
		{
			bytes_written += bytes_in_pass;
			buf += bytes_in_pass;
		}
		else if (bytes_in_pass < 0)
			return -1;
	}
	return bytes_written;
}

/* =============
 * MAIN FUNCTION
 * =============
 */

int main(int argc, char const *argv[])
{
	std::ifstream in_file("weather.txt", std::ios::in);
	std::string input;

	input_table table;

	/* ===============================================================
	 * FIRST, read in the file contents and input into the input_table
	 * ===============================================================
	 */
	if (in_file.is_open())
	{
		while (!in_file.eof())
		{
			getline(in_file, input);
			if (input.size() > 0)
			{
				table.add_entry(input);
			}
		}
		in_file.close();
	}
	else
	{
		perror("file");
		throw std::invalid_argument("Unable to read file!");
	}

	/* ===============================================================================
	 * SECOND, prompt the user for a port number to use in the creation of the socket.
	 * ===============================================================================
	 */
	int port;
	while (true)
	{
		std::cout << "Enter server port number:";
		try
		{
			std::cin >> input;
			port = stoi(input);
			break;
		}
		catch (std::invalid_argument)
		{
			std::cout << "Not a valid input! \n";
		}
	}

	int s,t;
	if ((s=establish(port)) < 0)
	{
		perror("establish");
		exit(1);
	}

	char read_buffer[64], write_buffer[1024];
	int byte_count;
	std::string output;

	std::cout << "Waiting for connection..." << std::endl;

	if ((t=get_connection(s)) < 0)
	{
		perror("accept");
		exit(1);
	}

	/* =====================================================
	 *	THIRD, wait for a message from the connected client.
	 * =====================================================
	 */
	while (true)
	{
		memset(&read_buffer, 0, 64);
		if ((byte_count = read_data(t, read_buffer, 64)) < 0)
		{
			perror("read");
			exit(1);
		}

		std::string city_input(read_buffer); //Convert contents of char array read_buffer to std::string city_input
		output = table.get_info(city_input); //Get corresponding info and store into std::string output
		std::cout << output << std::endl;

		memset(&write_buffer,0,1024);
		output.copy(write_buffer, 1024); //Put data in buffer

	/* ===================================================
	 * FOURTH, return data in message to connected client.
	 * ===================================================
	 */
		if ((byte_count = write_data(t, write_buffer, 1024)) < 0)
		{
			perror("write");
			exit(1);
		}

		//Repeat steps 3 and 4 until the server is terminated with Ctrl-C
	}
	return 0;
}
