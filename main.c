#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "mybool.h"

#define CONNECTION_QUEUE_SIZE 3  // я не понимаю, как это работает или не работает.


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
    if (bind(listener_socket_fd, &listener_socket_addr, listener_socket_addr_len) < 0) {
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

// Бесконечный цикл для соединения с клиенатми и обработки их запросов.
// Функция получает дескриптор клиентского сокета и создает отдельный поток для работы с запросом.
void loop_handle_client_requests(int server_fd) {
    int* client_socket_fd; // ЕСЛИ МНОГОПОТОК НЕ РАБОТАЕТ, ТО ЭТУ ПЕРЕМЕННУЮ МОЖНО СОЗДАВАТЬ ВНУТРИ ЦИКЛА.
    while (true) {
        // готовимся ппринимать соединение с клиентом, инициализируем структуры.
        int client_socket_fd = malloc(sizeof(int));
        struct sockaddr_in client_socket_addr;
        socklen_t client_socket_addr_len = sizeof(client_socket_addr);
        memset(&client_socket_addr, 0, client_socket_addr_len);
        // 
        client_socket_fd = accept(server_fd, &client_socket_addr, &client_socket_addr_len);
        if (client_socket_fd < 0) {
            perror("Accepting client connection failed\n");
            continue;
        }
        // КАК РАБОТАЕТ `pthread_detach()`?
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, client_socket_fd);
        pthread_detach(thread_id);

    }

    close(server_fd);
}


void *handle_client(void *arg) {

}


int main(int argc, char** argv) {
    if (argc < 2) {
        perror("The port for accepting the connection is not specified\n");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    int server_fd = setup_listener_tcp_socket(INADDR_ANY, port);
    printf("Server is listening port %d from all the interfaces\n", port);

    loop_handle_client_requests(server_fd);

    return 0;
}
