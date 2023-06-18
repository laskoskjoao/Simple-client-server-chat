#include <iostream>
#include <winsock2.h>
#include <string>
#include <thread>
#include <vector>

#pragma comment(lib, "ws2_32.lib") // Link com a biblioteca do Winsock

#define BUFFER_SIZE 1024

void chatRecv(SOCKET* clientSocket);
void chatSend(std::vector<SOCKET*>* clientSocket);

int main() {
    WSADATA wsaData;
    SOCKET serverSocket;
    std::vector<SOCKET*> clientSocket;
    sockaddr_in serverAddress, clientAddress;
    int clientAddressLength;

    /*Inicialização do Winsock*/
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {    //MAKEWORD(2,2) indica a versão do winsocks (no caso é a 2.2)
        //&wsaData é um ponteiro de WSADATA, que contém detalhes da implementação do socket
        std::cout << "Erro ao inicializar o Winsock." << std::endl;
        return 1;
    }

    /*Criação do socket do servidor*/
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);     //AF_INET é usado para TCP ou UDP.
    //SOCK_STREAM (fluxo de dados) -> TCP e SOCK_DGRAM (Datagrama) -> UDP. Em geral, esse parametro define o tipo de socket (fluxo, datagrama, bruto)
    //Terceiro parâmetro especifica o protocolo a ser utilizado (0 -> SO decide com base no tipo (2° parametro); IPPROTO_TCP; IPPROTO_UDP)
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Erro ao criar o socket do servidor." << std::endl;
        return 1;
    }

    /*Configuração do endereço do servidor*/
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    /*Vinculação do socket do servidor ao endereço*/
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {  //RELACIONA sockaddr_in (configs) com o socket. A função bind(...) atribui a um socket um endereço IP e uma porta que foram anteriormente definidas numa estrutura sockaddr_in
        std::cout << "Erro ao vincular o socket do servidor ao endereço." << std::endl;
        closesocket(serverSocket);
        return 1;
    }

    /*Espera por conexões de clientes*/
    if (listen(serverSocket, 1) == SOCKET_ERROR) {  //A função listen(...) coloca o socket (já configurado) em estado de escuta
        //primeiro parametro de listen -> server socket
        //segundo parametro de liste -> numeros de sockets que poderao se conectar ao sistema (sevidor).
        std::cout << "Erro ao aguardar conexões de clientes." << std::endl;
        closesocket(serverSocket);
        return 1;
    }

    std::cout << "Servidor aguardando conexões..." << std::endl;


    /* Enviar dados para os clientes conectados*/
    std::thread tSend(chatSend, &clientSocket);
    tSend.detach();

    while (true) {
        /*Aceita a conexão do cliente*/
        //apos uma conexao de cliente (accept é bloqueante)...
        clientAddressLength = sizeof(clientAddress);
        SOCKET* newClient = new SOCKET();
        *newClient = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        clientSocket.push_back(newClient);
        //clientSocket -> apos accept, contém as informacoes de outro computador (socket remoto)
        //primeiro parametro de accept  -> socket que está em estado de escuta
        //segundo parametro de accept   -> ponteiro para uma estrutura addr_in, contendo as informações do computador remoto.
        //terceiro parametro de accept  -> ponteiro do tamanho da estrutura addr_in presente no segundo parâmetro dessa função
        if (*newClient == INVALID_SOCKET) {
            std::cout << "Erro ao aceitar a conexão do cliente." << std::endl;
            closesocket(serverSocket);
            return 1;
        }

        std::cout << "Cliente conectado." << std::endl;

        /*Envio de dados para o cliente*/
        const char* message = "Conexao Estabelecida!";
        send(*newClient, message, strlen(message), 0);

        /* Receber dados do cliente */
        std::thread tRec(chatRecv, newClient);
        tRec.detach();
    }

    
    /*Encerramento das conexões e liberação dos recursos*/
    for (int i = 0; i < clientSocket.size(); i++) {
        closesocket(*clientSocket[i]);  //Encera o socket
        delete(clientSocket[i]);
    }
    clientSocket.clear();

    closesocket(serverSocket);  //Encera o socket
    WSACleanup();               //Finaliza o uso do winsocks (desaloca recursos alocados pela Winsock) - Boa prática

    return 0;
}


void chatRecv(SOCKET* clientSocket) {
    while (true) {
        char buffer[BUFFER_SIZE];
        int bytesRead = recv(*clientSocket, buffer, BUFFER_SIZE, 0);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << "Cliente: " << buffer << std::endl;
        }
        else if (bytesRead == 0) {
            std::cout << "Conexão encerrada pelo cliente." << std::endl;
            break;
        }
        else {
            std::cerr << "Erro ao receber dados do cliente." << std::endl;
            break;
        }
    }
}

void chatSend(std::vector<SOCKET*>* clientSocket) {
    while (true) {
        std::string message;
        std::getline(std::cin, message);
        const char* message_char = message.c_str();
        
        for (int i = 0; i < (*clientSocket).size(); i++) {
            /* Enviar dados para o servidor */
            if (send(*(*clientSocket)[i], message_char, strlen(message_char), 0) < 0) { //A função send(…) é utilizada para enviar os dados de um computador ao outro. Pode ser usada pelo servidor ou cliente.
                //primeiro parametro da funcao send -> socket que contém as informações do computador remoto a que se deseja enviar os dados.
                //segundo parametro da funcao send  -> ponteiro do tipo char (ou seja, qualquer dado que será enviado deve ser anteriormente convertido em chars) cujo conteúdo será o que será mandado ao outro computador.
                //terceiro parametro da funcao send -> inteiro com o tamanho do buffer a ser enviado.
                std::cout << "Erro ao enviar dados para o cliente." << std::endl;
            }
        }
    }
}