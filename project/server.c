#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>

#define CONNECTIONS 5
#define PORT 8080
#define BUF_SIZE 1024

char* requestHandler(char*); // parses http request, returns file name on the request header
char* fType(const char*); // returns file type
char* fStatus(const char*); // returns true file name if file is found
char* responseHeader(int, char*, int);// builds response header

int main() {
    // create server socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // set SO_REUSEPORT
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    // define the address
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // bind port
    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    // listen for connections
    listen(server_socket, CONNECTIONS);
 
    // connect to client
    int client_socket, numbytes;
    char buf[BUF_SIZE];
    client_socket = accept(server_socket, NULL, NULL);

    // get http request
    numbytes = recv(client_socket, buf, BUF_SIZE-1, 0);
    buf[numbytes] = '\0';
    printf("%s\n", buf);

    char* response;
    char* request;
    char* requestName;
    char* fileName;
    char* fileType;

    int fileLength, fileStatus;

    // get file name
    request = buf;
    requestName = requestHandler(request);
    //printf("%s\n", requestName);

    fileName = fStatus(requestName);

    if (fileName == NULL) {
        fileName = "";
        fileStatus = 0;
    }
    else fileStatus = 1;

    char tmp[strlen(fileName) + 1]; 
    strcpy(tmp, fileName);
    //printf("%s\n", fileName);
        
    // file is in directory
    if (fileStatus == 1) {
        // get type of the file
        fileType = fType(fileName);

        // get size of the file
        FILE* html_file = fopen(fileName, "r");
        fseek(html_file, 0, SEEK_END); 
        fileLength = ftell(html_file);
        fclose(html_file);
    }
    
    // send http response
    response = responseHeader(fileStatus, fileType, fileLength);
    printf("%s\n", response);

    strcpy(buf, response);
    send(client_socket, buf, strlen(buf), 0);
    
    // send file requested 
    if(fileStatus == 1) {
        int fd;
        fd = open(tmp, O_RDONLY);
        int sent_bytes = 0;
        while (sent_bytes < fileLength) {
            int read_bytes = read(fd, buf, BUF_SIZE);
            if (read_bytes < 0) {
                return 1;
            }
            sent_bytes += send(client_socket, buf, read_bytes, 0);
        }
    }
    else { // send 404 error html file
        strcpy(buf, "<html><body> 404 error Not Found </body></html>");
        send(client_socket, buf, strlen(buf), 0);
    }

    sleep(1);

    close(client_socket);
    close(server_socket);

    return 0;
}

char* requestHandler(char* request) {
    char* fileName;
    char* delim;
    char* tmp;

    // get first line of request header
    delim = "\r\n";
    tmp = strtok(request, delim);

    // get name of file requested
    delim = " "; 
    fileName = strtok(tmp, delim); 
    fileName = strtok(NULL, delim); // skip method field

    if (strcmp(fileName, "/") == 0) {
        fileName = "index.html";
        return fileName;
    }

    // replace %20 with space
    delim = "%20";
    char* r; // right substring of %20
    char* l; // left substring of %20

    while(strstr(fileName, "%20") != NULL) {
        r = strstr(fileName, delim);

        size_t size = strlen(fileName) - strlen(r) + 1;

        l = malloc(sizeof(char) * size);
        strncpy(l, fileName, size - 1);

        r = r + 3;

        l[strlen(l)] = ' ';
        l[strlen(l) + 1] = '\0';

        // add right substring to left substring
        while(l) {
            if (strcmp(r, "\0") == 0) {
                l[strlen(l)] = '\0';
                break;
            }
            l[strlen(l)] = r[0];
            r++;
        }
        fileName = l;
    }

    // replace %25 with %
    delim = "%25";
    while(strstr(fileName, "%25") != NULL) {
        r = strstr(fileName, delim);

        size_t size = strlen(fileName) - strlen(r) + 1;

        l = malloc(sizeof(char) * size);
        strncpy(l, fileName, size - 1);

        r = r + 3;

        l[strlen(l)] = '%';
        l[strlen(l) + 1] = '\0';

        // add right substring to left substring
        while(l) {
            if (strcmp(r, "\0") == 0) {
                l[strlen(l)] = '\0';
                break;
            }
            l[strlen(l)] = r[0];
            r++;
        }
        fileName = l;
    }

    // return file name
    return (fileName + 1);
}

char* fType(const char* fn) {
    char* fileName = malloc(sizeof(char) * strlen(fn));
    strcpy(fileName, fn);

    char* type; 
    char* filetype = strrchr(fileName,'.');

    if(filetype != NULL) {
        if((strcmp(filetype,".htm"))==0 || (strcmp(filetype,".html"))==0)
            type="text/html";
        else if(strcmp(filetype,".txt")==0) 
            type="text/plain";
        else if((strcmp(filetype,".jpg"))==0 || (strcmp(filetype,".jpeg"))==0) 
            type="image/jpeg";
        else if(strcmp(filetype,".png")==0) 
            type="image/png";
    }
    else type = "application/octet-stream";
    return type;
}

char* fStatus(const char* fn) {
    char* fileName = malloc(sizeof(char) * strlen(fn));
    strcpy(fileName, fn);
    char* trueName = NULL;

    for(int i = 0; i < strlen(fileName); i++) // lowercase
        fileName[i] = tolower(fileName[i]);

    // traverse directory to find file
    DIR *dir;
    struct dirent *de;

    dir = opendir("."); 
    while(dir)
    {
        de = readdir(dir);
        if (!de) break; // end of directory

        char tmp[strlen(de->d_name)];
        strcpy(tmp, de->d_name);
        for(int i = 0; i < strlen(tmp); i++) // lowercase
            tmp[i] = tolower(tmp[i]);

        if(strcmp(tmp, fileName)==0) { // file found
            FILE* html_file = fopen(de->d_name, "r"); // try to open
            if (html_file == NULL) {
                return NULL;
            }
            trueName = de->d_name;
            fclose(html_file);
            break;
        }
    }
    closedir(dir);
    return trueName;
}

char* responseHeader(int fileStatus, char* fileType, int fileLength) {
    // convert int to string
    char buf[20];
    snprintf(buf, 20, "%d", fileLength);

    // construct header
    char* header;
    char re[BUF_SIZE] = "HTTP/1.0";
    if(fileStatus == 1) {
        // status field
        strcat(re," 200 OK\r\n");
        strcat(re,"Content-Type: ");
        strcat(re,fileType);
        strcat(re,"\r\n");
        strcat(re,"Content-Length: ");
        strcat(re,buf);
        strcat(re,"\r\n");
        strcat(re,"Connection: close\r\n\n");
    }
    else {
        strcat(re," 404 Not Found\r\n\n");
    }

    header = malloc(strlen(re)+1);
    strcpy(header,re);
    return header;
}