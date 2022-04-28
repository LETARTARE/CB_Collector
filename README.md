# Collector-1.3.0, May 01, 2022 (Win-64, Linux-64)

Plugin to translate applications (wxWidgets) with 'Code::Blocks'

**Versions for all 'Code::Block, sdk >= 2.0.0, svn >= 12004'**

**Last: sdk = 2.17.0, svn = 12724 -> ...**

1 - What is this project :

    The goal is to translate, in the local language, the development projects
	under 'Code::Blocks', only those using 'wxWidgets'.

	The extension 'Collector' to generate in directory 'trlocale' a file :
		- 'name_projet.po' for a single project,
		- or 'leader_name_project.po' for cooperative project (workspace).

	This file is provided to 'Poedit', built-in 'Code::Blocks', which generates
	file '*.mo', to put in the executable directory for test

2- Prerequisites

	1- 'xgettex.exe' and 'msmerge.exe' must be installed in the system path
		(system variable PATH), or either with 'codeblocks.exe'

		-> 'Win-64' : 'Poedit' contient 'xgettext.exe' et 'msmerge.exe'
			'http://www.poedit.net/'
		To install 'Poedit' in 'Code::Blocks' :
			1- read 'http://wiki.codeblocks.org/index.php?title=Configure_tools',
			2- read 'infos/Collector_exp.en' paragraph 'SETUP 'Poedit',

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
					**Platform : 'Win-64', 'sdk-2.17.0', 'Collector-x.y.z', built the '05/01/22::16:09:15'** 
		- Lin-64 :
			- 'Build->Build' => 'lin_316/Collector.so' with size ... KB
				...
			- 'Plugins->Manage plugins... : Install New' 
				- choice '.../lin_316/Collector.cbpugin'
				=> in 'Collector' log
					**Platform : 'Linux-64', 'sdk-2.17.0', 'Collector-x.y.z', built the '05/01/22::16:09:15'**
				
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

