# Compiler flags...
CPP_COMPILER = mpic++
C_COMPILER = gcc

# Include paths...
Debug_Include_Path=
Release_Include_Path=

# Library paths...
Debug_Library_Path=
Release_Library_Path=

# Additional libraries...
Debug_Libraries=
Release_Libraries=

# Preprocessor definitions...
Debug_Preprocessor_Definitions=-D GCC_BUILD -D _DEBUG -D _CONSOLE 
Release_Preprocessor_Definitions=-D GCC_BUILD -D NDEBUG -D _CONSOLE 

# Implictly linked object files...
Debug_Implicitly_Linked_Objects=
Release_Implicitly_Linked_Objects=

# Compiler flags...
Debug_Compiler_Flags=-O0 -g 
Release_Compiler_Flags=-O2 -g 

# Builds all configurations for this project...
.PHONY: build_all_configurations
build_all_configurations: Debug Release 

# Builds the Debug configuration...
.PHONY: Debug
Debug: create_folders gccDebug/Demo.o gccDebug/EasyMPI.o 
	mpic++ gccDebug/Demo.o gccDebug/EasyMPI.o  $(Debug_Library_Path) $(Debug_Libraries) -Wl,-rpath,./ -o gccDebug/EasyMPI

# Compiles file Demo.cpp for the Debug configuration...
-include gccDebug/Demo.d
gccDebug/Demo.o: Demo.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c Demo.cpp $(Debug_Include_Path) -o gccDebug/Demo.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM Demo.cpp $(Debug_Include_Path) > gccDebug/Demo.d

# Compiles file EasyMPI.cpp for the Debug configuration...
-include gccDebug/EasyMPI.d
gccDebug/EasyMPI.o: EasyMPI.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c EasyMPI.cpp $(Debug_Include_Path) -o gccDebug/EasyMPI.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM EasyMPI.cpp $(Debug_Include_Path) > gccDebug/EasyMPI.d

# Builds the Release configuration...
.PHONY: Release
Release: create_folders gccRelease/Demo.o gccRelease/EasyMPI.o 
	mpic++ gccRelease/Demo.o gccRelease/EasyMPI.o  $(Release_Library_Path) $(Release_Libraries) -Wl,-rpath,./ -o gccRelease/EasyMPI

# Compiles file Demo.cpp for the Release configuration...
-include gccRelease/Demo.d
gccRelease/Demo.o: Demo.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c Demo.cpp $(Release_Include_Path) -o gccRelease/Demo.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM Demo.cpp $(Release_Include_Path) > gccRelease/Demo.d

# Compiles file EasyMPI.cpp for the Release configuration...
-include gccRelease/EasyMPI.d
gccRelease/EasyMPI.o: EasyMPI.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c EasyMPI.cpp $(Release_Include_Path) -o gccRelease/EasyMPI.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM EasyMPI.cpp $(Release_Include_Path) > gccRelease/EasyMPI.d

# Creates the intermediate and output folders for each configuration...
.PHONY: create_folders
create_folders:
	mkdir -p gccDebug/source
	mkdir -p gccRelease/source

# Cleans intermediate and output files (objects, libraries, executables)...
.PHONY: clean
clean:
	rm -f gccDebug/*.o
	rm -f gccDebug/*.d
	rm -f gccDebug/*.a
	rm -f gccDebug/*.so
	rm -f gccDebug/*.dll
	rm -f gccDebug/*.exe
	rm -f gccRelease/*.o
	rm -f gccRelease/*.d
	rm -f gccRelease/*.a
	rm -f gccRelease/*.so
	rm -f gccRelease/*.dll
	rm -f gccRelease/*.exe

