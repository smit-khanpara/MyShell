#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include <sstream>
#include<string.h>
#include<unordered_map>
using namespace std;

unordered_map<string, int> flag;

void init()
{
	cout << endl;
	cout << "*****************************************************************************" << endl;
	cout <<	"*                                                                           *"	<< endl;
	cout <<	"*                          Welcome To My Unix Shell                         *"	<< endl;
	cout <<	"*                                                                           *"	<< endl;
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

	flag["|"] = 0;
	flag[">>"] = 0;
	flag[">"] = 0;
}

void parse_command(char **cmd, char *in)
{
	int i = 0;

	if(strstr(in, "\""))
	{
		char *temp[128];
		temp[i] = strtok(in, "\"");
	    while( temp[i] != NULL ) 
	    {
	    	i++;
			temp[i] = strtok(NULL, "\"");
	    }

	    i = 0;
	    cmd[i] = strtok(temp[0], " ");
	    while( cmd[i] != NULL ) 
	    {
	    	i++;
			cmd[i] = strtok(NULL, " ");
	    }

	    cmd[i++] = temp[1];

	    cmd[i] = strtok(temp[2], " ");
	    while( cmd[i] != NULL ) 
	    {
	    	i++;
			cmd[i] = strtok(NULL, " ");
	    }
	}
	else
	{
		cmd[i] = strtok(in, " ");
	    while( cmd[i] != NULL ) 
	    {
	    	i++;
			cmd[i] = strtok(NULL, " ");
	    }
	}
    
}

void execute(char **arg)
{
	if(!strcmp(arg[0],"exit"))
	{
		cout << "exit" << endl;
		exit(0);
	}

	pid_t rid = fork();
	
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

void parse_and_excute()
{ 	
	char *arg[128];
	char buf[1024];
   	char *input; 
   	char *temp1, *temp2;
    int i = 0;
    fgets(buf, 1024, stdin); 
    
    input = strtok(buf, "\n");

    if(strstr(input, ">>"))
    {
    	temp1 = strtok(input, ">>");
    	temp2 = strtok(NULL, ">>");
    	flag[">>"] = 1;
    }
    else if(strstr(input, ">"))
    {
    	temp1 = strtok(input, ">");
    	temp2 = strtok(NULL, ">");
    	flag[">"] = 1;
    }
    else
    	temp1 = input;
    
    arg[0] = strtok(temp1, "|");
    while( arg[i] != NULL ) 
    {
    	i++;
		arg[i] = strtok(NULL, "|");
    }

    if(i > 1)
		flag["|"] = 1;

    if(flag[">"] || flag[">>"])
    {
    	arg[i] = temp2;
	    arg[++i] = NULL;
    }

	
	if(flag["|"])
	{
		if(flag[">"] || flag[">>"])
		{

		}
	}
	else if(flag[">"])
	{
		char *cmd[128];
		parse_command(cmd, arg[0]);
		char *file[128];
		parse_command(file, arg[1]);
	}
	else
	{
		char *cmd[128];
		parse_command(cmd, arg[0]);
		execute(cmd);
	}
}

int main()
{
	init();
	while(1)
	{
		prompt();
		parse_and_excute();
	}
	return 0;	
}