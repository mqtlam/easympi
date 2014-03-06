#ifndef EASYMPI_H
#define EASYMPI_H

#include <mpi.h>
#include <iostream>
#include <string>

namespace EasyMPI
{
	using namespace std;

	class EasyMPI
	{
	private:
		static int processID;
		static int numProcesses;
		static bool initialized;
		static bool finalized;
		static MPI_Status* mpiStatus;

	public:
		static void initialize(int argc, char* argv[]);
		static void finalize();
		
		static int getProcessID();
		static int getNumProcesses();
		static MPI_Status* getMPIStatus();

		static void masterWait(string slaveBroadcastMsg);
		static void slavesWait(string masterBroadcastMsg);
		static void synchronize(string slaveBroadcastMsg, string masterBroadcastMsg);
	};
}

#endif