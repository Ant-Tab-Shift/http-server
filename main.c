#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

#include "mybool.h"


#define CONNECTION_QUEUE_SIZE 3


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
    listener_socket_addr.sin_family = AF_INET;
    listener_socket_addr.sin_addr.s_addr = ip_address;
    listener_socket_addr.sin_port = htons(port);
    // связываем дескриптор сокета с локальным адресом.
    if (bind(listener_socket_fd, &listener_socket_addr, sizeof(listener_socket_addr)) < 0) {
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


int main(int argc, char** argv) {

    return 0;
}
