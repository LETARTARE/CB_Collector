A- OBJECTIF :

	L'objectif terminal est de traduire, dans la langue locale, les projets en
	développement sous 'C::B' directement,
	(actuellement uniquement les projets utilisant la bibliothèque 'wxWidgets').

	On utilise en final 'Poedit' (outil dans 'C::B'), auquel on fournit un
	fichier	'nom_de_projet.po' directement exploitable par 'Poedit'.

	L'extension binaire 'Collector' permet de générer ce fichier
	'nom_de_projet.po' à partir du projet actif dans 'C:B', en se fusionnant
	au contenu d'un éventuel 'nom_de_projet.po' précédent.

	Ce projet est soit unique, soit associé à d'autres projets coopératifs
	dans un espace de travail commun, il devient alors un projet pilote.

	===>
	LES PROJETS DOIVENT SE SITUER SUR LE MÊME DISQUE PHYSIQUE.
	EVITER LES FICHIERS AVEC DES FINS DE LIGNE PANACHEES ('CR', 'LF', 'CRLF')
	<===

	L'extension accepte deux modes de travail complémentaires:

	1- LISTER les chaînes à extraire pour vérifications de pertinence.
		1-1 sur un seul projet,
		1-2 sur un espace de travail, avec un projet pilote et des coopérateurs.
	2- EXTRAIRE les chaînes dans un fichier :
		- 'nom_de_projet.pot' pour le projet unique,
		- 'nom_de_projet_pilote_workspace.pot' pour l'espace de travail
		  (dupliqué en 'nom_de_projet_pilote_workspace.po' exploitable par 'Poedit').
		  
	** les deux modes peuvent être enchainés par
		'LISTER puis EXTRAIRE'

	Il prend en compte les fichiers éligibles suivants :
		'*.h', '*.cpp', '*.script', '*.xml', '*.xrc', '*.wxs',
		les deux derniers types génèrent des fichiers temporaires
			'*_xrc.str???, *_wxs.str???' (??? = numéro)	à l'aide de
			l'exécutable 'wxrc' (wxWidgets-3.x.y), qui sont ensuite supprimés,
		* les fichiers '*.xml' utilisent les règles de 'its' depuis les fichiers '*.its'
		  en l'occurence 'codeblocks.its' et 'codeblocks.its' voir : 
		  - 'https://www.w3.org/TR/its/'
		  - 'https://www.gnu.org/software/gettext/manual/html_node/Preparing-ITS-Rules.html'

	Il prend en compte le préfixe d'extraction :
		1- pour le listage 	: '_'
		2- pour l'extraction 	: '_'

	Pour cela, avec le projet analysé, cette extension  :

		1- reconnait si c'est un projet utilisant 'wxWidgets' ('Wx'),
		2- génére éventuellement des fichiers temporaires '*_xrc.str???',
		3- détecte automatiquement les fichiers éligibles à l'extraction,
		4- extrait toutes les chaînes à l'aide du préfixe d'extraction choisi,
		5- vérifie l'intégrité des chaînes extraites,
		6- supprime les fichiers temporaires créés,
		7- affiche dans l'éditeur le fichier 'nom_de_projet.po' créé pour
		   'Poedit' (en-tête à compléter partiellement, véracité des chaînes
		   à traduire, ...)
		8- appelle 'Poedit' pour la traduction et pour compléter l'en-tête.
		9- dans 'Poedit' traduire à la main, ou automatiquement 
		  (si vous avez créé une base de traduction automatique TM).

	Les points 1 à 7 sont gérés correctement par l'extension chargé au démarrage de 'C::B' 
	et accessible par de nouvelles entrées dans un menu '&Collecter' dans la barre des menus.
	Les points 8 et 9 sont seulement générés si 'Poedit' est installé dans le menu 'Outils->Poedit' 
	par vous-même.

			** sur un projet unique :'nom_de_projet.cbp'
		1- 'Collecter->Lister le projet',
			génère le fichier de suivi 'nom_de_projet.lst' ouvert dans l'éditeur, pour vérifier les chaînes proposées.
		2- 'Collecter->Extraire le projet',
			génère le fichier de suivi 'nom_de_projet.extr' ouvert dans l'éditeur,
			puis les fichiers de traduction
				- 'nom_de_projet.dup' (avec chaînes dupliquées) puis 
				- 'nom_de_projet.uni' (sans chaînes dupliquées) 
				aussi ouverts dans l'éditeur et enfin
				- 'nom_de_projet.pot' destiné à traducteur externe.
		3- 'Collecter->Lister puis Extraire le projet,
			réalise l'enchainement des points 1 et 2.

			** sur l'espace de travail : 'nom_de_projet_pilote.workspace'
		4- 'Collecter->Lister l'espace de travail'
			génère le fichier de suivi 'nom_de_projet_pilote_workspace.list' ouvert dans l'éditeur,
		5- 'Collecter->Extraire l'espace de travail',
			génère le fichier de suivi 'nom_de_projet_pilote_workspace.extr' ouvert dans l'éditeur,
			puis les fichiers de traduction
				- 'nom_de_projet_pilote_workspace.dup' (avec chaînes dupliquées) puis 
				- 'nom_de_projet_pilote_workspace.uni' (sans chaînes dupliquées) 
				aussi ouverts dans l'éditeur et enfin
				- 'nom_de_projet_pilote_workspace.pot' destiné au traducteur externe.
		6- 'Collecter->Lister puis Extraire l'espace de travail',
			réalise l'enchainement des points 4 et 5.

			** actions communes
		7- Initialiser le graphe d'état de 'Collecter'.
			initialise le graphe d'état de 'Collecter'
		8- Supprimer les fichiers temporaires.
			détruit les fichiers temporaires sur le disque
		9- Arréter l'action en cours.
			arrête l'action encours, Lister ou Extraire
			
