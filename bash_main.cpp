#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include <sstream>
#include<string.h>
#include <fcntl.h>
#include<vector>
#include<unordered_map>
using namespace std;

unordered_map<string, int> flag;
unordered_map<string, string> alias;
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

void print_cmd(char **cmd)
{
	int i=0;
	while(cmd[i])
	{
		cout << cmd[i] << " ";
		i++;
	}
	cout << endl;
}

void parse_command(char **cmd, char *in)
{
	int i = 0;

	if(strstr(in, "alias"))
	{
		cmd[i] = in;
		return;
	}

	if(strstr(in, "\"") || strstr(in, "\'"))
	{
		char *temp[128];
		temp[i] = strtok(in, "\"\'");
	    while( temp[i] != NULL ) 
	    {
	    	i++;
			temp[i] = strtok(NULL, "\"\'");
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
	string str;
	str = arg[0];
	if(alias.find(str) != alias.end())
	{
		char *cmd[128];
		str = alias[str];
		arg[0] = (char *) str.c_str();
		parse_command(cmd, arg[0]);
		int cnt = cnt_arg(cmd);
		int j = 1;
		while(arg[j])
		{
			cmd[cnt] = arg[j];
			cnt++;
			j++;
		}
		cmd[cnt] = NULL;
		execute(cmd);
		return;
	}

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

	if(strstr(arg[0], "alias"))
	{
		if(strstr(arg[0], "="))
		{
			string key, value;
			char *temp1, *temp2;
 			strtok(arg[0]," ");
 			temp1 = strtok(NULL,"=\'");
 			temp2 = strtok(NULL,"\'");
 			if(temp1 && temp2 && strcmp(temp2,"="))
			{
				key = temp1;
				value = temp2;
				alias[key] = value;
			}	
			else
				cout << "usage:  alias variable='comand'" << endl << "\talias variable" << endl << "\talias -p"<<endl;
		}
		else
		{
			string key;
			char *temp;
			strtok(arg[0]," ");
			temp = strtok(NULL," ");
			if(!temp)
			{
				cout << "usage:  alias variable='comand'" << endl << "\talias variable" << endl << "\talias -p"<<endl;
				return;
			}
			key = temp;
			if(key == "-p")
			{
				for (auto i : alias)
				{
					cout << "alias " << i.first << "='" << i.second << "'" <<endl;
				}
			}
			else
			{
				if(alias.find(key) != alias.end())
				{
					cout << "alias " << key << "='" << alias[key] << "'" <<endl;
				}
				else
				{
					cout << "alias: " << key << ": not found" << endl;
				}
			}
		}
		return;
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
		execute(cmd);
	    exit(0);
	}
	else
	{
		int wfptr, lenOut;
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
		while((lenOut = read(fd[0], buff, 1023)) > 0)
		{
			buff[lenOut] = '\0';
			write(wfptr, buff, lenOut);
		}	

		close(fd[0]);
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

	int fd[2];
	int pre_read = 0;
	char buff[1024];

 	for (int i = 0; i < cnt; ++i)
    {
    	pipe(fd);
    	int rid = fork();    	
    	if(rid == -1)
		{
			cout << "Process creation failed!!" << endl;
		} 
		else if(rid == 0)
		{
			dup2(pre_read, 0);

			if(flag[">"] || flag[">>"])
				dup2(fd[1], 1);
			else 
			{
				if(i < cnt-1)
					dup2(fd[1], 1);
			}

			
			close(fd[0]);
			close(fd[1]);
			parse_command(cmd, arg[i]);	
			execute(cmd);
			exit(0);
		}
		else
		{ 	
			wait(NULL);
			close(fd[1]);
			pre_read = fd[0];
		}
    }

    if(flag[">"] || flag[">>"])
    {
    	int wfptr, lenOut;
    	char *file[128];
    	parse_command(file, arg[cnt]);
    	cout << file[0];
		if(flag[">"])
			wfptr = open(file[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		else
			wfptr = open(file[0], O_WRONLY | O_CREAT | O_APPEND, 0644);
		if(wfptr == -1)
		{
			cout << "Invalid file path : " << file[0] << endl;
			return;
		}
		while((lenOut = read(fd[0], buff, 1023)) > 0)
		{
			buff[lenOut] = '\0';
			write(wfptr, buff, lenOut);
		}	

		close(fd[0]);
		return;
    }
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