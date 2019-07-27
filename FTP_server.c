#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<strings.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<dirent.h>
#include<errno.h>
#include<sys/time.h>
#include<ctype.h>
#include<sys/stat.h>


#define ERROR -1
#define SIZE 1024
#define MAX_CLIENT 10
#define PORT 10777


void getCommand(char *buffer, char *command) {
    
    int j = 0;
    int i;
    
    for(i = 0; buffer[i] != '\0'; i++)
    {
        if(buffer[i] != '\n' && buffer[i] != '\t' && buffer[i] != '\r')
        {
            command[j] = tolower(buffer[i]);
            j++;
        }
    }
    
    command[j] = '\0';
}



int getFileName(char *data, char *filename, int pos) {
    
    int i, j = 0, flag = 0;
    
    for(i = pos; data[i] != '\0'; i++)
    {
        if(data[i] == ' ')
        { flag = 1;
          break; }
        
        if(data[i] != '\n' && data[i] != '\t' && data[i] != '\r')
        { filename[j] = data[i];
          j++; }
    }
    
    if(flag == 0)
    { filename[j] = '\0'; }
    
    return flag;
}





int deleteFile(char *filename) {
    
    int flag = 0;
    
    DIR *dir = opendir(filename);
    int ret;
    
    if(dir)
    { ret = rmdir(filename); }
    
    else if(ENOENT == errno)
    { flag = 1; }
    
    else
    { flag = 2; }
    
    return flag;
}



int dirExists(char *filename) {
    
    int flag;
    
    DIR *dir = opendir(filename);
    
    if(dir)
    { flag = 0; }
    
    else if(ENOENT == errno)
    { flag = 1; }
    
    else
    { flag = 2; }
    
    return flag;
}



void getFileDetails(char *command, char *file_name, char *file_data) {
    
    int i = 0;
    
    while(command[i] != ',')
    { i++; }
        
    i++;
    
    int j = 0;
    
    while(command[i] != ',')
    { file_name[j] = command[i];
      i++;
      j++;
    }
    
    file_name[j] = '\0';
    
    i++;
    j = 0;
    
    
    while(command[i] != '\0')
    {
      if(command[i] == '\r')
      { i++;
        continue; }
        
      file_data[j] = command[i];
      i++;
      j++;
    }
    
    file_data[j] = '\0';
}





void list_command(char *list, char *cwd) {
    
    DIR *dir;
    struct dirent *sd;
    
    dir = opendir(cwd);
    
    if(dir == NULL)
    {
        perror("Unable to open directory...\n");
        exit(-1);
    }
    
    strcat(list, "\n");
    
    while((sd = readdir(dir)) != NULL)
    {
        if(sd->d_name[0] != '.')
        { strcat(list, sd->d_name);
          strcat(list, "\n"); }
    }
    
    closedir(dir);
}



void getClientDirectory(char *command, char *directory) {
    
    int i = 0;
    
    while(command[i] != ',')
    { i++; }
    
    i++;
    int j = 0;
    
    while(command[i] != ',')
    { directory[j] = command[i];
      i++;
      j++;
    }
    
    directory[j] = '\0';
}




int getReqFileName(char *command, char *filename) {
    
    int i;
    int j = 0;
    
    int flag = 0;
    
    for(i = 5; command[i] != ','; i++)
    {
        if(command[i] == ' ')
        { flag = 1;
          break; }
        
        if(command[i] != '\n' && command[i] != '\t' && command[i] != '\r')
        { filename[j] = command[i];
          j++; }
    }
    
    if(flag == 0)
    { filename[j] = '\0'; }
        
    return flag;
}




void serverReply(char *code, int port_no) {
    
    if(strcmp(code, "200") == 0)
    { printf("[200] Command Okay : Port No. %d\n", port_no); }
    
    else if(strcmp(code, "530") == 0)
    { printf("[530] Not Logged-in : Port No. %d\n", port_no); }
    
    else if(strcmp(code, "331") == 0)
    { printf("[331] Username okay, need password : Port No. %d\n", port_no); }
    
    else if(strcmp(code, "225") == 0)
    { printf("[225] Data connection open; no transfer in progress : Port No. %d\n", port_no); }
    
    else if(strcmp(code, "551") == 0)
    { printf("[551] Request action aborted; page type unknown : Port No. %d\n", port_no); }
    
    else if(strcmp(code, "502") == 0)
    { printf("[502] Command not implemented : Port No. %d\n", port_no); }
    
    else if(strcmp(code, "503") == 0)
    { printf("[503] Bad sequence of commands : Port No. %d\n", port_no); }
    
    else if(strcmp(code, "504") == 0)
    { printf("[504] Command not implemented for that parameter : Port No. %d\n", port_no); }
    
}





