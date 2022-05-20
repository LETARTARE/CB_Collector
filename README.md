# Collector-1.6.0, May 20, 2022 (Win-64, Linux-64)

Plugin to translate applications (wxWidgets) with 'Code::Blocks'

**Versions for all 'Code::Block, sdk >= 2.0.0, svn >= 12004'**

**Last: sdk = 2.18.0, svn = 12813 -> ...**

1 - What is this project :

    The goal is to translate, in the local language, the development projects
	under 'Code::Blocks', only those using 'wxWidgets'.

	The extension 'Collector' to generate in directory 'trlocale' a file :
		- 'name_projet.po' for a single project,
		- or 'leader_name_project_workspace.po' for cooperative project (workspace).

	This file is provided to 'Poedit', built-in 'Code::Blocks', which generates
	file '*.mo', to put in the executable directory for test

2- Prerequisites

	1- 'xgettex.exe' and 'msmerge.exe' must be installed in the system path
		(system variable PATH), or either with 'codeblocks.exe'

		-> 'Win-64' : 'Poedit' containt 'xgettext.exe' and 'msmerge.exe'
		-> 'Lin-64' : 'xgettext' is independent of 'Poedit'
			'http://www.poedit.net/'
			
        ** you will have to add in the 'gettext\share\its' directory the two files
            'codeblocks.its' and 'codeblocks.loc' provided in '...\its'.
            
		To install 'Poedit' in 'Code::Blocks' :
			1- read 'http://wiki.codeblocks.org/index.php?title=Configure_tools',
			2- read 'infos/Collector_exp_en.md' paragraph 'SETUP 'Poedit',

	2- 'wxrc' must be installed  
		- either with 'codeblocks' (for 'Win-64' : 'wxrc.exe' with target 'wxrc_315' or 'wxrc_316')
		- either with 'wxWidgets' (for 'Lin-64' : native)


3- Installation in 'Code::Blocks' (without 'Poedit') :

	1- install 'Collector-x.y.z' in any directory

	2- in 'Code::Blocks' 
		- load the project 'Collector-x.y.z.cbp'
		- choice target (lin_305, lin_315, win_315, win_316 ... : 305 => wx-3.0.5)
		- Win-64 :
			- 'Build->Build' => 
				- 'win_316\Collector.dll' with size ... KB
				- ...
			- 'Plugins->Manage plugins... : Install New' 
				- choice '...\win_316\Collector.cbpugin'
				=> in 'Collector' log
					** Platform : 'Win-64', 'sdk-2.18.0', 'Collector-1.5.0', built the '20/05/22::10:51:52'  
		- Lin-64 :
			- 'Build->Build' => 'lin_305/Collector.so' with size ... KB
				...
			- 'Plugins->Manage plugins... : Install New' 
				- choice '.../lin_305/Collector.cbpugin'
				=> in 'Collector' log
					** Platform : 'Linux-64', 'sdk-2.18.0', 'Collector-1.6.0', built the '20/05/22::16:09:15' 
				
	3- You see '&Collect' in menu bar or 'Collector Toolbar' in tools bar

4 - Fast Start in 'Code::Blocks' (without 'Poedit') :

	On a activated project :
	
	1- '&Collect->List from project',
		which lists the files with strings in 'Collector' log,

	2- '&Collect->Extract from project'
		which extracts the strings and creates the file 'trlocale\name_project.po'
			
	3- item '&Collect->Removes temporary files' 
		which eliminates, if necessary, potential temporary files created during listing.

5 - Detailed use

	The directory 'infos' files contains the explanatory use.

6- Menu and Tool bar

![Menu](https://github.com/LETARTARE/CB_Collector/tree/Images16/collector160_menu_bar.png  "Menu")

7- List logs

![ListB](https://github.com/LETARTARE/CB_Collector/tree/Images16/collector160_beginList.png  "BeginList")

![ListE](https://github.com/LETARTARE/CB_Collector/tree/Images16/collector160_endList.png   "EndList")

8- Extract logs

![ExtractB](https://github.com/LETARTARE/CB_Collector/tree/Images16/collector160_beginExtract.png    "BeginExtract")

![ExtractE](https://github.com/LETARTARE/CB_Collector/tree/Images16/collector160_endExtract.png  "EndExtract")

9- Editor

![EDITOR](https://github.com/LETARTARE/CB_Collector/tree/Images16/collector160_cb12814_editor.png   "EDITOR")

10- Poedit in French

![Poedit](https://github.com/LETARTARE/CB_Collector/tree/Images16/collector160_poedit_fr.png    "Poedit")

	
