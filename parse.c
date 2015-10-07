/**
* Team : SOJOS
* parse for parsing the given lines of input to shell.
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

//Gets the prompt of shell.
char* getPrompt()
{
	int sz =100;
	char *prompt = (char*)malloc(sizeof(char)*sz);
	*prompt='\0';
	strcat(prompt,getlogin());
	strcat(prompt,"@[");
	char *buf=(char*)malloc(sizeof(char)*sz);
	while(1)
	{
		errno=0;
		getcwd(buf,sz);
		if(errno==ERANGE)
		{
			sz=sz+100;
		}
		else
			break;
	}
	strcat(prompt,buf);
	strcat(prompt,"]");
	return prompt;
}


//Executes in the background.
int execFg(Cmd *cmd)
{
	switch (fork())
	{
		case -1:
			return -2;
		case 0:
			execvp(cmd->ex,cmd->arg_list);
			if(cmd->fileout!=NULL)
				close(cmd->fileout);
			if(cmd->filein!=NULL)
				close(cmd->filein);
			return -1;
		default:
			wait();
			return 0;
	}
}

//Executes in the background.
int execBg(char *ex,char **args)
{
	switch (fork())
	{
		case -1:
			return -2;
		case 0:
			execvp(ex,args);
			return -1;
		default:
			return 0;
	}
}

 proc** parsecmd(char * cmd)
 {
	cmd=strndup(cmd,strlen(cmd)-1);
 	int i=0;
 	proc** procs=(proc**)malloc(sizeof(proc*)*10);
 	int size=10;
 	int size1;
 	char* d=strtok(cmd,";");
 	char* d1;int j;
 	while(d!=NULL)
	{
 	//	printf("%s\n",d);
 		if(i>=size)
		{
 			realloc(procs,sizeof(proc*)*(size+5));
 			size=size+5;
 		}
 		procs[i]=(proc*)malloc(sizeof(proc));
 		procs[i]->cmds=(Cmd**)malloc(sizeof(Cmd*)*10);
 		procs[i]->cmds[0]=(Cmd*)malloc(sizeof(Cmd));
 		procs[i]->cmds[0]->ex=d;
 		d=strtok(NULL,";");
 		i++;
 	}

 	for(j=0;j<i;j++)
	{
 		d=procs[j]->cmds[0]->ex;
 		size1=10;
 		char* c=(char*)malloc(sizeof(char)*strlen(d));
 		strcpy(c,d);
 		d1=strtok(c,"|");
 		int s=0;
 		while(d1!=NULL)
		{

 			if(s>=size1)
			{
 				realloc(procs[j]->cmds,sizeof(Cmd*)*(size1+5));
 				size1=size1+5;
 			}
 			if(procs[j]->cmds[s]==NULL)
 				procs[j]->cmds[s]=(Cmd*)malloc(sizeof(Cmd));
  			procs[j]->cmds[s]->ex=d1;
 			d1=strtok(NULL,"|");
 			s++;
 		}
 		procs[j]->nocmd=s;
 		fill_proc(procs[j]);

 	}
 	return procs;
 }

void* fill_proc(proc* p)
{
	int i;
	char*d,*re;
	int flag=2,flag1=0;

	char* r;
	for(i=0;i<p->nocmd;i++){
		p->cmds[i]->BG=0;
		d=strtok(p->cmds[i]->ex," ");
		re=strtok(NULL,"");
		p->cmds[i]->arg_list=(char**)malloc(sizeof(char*)*10);
		if(d!=NULL){
			p->cmds[i]->ex=d;
			p->cmds[i]->arg_list[0]=d;
		}

		d=strtok(re," \n");

		int s=1;
		while(d!=NULL){

			if(strcmp(d,"")!=0){


				if(flag==-1 && flag1==-1)
					yyerror(p->cmds[i]->ex,d);

				if(*d=='>'){
					if(strcmp(d,">")==0){
						flag=1;
						d=strtok(NULL,"> ");
						p->cmds[i]->fileout=d;
						flag=0;
						flag1=-1;
						printf("%s \n",p->cmds[i]->fileout);
					}
					else {

						p->cmds[i]->fileout=(++d);
						flag=0;
						flag1=-1;
						printf("%s \n",p->cmds[i]->fileout);

					}
				}
				else if(*d=='&'){
					p->cmds[i]->BG=1;
					flag=-1;
					printf("flag set\n");
				}
				else{
					if(*d=='"' && d[strlen(d)-1]=='"'){
						d=d+1;
						d[strlen(d)-1]='\0';
					}
					p->cmds[i]->arg_list[s]=d;
					printf("%s\n",p->cmds[i]->arg_list[s]);
					s++;
				}
			}
			d=strtok(NULL," \n");
		}
		if(p->cmds[i]->BG==1){
			if(p->cmds[i]->fileout==NULL){
				char str[100];
				sprintf(str, "%s%d.bgoutput",p->cmds[i]->ex,i);
				p->cmds[i]->fileout=str;
			}
		}
	}

}

yyerror(char* ex,char* flag){
	printf("%s",ex);
	if(flag!=NULL){
		printf(" -%s",flag);
	}
	printf("command not found\n");

}

int execute(proc *prc)
{
	int pip[2];
	int i;
	int ncmd = prc->nocmd;

	i=0;
	while(i<ncmd)
	{
		if(i<ncmd-1)
		{
			pipe(pip);
			dup2(STDOUT_FILENO,pip[1]);
			prc->cmds[i]->fileout = pip[1];
			dup2(STDIN_FILENO,pip[0]);
			prc->cmds[i+1]->filein = pip[0];
		}
		execCmd(prc->cmds[i]);
		i++;
	}
	return 0;
}


int execCmd(Cmd *cmd)
{
	printf("%s\n",cmd->ex );
	//printf("%s\n",cmd->arg_list[1] );
	if(strcmp(cmd->ex,"cd")==0)
	{
		int res = chdir(cmd->arg_list[1]);
		if(res==-1)
			printf("change directory failed.\n");
	}
	else if(strcmp(cmd->ex,"exit")==0)
		exit(0);
	else if(strcmp(cmd->ex,"lsb")==0)
		printf("we'll do it.\n");
	else
		execFg(cmd);
}
