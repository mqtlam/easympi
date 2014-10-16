#include "EasyMPI.h"
#include <iostream>
#include <string>

void masterDemo();
void slaveDemo();
void singleProcessDemo();

int main(int argc, char* argv[])
{
	// initialize
	EasyMPI::EasyMPI::initialize(argc, argv);

	// print rank and num processes
	std::cout << "Rank=" << EasyMPI::EasyMPI::getProcessID() << std::endl;
	std::cout << "Size=" << EasyMPI::EasyMPI::getNumProcesses() << std::endl;

	// master slave demo
	if (EasyMPI::EasyMPI::getNumProcesses() == 1)
	{
		singleProcessDemo();
	}
	else if (EasyMPI::EasyMPI::getProcessID() == 0)
	{
		masterDemo();
	}
	else
	{
		slaveDemo();
	}

	// finalize
	EasyMPI::EasyMPI::finalize();

	return 0;
}

// The master is responsible for scheduling tasks to slaves.
// Simply create a list of (command, message) for slaves to process in parallel.
void masterDemo()
{
	// declare commands and messages
	std::vector<std::string> commands;
	std::vector<std::string> messages;

	// demo commands
	commands.push_back("DEMO1");
	messages.push_back("message1");
	commands.push_back("DEMO2");
	messages.push_back("message2");

	// schedule tasks
	EasyMPI::EasyMPI::masterScheduleTasks(commands, messages);
}

// The slave is responsible for checking (command, message) sent by the master.
// Simply create logic to handle different command cases.
void slaveDemo()
{
	// loop to wait for tasks
	while (true)
	{
		// wait for a task
		std::string command;
		std::string message;
		EasyMPI::EasyMPI::slaveWaitForTasks(command, message);

		std::cout << "Got command '" << command << "' and message '" << message << "'" << std::endl;

		// define branches here to perform task depending on command
		if (command.compare("DEMO1") == 0)
		{
			std::cout << "Got DEMO1 command on process " << EasyMPI::EasyMPI::getProcessID() 
				<< " with message: " << message << std::endl;

			//
			// do stuff like call another function
			//

			// declare finished
			EasyMPI::EasyMPI::slaveFinishedTask();
		}
		else if (command.compare("DEMO2") == 0)
		{
			std::cout << "Got DEMO2 command on process " << EasyMPI::EasyMPI::getProcessID() 
				<< " with message: " << message << std::endl;

			//
			// do stuff like call another function
			//

			// declare finished
			EasyMPI::EasyMPI::slaveFinishedTask();
		}
		else if (command.compare(EasyMPI::EasyMPI::MASTER_FINISH_MESSAGE) == 0)
		{
			std::cout << "Got the master finish command on process " << EasyMPI::EasyMPI::getProcessID()
				<< ". Exiting slave loop..." << std::endl;

			break;
		}
		else
		{
			std::cout << "Invalid command." << std::endl;
		}
	}
}

// Must handle logic when there is only one process.
void singleProcessDemo()
{
	// declare commands and messages
	std::vector<std::string> commands;
	std::vector<std::string> messages;

	// demo commands
	commands.push_back("DEMO1");
	messages.push_back("message1");
	commands.push_back("DEMO2");
	messages.push_back("message2");

	// loop to wait for tasks
	for (int i = 0; i < static_cast<int>(commands.size()); i++)
	{
		// wait for a task
		std::string command = commands[i];
		std::string message = messages[i];

		std::cout << "Got command '" << command << "' and message '" << message << "'" << std::endl;

		// define branches here to perform task depending on command
		if (command.compare("DEMO1") == 0)
		{
			//
			// do stuff like call another function
			//
		}
		else if (command.compare("DEMO2") == 0)
		{
			//
			// do stuff like call another function
			//
		}
		else
		{
			std::cout << "Invalid command." << std::endl;
		}
	}
}