B- DÉMARRAGE RAPIDE sur un projet unique :

	- activer le projet désiré
	- activer 'Collecter->Lister le projet' ce qui :
		- liste dans la page 'Collecteur' des journaux, tous les fichiers
		  analysés du projet en indiquant les fichiers élus possédant des
		  chaînes, ainsi que les dites chaînes et les avertissements éventuels
		- duplique ces résultats, ainsi que les chaînes concernées dans
		  l'éditeur, sous le nom 'nom_de_projet.list'.
	- activer 'Collecter->Extraire' (conserve le préfixe de 'Lister'), ce qui :
		- extrait les chaînes des fichiers élus dans un le journal 'Collecter'
		- duplique dans l'éditeur les résultats de page du journal sous le nom 'nom_de_projet.extr'.
		- crée (ou fusionne avec) le fichier 'nom_de_projet.dup' puis 'nom_de_projet.uni'
		- crée une copie de 'nom_de_projet.uni' vers 'nom_de_projet.pot'.

C- ENVIRONNEMENT de test, à la date du fichier :

	- Systèmes d'exploitation	:
		- Win-7 64 with Mingw64 avec gcc-8.1.0-seh (wx-3.2.1)
		- Leap-15.4 (64 bits) avec gcc-8.2.1 (wx-3.2.1)
	- Outils	:
		- Code::Blocks depuis r12524 (sdk 2.16.0 du 2021-09-04),
		  jusqu'a r13118 (sdk 2.23.0), avec wxWidgets 3.2.1 unicode,
	- Utilitaires	:
		- 'Poedit'
		- 'xgettext', 'msmerge', 'msguniq', '*.its'
			- et deux fichiers 'its\codeblocks.its', 'its\codeblocks.loc'
			à recopier dans le répertoire 'its' de 'xgettext\share\its\'
		- 'wxrc' utilitaire de 'wxWidgets' permettant aussi d'extraire les chaînes 
			à traduire des fichiers '*.xrc' et ''*.wxs'.
			* fournis avec 'Collecter' pour Win-7 dans les cibles 'wxrc_3xy'.			
		
	*** NOTE que seules les versions de 'C::B' depuis 'r12025' sont acceptées.

	*** A la date du fichier, aucun test sur 'MACOX'.
			
