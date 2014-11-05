#include "EasyMPI.h"
#include <queue>
#include <iomanip>
#include <sstream>
#include <cstdlib>

namespace EasyMPI
{
	/*** EasyMPI ***/

	const int MPIScheduler::MAX_MESSAGE_SIZE = 128;
	const int MPIScheduler::MAX_NUM_PROCESSES = 512;
	const string MPIScheduler::MASTER_FINISH_COMMAND = "MASTERFINISHEDALLTASKS";
	const string MPIScheduler::SLAVE_FINISH_COMMAND = "SLAVEFINISHEDTASK";
	const string MPIScheduler::SYNCHRONIZATION_MASTER_MESSAGE = "MASTERSYNC";
	const string MPIScheduler::SYNCHRONIZATION_SLAVE_MESSAGE = "SLAVESYNC";

	int MPIScheduler::processID = -1;
	int MPIScheduler::numProcesses = 0;
	bool MPIScheduler::initialized = false;
	bool MPIScheduler::finalized = false;
	int MPIScheduler::syncCounter = 0;
	MPI_Status* MPIScheduler::mpiStatus = NULL;

	void MPIScheduler::initialize(int argc, char* argv[])
	{
		// initialize MPI
		int rank, size;
		int rc = MPI_Init(&argc, &argv);
		if (rc != MPI_SUCCESS)
		{
			cerr << "Error starting MPI program. Terminating.";
			abortMPI(1);
		}

		// get size and rank
		MPI_Comm_size(MPI_COMM_WORLD, &size);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		// store
		MPIScheduler::processID = rank;
		MPIScheduler::numProcesses = size;
		MPIScheduler::mpiStatus = new MPI_Status();
		MPIScheduler::initialized = true;
		MPIScheduler::syncCounter = 0;
	}

	void MPIScheduler::finalize()
	{
		MPI_Finalize();
		MPIScheduler::finalized = true;
	}

	void MPIScheduler::abortMPI(int errcode)
	{
		if (MPIScheduler::initialized)
		{
			const int numProcesses = getNumProcesses();
			const int rank = getProcessID();
			cerr << "Process [" << rank << "/" << numProcesses << "] called ABORT!" << endl;

			MPI_Abort(MPI_COMM_WORLD, errcode);
		}
		else
		{
			exit(errcode);
		}
	}

	int MPIScheduler::getProcessID()
	{
		return MPIScheduler::processID;
	}

	int MPIScheduler::getNumProcesses()
	{
		return MPIScheduler::numProcesses;
	}

	MPI_Status* MPIScheduler::getMPIStatus()
	{
		return MPIScheduler::mpiStatus;
	}

