# Collector-1.8.0, January 19, 2023 (Win-64, Lin-64)

Plugin to translate applications (wxWidgets) with 'Code::Blocks'

**Versions for all 'Code::Block, sdk >= 2.0.0, svn >= 12004'**

**Last: sdk = 2.23.0, svn = 12896 -> ...**

1 - What is this project :

    The goal is to translate, in the local language, the development projects
	under 'Code::Blocks', only those using 'wxWidgets'.

	The extension 'Collector' to generate in directory 'trlocale' a file :
		- 'name_projet.po' for a single project,
		- or 'leader_name_project_workspace.po' for cooperative project (workspace).

	This file is provided to 'Poedit', built-in 'Code::Blocks', which generates
	file '*.mo', to put in the executable directory for test

2- Prerequisites

	1- 'xgettex.exe', 'msmerge.exe' and 'msguniq.exe'  must be installed in the system path
		(system variable PATH), or either with 'codeblocks.exe'

		-> 'Win-64' : 'Poedit' containt 'xgettext.exe', 'msmerge.exe', 'msguniq.exe'
		-> 'Lin-64' : 'xgettext' is independent of 'Poedit'
			'http://www.poedit.net/'
			
        ** you will have to add in the 'gettext\share\its' directory the two files
            codeblocks.its' and 'codeblocks.loc' provided in '...\its'.
            
		To install 'Poedit' in 'Code::Blocks' :
			1- read 'http://wiki.codeblocks.org/index.php?title=Configure_tools',
			2- read 'infos/Collector_exp.en' paragraph 'SETUP 'Poedit',

	2- 'wxrc' must be installed  
		- either with 'codeblocks' (for 'Win-64' : 'wxrc.exe' with target 'wxrc_320' or 'wxrc_321')
		- either with 'wxWidgets' (for 'Lin-64' : native)

3- Installation in 'Code::Blocks' (without 'Poedit') :

	1- install 'Collector-x.y.z' in any directory

	2- in 'Code::Blocks' 
		- load the project 'Collector-x.y.z.cbp'
		- choice target (lin_320, lin_321, win_320, win_321 ... : 320 => wx-3.2.0)
		- Win-64 :
			- 'Build->Build' => 
				- 'win_321\Collector.dll' with size ... KB
				- ...
			- 'Plugins->Manage plugins... : Install New' 
				- choice '...\win_321\Collector.cbpugin'
				=> in 'Collector' log
					** Platform : 'Win-64', 'sdk-2.23.0', 'Collector-1.8.0', built the '23/01/17::10:51:52'  
		- Lin-64 :
			- 'Build->Build' => 'lin_321/Collector.so' with size ... KB
				...
			- 'Plugins->Manage plugins... : Install New' 
				- choice '.../lin_321/Collector.cbpugin'
				=> in 'Collector' log
					** Platform : 'Linux-64', 'sdk-2.23.0', 'Collector-1.8.0', built the '23/01/17::11:09:15' 
				
	3- You see '&Collect' in menu bar or 'Collector Toolbar' in tools bar

4 - Fast Start in 'Code::Blocks' (without 'Poedit') :

	On a activated project :
	
	1- '&Collect->List from project',
		which lists the files with strings in 'Collector' log,

	2- '&Collect->Extract from project'
		which extracts the strings and creates the file 'trlocale\name_project.po'
		
	** You can perform these two operations in sequence by
		'Collect->List and Extract from project',
			
	3- item '&Collect->Removes temporary files' 
		which eliminates, if necessary, potential temporary files created during listing.
		
	4- '&Collect->Go to start state', 
		to correctly restart the next listing,
	
	5- '&Collect->Stop current action',
		stops the current listing or extraction.

5 - Detailed use

	The directory 'infos' files contains the explanatory use.

