# Builds all the projects in the solution...
.PHONY: all_projects
all_projects: EasyMPI 

# Builds project 'EasyMPI'...
.PHONY: EasyMPI
EasyMPI: 
	make --directory="." --file=EasyMPI.makefile
	cp gccRelease/EasyMPI .

# Cleans all projects...
.PHONY: clean
clean:
	make --directory="." --file=EasyMPI.makefile clean

