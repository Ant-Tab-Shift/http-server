#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "mybool.h"
#include "parser.h"


const int CONNECTION_QUEUE_SIZE = 3;  // я не понимаю, как это работает или не работает.
int BUFFER_SIZE = 10240;  // Размер буфера по умолчаению = 10 килобайт.

// Функция настраивает серверный (принимающий подключения) TCP сокет, работающий с IPv4 адресами.
int setup_listener_tcp_socket(in_addr_t ip_address, int port) {
    // создаем прослушивающий TCP-сокет
    int listener_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener_socket_fd < 0) {
        perror("Failed creating listener TCP socket\n");
        exit(EXIT_FAILURE);
    }
    // устанавливаем параметр на уровне сокета для повторного использования локального адреса.
    // https://man7.org/linux/man-pages/man7/socket.7.html
    if (setsockopt(listener_socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){true}, sizeof(int)) < 0) {
        perror("Failed setting socket option for reusing current address\n");
    }
    // задаем параметры структуры, описывающей адрес сокета.
    struct sockaddr_in listener_socket_addr;
    socklen_t listener_socket_addr_len = sizeof(listener_socket_addr);
    memset(&listener_socket_addr, 0, listener_socket_addr_len);
    listener_socket_addr.sin_family = AF_INET;
    listener_socket_addr.sin_addr.s_addr = ip_address;
    listener_socket_addr.sin_port = htons(port);
    // связываем дескриптор сокета с локальным адресом.
    if (bind(listener_socket_fd, (struct sockaddr*)&listener_socket_addr, listener_socket_addr_len) < 0) {
        perror("Failed binding listener TCP socket with local address\n");
        exit(EXIT_FAILURE);
    }
    // готовимся принимать в очередь запросы по дескриптору сокета на соединение.
    if (listen(listener_socket_fd, CONNECTION_QUEUE_SIZE) < 0) {
        perror("Failed preparing for listening on TCP socket\n");
        exit(EXIT_FAILURE);
    }

    return listener_socket_fd;
}

//
void build_http_response(char* request, char* response, size_t* response_len) {
    char* file_name;
    char* file_ext;
    char* file_params;
    //
    if (strcmp(request, "") == 0) {
        file_name = "index.html";
        file_ext = "html";
    } else {
        file_name = extract_filename(request);
        file_ext = extract_extension(file_name);
        file_params = extract_params(request);
    }
    //
    int file_fd = open(file_name, O_RDONLY);
    if (file_fd == -1) {
        snprintf(response, BUFFER_SIZE,
                 "HTTP/1.1 404 Not Found\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "404 Not Found");
        *response_len = strlen(response);
        return;
    }
    const char *mime_type = get_mime_type(file_ext);
    char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(header, BUFFER_SIZE,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "\r\n",
             mime_type);
    ///////////////////////////////////////////////////////////
    struct stat file_stat;
    fstat(file_fd, &file_stat);
    off_t file_size = file_stat.st_size;
    //
    *response_len = 0;
    memcpy(response, header, strlen(header));
    *response_len += strlen(header);
    //
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, 
                            response + *response_len, 
                            BUFFER_SIZE - *response_len)) > 0) {
        *response_len += bytes_read;
    }
    free(header);
    close(file_fd);
}

// 
void* handle_client(void *arg) {
    int client_socket_fd = *(int*)arg;
    char* buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
    // 
    ssize_t bytes_received = recv(client_socket_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        // 
        char* client_request = get_client_request(buffer);
        // 
        char* client_response = (char*)malloc(2*BUFFER_SIZE * sizeof(char));
        size_t response_len;
        build_http_response(client_request, client_response, &response_len);
        // 
        send(client_socket_fd, client_response, response_len, 0);

        free(client_response);
    }

    close(client_socket_fd);
    free(arg);
    free(buffer);

    return NULL;
}

// Бесконечный цикл для соединения с клиенатми и обработки их запросов.
// Функция получает дескриптор клиентского сокета и создает отдельный поток для работы с запросом.
void loop_handle_client_requests(int server_fd) {
    int* client_socket_fd;
    while (true) {
        // готовимся ппринимать соединение с клиентом, инициализируем структуры.
        client_socket_fd = (int*)malloc(sizeof(int));
        struct sockaddr_in client_socket_addr;
        socklen_t client_socket_addr_len = sizeof(client_socket_addr);
        memset(&client_socket_addr, 0, client_socket_addr_len);
        // создаем сокет для соединения с клиентом, если сокет помечен неблокирующимся, то возможна ошибка.
        *client_socket_fd = accept(server_fd, (struct sockaddr*)&client_socket_addr, &client_socket_addr_len);
        if (client_socket_fd < 0) {
            perror("Accepting client connection failed\n");
            continue;
        }
        // создаем поток для обработки запроса; поток помечается detatched - вернет ресурсы, завершившись.
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void*)client_socket_fd);
        pthread_detach(thread_id);
    }

    close(server_fd);
}


int main(int argc, char** argv) {
    if (argc < 2) {
        perror("The port for accepting the connection is not specified\n");
        exit(EXIT_FAILURE);
    } else if (argc >= 3) {
        BUFFER_SIZE = atoi(argv[2]);
    }
    int port = atoi(argv[1]);

    int server_fd = setup_listener_tcp_socket(INADDR_ANY, port);
    printf("Server is listening port %d from all the interfaces\n", port);

    loop_handle_client_requests(server_fd);

    return 0;
}
