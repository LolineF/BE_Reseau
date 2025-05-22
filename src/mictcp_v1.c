#include <mictcp.h>
#include <api/mictcp_core.h>


// Variable globale
mic_tcp_sock my_socket;

/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    if (initialize_components(sm) < 0)
        return -1;

    my_socket.fd = 0;
    my_socket.state = IDLE;
    memset(&my_socket.local_addr, 0, sizeof(mic_tcp_sock_addr));
    memset(&my_socket.remote_addr, 0, sizeof(mic_tcp_sock_addr));

    set_loss_rate(0);
    return my_socket.fd;
}



/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    if (my_socket.state != IDLE)
        return -1;

    my_socket.local_addr = addr;
    my_socket.state = BOUND;
    return 0;
}



/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    mic_tcp_pdu pdu;
    mic_tcp_sock_addr expediteur;

    while (1) {
        pdu = IP_recv(&expediteur);

        if (pdu.header.syn == 1) {
            if (addr != NULL)
                *addr = expediteur;

            my_socket.remote_addr = expediteur;
            my_socket.state = CONNECTED;
            return 0;
        }
    }
}



/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
 int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    mic_tcp_pdu pdu = {0};
    pdu.header.syn = 1;

    if (IP_send(pdu, addr) < 0)
        return -1;

    my_socket.remote_addr = addr;
    my_socket.state = CONNECTED;
    return 0;
}



/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send(int mic_sock, char* mesg, int mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    if (my_socket.state != CONNECTED) {
        fprintf(stderr, "[MIC-TCP][ERREUR] Socket non connectée\n");
        return -1;
    }

    mic_tcp_pdu pdu;
    memset(&pdu, 0, sizeof(pdu));

    pdu.header.syn = 0;
    pdu.header.ack = 0;
    pdu.header.fin = 0;

    pdu.payload.data = mesg;
    pdu.payload.size = mesg_size;

    if (IP_send(pdu, my_socket.remote_addr) < 0) {
        fprintf(stderr, "[MIC-TCP][ERREUR] Échec de l'envoi du PDU\n");
        return -1;
    }

    return mesg_size;
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

    // Lecture bloquante depuis le buffer de réception applicatif
    payload = app_buffer_get();

    if (payload.size > max_mesg_size) {
        fprintf(stderr, "[MIC-TCP][WARN] Message tronqué : buffer trop petit\n");
        payload.size = max_mesg_size;
    }

    memcpy(mesg, payload.data, payload.size);
    return payload.size;
}



/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction : %s\n", __FUNCTION__);

    if (my_socket.state == IDLE) {
        fprintf(stderr, "[MIC-TCP][ERREUR] Socket déjà fermée\n");
        return -1;
    }

    // Mise à jour de l’état
    my_socket.state = IDLE;
    memset(&my_socket.local_addr, 0, sizeof(mic_tcp_sock_addr));
    memset(&my_socket.remote_addr, 0, sizeof(mic_tcp_sock_addr));

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

    // Pas de traitement de header pour V1
    if (pdu.payload.size > 0) {
        app_buffer_put(pdu.payload);
    }
}
