#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include <signal.h>
#include<string.h>
#include<string>
#include <fcntl.h>
#include<vector>
#include<unordered_map>
#include <pwd.h>
using namespace std;
extern char **environ;
unordered_map<string, int> flag;
unordered_map<string, string> alias;
unordered_map<string, string> variable;
unordered_map<string, string> e;
char *env[128];
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
	struct passwd *p;
	int u;
	u = geteuid ();
  	p = getpwuid (u);
	
	char *host = (char *) malloc(128 * sizeof(char));
	char *h = (char *) malloc(128 * sizeof(char));
	gethostname(host, 128);
	strcpy(h, "HOSTNAME=");
	strcat(h, host);

	char *temp2 = (char *) malloc(128 * sizeof(char));
	strcpy(temp2,"HOME=");
	strcat(temp2,p->pw_dir);
	
	char *temp3 = (char *) malloc(128 * sizeof(char)); 
	strcpy(temp3,"USER=");
	strcat(temp3, p->pw_name);
	
	char *temp = (char *) malloc(1024 * sizeof(char));
	strcpy(temp,"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games");
	
	char *temp1 = (char *) malloc(2048 * sizeof(char));
	char cwd[4096];
	getcwd(cwd, 4096);
	strcpy(temp1,"PS1=");
	strcat(temp1, p->pw_name);
	strcat(temp1, "@");
	strcat(temp1, host);
	strcat(temp1, ":");
	strcat(temp1, cwd);
	strcat(temp1,"$");

	char *temp4 = (char *) malloc(20 * sizeof(char));
	strcpy(temp4, "TERM=xterm-256color");
	
	env[0] = temp2;
	env[1] = temp;
	env[2] =  h;
	env[3] = temp3;
	env[4] = temp1;
	env[5] = temp4;
	environ = env;
}

void prompt()
{
	char *user = getenv("USER");
	char *host = getenv("HOSTNAME");
	char *ps1 = getenv("PS1");
	cout << "\033[1;34m" << ps1 << "\033[0m";

	if(getuid() == 0)
	{
		cout << "# ";
	}
	else
	{
		cout << "$ ";
	}
	flag["|"] = 0;
	flag[">>"] = 0;
	flag[">"] = 0;
	flag["s"] = 0;
	flag["="] = 0;
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
	cout << "print cmd:" << endl;
	while(cmd[i])
	{
		cout << cmd[i] << " ";
		i++;
	}
	cout << endl ;
}

void resolve_var(char **cmd)
{
	int k = 0;
	while(cmd[k])
	{
		if(strstr(cmd[k], "$"))
		{
			if(!strstr(cmd[k]," "))
			{
				string str;
				str = cmd[k];
				if(str == "$HOME")
				{
					cmd[k] = environ[0];
				}
				else if(str == "$PATH")
				{
					cmd[k] = environ[1];
				}
				else if(str == "$PS1")
				{
					cmd[k] = environ[4];
				}
				else if(str == "HOSTNAME")
				{
					cmd[k] = environ[2];
				}
				else if(str == "$USER")
				{
					cmd[k] = environ[3];
				}
				else
					cmd[k] = (char *)variable[str].c_str();
			}
		}
		k++;
	}
}

void parse_command(char **cmd, char *in)
{
	int i = 0;

	if(strstr(in, "="))
	{
		flag["="] = 1;
		char *temp[128];
		temp[0] = strtok(in, "=");
		temp[1] = strtok(NULL, "=");
		i = 0;
		cmd[i] = strtok(temp[0], " ");
		while( cmd[i] != NULL ) 
		{
		  	i++;
			cmd[i] = strtok(NULL, " ");
		}
	    if(strstr(temp[1], "\"") || strstr(temp[1], "\'"))
		{
			cmd[i++] = strtok(temp[1], "\'\"");
			cmd[i] = strtok(NULL, "\'\"");
		    while( cmd[i] != NULL ) 
		    {
		    	i++;
				cmd[i] = strtok(NULL, " ");
		    }
		}
		else
		{
			cmd[i] = strtok(temp[1], " ");
		    while( cmd[i] != NULL ) 
		    {
		    	i++;
				cmd[i] = strtok(NULL, " ");
		    }
		}
		resolve_var(cmd);
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
	resolve_var(cmd);
}

void execute(char **arg)
{
	string str;
	str = arg[0];
	if(alias.find(str) != alias.end())
	{
		string val;
		char *cmd[128];
		val = alias[str];
		while(alias.find(val) != alias.end())
		{
			val = alias[val];
			if(val == str)
			{
				cout <<"Unable to execute command!!" << endl;
				return;
			}
		}
		str = val;

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

	if(!strcmp(arg[0],"alias"))
	{
		int cnt = cnt_arg(arg);
		if(flag["="])
		{
			if(cnt == 3)
			{
				string key, value;
				key = arg[1];
 				value = arg[2];
				alias[key] = value;
			}	
			else
				cout << "usage:  alias variable='comand' : to assign alias" 
			         << endl << "\talias variable : to check alias value" 
			         << endl << "\talias -p : to print all value"<<endl;
		}
		else
		{
			if(cnt == 1)
			{
				for (auto i : alias)
				{
					cout << "alias " << i.first << "='" << i.second << "'" <<endl;
				}
			}
			else if(cnt == 2)
			{
				string key;
				key = arg[1];
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
						cout << "usage:  alias variable='comand' : to assign alias" 
					         << endl << "\talias variable : to check alias value" 
					         << endl << "\talias -p : to print all value"<<endl;
				}

			}
			else
			{
				cout << "usage:  alias variable='comand'" << endl << "\talias variable" << endl << "\talias -p"<<endl;
				return;
			}
		}
		return;
	}

	if(flag["="])
	{
		if(cnt_arg(arg) == 2)
		{
			string var,val;
			var = arg[0];
			var = "$"+var;
			val = arg[1];
			variable[var] = val;
		}
		else
		{
			cout <<"Unable to execute command!!" << endl;
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
			buff[++lenOut] = '\0';
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

    if(buf[0] == '\n')
    	return;

    if(flag["s"])
    	return;

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

void signal_handler( int signal_num ) 
{ 
   signal(signal_num, signal_handler);
   cout << endl << "The interrupt signal is (" << signal_num << ")." << endl;
   flag["s"] = 1;
   return;
}

int main()
{
	signal(SIGINT, signal_handler);
	init();
	while(1)
	{
		prompt();
		parse_and_excute();
	}
	return 0;	
}