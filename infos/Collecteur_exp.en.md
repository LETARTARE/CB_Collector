===>
	IF YOU THINK A TRANSLATION ERROR HAS OCCURRED, PLEASE LET ME KNOW.
<===

A- PURPOSE :

	The ultimate goal is to translate, in the local language, the development project
	under 'C::B' directly (currently only projects using the library 'wxWidgets').

	This is done using 'Poedit' built-in 'C::B', in which is provided a file
	'project_name.po' directly translatable 'Poedit'.

	The extension script 'Collector.cbp' will generate this file
	'project_name.po' from the core project assets in 'C::B'.

	This project is either single core or associated with other cooperative
	projects in a common workspace.

	===>
	COLLABORATIVE PROJECTS SHOULD BE IN THE SAME DISC.
	<===

	It accepts two basic working modes :

		1- 'LIST' strings to extract checks for relevance,
			1-1 on a single project,
			1-2 on a workspace with a project basis and cooperative projects,
		2- 'EXTRACT' strings in a file 'project_name.po'
			- 'project_name.po' on single project,
			- 'leader_project_name.pot' on workspace,
			  (duplicated 'leader_project_name.po' readable 'Poedit').
			  
		** the two modes can be linked by
			'LIST then EXTRACT'

	It takes into account the eligible files :
		'*.h', '*.cpp', '*.script', '*.xrc', '*.wxs', (to come '*.xml')
	* files '*. xrc, *.wxs' generate temporary files 
		'*_xrc.str???, *_wxs.str???' (??? = number) using of 'wxrc' executable 
		(wxWidgets-3.1.x), which are then deleted.
	* files '*.xml' using of 'its' rules from '*.its' files

	It takes into account the prefixes of extractions:
		1- for the listing	: '_'
		2- for extracting 	: '_'

	To do this, the plugin for the analyzed project :

		1- recognize if it is a project using 'wxWidgets' ('wx')
		2- optionally generate temporary files  '*_xrc.str???' (??? = number)
		3- automatically detect files eligible for the extraction,
		4- extract all strings with the prefix of extraction used,
		5- verify the integrity of extracted strings,
		6- delete temporary files,
		7- display in the editor file 'project_name.po' created for 'Poedit'
			(header to be completed partially, and veracity of the strings to
			 translate to be checked...),
		8- call 'Poedit' to complete the translation and the header,
		9- in 'Poedit' translated by hand or automatically
			(if you have create	a translation database TM)

	Items 1-7 are managed correctly by the plugin loaded at startup
	'C::B' and accessible through new entries in a menu ''&Collect' in the menu
	bar or tools bar.
	Items 8 and 9 are only generated if 'Poedit' is installed in the 
	menu 'Tools->Poedit' by yourself.
	
			** on a single project: 'project_name.cbp
		1- 'Collect->List from project',
			which allows 'LIST' to extract strings for check to 'project_name.list'.
		2- 'Collect->Extract from project',
			which generates 'project_name.extr' then 'project_name.po'.
		3- 'Collect->List and Extract from project'
			
			* for a workspace with leader project : 'pilot_project_name.workspace'
		4- 'Collect->List from workspace'
		5- 'Collect->Extract from workspace',
		6- 'Collect->List and Extract workspace'
			
			* communs actions
		7- 'Collect->Init to first state',
			initialise the state graph of 'Collect'
		8- 'Collect->Clean temporary',
			delete temporary files,
		9- 'Collect->Stop current action',
			Stop the current action, list or extract.

B- QUICK START onto single project :

	- Activate the required project

	- Activate 'Collect->List from project' which :
		- lists in the 'Collector' page of the journals, all the parsed files of the
		  of the project, indicating the selected files with strings, as well as
		  chains and any warnings
		- duplicates these results, as well as the concerned strings in the editor,
		  under the name 'project_name.list'.
		  
	- Activate 'Collect->Extract from project' (retains the prefix of 'Lister'), which :
		- extracts the strings from the elected files in a 'Collect' log
		- duplicates the log page results in the editor under the name
			'project_name.extr'.
		- creates (or merges with) the file 'project_name.po
		- creates a copy of 'project_name.pot'.


