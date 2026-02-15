# Assignment 4 information (Raylib used)

Build has been provided in the .zip foldier via canvas as per usual in: bin/release/Assignment_4.exe

Alternatively to run code: Double click build-VisualStudio2022.bat to generate .sln and double click that with visual studio 2022 installed, and set up start up project within the solution.

# Other info

mapData.txt includes various cold data related to the values used such as time control etc.

# Visuals and time control
- On release mode via bin the program intentionally stops the program when 20 soldiers has been created. This can be disabled in main.cpp via the while loop.
- To speed up: Press arrow up
- To speed down: Press arrow down
Note that if you speed up or down then the time to make 20 soldiers in main.cpp (Accessed via debug build via visual studio won't be totally accurate, so it's ideal to change time control in mapData.txt).

- In the simulation it's strucuted as following:
- Pink text are categorized in 4 groups, one for each building; displaying each relevant resource local to that building.
- Additionally, there are some red global values that describe total trees etc to get a better grasp (Since distributing resources happen).
- At the bottom we display global values representing the quantity of each unit in their relevant colors
- In the simulation we ascribe buildings in the following states:
  - Black lines box means it hasn't been built nor is active (Just that we plan to build there).
  - Filled box with colors means that it has been built but isn't active.
  - Filled box and box within a box means that the building is active and performing its related task (Such as smelting)

- Map description:
  - Light green tile means it's grass
  - Dark green tile means it's a tree tile
  - Light green dots on dark green tile means that it's a tree
  - Blue tile is water tile
  - Dark gray/brown is a boulder tile
  - Transparent yellow-ish is a swamp tile
  - Black dots accross the map are iron ore

