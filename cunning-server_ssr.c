// TO-DO: custom alloc so resp isn't limited by size and we not heavy on the stack
// also some function to append quotes so we can work with web files easy
// then probably custom file handling
// ... eish!

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>

#define PORT 6969
#define BUFFER_SIZE 1024

size_t weasel_len(char *string) 
{
    char *p = string;
    size_t len = 0;

    while (*p) {
        len++;
        p++;
    }

    return len;
}

size_t cunning_weasel_len(char *str) 
{
    char *start = NULL;
    char *end = NULL;
    size_t len = 0;
    size_t cap = 10000; // char = 1 bit. 1 kb = 1024 bytes. 10000/1024 = 9.77 kb

    if (start && str >= start && str <= end) 
    {
        len = end - str;
        return len;
    }

    len = weasel_len(str);

    if (len > cap) 
    {
        start = str;
        end = str + len;
    }

    return len;
}

int main() 
{
    char buffer[BUFFER_SIZE];
    char resp[] = "HTTP/1.0 200 OK\r\n"
	"Server: webserver-c\r\n"
	"Content-type: text/html\r\n\r\n"
	"<html>"
	"<body style=\"background-color: black; color: white;\">"
	"welcome back commander weasel"
	"</body>"
	"</html>\r\n";

    // create a socket
    int32_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) 
    {
        perror("webserver (socket)");
        return 1;
    }
    printf("socket created successfully\n");

    // create the address to bind the socket to
    struct sockaddr_in host_addr;
    socklen_t host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // create client address
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    // bind the socket to the address
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) 
    {
        perror("webserver (bind)");
        return 1;
    }
    printf("socket successfully bound to address\n");

    // listen for incoming connections
    if (listen(sockfd, SOMAXCONN) != 0) 
    {
        perror("webserver (listen)");
        return 1;
    }
    printf("server listening for connections\n");

    for (;;)
    {
        // accept incoming connections
        int32_t newsockfd = accept(sockfd, (struct sockaddr *)&host_addr,
                                   &host_addrlen);
        if (newsockfd < 0) 
	{
            perror("error on webserver (accept)");
            continue;
        }
        printf("connection accepted\n");

        // get client address
        int32_t sockn = getsockname(newsockfd, (struct sockaddr *)&client_addr,
                                    &client_addrlen);
        if (sockn < 0) 
	{
            perror("error on webserver (getsockname)");
            continue;
        }

        // read from the socket
        int32_t valread = read(newsockfd, buffer, BUFFER_SIZE);
        if (valread < 0) 
	{
            perror("error on webserver (read)");
            continue;
        }

        // read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);
        printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port), method, version, uri);

        // Write to the socket
        int32_t valwrite = write(newsockfd, resp, weasel_len(resp));
        if (valwrite < 0) 
	{
            perror("error on webserver (write)");
            continue;
        }

        close(newsockfd);
    }

    return 0;
}