C- TEST ENVIRONMENT, to the date of the file :

	- OS 	:
		- Win-7 64 bits with Mingw64 and gcc-8.1.0-seh (wx-3.1.5)
		- Leap-15.3 (64 bits) with gcc-7.1.0 (wx-3.0.5)
	- Tools :
		- Code::Blocks from r121524 (sdk 2.2.0, 2021-09-04)
			up to r12803 (sdk 2.17.0), with wxWidgets 3.x.y unicode,
		- actually 'Cb-12803-wx-3.1.6', sdk=2.17.0
	- Utilities :
		- 'xgettext', 'msmerge' (using that of 'Poedit') and two files
		   'codeblocks.its', 'codeblocks.loc' to be put in the 'its' 
		   directory of 'Poedit
		- 'wxrc' executable of 'wxWidgets' utility, also allowing to extract
			the string to translate files '*.xrc'

	*** NOTE that only the versions of 'C::B' from r12025 are accepted.

	*** A file date, no testing on MACOX.

D- CONTENTS compressed package delivered :

	The proposed solution is contained in a package 'Collector.7z or '*.zip'
	which includes:

		1-  for 'Win-7' one target 'wxrc_3xy' to compile the correct version
			corrected utility 'wxrc' in UNICODE
			and files '*.xrc' 'from 'C::B' for testing,

		2- one target 'infos' not compilable, containing
			 2-1 this help file 'Collector_exp.en' text-UTF-8
				 and  also 'Collector_exp.fr'
			 2-2 two quick help text files :
				'Readme.txt', 'Alire.txt'

		3- 'lin_3xy' and 'win_3xy' compilable targets
			- Collector.so' or 'Collector.dll' the dynamic library
			- Collector.zip' containing the resources and the 'manifest.xml
			- Collector.cbplugin' containing the first two : this is the compressed file
			  file that will be loaded from the 'C::B' environment

		4- for 'Win-7' 'wxrc_3xy' targets which can be compiled by copying
		'wxrc.exe' executable in the directory where the 'codeblocks' executable is
		 located


