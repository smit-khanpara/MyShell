#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include <sstream>
#include<string.h>
using namespace std;

void greeting()
{
	cout << endl;
	cout << "*****************************************************************************" << endl;
	// cout <<	"*                                                                           *"	<< endl;
	// cout <<	"*                                                                           *"	<< endl;
	// cout <<	"*                                                                           *"	<< endl;
	cout <<	"*                                                                           *"	<< endl;
	cout <<	"*                          Welcome To My Unix Shell                         *"	<< endl;
	cout <<	"*                                                                           *"	<< endl;
	// cout <<	"*                                                                           *"	<< endl;
	// cout <<	"*                                                                           *"	<< endl;
	// cout <<	"*                                                                           *"	<< endl;
	cout << "*****************************************************************************" << endl;
	cout << endl;

}

void prompt()
{
	char *user = getenv("USER");
	char host[128];
	gethostname(host, 128);
	char cwd[4096];
	getcwd(cwd, 4096);

	cout << "\033[1;36m" << user << "@" << host << "\033[0m" << ":" << "\033[1;34m" << cwd  << "\033[0m" << "$ ";
}

void execute(char **arg)
{
	pid_t rid = fork();

	if(!strcmp(arg[0],"exit"))
		exit(0);
	if(rid == -1)
	{
		cout << "Failed!!" << endl;
	} 
	else if(rid == 0)
	{
		if (execvp(arg[0], arg) < 0) 
		{ 
            cout <<"Unable to execute command!!" << endl; 
        }
        exit(0);
	}
	else
	{
		wait(NULL);
		return;
	}
}

void parse_arguments(char **arg, string input)
{
	int i = 0; 
  	char *str = (char *) input.c_str();
    while(1)
    { 
        arg[i] = strsep(&str," ");  
        if(arg[i] == NULL)
        	break;
        if (strlen(arg[i]))
            i++;
    }    
}

int main()
{
	greeting();
	while(1)
	{
		prompt();

		string input;
		getline(cin, input);
		if(!input.size())
			continue;
		char *para[1024];
		parse_arguments(para, input);
		
		execute(para); 
	}
	
	return 0;	
}