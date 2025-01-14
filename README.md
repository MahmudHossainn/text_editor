# This is a  simple terminal-based text editor written in C. This editor allows you to create, open, edit, and save text files directly from the command line.
It supports basic text editing features such as inserting, deleting, and navigating through text using arrow keys. 
The editor is designed to work on both Windows and Unix-like systems (Linux/macOS).

## Features:

  **Open Files:** Open and view existing text files.
  
  **Create Files:** Create new text files.
  
  **Edit Text:** Navigate through text using arrow keys, insert/delete characters, and split lines. 
  
  **Save Files:** Save changes to the current file.
  
  **Cross-Platform:** Works on Windows, Linux, and macOS.

## Prerequisites:
  
  **Windows:** Ensure you have a C compiler like Visual Studio.
    
  **Linux/macOS:** Ensure you have gcc and ncurses installed. You can install ncurses using your package manager:
    
  **Ubuntu/Debian:** 
  
    sudo apt-get install libncurses5-dev libncursesw5-dev
    
  **macOS:** 
  
    brew install ncurses

## Compilation:

  ### 1. Clone the repository.

  ### 2. Compile the program:

   **Windows:**

    gcc text_editor.c -o text_editor.exe

   **Linux/macOS:**

	gcc text_editor.c -o text_editor -lncurses

## Running the Editor:

  **Windows:**

	text_editor.exe

 **Linux/macOS:**

	./text_editor

## Usage:

  ### Open a File:

  Select option 1 and enter the filename to open an existing file.

  ### Create a New File:
    
  Select option 2 and enter the name of the new file.

  ### Edit Text: 

  Select option 3 to enter edit mode.
  
  Use arrow keys to navigate.
  
  Press Enter to split a line.
  
  Press Backspace to delete characters.
  
  Press Ctrl+S to save the file.
  
  Press Esc to exit edit mode.

  ### Quit:

  Select option 4 to exit the editor.

## Supported Operating Systems:

  **Windows:** Uses conio.h for input handling.

  **Linux/macOS:** Uses ncurses for terminal handling.
