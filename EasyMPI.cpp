#include "EasyMPI.h"

namespace EasyMPI
{
	int EasyMPI::processID = -1;
	int EasyMPI::numProcesses = 0;
	bool EasyMPI::initialized = false;
	bool EasyMPI::finalized = false;
	MPI_Status* EasyMPI::mpiStatus = NULL;

	void EasyMPI::initialize(int argc, char* argv[])
	{
		// initialize MPI
		int rank, size;
		int rc = MPI_Init(&argc, &argv);
		if (rc != MPI_SUCCESS)
		{
			cerr << "Error starting MPI program. Terminating.";
			exit(1);
		}

		// get size and rank
		MPI_Comm_size(MPI_COMM_WORLD, &size);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		// store
		EasyMPI::processID = rank;
		EasyMPI::numProcesses = size;
		EasyMPI::mpiStatus = new MPI_Status();
		EasyMPI::initialized = true;
	}

	void EasyMPI::finalize()
	{
		// synchronize
		synchronize("FINAL1", "FINAL2");

		MPI_Finalize();
		EasyMPI::finalized = true;
	}

	int EasyMPI::getProcessID()
	{
		return EasyMPI::processID;
	}

	int EasyMPI::getNumProcesses()
	{
		return EasyMPI::numProcesses;
	}

	MPI_Status* EasyMPI::getMPIStatus()
	{
		return EasyMPI::mpiStatus;
	}

	void EasyMPI::synchronize(string slaveBroadcastMsg, string masterBroadcastMsg)
	{
		masterWait(slaveBroadcastMsg);
		slavesWait(masterBroadcastMsg);
	}

	void EasyMPI::masterScheduleTasks(vector<string> commands, vector<string> messages, string finishCommand)
	{
		// TODO
	}

	void EasyMPI::slaveWaitForTasks(string& command, string& message)
	{
		// TODO
	}

	void EasyMPI::masterWait(string slaveBroadcastMsg)
	{
		// receive buffer
		char recvbuff[256];
		const char* SLAVEBROADCASTMSG = slaveBroadcastMsg.c_str();
		const int SLAVEBROADCASTMSG_SIZE = slaveBroadcastMsg.length();
	
		// master needs to wait here until all slaves reaches here
		const int numProcesses = getNumProcesses();
		const int rank = getProcessID();
		if (rank == 0)
		{
			cout << "Master process [" << rank << "] is waiting to get " << slaveBroadcastMsg << " message from all slaves..." << endl;

			// wait until all slaves are done
			bool finish[512];
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
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &msgFlag, EasyMPI::mpiStatus);

				// if a messsage is available
				if (msgFlag)
				{
					// get the message tag and especially source
					int messageID = (*EasyMPI::mpiStatus).MPI_TAG;
					int messageSource = (*EasyMPI::mpiStatus).MPI_SOURCE;
					cout << "A message from process [" << messageSource << "]." << endl;

					// receive the message into the buffer and check if it is the correct command
					int ierr = MPI_Recv(recvbuff, SLAVEBROADCASTMSG_SIZE, MPI_CHAR, messageSource, 0, MPI_COMM_WORLD, EasyMPI::mpiStatus);
					
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
						cout << "Received " << slaveBroadcastMsg << " message from process [" << messageSource << "]." << endl;
						finish[messageSource] = true;

						// test if every process is finished
						bool allFinish = true;
						for (int i = 1; i < numProcesses; i++)
						{
							if (!finish[i])
							{
								cout << "Process [" << i << "] is still not here yet..." << endl;
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

			cout << "Master process [" << rank << "] has continued..." << endl;
		}
		else
		{
			cout << "Slave process [" << rank << "] is sending arrival message " << SLAVEBROADCASTMSG << " to master..." << endl;

			// send finish heuristic learning message to master
			int ierr = MPI_Send(const_cast<char*>(SLAVEBROADCASTMSG), SLAVEBROADCASTMSG_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

			cout << "Slave process [" << rank << "] has continued..." << endl;
		}
	}

	void EasyMPI::slavesWait(string masterBroadcastMsg)
	{
		// receive buffer
		char recvbuff[256];
		const char* MASTERBROADCASTMSG = masterBroadcastMsg.c_str();
		const int MASTERBROADCASTMSG_SIZE = masterBroadcastMsg.length();

		// slaves need to wait here until master reaches here
		const int numProcesses = getNumProcesses();
		const int rank = getProcessID();
		if (rank == 0)
		{
			cout << "Master process [" << rank << "] is telling slave processes to continue..." << endl;

			// tell each slave to continue
			for (int j = 1; j < numProcesses; j++)
			{
				cout << "Master is sending slave process [" << j << "] the " << MASTERBROADCASTMSG << " message to continue..." << endl;
				int ierr = MPI_Send(const_cast<char*>(MASTERBROADCASTMSG), MASTERBROADCASTMSG_SIZE, MPI_CHAR, j, 0, MPI_COMM_WORLD);
			}

			cout << "Master process [" << rank << "] is released..." << endl;
		}
		else
		{
			cout << "Slave process [" << rank << "] is waiting for master..." << endl;

			// now wait until master gives the continue signal
			while (true)
			{
				int ierr = MPI_Recv(recvbuff, MASTERBROADCASTMSG_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, EasyMPI::mpiStatus);

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
					cout << "Slave process [" << rank << "] got the " << masterBroadcastMsg << " message." << endl;
					break;
				}
			}

			cout << "Slave process [" << rank << "] is released..." << endl;
		}
	}
}