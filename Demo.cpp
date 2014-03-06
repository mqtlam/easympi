#include "EasyMPI.h"
#include <iostream>

int main(int argc, char* argv[])
{
	// initialize
	EasyMPI::EasyMPI::initialize(argc, argv);

	// print rank and num processes
	std::cout << "Rank=" << EasyMPI::EasyMPI::getProcessID() << std::endl;
	std::cout << "Size=" << EasyMPI::EasyMPI::getNumProcesses() << std::endl;

	// finalize
	EasyMPI::EasyMPI::finalize();

	return 0;
}