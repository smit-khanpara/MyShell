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
unordered_map<string, string> op;
char *env[128];
char *ps1;
int fdh, fds;
int err = 0;
char *out;

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
		if(!strcmp(cmd[k],"$$"))
		{
			pid_t pd = getpid();
			char * pid = (char *)malloc(8 * sizeof(char));
			sprintf(pid, "%d", pd);
			cmd[k] = pid;
		}
		else if(!strcmp(cmd[k],"$?"))
		{
			char *e = (char *)malloc(8 * sizeof(char));
			sprintf(e, "%d", err);
			cmd[k] = e;
		}
		else if(!strcmp(cmd[k],"~"))
		{
			cmd[k] = getenv("HOME");
		}
		else if(strstr(cmd[k], "$"))
		{
			if(!strstr(cmd[k]," "))
			{
				string str;
				str = cmd[k];
				if(str == "$HOME")
					cmd[k] = getenv("HOME");
				else if(str == "$PATH")
					cmd[k] = getenv("PATH");
				else if(str == "$PS1")
					cmd[k] = getenv("PS1");
				else if(str == "$HOSTNAME")
					cmd[k] = getenv("HOSTNAME");
				else if(str == "$USER")
					cmd[k] = getenv("USER");
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

	if(strstr(in,"MEDIA"))
	{
		char *temp = strtok(in, "[");
		char *temp1 = strtok(NULL,"]");
		cmd[i++] = strtok(temp,"=");
		cmd[i++] = strtok(temp1,"\'");
		cmd[i++] = strtok(NULL,"\',\'");
		cmd[i++] = strtok(NULL,"\',\'");
		cmd[i++] = strtok(NULL,"\'");
		cmd[i] = NULL;
		return;
	}

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
				cout <<"Command not found!!" << endl;
				err = 127;
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

	if(!strcmp(arg[0],"record"))
	{
		if(!strcmp(arg[1],"start"))
		{
			fds = open("script.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
			flag["r"] = 1;
			err = 0;
			if(flag["r"])
				write(fds,"\n",1);
		}
		else if(!strcmp(arg[1],"stop"))
		{
			close(fds);
			flag["r"] = 0;
			err = 0;
			if(flag["r"])
				write(fds,"\n",1);
		}
		else
		{
			strcpy(out,"command not found!!\n");
			if(flag["r"])
			{
				write(fds,out,strlen(out));
				write(fds,"\n",1);
			}
			cout << out;
           	err = 127;
		}
		return;
	}

	if(!strcmp(arg[0],"exit"))
	{
		strcpy(out,"exit\n");
		if(flag["r"])
		{
			write(fds,out,strlen(out));
			write(fds,"\n",1);
		}
		cout << out;
		exit(0);
	}

	if(!strcmp(arg[0],"cd"))
	{
		if(cnt_arg(arg) > 2)
		{
			strcpy(out,"cd : too many arguments\n");
			if(flag["r"])
			{
				write(fds,out,strlen(out));
				write(fds,"\n",1);
			}
			cout << out;
			err = 1;
			return;
		}
		if(chdir(arg[1]) != 0)
		{
			strcpy(out,"cd: Assign: No such file or directory\n");
			if(flag["r"])
			{
				write(fds,out,strlen(out));
				write(fds,"\n",1);
			}
			cout << out;
			err = 1;
		}
		if(flag["r"])
			write(fds,"\n",1);
		err = 0;
		return;
	}

	if(!strcmp(arg[0],"history"))
	{
		if(cnt_arg(arg) > 1)
		{
			cout << "try: history without any argument" << endl;
			err = 127;
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
		err = 0;
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
				err = 0;
				if(flag["r"])
					write(fds,"\n",1);
			}	
			else
			{
				strcpy(out,"usage:  alias variable='comand' : to assign alias\n\talias variable : to check alias value\n\talias -p : to print all value\n");
				if(flag["r"])
				{
					write(fds,out,strlen(out));
					write(fds,"\n",1);
				}
				cout << out;
			    err = 127;
			}	
		}
		else
		{
			if(cnt == 1)
			{
				for (auto i : alias)
				{
					strcpy(out,"alias ");
					strcat(out,(char *)i.first.c_str());
					strcat(out,"='");
					strcat(out,(char *)i.second.c_str());
					strcat(out,"'\n");
					if(flag["r"])
					{
						write(fds,out,strlen(out));
					}
					cout << out;
				}
				if(flag["r"])
					write(fds,"\n",1);
				err = 0;
			}
			else if(cnt == 2)
			{
				string key;
				key = arg[1];
				if(key == "-p")
				{
					for (auto i : alias)
					{
						strcpy(out,"alias ");
						strcat(out,(char *)i.first.c_str());
						strcat(out,"='");
						strcat(out,(char *)i.second.c_str());
						strcat(out,"'\n");
						if(flag["r"])
						{
							write(fds,out,strlen(out));
						}
						cout << out;
					}
					if(flag["r"])
						write(fds,"\n",1);
					err = 0;
				}
				else
				{
					if(alias.find(key) != alias.end())
					{
						strcpy(out,"alias ");
						strcat(out,(char *)key.c_str());
						strcat(out,"='");
						strcat(out,(char *)alias[key].c_str());
						strcat(out,"'\n");
						if(flag["r"])
						{
							write(fds,out,strlen(out));
							write(fds,"\n",1);
						}
						cout << out;
						err = 0;
					}
					else
					{
						strcpy(out,"alias: ");
						strcat(out,(char *) key.c_str());
						strcat(out,": not found\n");
						if(flag["r"])
						{
							write(fds,out,strlen(out));
							write(fds,"\n",1);
						}
						cout << out;
					    err = 127;
					}	
				}

			}
			else
			{
				strcpy(out,"usage:  alias variable='comand' : to assign alias\n\talias variable : to check alias value\n\talias -p : to print all value\n");
				if(flag["r"])
				{
					write(fds,out,strlen(out));
					write(fds,"\n",1);
				}
				cout << out;
				err = 127;
			}
		}
		return;
	}

	if(!strcmp(arg[0],"open"))
	{
		string ext;
		char file[128];
		strcpy(file,arg[1]);
		char *temp = strtok(file,".");
		ext = strtok(NULL, ".");
		ext = "."+ext;
		arg[0] = (char *) op[ext].c_str();
		err = 0;
	}

	if(!strcmp(arg[0],"MEDIA"))
	{
		string format, runner; 
		runner = arg[1];
		format = arg[3];
		op[format] = runner;
		char *path = getenv("PATH");
		if(!strstr(path,arg[2]))
		{
			char *temp = (char *)malloc(128 * sizeof(char));
			strcpy(temp,path);
			strcat(temp,":");
			strcat(temp,arg[2]);
			setenv("PATH",temp,1);
		}
		err = 0;
		if(flag["r"])
				write(fds,"\n",1);
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
			if(var == "$HOME")
				setenv("HOME",arg[1],1);
			else if(var == "$PATH")
				setenv("PATH",arg[1],1);
			else if(var == "$PS1")
				setenv("PS1",arg[1],1);
			else if(var == "$HOSTNAME")
				setenv("HOSTNAME",arg[1],1);
			else if(var == "$USER")
				setenv("USER",arg[1],1);
			else	
				variable[var] = val;
			err = 0;
			if(flag["r"])
				write(fds,"\n",1);
		}
		else
		{
			strcpy(out,"Command not found!!\n");
			if(flag["r"])
			{
				write(fds,out,strlen(out));
				write(fds,"\n",1);
			}
			cout << out;
			err = 127;
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
           cout <<"command not found!!" << endl;
           exit(127);
	    }
	    return;
	}
	else
	{
		wait(&err);
		err = WEXITSTATUS(err);
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
	    exit(err);
	}
	else
	{
		int wfptr, lenOut, e;
		close(fd[1]);
		wait(&e);
		e = WEXITSTATUS(e);
		if(e == 127)
		{
			cout << "Command not found!!" << endl;
			err = 127;
			close(fd[0]);
			return;
		}

		if(flag[">"])
			wfptr = open(file[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		else
			wfptr = open(file[0], O_WRONLY | O_CREAT | O_APPEND, 0644);
		if(wfptr == -1)
		{
			cout << file[0] << ": No such file or directory" << endl;
			err = 1;
			return;
		}
		
		while((lenOut = read(fd[0], buff, 1023)) > 0)
		{
			buff[++lenOut] = '\0';
			write(wfptr, buff, lenOut-1);
		}	
		err = 0;
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
	vector<int> pre_read;
	pre_read.push_back(0);
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
			dup2(pre_read[i], 0);

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
			pre_read.push_back(fd[0]);
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

void parse_and_execute()
{ 	
	char *arg[128];
	char buf[1024];
   	char *input; 
   	char *scr = (char *) malloc(1024 * sizeof(char));
   	char *temp1, *temp2;
    int i = 0;
    fgets(buf, 1024, stdin);

    if(buf[0] == '\n')
    	return;

    if(flag["s"])
    	return;
    
    if(flag["r"])
    {	
    	strcpy(scr, ps1);
    	strcat(scr, buf);
    	write(fds, scr, strlen(scr));	
    }
    
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
	
	if(strstr(arg[0],"record") || strstr(arg[0],"exit") 
	|| strstr(arg[0],"cd") || strstr(arg[0],"alias") 
	|| strstr(arg[0],"MEDIA") || strstr(arg[0],"=") )
	{
		execute_cmd(arg);
		return;
	}

	else if(flag["r"])
    {
    	int fd[2];
		char buf1[1024], buf2[1024];
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
	    	execute_cmd(arg);
		    exit(err);
		}
		else
		{
			int lenOut, e;
			close(fd[1]);
			wait(&e);
			err = WEXITSTATUS(e);

			while((lenOut = read(fd[0], buf1, 1023)) > 0)
			{
				buf1[lenOut] = '\0';
				strcpy(buf2,buf1);
				write(fds, buf1, lenOut);
				write(1, buf2,lenOut);
			}
			write(fds,"\n",1);
			close(fd[0]);
			return;
		}
    }
    else
		execute_cmd(arg);    
}

void init()
{
	cout << endl;
	cout << "*****************************************************************************" << endl;
	cout <<	"*                                                                           *"	<< endl;
	cout <<	"*                          Welcome To My Unix Shell                         *"	<< endl;
	cout <<	"*                                                                           *"	<< endl;
	cout << "*****************************************************************************" << endl;
	cout << endl;

	ps1 = (char *) malloc(1024 * sizeof(char)); 
	out = (char *) malloc(4096 * sizeof(char));
	flag["r"] = 0;
	fdh = open("history.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
	struct passwd *p;
	int u;
	u = geteuid();
  	p = getpwuid(u);
	
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
	strcat(temp1,"\0");

	char *temp4 = (char *) malloc(20 * sizeof(char));
	strcpy(temp4, "TERM=xterm-256color");

	char *temp5 = (char *) malloc(128 * sizeof(char));
	strcpy(temp5,"DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus");

	char *temp6 = (char *) malloc(128 * sizeof(char));
	strcpy(temp6, "DISPLAY=:0");

	char *temp7 = (char *) malloc(128 * sizeof(char));
	strcpy(temp7, "XDG_RUNTIME_DIR=/run/user/1000");
	

	env[0] = temp2;
	env[1] = temp;
	env[2] =  h;
	env[3] = temp3;
	env[4] = temp1;
	env[5] = temp4;
	env[6] = temp5;
	env[7] = temp6;
	env[8] = temp7;

	environ = env;

	FILE *rc;
	char buf[1024];
	char *line;
	line = (char *) malloc(1024 * sizeof(char));
	rc = fopen(".myrc","r");
	while(fgets(buf, 1024, rc) != NULL)
	{
		if(buf[0] == '\n')
    		continue;
    	if(buf[0] == EOF)
    		break;
    	line = strtok(buf,"\n");
    	char *cmd[128];
    	parse_command(cmd, line);
    	execute(cmd);
	}
	fclose(rc);
}

void prompt()
{
	char *temp1;
	char cwd[4096];
	getcwd(cwd, 4096);
	char *ps = getenv("PS1");
	if(strstr(ps,":"))
	{
		temp1 = strtok(ps,":");
		strcpy(ps1, temp1);
		strcat(ps1,":");
		strcat(ps1, cwd);
		setenv("PS1",ps1,1);
	}

	if(getuid() == 0)
		strcat(ps1, "# ");
	else
		strcat(ps1, "$ ");

	cout << "\033[1;34m" << ps1 << "\033[0m";

	flag["|"] = 0;
	flag[">>"] = 0;
	flag[">"] = 0;
	flag["s"] = 0;
	flag["="] = 0;
}

void signal_handler( int signal_num ) 
{ 
   	signal(signal_num, signal_handler);
   	strcpy(out,"\nKeyboard Interrupt\n");
   	if(flag["r"])
	{
		write(fds,out,strlen(out));
		write(fds,"\n",1);
	}	
   	cout << out;
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
		parse_and_execute();
	}
	return 0;	
}