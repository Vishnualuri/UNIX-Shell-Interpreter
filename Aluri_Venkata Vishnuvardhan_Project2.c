#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>

struct node{
	char **args;
	struct node *next;
};

void init(struct node **head){
	*head = NULL;
}

char *read_line(void){
  	char *input = NULL;
  	size_t bufsize = 0; 
  	getline(&input, &bufsize, stdin);
  	return input;
}

#define token_bufsize 64
#define token_delimiter " \t\r\n\a"
char **parse_input(char *input){
	int bufsize = token_bufsize, position = 0;
	char **tokens = (char**)malloc(bufsize * sizeof(char*));
	char *token;
	if(!tokens){
		fprintf(stderr,"memory allocation error\n");
		exit(EXIT_FAILURE);	
	}

	token = strtok(input,token_delimiter);
	while(token != NULL){
		tokens[position] = token;
		position++;
		if(position >= bufsize){
			bufsize += token_bufsize;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if(!tokens){
				fprintf(stderr, "allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL,token_delimiter);
	}
	tokens[position] = NULL;
	return tokens;
}

struct node* push(struct node* head, char **args){
	struct node* temp = (struct node*)malloc(sizeof(struct node));
        if (temp == NULL){
                printf("memory allocation error.\n");
        }
        temp->args = args;
        temp->next = head;
        head = temp;	
        return head;
}

int launch(char **args){
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0){
		// Child process
    		if (execvp(args[0], args) == -1){
      		perror("ls");
    		}
	    	exit(EXIT_FAILURE);
  	} 
	else if (pid < 0) {
		// Error forking
   		perror("abc");
  	} 
	else{
		// Parent process
		do{
      		wpid = waitpid(pid, &status, WUNTRACED);
    		}
		while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}
	return 1;
}

int cd(char **args);
int help(char **args);
int pexit(char **args);
int pwd(char **args);

char *builtin_str[] = {
  	"cd",
  	"help",
	"pwd",
	"exit"
};

int (*builtin_func[]) (char **) = {
  	&cd,
  	&help,
	&pwd,
	&pexit
};

int num_builtins(){
  	return sizeof(builtin_str) / sizeof(char *);
}

int cd(char **args){
 	if (args[1] == NULL){
	  	fprintf(stderr, "an argument is expected to \"cd\"\n");
	}
	else{
   		if (chdir(args[1]) != 0){
   			perror("error in args");
		}
	}
  	return 1;
}

int help(char **args){
	int i;
	printf("Hello \n");
	printf("please enter commands and arguments (if any) \n");
	printf("built in commands:\n");

	for (i = 0; i < num_builtins(); i++){
		printf("  %s\n", builtin_str[i]);
	}
	printf("Use the man command for information on other commands.\n");
	return 1;
}

int pwd(char **args) {
        char cwd[1024];
    chdir("/path/to/change/directory/to");
    getcwd(cwd, sizeof(cwd));
    printf("Current working dir: %s\n", cwd);
	return 1;
}


int pexit(char **args){
	return 0;
}

int execute_input(char **args){
	int i;
	if (args[0] == NULL){
	//no commands were entered(empty)
	return 1;
	}
	for (i = 0; i < num_builtins(); i++){
    		if (strcmp(args[0], builtin_str[i]) == 0){
			return (*builtin_func[i])(args);
		}
	}
	return launch(args);
}

void displayAll(struct node* head){
        struct node *ptr;
        ptr = head;
        int counter = 1;
        if (ptr == NULL){
                printf("(!) History stack is empty\n");
                return;
        }
	while (ptr != NULL){
                int i = 0;
                printf("(%d) ", counter);
                while (ptr->args[i] != NULL){
                        printf("%s ", ptr->args[i]);
                        i++;
                }
                printf("\n");
                ptr = ptr->next;
                counter++;
        }
}

char** History(struct node* head, int n){
        struct node *ptr;
        ptr = head;
        int counter = 1;
        if (ptr == NULL){
                printf("(!) History stack is empty\n");
                return NULL;
        }
        while (ptr != NULL){
                if (n == counter){
                        return ptr->args;
                }
                ptr = ptr->next;
                counter++;
        }
        printf("Command not found\n");
}

int main(int argc, char **argv){
	char *input;
	char **args;
	int status;
	struct node *head;
	init(&head);
	struct node* temp;
	int ctr;

	do{
		printf("VA>>:");
		input = read_line();
		args = parse_input(input);
		int i=0;
		if(strcmp(args[0],"!!") == 0){
			args = History(head,1);
		}
		else if(args[0][0] == '!'){
			int tmp;
			tmp = atoi(&args[0][1]);
			if (tmp <= 0){
				printf("invalid input \n");
				continue;
			}
			else{
				args = History(head,tmp);
			}
		}
		head = push(head, args);
		if(strcmp(args[0], "history") == 0){
			displayAll(head);
		}
		else if(args!=NULL){
			status = execute_input(args);
		}
	}while(status);
}