int main() {
    
    struct sockaddr_in server;
    struct sockaddr_in client;
    
    char cwd[FILENAME_MAX];
    getcwd(cwd, FILENAME_MAX);
    
    int sockfd;
    int new_client;
    unsigned int sockaddr_len = sizeof(struct sockaddr_in);
    
    char buffer[SIZE];
    char command[SIZE];
    
    char user[SIZE];
    char pass[SIZE];
    char greetings[SIZE] = "\n\nGreetings,\nYou have successfully Logged-in\n\n";
    char invalid_details[SIZE] = "\nInvalid User details....Try again\n";
    
    char command_list[SIZE] = "\n\nList of commands:\n\n1. USER\n2. PASS\n3. MKD\n4. RMD\n5. PWD\n6. CWD\n7. LIST\n8. STOR\n9. RETR\n10. QUIT";
    
    
    
    
    char end_session[SIZE] = "\nSession terminated...\n\n";
    char invalid_file[SIZE] = "Invalid Filename\nThere should be no blank space in the filename.....";
    char list[SIZE] = "";
    char invalid_input[SIZE] = "Invalid Input command........";
    
    
    pid_t childpid; // For each client entry
    
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
    {
        printf("Socket Error\n\n");
        exit(-1);
    }
    
    
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&server.sin_zero, 8);
    
    
    if((bind(sockfd, (struct sockaddr *)&server, sockaddr_len)) == ERROR)
    {
        printf("Bind error\n\n");
        exit(-1);
    }
    
    
    if((listen(sockfd, MAX_CLIENT)) == ERROR)
    {
        printf("Listening Error\n\n");
        exit(-1);
    }
    
    long data_len = 1;
    
    printf("Server is Online\n\n");
    
    while(1)
    {
        
        if((new_client = accept(sockfd, (struct sockaddr *)&client, &sockaddr_len)) == ERROR)
        {
            printf("Accepting Error\n\n");
            exit(-1);
        }
        
        printf("New Client connected with Port No. %d and IP %s\n\n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));
        
        
        if((childpid = fork()) == 0)        // This is how we deal with multiple clients
        {
            
            while(1)
            {
                serverReply("530", ntohs(client.sin_port));
                
                data_len = recv(new_client, buffer, SIZE, 0);
                buffer[data_len] = '\0';
                getCommand(buffer, user);
                
                bzero(buffer, SIZE);
                
                
                serverReply("331", ntohs(client.sin_port));
                
                data_len = recv(new_client, buffer, SIZE, 0);
                buffer[data_len] = '\0';
                getCommand(buffer, pass);
                
                bzero(buffer, SIZE);
                
                if(strcmp(user, "admin") == 0 && strcmp(pass, "12345") == 0)
                {
                    serverReply("225" , ntohs(client.sin_port));
                    send(new_client, greetings, sizeof(greetings), 0);
                    break;
                }
                
                else
                { send(new_client, invalid_details, sizeof(invalid_details), 0); }
                
                bzero(user, SIZE);
                bzero(pass, SIZE);
            }
            
            
            send(new_client, command_list, sizeof(command_list), 0);
            
            
            while(1)
            {
                data_len = recv(new_client, buffer, SIZE, 0);
                buffer[data_len] = '\0';
               // printf("Received Information: %s", buffer);
                
                
                //8. STOR
                if(buffer[0] == 's' && buffer[1] == 't' && buffer[2] == 'o' && buffer[3] == 'r' && buffer[4] == ' ' && buffer[5] != ' ' && buffer[5] != '\0' && buffer[5] != '\n' && buffer[6] != '\r')
                {
                    
                    char file_name[SIZE];
                    char file_data[SIZE];
                    char file_location[SIZE] = "";
                    
                    getFileDetails(buffer, file_name, file_data);
                    
                    strcat(file_location, cwd);
                    strcat(file_location, "/");
                    strcat(file_location, file_name);
                    
                    FILE *fpointer = fopen(file_location, "w");
                    fprintf(fpointer, file_data, SIZE);
                    fclose(fpointer);
                    
                    char file_stored[SIZE] = "File \"";
                    strcat(file_stored, file_name);
                    strcat(file_stored, "\" successfully stored on present server location: ");
                    strcat(file_stored, cwd);
            
                    
                    serverReply("200", ntohs(client.sin_port));
                    send(new_client, file_stored, sizeof(file_stored), 0);
            
                    continue;
                }
                
                
                getCommand(buffer, command);
                
                
                /************************/
                
                
                //1. PWD
                if(strcmp(command, "pwd") == 0)
                { serverReply("200", ntohs(client.sin_port));
                  send(new_client, cwd, FILENAME_MAX, 0); }
                
                
                
                //2. MKD
                else if(command[0] == 'm' && command[1] == 'k' && command[2] == 'd' && command[3] == ' ' && command[4] != ' ' && command[4] != '\0')
                {
                    char filename[SIZE];
                    char file_created[SIZE] = "File \"";
                    int ret;
                    
                    if(getFileName(command, filename, 4) == 0)
                    { ret = mkdir(filename, S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH);
                        
                      strcat(file_created, filename);
                      strcat(file_created, "\" created successfully....");
                        
                      serverReply("200", ntohs(client.sin_port));
                      send(new_client, file_created, sizeof(file_created), 0);
                    }
                    
                    else
                    { serverReply("504", ntohs(client.sin_port));
                      send(new_client, invalid_file, sizeof(invalid_file), 0); }
                    
                }
                
                
                //3. RMD
                else if(command[0] == 'r' && command[1] == 'm' && command[2] == 'd' && command[3] == ' ' && command[4] != ' ' && command[4] != '\0')
                {
                    char filename[SIZE];
                    char file_removed[SIZE] = "File \"";
                    
                    
                    if(getFileName(command, filename, 4) == 0)
                    { int removed = deleteFile(filename);
                        
                      strcat(file_removed, filename);
                        
                      if(removed == 0)
                      { serverReply("200", ntohs(client.sin_port));
                        strcat(file_removed, "\" removed successfully...."); }
                        
                      else
                      { serverReply("502", ntohs(client.sin_port));
                        strcat(file_removed, "\" does not exist....."); }
                        
                        send(new_client, file_removed, sizeof(file_removed), 0);
                    }
                    
                    
                    else
                    { serverReply("504", ntohs(client.sin_port));
                      send(new_client, invalid_file, sizeof(invalid_file), 0); }
                }
                
                
                //4. USER
                else if(strcmp(command, "user") == 0)
                { serverReply("200", ntohs(client.sin_port));
                  send(new_client, user, sizeof(user), 0); }
                
                
                //5. PASS
                else if(strcmp(command, "pass") == 0)
                { serverReply("200", ntohs(client.sin_port));
                  send(new_client, pass, sizeof(pass), 0); }
                
                
                //6. LIST
                else if(strcmp(command, "list") == 0)
                {   list_command(list, cwd);
                    serverReply("200", ntohs(client.sin_port));
                    send(new_client, list, sizeof(list), 0);
                    bzero(list, sizeof(list));
                }
                
                
                //7. CWD
                else if(command[0] == 'c' && command[1] == 'w' && command[2] == 'd' && command[3] == ' ' && command[4] != ' ' && command[4] != '\0')
                {
                    char dir_location[FILENAME_MAX];
                    char message[SIZE] = "File Location \"";
                    
                    if(getFileName(command, dir_location, 4) == 0)
                    {
                        strcat(message, dir_location);
                        
                        if(dirExists(dir_location) == 0)
                        { bzero(cwd, FILENAME_MAX);
                          strcat(cwd, dir_location);
                          
                          serverReply("200", ntohs(client.sin_port));
                          strcat(message, "\" set as present working director (pwd)");
                        }
                        
                        else
                        { serverReply("551", ntohs(client.sin_port));
                          strcat(message, "\" does NOT exist"); }
                        
                        
                        send(new_client, message, sizeof(message), 0);
                    }
                    
                    else
                    { serverReply("504", ntohs(client.sin_port));
                      send(new_client, invalid_file, sizeof(invalid_file), 0); }
                }
                
                
                
                //9. RETR
                else if(command[0] == 'r' && command[1] == 'e' && command[2] == 't' && command[3] == 'r' && command[4] == ' ' && command[5] != ' ' && command[5] != '\0')
                {
                    
                    char file_name[SIZE];
                    char message[SIZE] = "";
                    
                    
                    if(getReqFileName(command, file_name) == 0)
                    {
                        char file_location[SIZE] = "";
                        
                        strcat(file_location, cwd);
                        strcat(file_location, "/");
                        strcat(file_location, file_name);
                        
                        
                        FILE *fpointer = fopen(file_location, "r");
                        
                        
                        if(fpointer == NULL)
                        {
                            strcat(message, "File \"");
                            strcat(message, file_name);
                            strcat(message, "\" does not exist at current server location:  ");
                            strcat(message, cwd);
                            
                            serverReply("502", ntohs(client.sin_port));
                            send(new_client, message, sizeof(message), 0);
                            
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
                        
                        
                        char client_location[SIZE] = "";
                        char client_dir[SIZE];
                        getClientDirectory(command, client_dir);
                        
                        
                        strcat(client_location, client_dir);
                        strcat(client_location, "/");
                        strcat(client_location, file_name);
                        
                        
                        FILE *w_pointer = fopen(client_location, "w");
                        fprintf(w_pointer, file_data, SIZE);
                        fclose(w_pointer);
                        
                        
                        strcat(message, "Requested File \"");
                        strcat(message, file_name);
                        strcat(message, "\" successfully transferred to client location: ");
                        strcat(message, client_dir);
                        
                        serverReply("200", ntohs(client.sin_port));
                        send(new_client, message, sizeof(message), 0);
                        
                    }
                    
                    
                    else
                    {
                        serverReply("504", ntohs(client.sin_port));
                        send(new_client, invalid_file, sizeof(invalid_file), 0);
                    }
                    
                }
                
                
                
                
                //10. QUIT
                else if(strcmp(command, "quit") == 0)
                { serverReply("200", ntohs(client.sin_port));
                  break; }
                
                
                else
                { serverReply("503", ntohs(client.sin_port));
                  send(new_client, invalid_input, sizeof(invalid_input), 0); }
                
                
                bzero(buffer, SIZE);
                bzero(command, SIZE);
                
                
                /************************/
                
            }
            
            send(new_client, end_session, sizeof(end_session), 0);
            printf("\nClient with Port No. %d and IP %s Disconnected....\n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));
            close(new_client);
            
        }
        
    }
    
    
}
