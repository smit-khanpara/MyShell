#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include <sstream>
#include<string.h>
#include <fcntl.h>
#include<unordered_map>
using namespace std;

unordered_map<string, int> flag;
int fdh;

void init()
{
	cout << endl;
	cout << "*****************************************************************************" << endl;
	cout <<	"*                                                                           *"	<< endl;
	cout <<	"*                          Welcome To My Unix Shell                         *"	<< endl;
	cout <<	"*                                                                           *"	<< endl;
	cout << "*****************************************************************************" << endl;
	cout << endl;

	fdh = open("history.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
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

int cnt_arg(char **arg)
{
	int cnt = 0;
	int i=0;
	while(arg[i])
	{
		cnt++;
		i++;
	}

	return cnt;
}

void redirection(char **cmd, char **file)
{
	int fd[2];
	char buff[1024];

	if (pipe(fd)==-1) 
    { 
        cout << "Pipe failed" << endl; 
        return; 
    } 

	pid_t rid = fork();
	
	if(rid == -1)
	{
		cout << "Process creation failed!!" << endl;
	} 
	else if(rid == 0)
	{
		close(fd[0]); 
        dup2(fd[1], STDOUT_FILENO); 
        close(fd[1]);
		if (execvp(cmd[0], cmd) < 0) 
		{ 
	           cout <<"Unable to execute command!!" << endl; 
	    }
	    exit(0);
	}
	else
	{
		int wfptr, lenIn, lenOut;
		close(fd[1]);
		wait(NULL);
		if(flag[">"])
			wfptr = open(file[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		else
			wfptr = open(file[0], O_WRONLY | O_CREAT | O_APPEND, 0644);
		if(wfptr == -1)
		{
			cout << "Invalid file path : " << file[0] << endl;
			return;
		}
		while((lenOut = read(fd[0], buff, 1024)) > 0)
			lenIn = write(wfptr, buff, lenOut);

		close(fd[0]);
		return;
	}
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

	if(!strcmp(arg[0],"cd"))
	{
		if(arg[2])
		{
			cout << "cd : too many arguments" << endl;
			return;
		}
		if(chdir(arg[1]) != 0)
		{
			cout << "cd: Assign: No such file or directory " << endl;
		}
		return;
	}

	if(!strcmp(arg[0],"history"))
	{
		if(arg[1])
		{
			cout << "try: history without any argument" << endl;
			return;
		}

		char buffer[1024];
		int ln;
		int rdh = open("history.txt", O_RDONLY);
		
		while((ln = read(rdh, buffer, 1023)) > 0)
		{
			buffer[ln] = '\0';
			cout << buffer;
		}	

		close(rdh);
		return;
	}

	if(!strcmp(arg[0],"alias"))
	{

	}

	pid_t rid = fork();
	
	if(rid == -1)
	{
		cout << "Process creation failed!!" << endl;
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

void pipe(char **arg)
{
	int cnt = 0;
	char *cmd[128];
	int i = 0;

	cnt = cnt_arg(arg);

	if(flag[">"] || flag[">>"])
		cnt--;

	cout << cnt << endl;
}

void execute_cmd(char **arg)
{
	if(flag["|"])
	{
		pipe(arg);
	}
	else if(flag[">"] || flag[">>"])
	{
		char *cmd[128];
		parse_command(cmd, arg[0]);
		char *file[128];
		parse_command(file, arg[1]);
		redirection(cmd, file); 
	}
	else
	{
		char *cmd[128];
		parse_command(cmd, arg[0]);
		execute(cmd);
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

    write(fdh, buf, strlen(buf));
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

    execute_cmd(arg);
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