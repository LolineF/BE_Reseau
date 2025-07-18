#include <mictcp.h>
#include <api/mictcp_core.h>

typedef struct { 
    int *ack_array;
    int taille_max;
    int position;
} FenetreGlissante;



// Variable globale
struct mic_tcp_sock my_sockets[10];
int num_sock = 0;
int ports[10];
int seq[10];
int seq_num = 0;
float seuil = 0.6;
int taille_fenetre = 5;
float pourcentage_perte;
FenetreGlissante fenetre;




// Initialisation de le fenetre
void init_fenetre(FenetreGlissante *f, int taille_max) {
    f->ack_array = malloc(sizeof(int) * taille_max);
    f->taille_max = taille_max;
    f->position = 0;

    // Pour l'instant on considère que tout est ok
    for (int i = 0; i < taille_max; i++){
        f->ack_array[i] = 1;
    }  
}



// Mise à jour de la fenentre
float maj_fenetre(FenetreGlissante *f, int ack_recu) {
    f->ack_array[f->position] = ack_recu;
    f->position = (f->position + 1) % f->taille_max;
    
    int pertes = 0;
    for (int i = 0; i < f->taille_max; i++) {
        if (f->ack_array[i] == 0) {
            pertes ++;
        }
    }

    float taux = (float) pertes / f->taille_max;
    return taux;    
}


// liberation place
void liberer_fenetre(FenetreGlissante *f){
    free(f->ack_array);
} 


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
    set_loss_rate(10);

    my_sockets[num_sock].fd = num_sock;
    my_sockets[num_sock].state = IDLE;

    // Allocation dynamique de l'adresse IP
    my_sockets[num_sock].local_addr.ip_addr.addr = strdup("127.0.0.1");
    my_sockets[num_sock].local_addr.ip_addr.addr_size = strlen(my_sockets[num_sock].local_addr.ip_addr.addr) + 1;
    ports[num_sock] = 9000;
    my_sockets[num_sock].local_addr.port = ports[num_sock];

    seq[num_sock] = 0;
    
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

    if (my_sockets[socket].state != IDLE) {
        return -1;
    } 

    my_sockets[socket].local_addr.ip_addr = addr.ip_addr;
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

    mic_tcp_pdu pdu_syn;

    // Allouer la mémoire pour recevoir le payload
    pdu_syn.payload.data = malloc(1500);  // max taille pour UDP
    pdu_syn.payload.size = 0;             // elle sera définie par IP_recv

    struct mic_tcp_ip_addr *local_ip = &my_sockets[socket].local_addr.ip_addr;
    struct mic_tcp_ip_addr *remote_ip = &my_sockets[socket].remote_addr.ip_addr;

    // Allouer la mémoire pour l'adresse IP de l'expéditeur
    remote_ip->addr = malloc(100);
    remote_ip->addr_size = 100;

    int result = IP_recv(&pdu_syn, local_ip, remote_ip, 10000);

    if (result >= 0 && pdu_syn.header.syn == 1)
    {
        printf("[MIC-TCP] PDU SYN reçu dans accept \n");

        // Enregistrer l'adresse et le port du client
        my_sockets[socket].remote_addr.ip_addr.addr = strdup(remote_ip->addr);
        my_sockets[socket].remote_addr.ip_addr.addr_size = strlen(remote_ip->addr) + 1;
        my_sockets[socket].remote_addr.port = pdu_syn.header.source_port;

        // Retour d'adresse au niveau applicatif
        if (addr != NULL) {
            addr->ip_addr = my_sockets[socket].remote_addr.ip_addr;
            addr->port = my_sockets[socket].remote_addr.port;
        }

        // SYN_ACK
        mic_tcp_pdu pdu_syn_ack;
        pdu_syn_ack.header.source_port = my_sockets[socket].local_addr.port;
        pdu_syn_ack.header.dest_port = my_sockets[socket].remote_addr.port;
        pdu_syn_ack.header.seq_num = 0;
        pdu_syn_ack.header.ack_num = pdu_syn.header.seq_num + 1;
        pdu_syn_ack.header.syn = 1;
        pdu_syn_ack.header.ack = 1;
        pdu_syn_ack.header.fin = 0;
        pdu_syn_ack.payload.data = NULL;
        pdu_syn_ack.payload.size = 0;

        printf("[MIC-TCP] SYN recu\n");
        IP_send(pdu_syn_ack, my_sockets[socket].remote_addr.ip_addr );
        printf("[MIC-TCP] SYN-ACK envoyé\n");

        // Passage à l'état SYN_RECEIVED 
        my_sockets[socket].state = SYN_RECEIVED;

        // Recevoir ACK
        mic_tcp_pdu pdu_ack;
        pdu_ack.payload.data = malloc(1500);
        pdu_ack.payload.size = 0;
        memset(pdu_ack.payload.data, 0, 1500);

        int ack_result = IP_recv(&pdu_ack, local_ip, remote_ip, 10000);

        if (ack_result >= 0 && pdu_ack.header.ack == 1 && pdu_ack.header.syn == 0)
        {
            printf("[MIC-TCP] ACK final reçu\n");
            my_sockets[socket].state = ESTABLISHED;
        }

        free(pdu_ack.payload.data); 
    }
    else {
        printf("[MIC-TCP] Aucun SYN reçu ou erreur\n");
    }

    // Libérer le buffer de réception
    free(pdu_syn.payload.data);

    return 0;
}




