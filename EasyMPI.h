#ifndef EASYMPI_H
#define EASYMPI_H

#include <mpi.h>
#include <iostream>
#include <string>

namespace EasyMPI
{
	using namespace std;

	/*!
	 * EasyMPI is a class that implements basic high level parallelism functionality. 
	 * The current version uses a master-slave architecture where the slaves perform 
	 * parallel tasks while the master is responsible for the merging steps 
	 * and overall main computational steps.
	 *
	 * Simply include the header file in your program to use these functions.
	 * Make sure MPI is installed on your system.
	 */
	class EasyMPI
	{
	private:
		static int processID; //!< Process ID
		static int numProcesses; //!< Number of processes
		static bool initialized; //!< Whether called MPI initialized
		static bool finalized; //!< Whether called MPI finalized
		static MPI_Status* mpiStatus; //!< MPI Status object

	public:
		/*!
		 * Initialize MPI. Must be called before anything else!
		 */
		static void initialize(int argc, char* argv[]);

		/*!
		 * Finalize MPI. Must be called before exiting the program!
		 */
		static void finalize();
		
		/*!
		 * Get the process ID (rank).
		 */
		static int getProcessID();

		/*!
		 * Get the total number of processes.
		 */
		static int getNumProcesses();

		/*!
		 * Get the MPI status object.
		 */
		static MPI_Status* getMPIStatus();

		/*!
		 * The master waits until all slave processes 
		 * reach this point before continuing. 
		 * Slave processes continue past this point.
		 *
		 * @param[in] slaveBroadcastMsg Message to broadcast to slaves
		 */
		static void masterWait(string slaveBroadcastMsg);
		
		/*!
		 * Slave processes wait until the master process 
		 * reaches this point before continuing.
		 * The master process continues past this point.
		 *
		 * @param[in] masterBroadcastMsg Message to broadcast to master
		 */
		static void slavesWait(string masterBroadcastMsg);

		/*!
		 * Equivalent to the masterWait followed by slavesWait. 
		 * This means all processes must reach this point 
		 * before continuing.
		 *
		 * @param[in] slaveBroadcastMsg Message to broadcast to slaves
		 * @param[in] masterBroadcastMsg Message to broadcast to master
		 */
		static void synchronize(string slaveBroadcastMsg, string masterBroadcastMsg);
	};
}

#endif