	void MPIScheduler::masterScheduleTasks(vector<Task> taskList)
	{
		char recvbuff[MAX_MESSAGE_SIZE];
		const int numTasks = taskList.size();
		const int numProcesses = getNumProcesses();
		const int rank = getProcessID();

		if (numProcesses == 1)
		{
			cerr << "Cannot run master-slave with one process!" << endl;
			return;
		}

		if (numTasks == 0)
		{
			cout << "No tasks. Nothing to process." << endl;
		}
		else
		{
			// state variables
			vector<bool> finishedTasks; // maintain which tasks are completed
			vector<int> processTask; // maintain which process is assigned to a task
			queue<int> unassignedTasks; // maintain queue of tasks waiting to be processed
			queue<int> availableProcesses; // maintain queue of available processes for work

			// initialize state
			for (int i = 0; i < numTasks; i++)
			{
				finishedTasks.push_back(false);
				unassignedTasks.push(i);
			}
			processTask.push_back(-1); // process 0
			for (int i = 1; i < numProcesses; i++)
			{
				availableProcesses.push(i);
				processTask.push_back(-1);
			}

			// assign as many tasks to processes as possible
			while (!availableProcesses.empty())
			{
				if (unassignedTasks.empty())
					break;

				// get available process
				int slaveID = availableProcesses.front();
				availableProcesses.pop();

				// get task
				int taskID = unassignedTasks.front();
				unassignedTasks.pop();

				// assign task to available process by sending message to slave
				cout << "Master is assigning task to slave [" << slaveID << "/" << numProcesses << "]." << endl;
				string fullMessage = Task::constructFullMessage(taskList[taskID]);
				const char* fullMessageString = fullMessage.c_str();
				int ierr = MPI_Send(const_cast<char*>(fullMessageString), MAX_MESSAGE_SIZE, MPI_CHAR, slaveID, 0, MPI_COMM_WORLD);

				// update state
				processTask[slaveID] = taskID;
			}

			// wait for messages until all tasks are assigned and completed
			while (true)
			{
				int msgFlag = 0;
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &msgFlag, MPIScheduler::mpiStatus);

				// if a messsage is available
				if (msgFlag)
				{
					// get the message tag and especially the source
					int messageID = (*MPIScheduler::mpiStatus).MPI_TAG;
					int messageSource = (*MPIScheduler::mpiStatus).MPI_SOURCE;
					cout << "A message from process [" << messageSource << "/" << numProcesses << "]." << endl;

					// receive the message into the buffer and check if it is the correct (slave finish) command
					int ierr = MPI_Recv(recvbuff, MAX_MESSAGE_SIZE, MPI_CHAR, messageSource, 0, MPI_COMM_WORLD, MPIScheduler::mpiStatus);
				
					// process full message into command and message components
					string fullMessage = recvbuff;
					Task task = Task::parseFullMessage(fullMessage);
					string command = task.getCommand();
					string message = task.getParameters();

					// check if correct (slave finish) message
					const int commandSize = command.length();
					bool correctMessage = true;
					for (int i = 0; i < commandSize; i++)
					{
						if (command.at(i) != SLAVE_FINISH_COMMAND.at(i))
						{
							correctMessage = false;
							break;
						}
					}
		
					// if correct message, update state and check what else needs to be done
					if (correctMessage)
					{
						cout << "Master received finished message from slave [" << messageSource << "/" << numProcesses << "]." << endl;
					
						// get completed task ID
						int taskID = processTask[messageSource];

						// sanity check
						if (taskID < 0 || taskID >= numTasks)
						{
							cerr << "Task ID '" << taskID << "' gotten is invalid!" << endl;
							abortMPI(1);
						}

						// update state
						processTask[messageSource] = -1;
						finishedTasks[taskID] = true;
						availableProcesses.push(messageSource);
					
						// check if any other tasks need to be processed
						// otherwise check all tasks are completed
						if (!unassignedTasks.empty())
						{
							// get available process
							int slaveID = availableProcesses.front();
							availableProcesses.pop();

							// get task
							int taskID = unassignedTasks.front();
							unassignedTasks.pop();

							// assign task to available process by sending message to slave
							cout << "Master is assigning task to slave [" << slaveID << "/" << numProcesses << "]." << endl;
							string fullMessage = Task::constructFullMessage(taskList[taskID]);
							const char* fullMessageString = fullMessage.c_str();
							int ierr = MPI_Send(const_cast<char*>(fullMessageString), MAX_MESSAGE_SIZE, MPI_CHAR, slaveID, 0, MPI_COMM_WORLD);

							// update state
							processTask[slaveID] = taskID;
						}
						else
						{
							// test if every task is finished
							bool allFinish = true;
							for (int i = 0; i < numTasks; i++)
							{
								if (!finishedTasks[i])
								{
									cout << "Task " << i << " is still being processed..." << endl;
									allFinish = false;
								}
							}
							if (allFinish)
							{
								break;
							}
						}
					}
				}
			}
			cout << "All tasks are finished!" << endl;
		}

