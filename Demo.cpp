#include "EasyMPI.h"
#include <iostream>
#include <string>

void masterDemo(std::vector<std::string> commands, std::vector<std::string> messages);
void slaveDemo(std::vector<std::string> commands, std::vector<std::string> messages);

int main(int argc, char* argv[])
{
	// initialize
	EasyMPI::EasyMPI::initialize(argc, argv);

	// print rank and num processes
	std::cout << "Rank=" << EasyMPI::EasyMPI::getProcessID() << std::endl;
	std::cout << "Size=" << EasyMPI::EasyMPI::getNumProcesses() << std::endl;

	// demo: declare commands and messages
	std::vector<std::string> commands;
	std::vector<std::string> messages;
	commands.push_back("DEMO1");
	messages.push_back("message1");
	commands.push_back("DEMO2");
	messages.push_back("message2");

	// master slave demo
	if (EasyMPI::EasyMPI::getProcessID() == 0 && EasyMPI::EasyMPI::getNumProcesses() > 1)
	{
		masterDemo(commands, messages);
	}
	else
	{
		slaveDemo(commands, messages);
	}

	// finalize
	EasyMPI::EasyMPI::finalize();

	return 0;
}

// The master is responsible for scheduling tasks to slaves.
// Simply create a list of (command, message) for slaves to process in parallel.
void masterDemo(std::vector<std::string> commands, std::vector<std::string> messages)
{
	// schedule tasks
	EasyMPI::EasyMPI::masterScheduleTasks(commands, messages);
}

// The slave is responsible for checking (command, message) sent by the master.
// Simply create logic to handle different command cases.
void slaveDemo(std::vector<std::string> commands, std::vector<std::string> messages)
{
	std::vector<std::string> commandSet = commands;
	std::vector<std::string> messageSet = messages;

	// loop to wait for tasks
	while (true)
	{
		// wait for a task
		std::string command;
		std::string message;

		// wait for task if more than one process
		// otherwise if only one process, then perform task on master process
		if (EasyMPI::EasyMPI::getNumProcesses() > 1)
		{
			EasyMPI::EasyMPI::slaveWaitForTasks(command, message);
		}
		else
		{
			if (commandSet.empty() || messageSet.empty())
				break;

			command = commandSet.back();
			message = messageSet.back();
			commandSet.pop_back();
			messageSet.pop_back();
		}

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
