# Collecteurr-1.7.8,  17 décembre 2022 (Win-64, Linux-64)

Extension de traduction des apllications (wxWidgets) avec 'Code::Blocks'

**Pour toutes les versions 'Code::Block, sdk >= 2.0.0, svn >= 12004'**

**La dernière: sdk = 2.23.0, svn = 12896 -> ...**

A - Quel est ce projet ?

	Le but est de traduire, dans la langue locale, les projets de développement sous 'Code::Blocks', 
	uniquement ceux utilisant 'wxWidgets'.

	L'extension 'Collector' permet de générer, dans le répertoire 'trlocale', un fichier :
		- 'nom_projet.po' pour un seul projet, ou
		- 'nom_projet_pilote_workspace.po' pour un projet coopératif (workspace).

	Ce fichier est fourni à 'Poedit', intégré à 'Code::Blocks', qui génère un fichier 
	'*.mo', à placer, par l'utilisateur, dans le répertoire exécutable pour le test.

B- Conditions préalables

	1- 'xgettex' et 'msmerge'et 'msguniq' doivent être installés dans le chemin du système
		(variable système PATH), ou soit avec 'codeblocks'.

		-> Poedit contient "xgettext" et "msmerge" et "msguniq"
			http://www.poedit.net'
		Pour installer 'Poedit' dans 'Code::Blocks' :
			1- lisez 'http://wiki.codeblocks.org/index.php?title=Configure_tools',
			2- lire 'infos/Collector_exp.en' paragraphe 'SETUP 'Poedit',
			
		** vous devrez ajouter dans le répertoire 'gettext\share\its' les deux fichiers
            codeblocks.its' et 'codeblocks.loc' fournis dans '...\its'.

	2- 'wxrc' doit être installé  
		- soit avec 'codeblocks.exe' (pour 'Win-64' à l'aide de la cible 'wxrc_321')
		- soit avec 'wxWidgets' (pour 'Lin-64')

 C- Installation dans 'Code::Blocks' (sans 'Poedit') :

	1- Installez 'Collector-x.y.z' dans n'importe quel répertoire.

	2- dans 'Code::Blocks' 
		- chargez le projet 'Collector-x.y.z.cbp'.
		- choisissez la cible (lin_320, lin_321, win_320, wx_321 ... : 320 => wx-3.2.0)
		- Win-64
			- 'Générer->Générer' => 
				- 'win_321\Collector.dll' avec une taille de ... KB
				- ...
			- 'Extensions->Gérer les extensions... : installer une nouvelle' 
				- choisissez '...\win_321\Collector.cbpugin'
				- ...
				=> vous verrez dans le journal 'Collector'
			** "Plate-forme : 'Win-64', 'sdk-2.23.0', 'Collector-1.7.8', construit le '22/12/317::10:09:15'" 

		- Lin-64 :
			-  Générer->Générer' => 
				- 'lin_321/Collector.so' avec une taille de ... KB
				- ...
			- 'Extensions->Gérer les extensions... : installer une nouvelle'
				- choisissez '.../lin_321/Collector.cbpugin'
				- ...
				=> dans le journal 'Collecter'
			** "Plate-forme : 'Lin-64', 'sdk-2.23.0', 'Collector-1.7.8', construit le '22/12/317::10:09:15'" 
				
	3- Vous devez voir le menu '&Collecter' ou 'Collecteur Toolbar' dans la barre d'outils.

4 - Démarrage rapide dans 'Code::Blocks' (sans 'Poedit') :

	Sur un projet activé :
	
	1- '&Collecter->Lister depuis le projet',
		qui liste les fichiers avec des chaînes de caractères dans le journal de 'Collecteur',

	2- '&Collecter->Extraire du projet',
		qui extrait les chaînes de caractères et crée le fichier
			fichier 'trlocale\name_project.po',
			
	** Vous pouvez effectuer ces deux opérations en séquence par
		'&Collecter->Lister et Extraire du projet',
			
	3- '&Collecter->Supprimer les temporaires', 
		qui supprime, si nécessaire, les fichiers temporaires créés lors du listage,

	4- '&Collecter->Aller à l'état de départ', 
		pour redémarrer correctement le prochain listing,
	
	5- '&Collecter->Arréter l'action en cours',
		arrête le listage ou l'extraction en cours.

5 - Utilisation détaillée

	Le répertoire 'infos' contient l'utilisation explicative.
