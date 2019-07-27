#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<ctype.h>


#define ERROR -1
#define SIZE 1024
#define PORT 10777


void feedInfo(char *buffer, char *info) {
    
    int j = 0;
    int i;
    
    for(i = 0; buffer[i] != '\0'; i++)
    {
        if(buffer[i] != '\n' && buffer[i] != '\t' && buffer[i] != '\r')
        {
            info[j] = tolower(buffer[i]);
            j++;
        }
    }
    
    info[j] = '\0';
}


void readFileName(char *process_cmd, char *filename) {
    
    int i;
    int j = 0;
    
    for(i = 5; process_cmd[i] != '\0'; i++)
    { filename[j] = process_cmd[i];
      j++;
    }
    
    filename[j] = '\0';
}




int main() {
    
    
    struct sockaddr_in remote_server;
    
    
    char pwd[FILENAME_MAX];
    getcwd(pwd, FILENAME_MAX);
    
    int sockfd;
    
    char command[SIZE];
    char user[SIZE];
    char pass[SIZE];
    
    char server_message[SIZE];
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
    {
        printf("Socket Error\n\n");
        exit(-1);
    }
    
    remote_server.sin_family = AF_INET;
    remote_server.sin_port = htons(PORT);
    remote_server.sin_addr.s_addr = INADDR_ANY;
    bzero(&remote_server.sin_zero, 8);
    
    if(connect(sockfd, (struct sockaddr *)&remote_server, sizeof(struct sockaddr_in)) == ERROR)
    {
        printf("Connection Error\n\n");
        exit(-1);
    }
    
    long data_len = 1;
    char process_cmd[SIZE];
    
    while(1)
    {
        printf("\nUser Login:-\n");
        
        printf("Username: ");
        fgets(user, SIZE, stdin);
        send(sockfd, user, sizeof(user), 0);
        
        printf("Password: ");
        fgets(pass, SIZE, stdin);
        send(sockfd, pass, sizeof(pass), 0);
        
        data_len = recv(sockfd, server_message, SIZE, 0);
        server_message[data_len] = '\0';
        printf("%s", server_message);
        
        
        if(strcmp(server_message, "\n\nGreetings,\nYou have successfully Logged-in\n\n") == 0)
        { bzero(server_message, SIZE);
          break; }
        
        bzero(server_message, SIZE);
        bzero(user, SIZE);
        bzero(pass, SIZE);
    }
    
    
    data_len = recv(sockfd, server_message, SIZE, 0);
    server_message[data_len] = '\0';
    printf("%s", server_message);
    bzero(server_message, SIZE);
    
    
    while(1)
    {
        printf("\n\n\nEnter Command: ");
        fgets(command, SIZE, stdin);
        
        feedInfo(command, process_cmd);
        
        
        // STOR
        if(process_cmd[0] == 's' && process_cmd[1] == 't' && process_cmd[2] == 'o' && process_cmd[3] == 'r' && process_cmd[4] == ' ' && process_cmd[5] != '\0' && process_cmd[5] != ' ')
        {
            char filename[SIZE];
            char fileLocation[SIZE] = "";
         
            readFileName(process_cmd, filename);
            
            strcat(fileLocation, pwd);
            strcat(fileLocation, "/");
            strcat(fileLocation, filename);
            
            FILE *fpointer = fopen(fileLocation, "r");
            
            if(fpointer == NULL)
            {
                printf("Invalid File name....No such file exists");
                
                bzero(command, SIZE);
                bzero(process_cmd, SIZE);
                continue;
            }
            
            char data[SIZE];
            char file_data[SIZE] = "";
            
            while(!feof(fpointer))
            {
                fgets(data, SIZE, fpointer);
                strcat(file_data, data);
            }
            
            fclose(fpointer);
            
            strcat(command, ",");
            strcat(command, filename);
            strcat(command, ",");
            strcat(command, file_data);
        }
        
        
        //RETR
        else if(process_cmd[0] == 'r' && process_cmd[1] == 'e' && process_cmd[2] == 't' && process_cmd[3] == 'r' && process_cmd[4] == ' ' && process_cmd[5] != '\0' && process_cmd[5] != ' ')
        {
            strcat(command, ",");
            strcat(command, pwd);
            strcat(command, ",");
        }
        
        
        send(sockfd, command, sizeof(command), 0);
        
        if(strcmp(process_cmd, "quit") == 0)
        { break; }
        
        data_len = recv(sockfd, server_message, SIZE, 0);
        server_message[data_len] = '\0';
        printf("%s", server_message);
        
        bzero(server_message, SIZE);
        bzero(command, SIZE);
        bzero(process_cmd, SIZE);
    }
    
    printf("\n\n\nSession Terminated.....\n\n");
    
    return 0;
}
