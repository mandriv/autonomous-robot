# Deployment instructions

## Method 1 (preferred - using PlatformIO)

### Prerequisites
* C++ compiler (e.g. `which g++` / `which gcc`)
* [Atom](https://atom.io/)
* [PlatformIO](https://platformio.org/install/ide?install=atom) plugin for Atom

### Instructions
1. Clone the repository or unzip the repo package `git clone git@github.com:mandriv/autonomous-robot.git` / `unzip <package-name.zip>`.2. Open project in Atom `atom autonomous-robot`.
3. Connect the robot.
4. Hit 'Upload' button.

## Method 2 (using Arduino IDE)

### Prerequisites
* [Arduino IDE](https://www.arduino.cc/en/Main/Software)

### Instructions
1. Clone the repository or unzip the repo package `git clone git@github.com:mandriv/autonomous-robot.git` / `unzip <package-name.zip>`.
2. Rename the `src` directory to `main` - `mv src main`
3. Remove first line that links Arduino.h
4. Pick the port and build the project.