/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
 int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: %s\n", __FUNCTION__);

    mic_tcp_pdu pdu_syn;

       // Associer l'adresse du serveur (distant) au socket
    my_sockets[socket].remote_addr.ip_addr = addr.ip_addr;
    my_sockets[socket].remote_addr.port = addr.port;

    // SYN
    pdu_syn.header.source_port = my_sockets[socket].local_addr.port;
    pdu_syn.header.dest_port = my_sockets[socket].remote_addr.port;

    pdu_syn.header.seq_num = 0;
    pdu_syn.header.ack_num = 0;
    pdu_syn.header.syn = 1;
    pdu_syn.header.ack = 0;
    pdu_syn.header.fin = 0;

    pdu_syn.payload.data = NULL;
    pdu_syn.payload.size = 0;



    IP_send(pdu_syn, my_sockets[socket].remote_addr.ip_addr);
    printf("[MIC-TCP] Envoi SYN\n");
    
    // Attente du SYN-ACK
    mic_tcp_pdu pdu_syn_ack;
    pdu_syn_ack.payload.data = malloc(1500);
    pdu_syn_ack.payload.size = 0;

    struct mic_tcp_ip_addr *local_ip = &my_sockets[socket].local_addr.ip_addr;
    struct mic_tcp_ip_addr *remote_ip = &my_sockets[socket].remote_addr.ip_addr;

    int result = IP_recv(&pdu_syn_ack, local_ip, remote_ip, 10000);

    if (result >= 0 && pdu_syn_ack.header.syn == 1 && pdu_syn_ack.header.ack == 1)
    {
        printf("[MIC-TCP] SYN-ACK reçu\n");

        // Vérification du ack_num
        if (pdu_syn_ack.header.ack_num != pdu_syn.header.seq_num + 1) {
            printf("[MIC-TCP][ERREUR] Mauvais ack_num dans SYN-ACK\n");
            free(pdu_syn_ack.payload.data);
            return -1;
        }
        // Envoi ACK
        mic_tcp_pdu pdu_ack;
        pdu_ack.header.source_port = my_sockets[socket].local_addr.port;
        pdu_ack.header.dest_port = my_sockets[socket].remote_addr.port;
        pdu_ack.header.seq_num = 1;
        pdu_ack.header.ack_num = pdu_syn_ack.header.seq_num + 1;
        pdu_ack.header.syn = 0;
        pdu_ack.header.ack = 1;
        pdu_ack.header.fin = 0;
        pdu_ack.payload.data = NULL;
        pdu_ack.payload.size = 0;

        IP_send(pdu_ack, my_sockets[socket].remote_addr.ip_addr);
        printf("[MIC-TCP] ACK envoyé \n");
    } 

    init_fenetre(&fenetre, taille_fenetre);
    


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
    pdu.header.seq_num = seq[mic_sock];
    pdu.header.ack_num = 0;
    pdu.header.syn = 0;
    pdu.header.ack = 0;
    pdu.header.fin = 0;

    // Payload
    pdu.payload.data = mesg;
    pdu.payload.size = mesg_size;

    int ack_recu = 0;

    struct mic_tcp_ip_addr *local_ip = &my_sockets[mic_sock].local_addr.ip_addr;
    struct mic_tcp_ip_addr *remote_ip = &my_sockets[mic_sock].remote_addr.ip_addr;

    while (!ack_recu) {
        IP_send(pdu, my_sockets[mic_sock].remote_addr.ip_addr);

        mic_tcp_pdu ack_pdu;
        ack_pdu.header.ack_num = -1;

        int result = IP_recv(&ack_pdu, local_ip, remote_ip, 1000);

        if(result != -1){
            if(ack_pdu.header.ack_num == seq[mic_sock]+1) {
                ack_recu = 1;
                maj_fenetre(&fenetre, 1);
                printf("[MIC-TCP] ACK reçu\n");
                seq[mic_sock] ++;
            } else {
                maj_fenetre(&fenetre, 0);
                printf("[MIC-TCP] ACK reçu mais mauvais num de seq recu %d, (voulu %d)\n", ack_pdu.header.ack_num,seq[mic_sock]+1);
            } 
        
        } else {
            pourcentage_perte = maj_fenetre(&fenetre, 0);

            if (pourcentage_perte > seuil){
                ack_recu = 1;
                printf("[MIC-TCP] Expiration timer\n");
                printf("[MIC-TCP] Trop de perte abandon du pdu\n");
            } else {
                printf("[MIC-TCP] Expiration timer et renvoie\n");
            } 
        } 
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
    int result = -1;

    payload.data = mesg;
    payload.size = max_mesg_size;

    result = app_buffer_get(payload);

    printf("[MIC-TCP] Reçu :  %s\n", payload.data);

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

    liberer_fenetre(&fenetre);

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

    // Gère le ACK final du handshake
    if (pdu.payload.size == 0 && pdu.header.syn == 0 && pdu.header.fin == 0 && pdu.header.ack == 1) {
        printf("[MIC-TCP] ACK pur reçu ignoré par process_received_PDU\n");

        // Cherche le socket en SYN_RECEIVED
        for (int i = 0; i < num_sock; i++) {
            if (my_sockets[i].state == SYN_RECEIVED &&
                my_sockets[i].remote_addr.port == pdu.header.source_port &&
                strcmp(my_sockets[i].remote_addr.ip_addr.addr, remote_addr.addr) == 0) {
                my_sockets[i].state = ESTABLISHED;
                printf("[MIC-TCP] Passage à l'état ESTABLISHED\n");
                return;
            }
        }

        // Sinon : ACK inconnu, on l'ignore
        printf("[MIC-TCP] ACK reçu mais pas en SYN_RECEIVED, ignoré\n");
        return;
    }

    // Traitement des données normales
    if (pdu.header.seq_num != seq[num_sock]){
        printf("[MIC-TCP] Paquet hors séquence (attendu=%d, reçu=%d), ignoré\n", seq[num_sock], pdu.header.seq_num);
    } else {
        app_buffer_put(pdu.payload);
        seq[num_sock]++;
    }

    // Envoi de l’ACK
    mic_tcp_pdu ack_pdu;
    ack_pdu.header.seq_num = 0;
    ack_pdu.header.ack_num = seq[num_sock];
    ack_pdu.header.dest_port = pdu.header.source_port;
    ack_pdu.header.source_port = pdu.header.dest_port;
    ack_pdu.header.syn = 0;
    ack_pdu.header.ack = 1;
    ack_pdu.header.fin = 0;
    ack_pdu.payload.data = NULL;
    ack_pdu.payload.size = 0;

    IP_send(ack_pdu, remote_addr);
}