		// everything finished, so send finish command to all slaves
		for (int slaveID = 1; slaveID < getNumProcesses(); slaveID++)
		{
			cout << "Master is telling slave [" << slaveID << "/" << numProcesses << "] that all tasks are done." << endl;
			string fullMessage = Task::constructFullMessage(Task(MASTER_FINISH_COMMAND));
			const char* fullMessageString = fullMessage.c_str();
			int ierr = MPI_Send(const_cast<char*>(fullMessageString), MAX_MESSAGE_SIZE, MPI_CHAR, slaveID, 0, MPI_COMM_WORLD);
		}
	}

	Task MPIScheduler::slaveWaitForTasks()
	{
		const int numProcesses = getNumProcesses();
		const int rank = getProcessID();
		
		Task task;

		if (numProcesses == 1)
			return task;
		
		char recvbuff[MAX_MESSAGE_SIZE];

		// wait until get a message from master
		// will block until a message comes from the master!
		while (true)
		{
			// wait for message from master
			int ierr = MPI_Recv(recvbuff, MAX_MESSAGE_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPIScheduler::mpiStatus);

			// process full message into command and message components
			string fullMessage = recvbuff;
			task = Task::parseFullMessage(fullMessage);

			if (!task.isEmpty())
			{
				cout << "Slave [" << rank << "/" << numProcesses << "]" << " got the command '" 
					<< task.getCommand() << "' and parameters '" << task.getParameters() << "' from master." << endl;

				break;
			}
			else
			{
				cout << "Got empty task!" << endl;
			}
		}

		return task;
	}

	void MPIScheduler::slaveFinishedTask()
	{
		const int numProcesses = getNumProcesses();
		const int rank = getProcessID();

		if (numProcesses == 1)
			return;

		// send master the finished message
		cout << "Slave [" << rank << "/" << numProcesses << "] is telling master it has finished a task." << endl;
		string fullMessage = Task::constructFullMessage(Task(SLAVE_FINISH_COMMAND));
		const char* fullMessageString = fullMessage.c_str();
		int ierr = MPI_Send(const_cast<char*>(fullMessageString), MAX_MESSAGE_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}

	void MPIScheduler::synchronize()
	{
		stringstream ssMasterMessage;
		ssMasterMessage << MPIScheduler::SYNCHRONIZATION_MASTER_MESSAGE << MPIScheduler::syncCounter;

		stringstream ssSlaveMessage;
		ssSlaveMessage << MPIScheduler::SYNCHRONIZATION_SLAVE_MESSAGE << MPIScheduler::syncCounter;

		synchronize(ssMasterMessage.str(), ssSlaveMessage.str());

		MPIScheduler::syncCounter++;
	}

	void MPIScheduler::synchronize(string slaveBroadcastMsg, string masterBroadcastMsg)
	{
		masterWait(slaveBroadcastMsg);
		slavesWait(masterBroadcastMsg);
	}

	void MPIScheduler::masterWait(string slaveBroadcastMsg)
	{
		// receive buffer
		char recvbuff[MAX_MESSAGE_SIZE];
		const char* SLAVEBROADCASTMSG = slaveBroadcastMsg.c_str();
		const int SLAVEBROADCASTMSG_SIZE = slaveBroadcastMsg.length();
	
		// master needs to wait here until all slaves reaches here
		const int numProcesses = getNumProcesses();
		const int rank = getProcessID();
		if (rank == 0)
		{
			cout << "Master process [" << rank << "/" << numProcesses << "] is waiting to get " << slaveBroadcastMsg << " message from all slaves..." << endl;

			// wait until all slaves are done
			bool finish[MAX_NUM_PROCESSES];
			finish[0] = true;
			for (int i = 1; i < numProcesses; i++)
			{
				finish[i] = false;
			}

			// master process is now waiting for all slave processes to finish
			while (true)
			{
				if (numProcesses <= 1)
					break;

				int msgFlag = 0;
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &msgFlag, MPIScheduler::mpiStatus);

				// if a messsage is available
				if (msgFlag)
				{
					// get the message tag and especially source
					int messageID = (*MPIScheduler::mpiStatus).MPI_TAG;
					int messageSource = (*MPIScheduler::mpiStatus).MPI_SOURCE;
					cout << "A message from process [" << messageSource << "/" << numProcesses << "]." << endl;

					// receive the message into the buffer and check if it is the correct command
					int ierr = MPI_Recv(recvbuff, SLAVEBROADCASTMSG_SIZE, MPI_CHAR, messageSource, 0, MPI_COMM_WORLD, MPIScheduler::mpiStatus);
					
					// check if correct message
					bool correctMessage = true;
					for (int i = 0; i < SLAVEBROADCASTMSG_SIZE; i++)
					{
						if (recvbuff[i] != slaveBroadcastMsg[i])
						{
							correctMessage = false;
							break;
						}
					}
					
					if (correctMessage)
					{
						cout << "Received " << slaveBroadcastMsg << " message from process [" << messageSource << "/" << numProcesses << "]." << endl;
						finish[messageSource] = true;

						// test if every process is finished
						bool allFinish = true;
						for (int i = 1; i < numProcesses; i++)
						{
							if (!finish[i])
							{
								cout << "Process [" << i << "/" << numProcesses << "] is still not here yet..." << endl;
								allFinish = false;
							}
						}
						if (allFinish)
						{
							break;
						}
						cout << "Still waiting for all slaves to get here with the master..." << endl;
					}
				}
			}
		
			// MASTER CAN DO STUFF HERE BEFORE SLAVES PROCEED

			cout << "Master process [" << rank << "/" << numProcesses << "] has continued..." << endl;
		}
		else
		{
			cout << "Slave process [" << rank << "/" << numProcesses << "] is sending arrival message " << SLAVEBROADCASTMSG << " to master..." << endl;

			// send finish heuristic learning message to master
			int ierr = MPI_Send(const_cast<char*>(SLAVEBROADCASTMSG), SLAVEBROADCASTMSG_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

			cout << "Slave process [" << rank << "/" << numProcesses << "] has continued..." << endl;
		}
	}

	void MPIScheduler::slavesWait(string masterBroadcastMsg)
	{
		// receive buffer
		char recvbuff[MAX_MESSAGE_SIZE];
		const char* MASTERBROADCASTMSG = masterBroadcastMsg.c_str();
		const int MASTERBROADCASTMSG_SIZE = masterBroadcastMsg.length();

		// slaves need to wait here until master reaches here
		const int numProcesses = getNumProcesses();
		const int rank = getProcessID();
		if (rank == 0)
		{
			cout << "Master process [" << rank << "/" << numProcesses << "] is telling slave processes to continue..." << endl;

			// tell each slave to continue
			for (int j = 1; j < numProcesses; j++)
			{
				cout << "Master is sending slave process [" << j << "/" << numProcesses << "] the " << MASTERBROADCASTMSG << " message to continue..." << endl;
				int ierr = MPI_Send(const_cast<char*>(MASTERBROADCASTMSG), MASTERBROADCASTMSG_SIZE, MPI_CHAR, j, 0, MPI_COMM_WORLD);
			}

			cout << "Master process [" << rank << "/" << numProcesses << "] is released..." << endl;
		}
		else
		{
			cout << "Slave process [" << rank << "/" << numProcesses << "] is waiting for master..." << endl;

			// now wait until master gives the continue signal
			while (true)
			{
				int ierr = MPI_Recv(recvbuff, MASTERBROADCASTMSG_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPIScheduler::mpiStatus);

				// check if correct message
				bool correctMessage = true;
				for (int i = 0; i < MASTERBROADCASTMSG_SIZE; i++)
				{
					if (recvbuff[i] != masterBroadcastMsg.at(i))
					{
						correctMessage = false;
						break;
					}
				}

				if (correctMessage)
				{
					cout << "Slave process [" << rank << "/" << numProcesses << "] got the " << masterBroadcastMsg << " message." << endl;
					break;
				}
			}

			cout << "Slave process [" << rank << "/" << numProcesses << "] is released..." << endl;
		}
	}



	/*** Command ***/
	
	Task::Task()
	{
		this->command = "";
		this->parameters = "";
	}

	Task::Task(string command)
	{
		this->command = command;
		this->parameters = "";
	}

	Task::Task(string command, string parameters)
	{
		this->command = command;
		this->parameters = parameters;
	}

	string Task::getCommand() const
	{
		return this->command;
	}

	string Task::getParameters() const
	{
		return this->parameters;
	}

	bool Task::isEmpty() const
	{
		return this->command.compare("") == 0 && this->parameters.compare("");
	}

	string Task::constructFullMessage(Task task)
	{
		string command = task.getCommand();
		string message = task.getParameters();

		// size<commandstring;messagestring>XXX...

		// sanity check
		size_t foundSemicolon1 = command.find(";");
		if (foundSemicolon1 != std::string::npos)
		{
			cerr << "command cannot contain a semicolon!";
			MPIScheduler::abortMPI(1);
		}
		size_t foundSemicolon2 = message.find(";");
		if (foundSemicolon2 != std::string::npos)
		{
			cerr << "message cannot contain a semicolon!";
			MPIScheduler::abortMPI(1);
		}

		// calculate size of full message
		int commandLength = command.length();
		int messageLength = message.length();
		int size = 3 + 1 + commandLength + 1 + messageLength + 1;

		if (size > MPIScheduler::MAX_MESSAGE_SIZE)
		{
			cerr << "Message length exceeds max message size!" << endl;
			MPIScheduler::abortMPI(1);
		}

		// convert size to string
		stringstream sizeSS;
		sizeSS << setfill('0') << setw(3) << size;
		
		// construct full message
		stringstream ss;
		ss << sizeSS.str() << "<" << command << ";" << message << ">";
		stringstream fullMessageSS;
		fullMessageSS << std::left << setfill('X') << setw(MPIScheduler::MAX_MESSAGE_SIZE) << ss.str();
		
		//cout << "Full message constructed: '" << fullMessageSS.str() << "'" << endl;

		return fullMessageSS.str();
	}

	Task Task::parseFullMessage(string fullMessage)
	{
		string command;
		string message;
		Task task;

		// size<commandstring;messagestring>XXX...

		// get full message size
		string messageSizeString = fullMessage.substr(0, 3);
		int messageSize = atoi(messageSizeString.c_str());

		// some sanity check
		if (fullMessage.at(3) != '<' || fullMessage.at(messageSize-1) != '>')
		{
			cerr << "The message received is not a valid message." << endl;
		}

		// get content between < and >
		string line = fullMessage.substr(4, messageSize-5);

		// parse content to get command and message
		stringstream ss(line);
		getline(ss, command, ';');
		getline(ss, message, ';');
		task = Task(command, message);

		//cout << "Command parsed: '" << command << "'" << endl;
		//cout << "Message parsed: '" << message << "'" << endl;

		return task;
	}
}