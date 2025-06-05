#include <mictcp.h>
#include <api/mictcp_core.h>


// Variable globale
struct mic_tcp_sock my_sockets[10];
int num_sock = 0;
int ports[10];

/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    int result = initialize_components(sm);
    if (result < 0){
        return result;
    }
    set_loss_rate(0);

    my_sockets[num_sock].fd = num_sock;
    my_sockets[num_sock].state = IDLE;

    // Allocation dynamique de l'adresse IP
    my_sockets[num_sock].local_addr.ip_addr.addr = strdup("127.0.0.1");
    my_sockets[num_sock].local_addr.ip_addr.addr_size = strlen(my_sockets[num_sock].local_addr.ip_addr.addr) + 1;
    
    num_sock ++;

    return my_sockets[num_sock-1].fd;
}



/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    if (my_sockets[socket].state != IDLE)
        return -1;

    my_sockets[socket].local_addr.ip_addr.addr = strdup(addr.ip_addr.addr);
    my_sockets[socket].local_addr.ip_addr.addr_size = strlen(addr.ip_addr.addr) + 1;
    my_sockets[socket].local_addr.port = addr.port;

    ports[socket] = addr.port;

    return 0;
}



/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    my_sockets[socket].state = ESTABLISHED;

    return 0;
}



/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
 int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    // Associer l'adresse du serveur (distant) au socket
    my_sockets[socket].remote_addr = addr;

    // Mettre à jour l'état du socket si besoin
    my_sockets[socket].state = ESTABLISHED;
    
    return 0;
}



/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send(int mic_sock, char* mesg, int mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    if (my_sockets[mic_sock].state != ESTABLISHED) {
        fprintf(stderr, "[MIC-TCP][ERREUR] Socket non connectée\n");
        return -1;
    }

    mic_tcp_pdu pdu;
    
    // Ports
    pdu.header.source_port = my_sockets[mic_sock].local_addr.port;
    pdu.header.dest_port = my_sockets[mic_sock].remote_addr.port;

    // En tête
    pdu.header.seq_num = 0;
    pdu.header.ack_num = 0;
    pdu.header.syn = 0;
    pdu.header.ack = 0;
    pdu.header.fin = 0;

    // Payload
    pdu.payload.data = mesg;
    pdu.payload.size = mesg_size;

    int effective_send = IP_send(pdu,my_sockets[mic_sock].remote_addr.ip_addr);

    return effective_send;
}



/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    mic_tcp_payload payload;
    int result = -1;

    payload.data = mesg;
    payload.size = max_mesg_size;

    result = app_buffer_get(payload);

    return result;
}



/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction : %s\n", __FUNCTION__);

    my_sockets[socket].state = CLOSED;

    free(my_sockets[socket].local_addr.ip_addr.addr);
    free(my_sockets[socket].remote_addr.ip_addr.addr);

    printf("[MIC-TCP] Socket fermé correctement\n");
    return 0;
}



/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_ip_addr local_addr, mic_tcp_ip_addr remote_addr)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    mic_tcp_payload payload = pdu.payload;
    app_buffer_put(payload);
    
}
