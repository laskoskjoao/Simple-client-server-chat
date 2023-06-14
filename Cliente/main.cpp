#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

void chatSend(SOCKET clientSocket);
void chatRecv(SOCKET clientSocket);

int main() {
    /* Inicializar o Winsock */
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Falha ao inicializar o Winsock." << std::endl;
        return 1;
    }

    /* Criar o socket */
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Erro ao criar o socket." << std::endl;
        WSACleanup();
        return 1;
    }

    /* Definir as informações de conexão do servidor */
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    //serverAddress.sin_addr.s_addr = inet_pton("127.0.0.1"); //localhost -> compilador falou que é obsoleto e pediu para substituir por inet_pton (abaixo codigo gerado pelo chat gpt)
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        std::cerr << "Endereço inválido\n";
        return 1;
    }

    /* Conectar ao servidor */
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { //connect é utilizado pelo lado do cliente para conectar com o servidor (conecta o socket cliente com o socket remoto servidor).
        //primeiro parametro de connect -> socket criado através da função socket.
        //segundo parametro de connect  -> sockaddr_in que deve ser criado e possuir o endereço IP e porta do servidor ao qual se quer conectar.
        //terceiro parametro de connect -> tamanho da estrutura presente no segundo parâmetro dessa função.
        std::cerr << "Falha ao conectar ao servidor." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    /* Receber dados do servidor */
    char buffer[BUFFER_SIZE];
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout << "Dados recebidos do servidor: " << buffer << std::endl;
    }
    else if (bytesRead == 0) {
        std::cout << "Conexão encerrada pelo servidor." << std::endl;
    }
    else {
        std::cerr << "Erro ao receber dados do servidor." << std::endl;
    }

    /* Receber dados do servidor */
    std::thread t(chatRecv, clientSocket);
    t.detach(); // Liberar a thread para executar em segundo plano

    /* Enviar dados para o servidor*/
    chatSend(clientSocket);
    
    /* Encerrar a conexão e limpar recursos */
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

void chatSend(SOCKET clientSocket) {
    while (1) {
        std::string message;
        std::getline(std::cin, message);
        const char* message_char = message.c_str();

        /* Enviar dados para o servidor */
        if (send(clientSocket, message_char, strlen(message_char), 0) < 0) { //A função send(…) é utilizada para enviar os dados de um computador ao outro. Pode ser usada pelo servidor ou cliente.
            //primeiro parametro da funcao send -> socket que contém as informações do computador remoto a que se deseja enviar os dados.
            //segundo parametro da funcao send  -> ponteiro do tipo char (ou seja, qualquer dado que será enviado deve ser anteriormente convertido em chars) cujo conteúdo será o que será mandado ao outro computador.
            //terceiro parametro da funcao send ->inteiro com o tamanho do buffer a ser enviado.
            std::cout << "Erro ao enviar dados para o servidor." << std::endl;
        }
    }
}

void chatRecv(SOCKET clientSocket) {
    while (1) {
        char buffer[BUFFER_SIZE];
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << "Servidor: " << buffer << std::endl;
        }
        else if (bytesRead == 0) {
            std::cout << "Conexão encerrada pelo servidor." << std::endl;
            break;
        }
        else {
            std::cerr << "Erro ao receber dados do servidor." << std::endl;
            break;
        }
    }
}