D- INSTALLATION de l'extension :

	1- on suppose que l'exécutable 'codeblocks' est installé en '$(cb_exe)',

	2- 'Poedit' doit-être installé et configuré dans 'C::B' dans l'un des menus :
		de 'C::B->Outils' avec pour titre 'Poedit' sans ajout

	3- les exécutables 'xgettext' et 'msmerge' font partie de 'Poedit', pour les utiliser sous 'Win'
	   vous devrez inclure le chemin d'exécution de 'Poedit' dans la variable système 'PATH'.
	
	4- deux fichiers 'its\codeblocks.its', 'its\codeblocks.loc' to be put in the 'its' directory of  
	  'gettext\share\its\'
	   
	5- vous pouvez récupérer l'extensiont sur
		- 'https://github.com/LETARTARE/Collector'

		5-1 si vous utiliser les sources, vous devrez compiler le projet 'Collector.cbp' lequel 
		    utilise les éléments suivants :
			- dans le projet :
				'Projet->Options de génération...'
					->Collecteur->Variables personnalisées'
				- $name = Collector
				- $cb = $(#sdk2230)
			- dans la cible	: 'local_3xx->Variables personnalisées'
				- Win-7 : $wx = $(#wx3xx_64)
				- Linux	: 'lin_3xy' qui utilise
					- "`wx-config --cflags  --version=3.2`"
					- "`wx-config --libs  --version=3.2`"
					
			5-1-1 vous devrez dans 'C::B' (svn >= r12025 => sdk-2.2.0) définir :
				- les variables globales :'Configuration->Variables globales...'
					 - #sdk2230	= "chemin_de_votre_version_source_CB-sdk\src"
					 - #wx3xx_64 	= "chemin_de_votre_version_wx3xx"
				- le même compilateur sera utilisé pour compiler
					- wxWidgets-3.x.x'
					- version de 'C:B' utilisé pour construire cette extension

			5-1-2 'Win-7' : construire la cible 'wxrc_3xx' :
				l'exécutable 'wxrc.exe' est copié automatiquement à coté de
				l'exécutable 'codeblocks'

			5-1-2 construire la cible '..._3xx'
			5-1-3 charger manuellement cette extension par le menu :
				- 'Extensions->Gestion des extensions...->Installer nouveau...'
				en choisissant le fichier '....\..._3xx\Collector.cbplugin'

			5-1-4 vérifier que 'Aide->Extension->Collecteur...' existe et dans
			laquelle vous trouverez un début de procédure de travail :
	===>
		Bienvenue dans 'Collecteur v ?.?.?'
		Extension d'aide à la traduction des projets dans 'Code::Blocks'
		Vous pouvez par le menu contextuel 'Collecter-> ...'
			1- 'Lister ...' les chaînes avec le mot clé choisi.
			2- 'Extraire' les chaînes vers 'nom_de_projet.po' à l'aide du mot clé précédent.
	<===
			- v ?.?.?  indique la version
			Votre traducteur est prêt a être utilisé.

		5.2- si des actualisations sont disponibles
			- actuellement :
				- Win64, wxWidgets3.2.1, 'C::B' >= r12025 (sdk >= 2.2.0)
				- Linux, wxWidgets3.2.1, 'C::B' >= r12025 (sdk >= 2.2.0)
			5-2-1 choisir (si elle existe) celle correspondant à
				- votre système d'exploitation : Win64, Lin64
				- votre version de 'C:B' et 'wxWidgets'
			5-2-2 placer les deux éléments suivants
				- 'chemin_cb_executable\share\CodeBlocks\Collector.zip'
				- 'chemin_cb_executable\share\CodeBlocks\plugins\Collector.dll'
			5-2-3 redémarrer 'C::B'

E UTILISATION SUR UN PROJET UNIQUE

	Tous les fichiers générés seront créés dans le répertoire :
		'dir_projet\trlocale\'

	1- sélectionner n'importe quel projet ...

	2- choisissez l'entrée du menu principal ou la barre d'outils:
		'&Collecter->Lister le projet actif ...'

		2-1- si c'est un projet non basé sur 'Wx' vous aurez un message
		     d'avertissement indiquant que ce n'est pas un projet utilisant 'Wx'

		2-2- pour un projet utilisant une bibliothèque 'Wx' et si vous êtes en
			mode 'LISTER' :
			- si tout est correct et si des chaînes existent dans les fichiers analysés, 
			  vous verrez apparaître une liste des fichiers du projet ainsi que les fichiers élus,
			- en fin de listage, le contenu du journal est sauvegardé sous le nom 'nom_de_projet.lst'
			  et sera automatiquement ouvert dans l'éditeur avec, en plus, toutes les chaînes extractibles,
			- vous pouvez alors consulter la liste produite et vérifier si vous ne trouvez pas d'incongruités
			  dans ces chaînes :
			  - exemple '_("...")' : cette chaîne n'a pas besoin de traduction !

		2-3- ensuite, si vous désirez créer les fichiers :
			'nom_de_projet.po' et 'nom_de_projet.pot'
			vous devez utiliser le mode 'EXTRAIRE' par le menu principal ou la barre d'outils 
			de 'Collecter->Extraire depuis le projet'

			- notez que le préfixe d'extraction est celui du listage précédent.

			- l'utilisation de 'Extraire'  permet de démarrer

			1- l'extraction des chaînes à partir des fichiers élus dans un fichier 'nom_de_projet.dup',
			2- la vérification d'intégrité de ce dernier fichier en
				- supprimant les caractères éventuels "\r",
				- remplaçant ses chaînes éventuelles "$$..." par "$...",
				- remplaçant ses chaînes éventuelles "%%..." par "%...",
				- modifiant l'en-tête à l'aide de quelques macros de 'C::B',
			3- la copie du fichier 'nom_de_projet.uni' -> '*.po', ou la fusion du premier avec le 
			   second s'il existe déjà,
			4- l'ouverture dans l'éditeur du fichier 'nom_de_projet.po' pour vérifications et 
			   modifications de l'en-tête éventuellement,
			5- la sauvegarde des résultats précédents dans un fichier de nom 'nom_de_projet.extr' 
			   qui sera ouvert dans l'éditeur,
			6- l'appel de 'Poedit' qui charge 'nom_de_projet.po'.

			** si 'Poedit' n'apparait pas automatiquement, vous devrez l'appeler manuellement pour 
			charger 'nom_de_projet.po', vous pourrez alors compléter l'en-tête dans le menu de 'Poedit' :
				'Catalogue->Configuration...'
			  en remplissant seulement les champs :
				- Langue,
				- Pays,
				- Jeu de caractères du code source : utf-8
				- Formes plurielles éventuellement : 'nplurals=2; plural n>1'
                 		(voir "http://translate.sourceforge.net/wiki/l10n/pluralforms")

		2-4- si vous désirez revenir au mode 'LISTER' utilisez l'initialisation du graphe d'état dans 
		     le menu 'Collect' ou la barre d'outil.

F UTILISATION SUR PROJETS MULTIPLES

	Tous les fichiers générés seront créés dans le répertoire :
		'dir_projet_pilote\trlocale\'

	Si vous avez un projet pilote, qui utilise d'autres projets contributifs et que vous désirez 
	un fichier de traduction unique, alors vous devez construire un espace de travail particulier
	qui les englobe tous :
		exemple :
		'C::B' avec ses extensions externes pourrait s'appeler
		'CodeBlocks_wx32_64.workspace' ou 'CodeBlocks_wx32-unix.workspace'
		lequel contiendra 'CodeBlocks.cbp' + 'ContribPlugins.worspace' actuel.

	1- chargez cet espace de travail dans 'C::B'
		- le projet pilote peut se situer n'importe ou dans l'arborescence,

	2- rendez actif le PROJET PILOTE, puis utilisez l''entrée de menu :
		'Collecter->Lister espace de travail'
		===>
			ATTENTION ne vous trompez pas de projet pilote, car l'extension'
			n'a aucun moyen de le repérer toute seule.
		<===

	3- le préfixe d'extraction utilisé ('_') est rappelé dans le menu ou la barre d'outil,
		- les projets retenus (basés sur 'wxWidgets') seront analysés 
			- tout d"abord le projet pilote 
			- et ensuite en suivant l'ordre de l'arborescence, les fichiers élus seront 
			  listés dans le journal 'Collecter',
		- en fin de listage, le contenu du journal est sauvegardé dans le fichier 'nom_de_projet_pilote.lst',
		  automatiquement ouvert dans l'éditeur.

	4- pour créer les fichiers 'nom_de_projet_pilote.po' et '*.pot' suivre la même démarche que pour un projet unique.

G- REMARQUES

	1- les fichiers '*.dup', '*.po' et '*.pot' sont notés différemment :
		- en projet unique : 'nom_de_projet.dup' et '*.po',
		- en multiprojets  : 'nom_de_projet_pilote_workspace.dup' et '*.po',
		ainsi seuls les fichiers '*.dup' (source) seront différents, alors qu'il
		ne peut exister qu'un seul 'nom_de_projet_pilote_workspace.po' chargé par 'Poedit'.

	2- le projet ou l'espace de travail concerné doit préalablement avoir été COMPLILé avec succès !!, 
	   de telle sorte que les fichiers soient EXEMPTS D'ERREURS DE SYNTAXE, sinon la visualisation et 
	   l'extraction ne seront pas correctes, ou généreront des avertissements.

	3- emplacement des fichiers générés : 'nom_de_projet_pilote.po', ...

		3-1- les fichiers sont écrit en
			'dir_projet_pilote\trlocale\name_de_projet_pilote_workspace.po'.

		3-2- si le répertoire 'trlocale' n'existe pas il sera créé, or la création de répertoire est
		     une opération protégée, 'C::B' vous demandera peut-être la première fois, si vous autorisez 
		     l'extension à réaliser cette opération de façon permanente ou pas :
			- choisissez de façon permanente.

		3-3- il faudra aussi indiquer ce chemin à 'Poedit' (voir ci-dessous)

	4- configuration de 'Poedit' appelé dans 'C::B'

		4-1- dans le menu 'C::B->Outils->Configurer les outils'
			- éditer la configuration de 'Poedit' et positionner :
				Paramètres = "$(PROJECT_DIR)trlocale\\$(PROJECT_NAME).po"
			- respecter les "" car ils permettent d'avoir des noms de projets avec des espaces (exemple 'Inno Setup' )
			- respecter '$(PROJECT_NAME)' car il correspond à 'nom_de_projet'
			- '$(PROJECT_DIR)' représente 'dir_projet_pilote',
			- '$(PROJECT_DIR)trlocale\\' définit le répertoire de recherche

	5- si un nom de projet comporte des caractères non gérés par les gestionnaires de fichiers externes, 
		par exemple : 'Code::Block' !!
		il faut	modifier le nom du projet en supprimant ces caractères soit 'CodeBlocks'
		car sinon le fichier généré 'Code::Block.po' ne serait jamais créé !!

	6- utilisation de cette extension sur le projet 'CodeBlocks-xxx.workspace'

		6-1 Sur mon environnement de test ''Win-7' : 'Collecteurr-1.7.8'
			- avec le mot clé '_' et 'codeblocks.its' pour '*.xml'

			6-1-1 'Cb-13118-wx3.2.1-(64bit)'
				avec 1 projet contenant 1216 fichiers,
				- listage 
					- 7789 chaines détectées depuis 398 fichiers éligibles,
					- durée : 0 min), 48 S
				- extraction 
					- 7789 chaines extraites depuis 398 fichiers éligibles,
					- 133 fichiers temporaires,
					- Cb-13118-wx3.2.1-(64 bit).dup'  (852 409 bytes) 
					- durée : 1 min, 8 S				

			6-1-2 'CodeBlocks-r13118 + les extensions externes + 'Collecteur-1.7.8'
				dans un espace de travail de 43 projets contenant au total 7962 fichiers,
				- listage 
					- 12677 chaines détectées depuis  2412 fichiers éligibles,
					- durée : 0 min 55 S,
				- extraction 
                    -> 'codeblocks-13118-wx3.2.1-(64bit)_workspace.dup' ( 2 022 699 bytes )
					- 12677 chaines extraites depuis  2412 fichiers éligibles,
					- 172 fichiers temporaires,
					- durée : 1 min 35 S 
					
		6-2 Sur mon environnement de test 'Leap15.4' :  'Collecteurr-1.7.8'
			
			6-2-1 Cb-13118-wx3.2.1-(64bit)' 
				avec 1 projet contenant 1216 fichiers,
				- listage
					- 7778 chaines détectées depuis 392 fichiers éligibles,
					- durée : 0 min), 43 S
				- extraction 
					- 7778 chaines extraites depuis 392 fichiers éligibles,
					- 133 fichiers temporaires,
					- Cb-13118-wx3.2.1-(64 bit).dup'  (851 202 bytes) 
					- durée : 0 min, 28 S				

			6-2-2 Cb-13118 + les extensions externes + 'Collector-1.7.8'
				dans un espace de travail de 43 projets contenant au total 2654 fichiers,
				- listing 
					- 10573 chaines détectées depuis 735 fichiers éligibles,
					- durée : 0 min 55 S,
				- extraction 
                    -> 'codeblocks-13118-wx3.2.1-(64bit)_workspace.dup' ( 1 906 549 octets )
					- 10573 chaines extraites depuis 735 fichiers éligibles,
					- 157 fichiers temporaires,
					- durée : 1 min 35 S 
			
		6-3 Sur environnement 'MACOX'
		
			- ???

