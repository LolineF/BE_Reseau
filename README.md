# BE RESEAU
## TPs BE Reseau - 3 MIC

Les détails du sujet du BE est accessible depuis le cours "Programmation Système et Réseau" sur moodle.

## 👥 Auteurs

Ce projet a été réalisé dans le cadre du BE Réseaux à l’INSA Toulouse, par :

- **Loline Fornalik** – LolineF
- **Annalisa Josse** – AnnaLisa18

(Tous les commits proviennent d'un même compte car nous avons toujours coder ensemble depuis le même compte)
Encadré par l’équipe pédagogique du département Informatique et Réseaux.

## 📌 Avancement par version

### ✅ MICTCP-v1 — Transfert sans garantie de fiabilité
- Envoi simple de données sans retransmission
- Pas de gestion d’ACK, ni de vérification d’arrivée des paquets

### ✅ MICTCP-v2 — Fiabilité complète (Stop-and-Wait)
- Réémission du PDU si aucun ACK reçu
- Attente active d’un accusé de réception avant d’envoyer le suivant
- Utilisation d’un numéro de séquence attendu

### ✅ MICTCP-v3 — Fiabilité partielle avec seuil + fenêtre glissante
- Introduction d’un seuil statique (ex. : seuil = 0.6)
- Mécanisme de fenêtre glissante pour suivre les derniers ACK reçus
- Si le taux de pertes observé est inférieure au seuil → retransmission
- Sinon → pas de réémission pour améliorer les performances
- **Remarque :** le protocole fonctionne correctement avec tsock_texte, mais tsock_video pose encore problème et ne marche pas à chaque fois.

### 🟢 MICTCP-v4.1 — Ajout du mécanisme de connexion
- Implémentation de mic_tcp_connect / mic_tcp_accept avec échanges SYN / SYN-ACK / ACK  


## Contenu du dépôt « template » fourni
Ce dépôt inclut le code source initial fourni pour démarrer le BE. Plus précisément : 
  - README.md (ce fichier) qui notamment décrit la préparation de l’environnement de travail et le suivi des versions de votre travail; 
  - tsock_texte et tsock_video : lanceurs pour les applications de test fournies. 
  - dossier include : contenant les définitions des éléments fournis que vous aurez à manipuler dans le cadre du BE.
  - dossier src : contenant l'implantation des éléments fournis et à compléter dans le cadre du BE.
  - src/mictcp.c : fichier au sein duquel vous serez amenés à faire l'ensemble de vos contributions (hors bonus éventuels). 


## Création du dépôt mictcp 

1. Création d’un compte git étudiant : Si vous ne disposez pas d’un compte git, connectez vous sur http://github.com/ et créez un compte par binôme. 

2. Afin d’être capable de mettre à jour le code que vous aurez produit sur le dépôt central Github, il vous faudra créer un jeton d’accès qui jouera le rôle de mot de passe. Veuillez le sauvegarder, car il vous le sera demandé lors de l'accès au dépôt central. Pour ce faire, veuillez suivre les étapes décrites : https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token

3. Création d’un dépôt Etudiant sur GitHub pour le BE Reseau
  
   Créer une copie du dépôt template enseignant : https://github.com/rezo-insat/mictcp, en vous y rendant et en cliquant sur le bouton « use this template » situé dans le coin en haut, plutôt à droite de la page. Il est demandé de le choisir comme dépôt privé. Il est impératif pour les corrections que vous rajoutiez le compte : rezo-insat comme collaborateur afin de permettre à vos enseignants d'accéder à votre dépôt. Pour ce faire, sélectionner le bouton "settings" puis "collaborators" et rajouter comme utilisateur : rezo-insat. La marche à suivre est décrite ci-après : https://docs.github.com/en/organizations/managing-access-to-your-organizations-repositories/adding-outside-collaborators-to-repositories-in-your-organization


4. Créer un clone local de votre dépôt Github, i.e. une copie locale du dépôt sur votre compte insa. 
  
    cliquer sur le bouton « code » de votre dépôt, copier l’URL qui permet de l’identifier. 
	Ouvrir un terminal de votre machine. En vous plaçant dans le répertoire de travail de votre choix, taper depuis le terminal :

        git clone <url de votre dépôt>

    Vous avez désormais une copie locale de votre dépôt, que vous pouvez mettre à jour et modifier à volonté au gré de votre avancement sur les TPs. 

5. Afin de nous permettre d’avoir accès à votre dépôt, merci de bien vouloir renseigner l'URL de votre dépôt sur le fichier accessible depuis le lien "fichier URLs dépôts étudiants" se trouvant sur moodle (au niveau de la section: BE Reseau).

## Compilation du protocole mictcp et lancement des applications de test fournies

Pour compiler mictcp et générer les exécutables des applications de test depuis le code source fourni, taper :

    make

Deux applicatoins de test sont fournies, tsock_texte et tsock_video, elles peuvent être lancées soit en mode puits, soit en mode source selon la syntaxe suivante:

    Usage: ./tsock_texte [-p|-s destination] port
    Usage: ./tsock_video [[-p|-s] [-t (tcp|mictcp)]

Seul tsock_video permet d'utiliser, au choix, votre protocole mictcp ou une émulation du comportement de tcp sur un réseau avec pertes.

## Suivi de versions de votre travail

Vous pouvez travailler comme vous le souhaitez sur le contenu du répertoire local. Vous pouvez mettre à jour les fichiers existants, rajouter d’autres ainsi que des dossiers et en retirer certains à votre guise. 

Pour répercuter les changements que vous faites sur votre répertoire de travail local sur le dépôt central GitHub, sur votre terminal, taper :
 
    git add .
    git commit -m «un message décrivant la mise à jour»
    git push

- Marquage des versions successives de votre travail sur mictcp 
 
Lorsque vous le souhaitez, git permet d'associer une étiquette à un état précis de votre dépôt par l'intermédiaires de tags. Il vous est par exemple possible d'utiliser ce mécanisme pour marquer (et par conséquence pouvoir retrouver) l'état de votre dépôt pour chacune des versions successives de votre travail sur mictcp.

Pour Créer un tag « v1 » et l'associer à l'état courrant de votre dépôt, vous taperez la commande suivante sur votre terminal :

    git tag v1

Pour lister les tags existants au sein de votre dépôt

    git tag -l

Pour transférer les tags de votre dépôt local vers le dépôt central sur github:

    git push origin --tags


Ceci permettra à votre enseignant de positionner le dépôt dans l'état ou il était au moment du marquage avec chacun des tags que vous définissez. 
   
## Suivi de votre avancement 

Veuillez utiliser, à minima, un tag pour chacune des versions successives de mictcp qui sont définies au sein du sujet du BE disponible sous moodle.


## Liens utiles 

Aide pour la création d’un dépôt depuis un template : https://docs.github.com/en/repositories/creating-and-managing-repositories/creating-a-repository-from-a-template

Manuel d'utilisation de git: https://git-scm.com/docs
