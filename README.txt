Readme File and Frequently Anticipated Questions
=================================================================
Minimum Requirements
-----------------------------------------------------------------
See minimum_requirements.txt. The main requirement is
that your operating system is Microsoft Windows 7
or later. (A Windows emulator works as well, but note it
will require more work. Emulators are not at all supported by
the author.)

Compiling the Source Code (if you downloaded the GitHub repo)
-----------------------------------------------------------------
If you downloaded this program from GitHub, you will need to
compile the program. You do not need to follow the instructions
on how to install the program if you take this approach. To
compile the program, follow the following steps:

1. Download the main branch of the source repo from GitHub.
Extract the .zip file to some location on your computer.
2. Download Microsoft Visual Studio 2019 (just search for it)
and install it. Make sure to select the package titled
Desktop development with C++ and the package titled Game
development with C++ during installation.
3. Go to the directory where you extracted the repository. In
the root directory, there should be a file called
TowerDefense.sln. If you installed Visual Studio correct, you
should be able to double-click the file to open the project.
4. Towards the top, you will see two combo boxes. Make sure
the first combo box has Release selected, and for the second
one, choose x86 only if you have a 32-bit operating system.
Otherwise, select x64 from the dropdown.
5. From the menu bar, select Build and then select Build
Solution from the submenu.
6. The program should compile without any errors.
7. To run the program, go to the directory where you stored
the code repository. You should see a folder named bin. Open
that folder. You will see a bunch of files and folders. Double-
click the .exe file to run the program. (The filename should
look something like x64_Release_TowerDefense.exe.)

Installing the Program (if applicable)
-----------------------------------------------------------------
Double-click on the installer and simply follow the directions
it gives you.

If you are NOT using a Windows machine and thus cannot use the installer,
use a program like 7zip to extract the files from the setup file.
The relevant files will be split into various folders (eg: $0, etc.), but you
should be able to figure out how to piece them together.

The overall structure should look something like this:

root
- config
- resources
 + graphs
 + levels
- userdata
- Playing Guide
- tower_defense.exe

You will also need to edit config/config.ini and change the following line to
false:
`use_appdata_folder = true`

Running the Program
-----------------------------------------------------------------
All you have to do is double-click on the program
(ending with an extension of .exe) and the program should load.

Updating the Program
-----------------------------------------------------------------
Please uninstall/delete the old version of the program and then
reinstall the program. If you are on Windows and have version
3.1.2 or later of this program, you do not have to do anything
to keep your old userdata. If not, you should look for your
save file in the userdata folder and copy that to another
location before uninstall/deleting the old version.

If you make changes to any of the resource files and would like
to keep those changes, you should change the following line
in config/config.ini (in the Program Files folder) to false:
`do_copy = true`

(Note that changing the above line means that any new levels
or balance changes will NOT be seen in your version of the
game.)

I want to give feedback on the program.
-----------------------------------------------------------------
Great! Send me an email at i y h o f f 1 3 @ g m a i l . c o m
(without the spaces) with your feedback. Please include an
appropriate subject line; I may ignore random emails that
do not follow this recommendation. All feedback, whether
it be positive, negative, or something in-between is welcome!

Can I distribute this to my friends?
-----------------------------------------------------------------
You may freely redistribute this program in its **original**,
unmodified form with all accompany resources/documentation to
anyone.

I want to make something like this!
-----------------------------------------------------------------
The source code is included, but you should double check the
license before copying my code.

Can I make modifications of this program?
-----------------------------------------------------------------
Please read the included license before you copy or modify my
code. If no license is included, you should assume any changes
or copying is prohibited.

The game is too hard!
-----------------------------------------------------------------
You have some options:

1. Keep playing. The more you play the game, the easier it
should become. If using strategy X is not working well, try
using strategy Y.
2. Choose an easier difficulty at the start of the game. It
is recommended only experienced players try the challenging
and expert levels of difficulty.
3. Try reading the playing guide is provided with the game.
Maybe you will learn something you did not know. A hints and
tips chapter if you are really having a hard time.
4. A lot of things like the statistics of enemies are stored
in configuration files. You can identify configuration files
by the extension .ini at the end of the file name. The files
named .ini.format are annotated files that serve as a guide
to the format of the associated configuration file. You can
modify these files to make the game easier.
5. Send me an email about the game's difficulty! (Bonus points
if you include screenshots of troublesome levels and/or some
of your savefiles!) While I am not guaranteed to respond,
I may use the feedback to rebalance a future version of the
game.
6. As of the time I last updated this file, there are **NO**
cheat codes. Do not waste your time trying to find some.

The game is slow!
-----------------------------------------------------------------
I can't help you much with that. My best advice is to get
a faster computer! (Don't bother buying more RAM though; it is
very unlikely that memory is the bottleneck.) The slowest parts
of the program are enemy path calculations and rendering the game,
so if you can find ways to reduce the number of times the game
performs those operations, the game should run faster.

Oh, and being better at the game also helps by reducing the
amount of work the computer has to do each frame! :)

Reporting Bugs
-----------------------------------------------------------------
If you encounter a random crash, please send me the following information:
1) Your save file.
2) Any information that will aid in reproducing the crash.

For any other bugs, please send me the following information:
1) A description of the bug. What did you expect? What actually happened?
2) Steps to reproduce the bug.
 2a) Be specific!
 2b) If you cannot reproduce the bug, let me know anyway and send me your
     save file.