H- A FAIRE

	- supprimer les lignes vides dans les chaînes traduites ...
	- ajouter le préfixe 'wxPLURAL' ??
	- ajouter d'autres types de projet (Qt, ?)
	- adapter pour MACOX ...
	- ...


I- ANNEXES

	Dans 'Poedit' la traduction automatique nécessite une validation message
	par message à la main !!!.

	La sauvegarde du fichier 'nom_de_projet.po' génère en plus le fichier
	'nom_de_projet.mo' dans le même répertoire, si vous avez coché la case
		'Fichier->Préférences->Editeur'
		:x: compiler automatiquement les fichiers '.mo' lors de la sauvegarde,

	1- si vous êtes sûr de vos traductions dans la base de données, par exemple
		pour 'C::B' ou vous avez déjà traduit des versions précédentes et que vous
		traduisez une version nocturne; vous pouvez alors :

		1-1- dans 'Poedit' charger 'nom_de_projet.po'
			- lancer la traduction automatique TM,
			- réaliser les nouvelles traductions,
			- puis sauvegarder le fichier en générant le 'nom_de_projet.mo'
			- vous aurez probablement des ajustements dus à la traduction TM...
			- quitter 'Poedit'.

		1-2- dans l'explorateur de 'C::B' en mode 'Fichiers' :
			- charger 'nom_de_projet.po' dans l'éditeur,
			- supprimer toutes les occurrences de '#, fuzzy' (traduction farfelue),
				( attention aux '#, fuzzy, c-format' )
			- sauvegarder et fermer ce fichier.

		1-3- dans 'Poedit' recharger 'nom_de_projet.po' pour VéRIFICATION
			- sauvegarder le fichier en générant le 'nom_de_projet.mo',
			- probablement certaines erreurs apparaitront, dues à la prise en compte
				des nouvelles chaînes validées par la suppression des '#, fuzzy',
			- sauvegarder éventuellement le fichier d'erreurs 'log.txt'
			- fermer 'Poedit'

		1-4 dans 'C::B' charger ce fichier 'log.txt'
			- éditer 'nom_de_projet.po' pour appliquer les modifications indiquées
			  dans 'log.txt',
			- sauvegarder 'nom_de_projet.po'

		1-5 recharger 'nom_de_projet.po' dans 'Poedit' pour finir le travail
			- sauvegarder le fichier en générant le fichier 'nom_de_projet.mo'

		1-6- il ne reste plus qu'a placer le fichier 'nom_de_projet.mo' à l'endroit
			voulu.

	2- une solution alternative serait, dans 'Poedit', d'actualiser le fichier
		'nom_de_base.po' par
			'Catalogue->Mettre à jour depuis un fichier POT'
		 par un ancien fichier 'nom_de_projet.po', déjà traduit préalablement, et
		 renommé 'nom_de_projet.pot', ainsi 'Poedit' fusionnera les deux fichiers
		 en récupérant les anciennes traductions.