E- INSTALLING the package:

	1- it is assumed that 'codeblocks.exe' is installed in '$(cb_exe)'

	2- 'Poedit' must be installed in a choice of menus:
		- 'C::B-> Tools' with the title 'Poedit' without any addition

	3 - for 'Win-64' the executables 'xgettext.exe' and 'msmerge.exe' are part of 
		'poedit.exe', and for use, you must include the 'poedit' path into environment
		variable 'PATH',

	4- you can get the package at
		- 'https://github.com/LETARTARE/CB_Collector'

		4-1 If you are using the sources, you will need to compile the project
			'Collector.cbp' project which uses the following elements
			- in the project :
				'Project->Generation options...'
					->Collector->Custom Variables'.
				- $name = Collector
				- $cb = $(#sdk2170_3xy)
			- in the target: '..._3xy->Custom variables'
				- Win-7 : 'win_3xy' => $wx = $(#wx3xy_64)
				- Lin 	: 'lin_3xy' use
					- "`wx-config --cflags  --version=3.0`"
					- "`wx-config --libs  --version=3.0`"

			4-1-1 you will need to in 'C::B' (svn >= r12025 => sdk-2.2.0) define :
				- global variables: 'Configuration->Global variables...'
					 - #sdk2170_3xy = "path_of_your_version_source_CB\src"
					 - #wx3xy_64 = "path_of_your_version_wx3xx"
				- the same compiler will be used to compile
					-' wxWidgets-3.x.y'
					- version of 'C:B' used to build this extension

			4-1-2 'Win-7' : build the target 'wxrc_3xy' :
				the executable 'wxrc.exe' is copied automatically next to
				the 'codeblocks' executable

			4-1-2 build the target '..._3xy'
			4-1-3 manually load this extension through the menu :
				- 'Plugins->Management of extensions...->Install new...'
				by choosing the file '....\..._3xy\Collector.cbplugin'.

			4-1-4 check that 'Help->Plugins->Collector...' exists and in
			where you will find a start of the working procedure:
	===>	
		Help translating for 'C::B' (sdk-2.17.0) projects
		Welcome to 'Collector' : plugin for help translate projects in 'CODE::BLOCKS'
		You can, by menu '&Collect' on menu bar
			1- 'List from project' : list strings with selection keyword ('_').
			2- 'Extract from project' : extract strings to 'project_name.po'
				with previous keyword.
			3- 'Stop current action' : abort current action.
	<===
			- v ?.?.? indicates the version
			Your translator is ready to be used.
		
		4.2- if updates are available
			- on the date of this file :
				- 'Win-64', 'wxWidgets-3.1.6', 'C::B' >= r12724 (sdk >= 2.16.0)
				- 'Lin-64', 'wxWidgets-3.0.5', 'C::B' >= r12025 (sdk >= 2.2.0)
			4-2-1 choose (if it exists) the one corresponding to
				- your operating system: 'Win-64', 'Lin-64'
				- your version of 'C:B' and 'wxWidgets-3.x.y'
			4-2-2 place the following two elements
				- 'path_cb_executable\share\CodeBlocks\Collector.zip'
				- 'path_cb_executable\share\CodeBlocks\plugins\Collector.dll'.
			4-2-3 restart 'C::B'.

F1- USE ON SINGLE PROJECT

	All generated files will be created in the directory :
		'dir_projet\trlocale\'
	
	1- select any project ...

	2- choose the main menu item or the toolbar:
		'Collect->List from project'

		2-1- if a project is not based on 'wx' you will see a window warning that
			this is not a project using 'wx',

		2-2- a project using a library 'wx' you are in 'List' mode :
			- if everything is correct and if strings exist in the files
			  you will see a list of the files in the project as well as the 
			  elected files,
			- At the end of the listing, the content of the log is saved under the
				name 'project_name.list' and will be automatically opened in the editor
				with all their extractable strings,
			- you can then look at the list produced and check if you for any incongruities 
			in these strings:
			  - example '_("...")': this string does not need translation!

		2-3- then, if you want to create the :
				'project_name.po' and 'project_name.po
			you must use the 'EXTRACT' mode via the main menu or the toolbar of
					'Collect->Extract from project'

			- note that the extraction prefix is the one from the previous listing.

			- Using 'Extract' will start

			1- extracting the strings from the elected files in  a file 
				'project_name.po',
			2- the integrity check of this last file by
				- removing the possible "\r" characters,
				- replacing its possible strings "$$..." by "$...",
				- replacing its possible strings "%%..." with "%...",
				- modifying the header using some 'C::B' macros,
			3- copying the file 'project_name.po' -> '*.pot',
				or merging the first with the second if it already exists,
			4- opening the 'project_name.po' file in the editor for checking and 
				modifying the header if necessary,
			5- save the previous results in a file named 'project_name.extr' which 
				will be opened in the editor,
			6- calling 'Poedit' which loads 'project_name.po'.

			** if 'Poedit' does not appear automatically, you will have to call it
			  manually to load 'project_name.po', you can then complete the header in the
			  complete the header in the 'Poedit' menu:
				'Catalogue->Configuration...'
			  by filling in only the fields :
				- Language,
				- Country,
				- Source code character set: utf-8
				- Plural forms possibly: 'nplurals=2; plural n>1'
                 (see "http://translate.sourceforge.net/wiki/l10n/pluralforms")

		2-4- If you want to return to the 'LIST' mode use the of the status graph in the 
			'Collect' menu or the toolbar.

F-2 USE ON MULTIPLE PROJECTS

	All generated files will be created in the directory :
		'dir_leader_project\trlocale\'

	If you have a leader project, which uses other contributing projects
	and you want a single translation file, then you need to build a special 
	workspace that encompasses them all:
		example :
		C::B' with its external extensions could be called
			'CodeBlocks_wx31_64.workspace' or 'CodeBlocks_wx31-unix.workspace '
	which will contain the current 'CodeBlocks.cbp' + 'ContribPlugins.worspace'.

	1- load this workspace into 'C::B
		- the leader project can be located anywhere in the tree,

	2- make the LEADER PROJECT active, then use the menu entry :
			'Collect->List from workspace'
		===>
			ATTENTION don't get the wrong leader project, because the extension has 
			no way of locating it on its own.
		<===

	3- the extraction prefix used ('_') is recalled in the menu or the toolbar,

		- the selected projects (based on 'wxWidgets') will be analysed following
			the order of the tree structure, and the selected files will be listed in the
			the 'Collector' log,
		- At the end of the listing, the contents of the log are saved in the
			file 'leader_project_name.po.list', automatically opened in the editor.
			

	4- to create the 'leader_project_name.po' and '*.pot' files
	 follow the same procedure as for a single project.


G- NOTES

	1- '*.po' and '*.pot' files are noted differently:
		- in single project: 'project_name.pot' and '*.po',
		- in multi-projects: 'leader_project_name.po' and '*'.pot',
		so only the '*.pot' (source) files will be different, whereas there can only be one
		can exist only one 'leader_project_name.po' loaded by 'Poedit'.

	2- the project or workspace concerned must first have been successfully
		successfully compiled, so that the files are SYNTAX ERRORS, otherwise viewing and 
		extraction will not be correct, or will generate warnings.

	3- location of the generated files: 'leader_project_name.po', ...

		3-1- the files are written in
			'dir_leader_projet\trlocale\leader_project_name.po'.

		3-2- if the directory 'trlocale' does not exist it will be created, but the
			directory creation is a protected operation, 'C::B' will perhaps ask you
			may ask you the first time, if you allow the extension to perform this
			to perform this operation permanently or not:
				-> choose permanent.

		3-3- you will also have to indicate this path to 'Poedit' (see below)

	4- Setup 'Poedit' called in 'C::B'

		4-1- in the menu 'C::B->Tools->Configure tools...'
			- edit the configuration of 'Poedit' and set:
				Parameters = "$(ACTIVE_EDITOR_FILENAME)"
				Working directory = $(PROJECT_DIR)trlocale
				Lauching options : check the last one,
			- respect the "" because they allow to have names of projects with spaces
				(eg 'Inno Setup'),
			- '$(PROJECT_DIR)' is 'dir_leader_project_name',
			- '$(PROJECT_DIR)trlocale' defines the directory search.

	5- if a project name contains characters that are not managed by external
		file managers, such as :
			'Code::Block'  !!
		should change the name of the project by removing these characters is
			'CodeBlocks'
		otherwise the generated file
			'Code::Block.po'
		would never be created !

	6- using this extension on the 'CodeBlocks-xxx.workspace' project

		On my test environment 'Win-7' :

			Cb-12803-wx3.1.6-(64bit)' with the keyword '_'
			1215 files, 25 targets, 275 eligible files:
				- listing = 1 min 16 S, with 2033 strings collected,
				- extraction = 28 S with 88 temporary files

			Cb-12803 + external extensions
			in a workspace of 38 projects in total:
				- listing = 3 min 35 S
					- 8934 extractable strings in 2453 files of which 636 are elected
				- extraction = 6 min -> 'Cb-7953.pot'' (1306191 bytes)
					- 99 '\r' deletions ( 2 mm 25 S !)
					- 41 changes of '$$...' to '$...
					- 6594 strings to be translated

H- TO DO

	- take into account other types of '*.xml' files
	- remove empty lines in translated strings ...
	- add 'wxPLURAL' prefix ??
	- add other project types (Qt, ?) ...
	- adapt for MACOX ...
	- ...

J- ANNEXES

	With 'Poedit' :
	
	- machine translation requires message-by-message validation by hand!!!
	- saving the file 'project_name.po' also generates the file 'project_name.mo'
	  in the same directory, if you have checked the box
		File->Preferences->Editor' box
		:x: automatically compile '.mo' files when saving,

	1- if you are sure of your translations in the database, for example
		for 'C::B' or you have already translated previous versions and you are
		translate a night version; you can then :

		1-1- in 'Poedit' load 'project_name.po
			- start the automatic translation TM,
			- make the new translations,
			- then save the file by generating the 'project_name.mo
			- you will probably have some adjustments due to the TM translation...
			- quit 'Poedit'.

		1-2- in the explorer of 'C::B' in 'Files' mode:
			- load 'project_name.po' in the editor,
			- delete all occurrences of '#, fuzzy',
				( beware of '#, fuzzy, c-format' )
			- save and close this file.

		1-3- in 'Poedit' reload 'base_name.po' for VERIFICATION
			- save the file by generating the 'project_name.mo',
			- probably some errors will appear, due to the taking into account
				new strings validated by the removal of '#, fuzzy',
			- possibly save the error file 'log.txt
			- close 'Poedit

		1-4 in 'C::B' load this 'log.txt' file
			- edit 'project_name.po' to apply the changes indicated
			  in 'log.txt',
			- save 'project_name.po

		1-5 reload 'project_name.po' into 'Poedit' to finish the job
			- save the file by generating the file 'project_name.mo

		1-6- it only remains to place the file 'project_name.mo' in the desired
			desired location.

	2- an alternative solution would be, in 'Poedit', to update the file with
			'Catalogue->Update from POT file'
		 by an old file 'project_name.po', already translated beforehand, and
		 renamed 'project_name.pot', so 'Poedit' will merge the two files
		 files, recovering the old translations.
